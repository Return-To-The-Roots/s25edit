// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CGame.h"
#include "CIO/CFile.h"
#include "CIO/CMenu.h"
#include "CIO/CWindow.h"
#include "CMap.h"
#include "callbacks.h"
#include "globals.h"
#include "lua/GameDataLoader.h"
#include <glad/glad.h>
#include <iostream>

bool CGame::CreateWindow()
{
    if(window_)
        return false;

    window_.reset(SDL_CreateWindow("Return to the Roots Map editor [BETA]", SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED, GameResolution.x, GameResolution.y,
                                   SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE));
    if(!window_)
        return false;

    glContext_ = SDL_GL_CreateContext(window_.get());
    if(!glContext_ || !gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        return false;

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 1);

    SDL_ShowWindow(window_.get());

    ApplyWindowChanges();
    if(!displayTexture_.isValid() || !Surf_Display)
        return false;

    SetAppIcon();

    return true;
}

void CGame::ApplyWindowChanges()
{
    if(!window_)
        return;
    if(GameResolution == appliedResolution_ && fullscreen == appliedFullscreen_)
        return;

    if(fullscreen)
    {
        SDL_DisplayMode dm;
        SDL_zero(dm);
        dm.w = static_cast<int>(GameResolution.x);
        dm.h = static_cast<int>(GameResolution.y);
        dm.format = 0; // let SDL pick a supported format
        dm.refresh_rate = 0;
        if(SDL_SetWindowDisplayMode(window_.get(), &dm) != 0)
            std::cerr << "SDL_SetWindowDisplayMode failed: " << SDL_GetError() << std::endl;

        const Uint32 flags = SDL_GetWindowFlags(window_.get());
        if(!(flags & SDL_WINDOW_FULLSCREEN))
        {
            if(SDL_SetWindowFullscreen(window_.get(), SDL_WINDOW_FULLSCREEN) != 0)
                std::cerr << "SDL_SetWindowFullscreen failed: " << SDL_GetError() << std::endl;
        } else if(GameResolution != appliedResolution_)
        {
            // Already fullscreen and the resolution changed. Toggle fullscreen off and
            // back on so SDL/Wayland actually applies the new display mode.
            if(SDL_SetWindowFullscreen(window_.get(), 0) != 0)
                std::cerr << "SDL_SetWindowFullscreen(0) failed: " << SDL_GetError() << std::endl;
            SDL_SetWindowSize(window_.get(), GameResolution.x, GameResolution.y);
            if(SDL_SetWindowFullscreen(window_.get(), SDL_WINDOW_FULLSCREEN) != 0)
                std::cerr << "SDL_SetWindowFullscreen failed: " << SDL_GetError() << std::endl;
        }
    } else
    {
        if(SDL_SetWindowFullscreen(window_.get(), 0) != 0)
            std::cerr << "SDL_SetWindowFullscreen failed: " << SDL_GetError() << std::endl;
        SDL_SetWindowSize(window_.get(), GameResolution.x, GameResolution.y);
        SDL_SetWindowPosition(window_.get(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }

    UpdateDisplaySize(GameResolution);
}

void CGame::setGLViewport()
{
    if(!window_)
        return;
    int w = 0, h = 0;
    SDL_GL_GetDrawableSize(window_.get(), &w, &h);
    if(w == 0 || h == 0)
        return;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, GameResolution.x, GameResolution.y, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void CGame::UpdateDisplaySize(const Extent& newSize)
{
    GameResolution = newSize;
    appliedResolution_ = GameResolution;
    appliedFullscreen_ = fullscreen;

    Surf_Display = makeRGBSurface(GameResolution.x, GameResolution.y, true);
    displayTexture_.createEmpty(GameResolution);

    setGLViewport();
    for(auto& menu : Menus)
    {
        menu->resetBgTexture();
        menu->resetSurface();
    }
    for(auto& wnd : Windows)
        wnd->resetSurface();
}

bool CGame::Init()
{
    std::cout << "Return to the Roots Map editor\n";

    SDL_ShowCursor(SDL_DISABLE);

    std::cout << "Create Window...";
    if(!CreateWindow())
    {
        std::cout << "failure";
        return false;
    }
    CFile::init();

    /*NOTE: its important to load a palette at first,
     *      otherwise all images will be black (in exception of the LBM-Files, they have their own palette).
     *      if its necessary to load pictures from a
     *      specified file with a special palette, so
     *      load this palette from a file and set
     *      CFile::palActual = CFile::palArray - 1 (--> last loaden palette)
     *      and after loading the images set
     *      CFile::palActual = CFile::palArray (--> first palette)
     */

    // load some pictures (after all the splash-screens)
    // at first GFX/PICS/SETUP997.LBM, cause this is the S2-loading picture
    std::cout << "\nLoading file: GFX/PICS/SETUP997.LBM...";
    if(!CFile::open_file(global::gameDataFilePath / "GFX/PICS/SETUP997.LBM", LBM))
    {
        std::cout << "failure";
        // if SETUP997.LBM doesn't exist, it's probably settlers2+mission cd and there we have SETUP998.LBM instead
        std::cout << "\nTry to load file: GFX/PICS/SETUP998.LBM instead...";
        if(!CFile::open_file(global::gameDataFilePath / "GFX/PICS/SETUP998.LBM", LBM))
        {
            std::cout << "failure";
            return false;
        }
    }

    // Create texture for splash background
    splashBg_.load(global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface.get(), true);

    // std::cout << "\nShow loading screen...";
    showLoadScreen = true;
    glClear(GL_COLOR_BUFFER_BIT);
    splashBg_.Draw(Rect(0, 0, GameResolution.x, GameResolution.y));
    SDL_GL_SwapWindow(window_.get());

    GameDataLoader gdLoader(global::worldDesc);
    if(!gdLoader.Load())
    {
        std::cerr << "Failed to load game data!" << std::endl;
        return false;
    }

    // continue loading pictures
    for(const std::string file :
        {"GFX/PICS/SETUP000.LBM", "GFX/PICS/SETUP010.LBM", "GFX/PICS/SETUP011.LBM", "GFX/PICS/SETUP012.LBM",
         "GFX/PICS/SETUP013.LBM", "GFX/PICS/SETUP014.LBM", "GFX/PICS/SETUP015.LBM"})
    {
        std::cout << "\nLoading file: " << file << "...";
        if(!CFile::open_file(global::gameDataFilePath / file, LBM))
        {
            std::cout << "failure";
            // if it doesn't exist, it's probably settlers2+missioncd and we simply load SETUP010.LBM instead
            std::cout << "\nLoading file: GFX/PICS/SETUP010.LBM instead...";
            if(!CFile::open_file(global::gameDataFilePath / "GFX/PICS/SETUP010.LBM", LBM))
            {
                std::cout << "failure";
                return false;
            }
        }
    }

    for(const std::string file :
        {"GFX/PICS/SETUP666.LBM", "GFX/PICS/SETUP667.LBM", "GFX/PICS/SETUP801.LBM", "GFX/PICS/SETUP802.LBM",
         "GFX/PICS/SETUP803.LBM", "GFX/PICS/SETUP804.LBM", "GFX/PICS/SETUP805.LBM", "GFX/PICS/SETUP806.LBM",
         "GFX/PICS/SETUP810.LBM", "GFX/PICS/SETUP811.LBM", "GFX/PICS/SETUP895.LBM", "GFX/PICS/SETUP896.LBM"})
    {
        std::cout << "\nLoading file: " << file << "...";
        if(!CFile::open_file(global::gameDataFilePath / file, LBM))
        {
            std::cout << "failure";
            return false;
        }
    }

    for(const std::string file : {"GFX/PICS/SETUP897.LBM", "GFX/PICS/SETUP898.LBM"})
    {
        std::cout << "\nLoading file: " << file << "...";
        if(!CFile::open_file(global::gameDataFilePath / file, LBM))
        {
            std::cout << "failure";
            // if it doesn't exist, it's probably settlers2+missioncd and we simply load SETUP896.LBM instead
            std::cout << "\nLoading file: GFX/PICS/SETUP896.LBM instead...";
            if(!CFile::open_file(global::gameDataFilePath / "GFX/PICS/SETUP896.LBM", LBM))
            {
                std::cout << "failure";
                return false;
            }
        }
    }

    for(const std::string file :
        {"GFX/PICS/SETUP899.LBM", "GFX/PICS/SETUP990.LBM", "GFX/PICS/WORLD.LBM", "GFX/PICS/WORLDMSK.LBM"})
    {
        std::cout << "\nLoading file: " << file << "...";
        if(!CFile::open_file(global::gameDataFilePath / file, LBM))
        {
            std::cout << "failure";
            return false;
        }
    }

    // load gouraud data
    for(const std::string file : {"DATA/TEXTURES/GOU5.DAT", "DATA/TEXTURES/GOU6.DAT", "DATA/TEXTURES/GOU7.DAT"})
    {
        std::cout << "\nLoading file: " << file << "...";
        if(!CFile::open_file(global::gameDataFilePath / file, GOU))
        {
            std::cout << "failure";
            return false;
        }
    }

    // load only the palette at this time from editres.idx
    std::cout << "\nLoading palette from file: DATA/EDITRES.IDX...";
    if(!CFile::open_file(global::gameDataFilePath / "DATA/EDITRES.IDX", IDX, true))
    {
        std::cout << "failure";
        return false;
    }
    // set the right palette
    CFile::set_palActual(CFile::get_palArray() - 1);
    std::cout << "\nLoading file: DATA/EDITRES.IDX...";
    if(!CFile::open_file(global::gameDataFilePath / "DATA/EDITRES.IDX", IDX))
    {
        std::cout << "failure";
        return false;
    }
    // set back palette
    CFile::set_palActual(CFile::get_palArray());
    // load only the palette at this time from editio.idx
    std::cout << "\nLoading palette from file: DATA/IO/EDITIO.IDX...";
    if(!CFile::open_file(global::gameDataFilePath / "DATA/IO/EDITIO.IDX", IDX, true))
    {
        std::cout << "failure";
        return false;
    }
    // set the right palette
    CFile::set_palActual(CFile::get_palArray() - 1);
    std::cout << "\nLoading file: DATA/IO/EDITIO.IDX...";
    if(!CFile::open_file(global::gameDataFilePath / "DATA/IO/EDITIO.IDX", IDX))
    {
        std::cout << "failure";
        return false;
    }
    // set back palette
    CFile::set_palActual(CFile::get_palArray());
    std::cout << "\nLoading file: DATA/EDITBOB.LST...";
    if(!CFile::open_file(global::gameDataFilePath / "DATA/EDITBOB.LST", LST))
    {
        std::cout << "failure";
        return false;
    }

    // texture tilesets
    for(const std::string file : {"GFX/TEXTURES/TEX5.LBM", "GFX/TEXTURES/TEX6.LBM", "GFX/TEXTURES/TEX7.LBM"})
    {
        std::cout << "\nLoading file: " << file << "...";
        if(!CFile::open_file(global::gameDataFilePath / file, LBM))
        {
            std::cout << "failure";
            return false;
        }
    }

    /*
    std::cout << "\nLoading palette from file: GFX/PALETTE/PAL5.BBM...";
    if ( !CFile::open_file(global::gameDataFilePath / "GFX/PALETTE/PAL5.BBM", BBM, true) )
    {
        std::cout << "failure";
        return false;
    }
    */

    // EVERY MISSION-FILE SHOULD BE LOADED SEPARATLY IF THE SPECIFIED MISSION GOES ON -- SO THIS IS TEMPORARY
    for(const std::string file : {"DATA/MIS0BOBS.LST", "DATA/MIS1BOBS.LST", "DATA/MIS2BOBS.LST", "DATA/MIS3BOBS.LST",
                                  "DATA/MIS4BOBS.LST", "DATA/MIS5BOBS.LST"})
    {
        std::cout << "\nLoading file: " << file << "...";
        if(!CFile::open_file(global::gameDataFilePath / file, LST))
        {
            std::cout << "failure";
            return false;
        }
    }

    // create the mainmenu
    callback::mainmenu(INITIALIZING_CALL);

    // Create textures for cursor
    cursor_.load(global::bmpArray[CURSOR].surface.get());
    cursorClicked_.load(global::bmpArray[CURSOR_CLICKED].surface.get());
    cross_.load(global::bmpArray[CROSS].surface.get());

    return true;
}

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
#include <libsiedler2/Archiv.h>
#include <libsiedler2/ArchivItem_Bitmap.h>
#include <libsiedler2/ArchivItem_Palette.h>
#include <libsiedler2/ErrorCodes.h>
#include <libsiedler2/libsiedler2.h>
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

    SetAppIcon();

    return true;
}

void CGame::ApplyWindowChanges()
{
    if(!window_)
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
        {
            std::cerr << "SDL_SetWindowDisplayMode failed: " << SDL_GetError() << std::endl;
            return;
        }

        const Uint32 flags = SDL_GetWindowFlags(window_.get());
        if(!(flags & SDL_WINDOW_FULLSCREEN))
        {
            if(SDL_SetWindowFullscreen(window_.get(), SDL_WINDOW_FULLSCREEN) != 0)
            {
                std::cerr << "SDL_SetWindowFullscreen failed: " << SDL_GetError() << std::endl;
                return;
            }
        } else if(GameResolution != appliedResolution_)
        {
            // Already fullscreen and the resolution changed. Toggle fullscreen off and
            // back on so SDL/Wayland actually applies the new display mode.
            if(SDL_SetWindowFullscreen(window_.get(), 0) != 0)
            {
                std::cerr << "SDL_SetWindowFullscreen(0) failed: " << SDL_GetError() << std::endl;
                return;
            }
            SDL_SetWindowSize(window_.get(), GameResolution.x, GameResolution.y);
            if(SDL_SetWindowFullscreen(window_.get(), SDL_WINDOW_FULLSCREEN) != 0)
            {
                std::cerr << "SDL_SetWindowFullscreen failed: " << SDL_GetError() << std::endl;
                return;
            }
        }
    } else
    {
        if(SDL_SetWindowFullscreen(window_.get(), 0) != 0)
        {
            std::cerr << "SDL_SetWindowFullscreen failed: " << SDL_GetError() << std::endl;
            return;
        }
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

    setGLViewport();
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
    if(!global::loadArchive(ArchiveID::SETUP997, global::gameDataFilePath / "GFX/PICS/SETUP997.LBM", nullptr))
    {
        std::cout << "failure";
        // if SETUP997.LBM doesn't exist, it's probably settlers2+mission cd and there we have SETUP998.LBM instead
        std::cout << "\nTry to load file: GFX/PICS/SETUP998.LBM instead...";
        if(!global::loadArchive(ArchiveID::SETUP997, global::gameDataFilePath / "GFX/PICS/SETUP998.LBM", nullptr))
        {
            std::cout << "failure";
            return false;
        }
    }

    // Create texture for splash background
    {
        auto& archiv = global::typedArchives[ArchiveID::SETUP997];
        auto* bmp = dynamic_cast<const libsiedler2::baseArchivItem_Bitmap*>(archiv.get(0));
        if(bmp)
            splashBg_.load(*bmp);
    }

    // std::cout << "\nShow loading screen...";
    showLoadScreen = true;
    glClear(GL_COLOR_BUFFER_BIT);
    splashBg_.draw(Rect(0, 0, GameResolution.x, GameResolution.y));
    SDL_GL_SwapWindow(window_.get());

    GameDataLoader gdLoader(global::worldDesc);
    if(!gdLoader.Load())
    {
        std::cerr << "Failed to load game data!" << std::endl;
        return false;
    }

    // continue loading pictures
    const struct
    {
        ArchiveID id;
        const char* path;
    } setupFiles[] = {
      {ArchiveID::SETUP000, "GFX/PICS/SETUP000.LBM"}, {ArchiveID::SETUP010, "GFX/PICS/SETUP010.LBM"},
      {ArchiveID::SETUP011, "GFX/PICS/SETUP011.LBM"}, {ArchiveID::SETUP012, "GFX/PICS/SETUP012.LBM"},
      {ArchiveID::SETUP013, "GFX/PICS/SETUP013.LBM"}, {ArchiveID::SETUP014, "GFX/PICS/SETUP014.LBM"},
      {ArchiveID::SETUP015, "GFX/PICS/SETUP015.LBM"},
    };
    for(const auto& sf : setupFiles)
    {
        std::cout << "\nLoading file: " << sf.path << "...";
        if(!global::loadArchive(sf.id, global::gameDataFilePath / sf.path, nullptr))
        {
            std::cout << "failure";
            // if it doesn't exist, it's probably settlers2+missioncd and we simply load SETUP010.LBM instead
            std::cout << "\nLoading file: GFX/PICS/SETUP010.LBM instead...";
            if(!global::loadArchive(ArchiveID::SETUP010, global::gameDataFilePath / "GFX/PICS/SETUP010.LBM", nullptr))
            {
                std::cout << "failure";
                return false;
            }
        }
    }

    { // batch 2: SETUP666-896
        const struct
        {
            ArchiveID id;
            const char* path;
        } files[] = {
          {ArchiveID::SETUP666, "GFX/PICS/SETUP666.LBM"}, {ArchiveID::SETUP667, "GFX/PICS/SETUP667.LBM"},
          {ArchiveID::SETUP801, "GFX/PICS/SETUP801.LBM"}, {ArchiveID::SETUP802, "GFX/PICS/SETUP802.LBM"},
          {ArchiveID::SETUP803, "GFX/PICS/SETUP803.LBM"}, {ArchiveID::SETUP804, "GFX/PICS/SETUP804.LBM"},
          {ArchiveID::SETUP805, "GFX/PICS/SETUP805.LBM"}, {ArchiveID::SETUP806, "GFX/PICS/SETUP806.LBM"},
          {ArchiveID::SETUP810, "GFX/PICS/SETUP810.LBM"}, {ArchiveID::SETUP811, "GFX/PICS/SETUP811.LBM"},
          {ArchiveID::SETUP895, "GFX/PICS/SETUP895.LBM"}, {ArchiveID::SETUP896, "GFX/PICS/SETUP896.LBM"},
        };
        for(const auto& f : files)
        {
            std::cout << "\nLoading file: " << f.path << "...";
            if(!global::loadArchive(f.id, global::gameDataFilePath / f.path, nullptr))
            {
                std::cout << "failure";
                return false;
            }
        }
    }

    { // batch 3: SETUP897-898 with fallback to SETUP896
        const struct
        {
            ArchiveID id;
            const char* path;
        } files[] = {
          {ArchiveID::SETUP897, "GFX/PICS/SETUP897.LBM"},
          {ArchiveID::SETUP898, "GFX/PICS/SETUP898.LBM"},
        };
        for(const auto& f : files)
        {
            std::cout << "\nLoading file: " << f.path << "...";
            if(!global::loadArchive(f.id, global::gameDataFilePath / f.path, nullptr))
            {
                std::cout << "failure";
                std::cout << "\nLoading file: GFX/PICS/SETUP896.LBM instead...";
                if(!global::loadArchive(f.id, global::gameDataFilePath / "GFX/PICS/SETUP896.LBM", nullptr))
                {
                    std::cout << "failure";
                    return false;
                }
            }
        }
    }

    { // batch 4: remaining loading screens
        const struct
        {
            ArchiveID id;
            const char* path;
        } files[] = {
          {ArchiveID::SETUP899, "GFX/PICS/SETUP899.LBM"},
          {ArchiveID::SETUP990, "GFX/PICS/SETUP990.LBM"},
          {ArchiveID::WORLD_LBM, "GFX/PICS/WORLD.LBM"},
          {ArchiveID::WORLDMSK_LBM, "GFX/PICS/WORLDMSK.LBM"},
        };
        for(const auto& f : files)
        {
            std::cout << "\nLoading file: " << f.path << "...";
            if(!global::loadArchive(f.id, global::gameDataFilePath / f.path, nullptr))
            {
                std::cout << "failure";
                return false;
            }
        }
    }

    // Load the default palette first so all subsequent loads have it available
    std::cout << "\nLoading default palette from file: GFX/PALETTE/PAL5.BBM...";
    if(!global::loadPalette(global::gameDataFilePath / "GFX/PALETTE/PAL5.BBM"))
    {
        std::cout << "failure";
        return false;
    }

    // Load EDITRES.IDX as a complete bundle in typedArchives
    {
        libsiedler2::Archiv editres;
        int ec = libsiedler2::Load(global::gameDataFilePath / "DATA/EDITRES.IDX", editres, global::currentPalette);
        if(ec)
        {
            std::cout << "\nError loading EDITRES.IDX: " << libsiedler2::getErrorString(ec) << std::endl;
            return false;
        }
        // The palette in EDITRES (item 1) may override the default
        auto* pal = dynamic_cast<libsiedler2::ArchivItem_Palette*>(editres.get(1));
        if(pal)
            global::currentPalette = pal;
        // Store the complete Archiv in typedArchives — fonts stay inside for CFont to read directly
        // Indices match file positions: 0=Font, 1=Palette, 2=Font, 3=Font, 4-56=Bitmaps
        global::typedArchives[ArchiveID::EDITRES] = std::move(editres);
    }

    std::cout << "\nLoading file: DATA/IO/EDITIO.IDX...";
    if(!global::loadArchive(ArchiveID::EDITIO, global::gameDataFilePath / "DATA/IO/EDITIO.IDX", global::currentPalette))
    {
        std::cout << "failure";
        return false;
    }
    std::cout << "done";

    std::cout << "\nLoading file: DATA/EDITBOB.LST...";
    if(!global::loadArchive(ArchiveID::EDITBOB, global::gameDataFilePath / "DATA/EDITBOB.LST", global::currentPalette))
    {
        std::cout << "failure";
        return false;
    }
    std::cout << "done";

    // texture tilesets
    const ArchiveID texArchives[3] = {ArchiveID::TEX5, ArchiveID::TEX6, ArchiveID::TEX7};
    const std::string texFiles[3] = {"GFX/TEXTURES/TEX5.LBM", "GFX/TEXTURES/TEX6.LBM", "GFX/TEXTURES/TEX7.LBM"};
    for(unsigned ti = 0; ti < 3; ti++)
    {
        std::cout << "\nLoading file: " << texFiles[ti] << "...";
        if(!global::loadTileset(texArchives[ti], ti, global::gameDataFilePath / texFiles[ti]))
        {
            std::cout << "failure";
            return false;
        }
        std::cout << "done";
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
    const ArchiveID misArchives[] = {ArchiveID::MIS0BOBS, ArchiveID::MIS1BOBS, ArchiveID::MIS2BOBS,
                                     ArchiveID::MIS3BOBS, ArchiveID::MIS4BOBS, ArchiveID::MIS5BOBS};
    const std::string misFiles[] = {"DATA/MIS0BOBS.LST", "DATA/MIS1BOBS.LST", "DATA/MIS2BOBS.LST",
                                    "DATA/MIS3BOBS.LST", "DATA/MIS4BOBS.LST", "DATA/MIS5BOBS.LST"};
    for(unsigned mi = 0; mi < 6; mi++)
    {
        std::cout << "\nLoading file: " << misFiles[mi] << "...";
        if(!global::loadArchive(misArchives[mi], global::gameDataFilePath / misFiles[mi], global::currentPalette))
        {
            std::cout << "failure";
            return false;
        }
    }

    // create the mainmenu
    callback::mainmenu(INITIALIZING_CALL);

    // Create textures for cursor (from typed archive)
    auto& resArchiv = global::typedArchives[ArchiveID::EDITRES];
    if(auto* bmp = dynamic_cast<const libsiedler2::baseArchivItem_Bitmap*>(resArchiv.get(CURSOR)))
        cursor_.load(*bmp);
    if(auto* bmp = dynamic_cast<const libsiedler2::baseArchivItem_Bitmap*>(resArchiv.get(CURSOR_CLICKED)))
        cursorClicked_.load(*bmp);
    if(auto* bmp = dynamic_cast<const libsiedler2::baseArchivItem_Bitmap*>(resArchiv.get(CROSS)))
        cross_.load(*bmp);

    return true;
}

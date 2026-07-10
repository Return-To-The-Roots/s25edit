// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CGame.h"
#include "WindowManager.h"
#include "dskMainMenu.h"
#include "defines.h"
#include "globals.h"
#include "Loader.h"
#include "lua/GameDataLoader.h"
#include "ogl/glAllocator.h"
#include "ogl/glArchivItem_Bitmap.h"
#include <libsiedler2/Archiv.h>
#include <libsiedler2/ArchivItem_Bitmap.h>
#include <libsiedler2/ArchivItem_Palette.h>
#include <libsiedler2/ErrorCodes.h>
#include <libsiedler2/libsiedler2.h>
#include "drivers/VideoDriverWrapper.h"
#include <glad/glad.h>
#include <boost/filesystem.hpp>
#include <iostream>
#include <exception>

bool CGame::CreateWindow()
{
    if(window_)
        return false;

    // Load the SDL2 video driver plugin
    std::string driverName = "SDL2";
    try
    {
        if(!VIDEODRIVER.LoadDriver(driverName))
        {
            std::cerr << "LoadDriver failed" << std::endl;
            return false;
        }
    } catch(std::exception& e)
    {
        std::cerr << "LoadDriver exception: " << e.what() << std::endl;
        return false;
    } catch(...)
    {
        std::cerr << "LoadDriver unknown exception" << std::endl;
        return false;
    }

    if(!VIDEODRIVER.CreateScreen({static_cast<unsigned short>(GameResolution.x),
                                  static_cast<unsigned short>(GameResolution.y)},
                                 DisplayMode::Windowed))
        return false;

    // Keep our own SDL window handle for event polling and resize handling
    window_.reset(SDL_GL_GetCurrentWindow());

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0, 0, 0, 1);

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
     */

    // Initialize basic LOADER infrastructure early (before LoadFilesAtStart)
    libsiedler2::setAllocator(new GlAllocator());
    LOADER.initResourceFolders();

    // Load the loading screen splash before the heavier LoadFilesAtStart
    {
        auto setup997Path = global::gameDataFilePath / "GFX/PICS/SETUP997.LBM";
        if(!boost::filesystem::exists(setup997Path))
            setup997Path = global::gameDataFilePath / "GFX/PICS/SETUP998.LBM";
        if(boost::filesystem::exists(setup997Path))
        {
            LOADER.Load(setup997Path, nullptr);
            splashBg_ = LOADER.GetImageN(ResourceId::make(setup997Path), 0);
        }
    }

    showLoadScreen = true;
    glClear(GL_COLOR_BUFFER_BIT);
    if(splashBg_)
        splashBg_->DrawFull(Rect(0, 0, GameResolution.x, GameResolution.y));
    SDL_GL_SwapWindow(window_.get());

    // Now load the full set of LOADER files
    LOADER.LoadFilesAtStart();

    GameDataLoader gdLoader(global::worldDesc);
    if(!gdLoader.Load())
    {
        std::cerr << "Failed to load game data!" << std::endl;
        return false;
    }

    // Load EDITRES for fonts, and palette
    std::cout << "\nLoading default palette from file: GFX/PALETTE/PAL5.BBM...";
    // Load editor resources into the LOADER (already initialized at top)
    {
        const auto* pal5 = LOADER.GetPaletteN("pal5", 0);

        // Main menu background (not in LoadFilesAtStart)
        const auto setup010Path = global::gameDataFilePath / "GFX/PICS/SETUP010.LBM";
        if(boost::filesystem::exists(setup010Path))
            LOADER.Load(setup010Path, nullptr);

        // Editor archives keyed separately from game's IO.IDX etc.
        const auto editioPath = global::gameDataFilePath / "DATA/IO/EDITIO.IDX";
        LOADER.Load(editioPath, pal5);
        const auto editresPath = global::gameDataFilePath / "DATA/EDITRES.IDX";
        LOADER.Load(editresPath, pal5);
        const auto editbobPath = global::gameDataFilePath / "DATA/EDITBOB.LST";
        LOADER.Load(editbobPath, pal5);

        auto* cursorNorm = LOADER.GetImageN("editres", CURSOR);
        auto* cursorPressed = LOADER.GetImageN("editres", CURSOR_CLICKED);
        if(cursorNorm)
            WINDOWMANAGER.SetCursorImage(Cursor::Hand, cursorNorm, cursorPressed);
    }

    // Show the new Desktop-based main menu via WindowManager
    WINDOWMANAGER.Switch(std::make_unique<dskMainMenu>());

    // All resources loaded, dismiss the loading splash so the Desktop renders
    showLoadScreen = false;

    return true;
}

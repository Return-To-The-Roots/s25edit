// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2024 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CGame.h"
#include "CIO/CFont.h"
#include "CIO/CMenu.h"
#include "CIO/CWindow.h"
#include "CMap.h"
#include "CSurface.h"
#include "globals.h"
#include <glad/glad.h>
#ifdef _WIN32
#    include "s25editResource.h"
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <windows.h>
#    include <SDL_syswm.h>
#endif

void CGame::SetAppIcon()
{
#ifdef _WIN32
    LPARAM icon = (LPARAM)LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_SYMBOL));
    SendMessage(GetConsoleWindow(), WM_SETICON, ICON_BIG, icon);
    SendMessage(GetConsoleWindow(), WM_SETICON, ICON_SMALL, icon);

    SDL_SysWMinfo info;
    // get window handle from SDL
    SDL_VERSION(&info.version);
    if(SDL_GetWindowWMInfo(window_.get(), &info) != 1)
        return;
    SendMessage(info.info.win.window, WM_SETICON, ICON_BIG, icon);
    SendMessage(info.info.win.window, WM_SETICON, ICON_SMALL, icon);
#endif // _WIN32
}

void CGame::Render()
{
    suppressResizeEvents_ = 0;

    // Ensure viewport and ortho projection match current window size (handles resize)
    glViewport(0, 0, GameResolution.x, GameResolution.y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, GameResolution.x, GameResolution.y, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Clear the framebuffer and the software overlay surface.
    // GL draws (backgrounds) and the final Surf_Display overlay are
    // composited via blending.
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_FillRect(Surf_Display.get(), nullptr, SDL_MapRGBA(Surf_Display->format, 0, 0, 0, 0));

    if(Extent(Surf_Display->w, Surf_Display->h) != GameResolution
       || fullscreen != ((SDL_GetWindowFlags(window_.get()) & SDL_WINDOW_FULLSCREEN) != 0))
    {
        ReCreateWindow();
    }

    // if the S2 loading screen is shown, render only this until user clicks a mouse button
    if(showLoadScreen)
    {
        CSurface::DrawStretched(Surf_Display, global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface);
        SDL_GL_SwapWindow(window_.get());
        return;
    }

    // render the map if active
    if(MapObj && MapObj->isActive())
    {
        CSurface::Draw(Surf_Display, MapObj->getSurface(), 0, 0);
        std::array<char, 100> textBuffer;
        // text for x and y of vertex (shown in upper left corner)
        std::snprintf(textBuffer.data(), textBuffer.size(), "%d    %d", MapObj->getVertexX(), MapObj->getVertexY());
        CFont::writeText(Surf_Display, textBuffer.data(), Position(20, 20));
        // text for MinReduceHeight and MaxRaiseHeight
        std::snprintf(textBuffer.data(), textBuffer.size(),
                      "min. height: %#04x/0x3C  max. height: %#04x/0x3C  NormalNull: 0x0A",
                      MapObj->getMinReduceHeight(), MapObj->getMaxRaiseHeight());
        CFont::writeText(Surf_Display, textBuffer.data(), Position(100, 20));
        // text for MovementLocked
        if(MapObj->isHorizontalMovementLocked() && MapObj->isVerticalMovementLocked())
            CFont::writeText(Surf_Display, "Movement locked (F9 or F10 to unlock)", Position(20, 40), FontSize::Large,
                             FontColor::Orange);
        else if(MapObj->isHorizontalMovementLocked())
            CFont::writeText(Surf_Display, "Horizontal movement locked (F9 to unlock)", Position(20, 40),
                             FontSize::Large, FontColor::Orange);
        else if(MapObj->isVerticalMovementLocked())
            CFont::writeText(Surf_Display, "Vertical movement locked (F10 to unlock)", Position(20, 40),
                             FontSize::Large, FontColor::Orange);
    }

    // render active menus
    for(auto& Menu : Menus)
    {
        if(Menu->isActive())
        {
            // Draw menu background via OpenGL
            auto& bgBmp = global::bmpArray[Menu->getBackground()];
            CSurface::DrawStretched(Surf_Display, bgBmp.surface);
            // Draw UI overlay on top
            CSurface::Draw(Surf_Display, Menu->getSurface(), 0, 0);
        }
    }

    // render windows ordered by priority
    int highestPriority = 0;
    // first find the highest priority
    for(auto& Window : Windows)
    {
        if(Window->getPriority() > highestPriority)
            highestPriority = Window->getPriority();
    }
    // render from lowest priority to highest
    for(int actualPriority = 0; actualPriority <= highestPriority; actualPriority++)
    {
        for(auto& Window : Windows)
        {
            if(Window->getPriority() == actualPriority)
                CSurface::Draw(Surf_Display, Window->getSurface(), Window->getX(), Window->getY());
        }
    }

    // render mouse cursor (drawn in RenderPresent via GL to stay on top of all overlays)
    if(Cursor.clicked)
    {
        if(Cursor.button.right)
            CSurface::Draw(Surf_Display, global::bmpArray[CROSS].surface, Cursor.pos);
        else
            CSurface::Draw(Surf_Display, global::bmpArray[CURSOR_CLICKED].surface, Cursor.pos);
    } else
        CSurface::Draw(Surf_Display, global::bmpArray[CURSOR].surface, Cursor.pos);

#ifdef _ADMINMODE
    FrameCounter++;
#endif

    ++framesPassedSinceLastFps;

    const auto curTicks = SDL_GetTicks();
    const auto diffTicks = curTicks - lastFpsTick;
    if(diffTicks > 1000)
    {
        lastFps.setText(std::to_string((framesPassedSinceLastFps * 1000) / diffTicks) + " FPS");
        framesPassedSinceLastFps = 0;
        lastFpsTick = curTicks;
    }
    CSurface::Draw(Surf_Display, lastFps.getSurface(), 0, 0);

    RenderPresent();

    if(msWait)
        SDL_Delay(msWait);

    const auto timeSinceLastFrame = curTicks - lastFrameTime;
    const auto targetFPS = 60;
    const auto targetMsPerFrame = 1000 / targetFPS;
    if(timeSinceLastFrame < targetMsPerFrame)
    {
        const auto timeToSleep = targetMsPerFrame - timeSinceLastFrame;
        SDL_Delay(timeToSleep);
    }
    lastFrameTime = curTicks;
}

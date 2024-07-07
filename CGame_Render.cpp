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
#include "SGE/sge_blib.h"
#include "globals.h"
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
    if(Extent(Surf_Display->w, Surf_Display->h) != GameResolution
       || fullscreen != ((SDL_GetWindowFlags(window_.get()) & SDL_WINDOW_FULLSCREEN) != 0))
    {
        ReCreateWindow();
    }

    // if the S2 loading screen is shown, render only this until user clicks a mouse button
    if(showLoadScreen)
    {
        // CSurface::Draw(Surf_Display, global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface, 0, 0);
        auto& surfLoadScreen = global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface;
        sge_TexturedRect(Surf_Display.get(), 0, 0, Surf_Display->w - 1, 0, 0, Surf_Display->h - 1, Surf_Display->w - 1,
                         Surf_Display->h - 1, surfLoadScreen.get(), 0, 0, surfLoadScreen->w - 1, 0, 0,
                         surfLoadScreen->h - 1, surfLoadScreen->w - 1, surfLoadScreen->h - 1);
        RenderPresent();
        return;
    }

    // render the map if active
    if(MapObj && MapObj->isActive())
    {
        CSurface::Draw(Surf_Display, MapObj->getSurface(), 0, 0);
        std::array<char, 100> textBuffer;
        // text for x and y of vertex (shown in upper left corner)
        std::snprintf(textBuffer.data(), textBuffer.size(), "%d    %d", MapObj->getVertexX(), MapObj->getVertexY());
        CFont::writeText(Surf_Display, textBuffer.data(), 20, 20);
        // text for MinReduceHeight and MaxRaiseHeight
        std::snprintf(textBuffer.data(), textBuffer.size(),
                      "min. height: %#04x/0x3C  max. height: %#04x/0x3C  NormalNull: 0x0A",
                      MapObj->getMinReduceHeight(), MapObj->getMaxRaiseHeight());
        CFont::writeText(Surf_Display, textBuffer.data(), 100, 20);
        // text for MovementLocked
        if(MapObj->isHorizontalMovementLocked() && MapObj->isVerticalMovementLocked())
            CFont::writeText(Surf_Display, "Movement locked (F9 or F10 to unlock)", 20, 40, FontSize::Large,
                             FontColor::Orange);
        else if(MapObj->isHorizontalMovementLocked())
            CFont::writeText(Surf_Display, "Horizontal movement locked (F9 to unlock)", 20, 40, FontSize::Large,
                             FontColor::Orange);
        else if(MapObj->isVerticalMovementLocked())
            CFont::writeText(Surf_Display, "Vertical movement locked (F10 to unlock)", 20, 40, FontSize::Large,
                             FontColor::Orange);
    }

    // render active menus
    for(auto& Menu : Menus)
    {
        if(Menu->isActive())
            CSurface::Draw(Surf_Display, Menu->getSurface(), 0, 0);
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

    // render mouse cursor
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

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
#include "Texture.h"
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
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // if the S2 loading screen is shown, render only this until user clicks a mouse button
    if(showLoadScreen)
    {
        splashBg_.draw(Rect(0, 0, GameResolution.x, GameResolution.y));
        SDL_GL_SwapWindow(window_.get());
        return;
    }

    // render the map if active
    if(MapObj && MapObj->isActive())
    {
        if(MapObj->getMap())
            CSurface::UpdatePaletteAnimations(MapObj->getMap()->type);
        if(auto* mapSurf = MapObj->getSurface())
        {
            std::array<char, 100> textBuffer;
            std::snprintf(textBuffer.data(), textBuffer.size(), "%d    %d", MapObj->getVertexX(), MapObj->getVertexY());
            CFont::writeText(mapSurf, textBuffer.data(), 20, 20);
            std::snprintf(textBuffer.data(), textBuffer.size(),
                          "min. height: %#04x/0x3C  max. height: %#04x/0x3C  NormalNull: 0x0A",
                          MapObj->getMinReduceHeight(), MapObj->getMaxRaiseHeight());
            CFont::writeText(mapSurf, textBuffer.data(), 100, 20);
            if(MapObj->isHorizontalMovementLocked() && MapObj->isVerticalMovementLocked())
                CFont::writeText(mapSurf, "Movement locked (F9 or F10 to unlock)", 20, 40, FontSize::Large,
                                 FontColor::Orange);
            else if(MapObj->isHorizontalMovementLocked())
                CFont::writeText(mapSurf, "Horizontal movement locked (F9 to unlock)", 20, 40, FontSize::Large,
                                 FontColor::Orange);
            else if(MapObj->isVerticalMovementLocked())
                CFont::writeText(mapSurf, "Vertical movement locked (F10 to unlock)", 20, 40, FontSize::Large,
                                 FontColor::Orange);

            mapTex_.load(mapSurf);
            mapTex_.draw(Rect(0, 0, GameResolution.x, GameResolution.y));
        }
    }

    // render active menus
    for(auto& Menu : Menus)
    {
        if(Menu->isActive())
            Menu->draw(Position(0, 0));
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
                Window->draw(Position(0, 0));
        }
    }

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
    lastFps.draw(Position(0, 0));

    const auto& cursorImg = Cursor.clicked ? (Cursor.button.right ? cross_ : cursorClicked_) : cursor_;
    cursorImg.draw(Cursor.pos);

    SDL_GL_SwapWindow(window_.get());

#ifdef _ADMINMODE
    FrameCounter++;
#endif

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

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
#include "TerrainRenderer.h"
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
        auto& surfLoadScreen = global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface;
        // Convert 8-bit paletted splash to match Surf_Display format (32-bit no alpha)
        // Use same pixel format as Surf_Display so alpha from palette doesn't cause transparency
        SDL_Surface* splash32 = SDL_ConvertSurface(surfLoadScreen.get(), Surf_Display->format, 0);
        if(splash32)
        {
            // Remove any colorkey so full image renders
            Uint32 key;
            if(SDL_GetColorKey(splash32, &key) == 0)
                SDL_SetColorKey(splash32, SDL_FALSE, 0);
            SDL_BlitScaled(splash32, nullptr, Surf_Display.get(), nullptr);
            SDL_FreeSurface(splash32);
        }
        RenderPresent();
        return;
    }

    // Render terrain via OpenGL, then composite UI on top
    glClear(GL_COLOR_BUFFER_BIT);

    // Clear UI overlay surface so old pixels don't persist
    SDL_FillRect(Surf_Display.get(), nullptr, SDL_MapRGBA(Surf_Display->format, 0, 0, 0, 0));

    // render the map if active
    if(MapObj && MapObj->isActive())
    {
        RectBase<Sint32> viewRect = MapObj->getDisplayRect();

        // Set camera projection for map-space rendering
        TerrainRenderer::setCamera(viewRect.left, viewRect.top, GameResolution.x, GameResolution.y);

        // Only re-render and re-upload the map UI overlay when something changed.
        // When idle (no mouse/keyboard input), the overlay content is identical
        // to the previous frame and we can skip the expensive texture upload.
        if(MapObj->isOverlayDirty() || !displayGLTex_.valid())
        {
            if(displayGLTex_.valid())
                displayGLTex_.updateFromSurface(MapObj->getSurface());
            else
                displayGLTex_.createFromSurface(MapObj->getSurface());
            MapObj->markOverlayClean();
        }

        // Draw terrain (+ pre-computed borders) from VBOs
        TerrainRenderer::Draw(viewRect);

        // Switch to screen projection for all UI overlay drawing
        TerrainRenderer::setScreenProjection(GameResolution.x, GameResolution.y);

        // Draw map overlay (frame, menubar, objects) in screen-space.
        if(displayGLTex_.valid())
            displayGLTex_.draw(0, 0);

        // HUD text (coordinates, height info, lock status) — drawn to Surf_Display
        // in screen-space.
        std::array<char, 100> textBuffer;
        std::snprintf(textBuffer.data(), textBuffer.size(), "%d    %d", MapObj->getVertexX(), MapObj->getVertexY());
        CFont::writeText(Surf_Display, textBuffer.data(), 20, 20);
        std::snprintf(textBuffer.data(), textBuffer.size(),
                      "min. height: %#04x/0x3C  max. height: %#04x/0x3C  NormalNull: 0x0A",
                      MapObj->getMinReduceHeight(), MapObj->getMaxRaiseHeight());
        CFont::writeText(Surf_Display, textBuffer.data(), 100, 20);
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

#ifdef _ADMINMODE
    FrameCounter++;
#endif

    // FPS counter.
    ++framesPassedSinceLastFps;
    const auto curTicks = SDL_GetTicks();
    const auto diffTicks = curTicks - lastFpsTick;
    if(diffTicks > 1000)
    {
        lastFps.setText(std::to_string((framesPassedSinceLastFps * 1000) / diffTicks) + " FPS");
        framesPassedSinceLastFps = 0;
        lastFpsTick = curTicks;
        uiDirty_ = true;
    }
    CSurface::Draw(Surf_Display, lastFps.getSurface(), 0, 0);

    // Upload final Surf_Display content to GL and draw it.
    // Skip re-upload when nothing changed (idle state).
    if(uiDirty_ || !uiOverlayTex_.valid())
    {
        if(uiOverlayTex_.valid())
            uiOverlayTex_.updateFromSurface(Surf_Display.get());
        else
            uiOverlayTex_.createFromSurface(Surf_Display.get());
        uiDirty_ = false;
    }
    if(uiOverlayTex_.valid())
        uiOverlayTex_.draw(0, 0);

    // render mouse cursor
    EditorGLTexture cursorTex;
    SDL_Surface* cursorSurf = nullptr;
    if(Cursor.clicked)
    {
        if(Cursor.button.right)
            cursorSurf = global::bmpArray[CROSS].surface.get();
        else
            cursorSurf = global::bmpArray[CURSOR_CLICKED].surface.get();
    } else
        cursorSurf = global::bmpArray[CURSOR].surface.get();
    if(cursorSurf && cursorTex.createFromSurface(cursorSurf))
        cursorTex.draw(Cursor.pos.x, Cursor.pos.y);

    glFlush();
    SDL_GL_SwapWindow(window_.get());

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

#include "CGame.h"
#include "CIO/CFont.h"
#include "CIO/CMenu.h"
#include "CIO/CWindow.h"
#include "CMap.h"
#include "CSurface.h"
#include "SGE/sge_blib.h"
#include "globals.h"
#ifdef _WIN32
#include "s25editResource.h"
#undef WIN32_LEAN_AND_MEAN
#include <SDL_syswm.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
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
    if(SDL_GetWMInfo(&info) != 1)
        return;
    SendMessage(info.window, WM_SETICON, ICON_BIG, icon);
    SendMessage(info.window, WM_SETICON, ICON_SMALL, icon);
#endif // _WIN32
}

void CGame::Render()
{
    if(Extent(Surf_Display->w, Surf_Display->h) != GameResolution || fullscreen != ((Surf_Display->flags & SDL_FULLSCREEN) != 0)
       || useOpenGL != CSurface::useOpenGL)
    {
        ReCreateWindow();
    }

    // if the S2 loading screen is shown, render only this until user clicks a mouse button
    if(showLoadScreen)
    {
        // CSurface::Draw(Surf_Display, global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface, 0, 0);
        auto& surfLoadScreen = global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface;
        sge_TexturedRect(Surf_Display.get(), 0, 0, Surf_Display->w - 1, 0, 0, Surf_Display->h - 1, Surf_Display->w - 1, Surf_Display->h - 1,
                         surfLoadScreen.get(), 0, 0, surfLoadScreen->w - 1, 0, 0, surfLoadScreen->h - 1, surfLoadScreen->w - 1,
                         surfLoadScreen->h - 1);

        if(useOpenGL)
        {
            SDL_BlitSurface(Surf_Display.get(), nullptr, Surf_DisplayGL.get(), nullptr);
            SDL_GL_SwapBuffers();
        } else
            SDL_Flip(Surf_Display.get());
        return;
    }

    // render the map if active
    if(MapObj && MapObj->isActive())
    {
        CSurface::Draw(Surf_Display, MapObj->getSurface(), 0, 0);
        std::array<char, 100> textBuffer;
        // text for x and y of vertex (shown in upper left corner)
        sprintf(textBuffer.data(), "%d    %d", MapObj->getVertexX(), MapObj->getVertexY());
        CFont::writeText(Surf_Display, textBuffer.data(), 20, 20);
        // text for MinReduceHeight and MaxRaiseHeight
        sprintf(textBuffer.data(), "min. height: %#04x/0x3C  max. height: %#04x/0x3C  NormalNull: 0x0A", MapObj->getMinReduceHeight(),
                MapObj->getMaxRaiseHeight());
        CFont::writeText(Surf_Display, textBuffer.data(), 100, 20);
        // text for MovementLocked
        if(MapObj->isHorizontalMovementLocked() && MapObj->isVerticalMovementLocked())
            CFont::writeText(Surf_Display, "Movement locked (F9 or F10 to unlock)", 20, 40, 14, FONT_ORANGE);
        else if(MapObj->isHorizontalMovementLocked())
            CFont::writeText(Surf_Display, "Horizontal movement locked (F9 to unlock)", 20, 40, 14, FONT_ORANGE);
        else if(MapObj->isVerticalMovementLocked())
            CFont::writeText(Surf_Display, "Vertikal movement locked (F10 to unlock)", 20, 40, 14, FONT_ORANGE);
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
            CSurface::Draw(Surf_Display, global::bmpArray[CROSS].surface, Cursor.x, Cursor.y);
        else
            CSurface::Draw(Surf_Display, global::bmpArray[CURSOR_CLICKED].surface, Cursor.x, Cursor.y);
    } else
        CSurface::Draw(Surf_Display, global::bmpArray[CURSOR].surface, Cursor.x, Cursor.y);

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

    if(useOpenGL)
    {
        SDL_BlitSurface(Surf_Display.get(), nullptr, Surf_DisplayGL.get(), nullptr);
        SDL_Flip(Surf_DisplayGL.get());
        SDL_GL_SwapBuffers();
    } else
        SDL_Flip(Surf_Display.get());

    if(msWait)
        SDL_Delay(msWait);
}

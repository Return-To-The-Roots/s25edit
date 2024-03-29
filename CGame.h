// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "CIO/CFont.h"
#include "SdlSurface.h"
#include <Point.h>
#include <memory>
#include <vector>

class CWindow;
class CMap;
class CMenu;

class CGame
{
    friend class CDebug;

public:
    Extent GameResolution;
    bool fullscreen;

    bool Running;
    bool showLoadScreen;
    SdlSurface Surf_Display;
    SdlTexture displayTexture_;
    SdlRenderer renderer_;
    SdlWindow window_;

private:
#ifdef _ADMINMODE
    // some debugging variables
    unsigned long int FrameCounter = 0;
#endif
    // milliseconds for SDL_Delay()
    Uint32 msWait = 0;
    Uint32 framesPassedSinceLastFps = 0;
    Uint32 lastFpsTick = 0;
    CFont lastFps;

    Uint32 lastFrameTime = 0;

    // structure for mouse cursor
    struct
    {
        Position pos = {0, 0};
        bool clicked = false;
        struct
        {
            bool left = false;
            bool right = false;
        } button;
    } Cursor;

    // Object for Menu Screens
    std::vector<std::unique_ptr<CMenu>> Menus;
    // Object for Windows
    std::vector<std::unique_ptr<CWindow>> Windows;
    // Object for Callbacks
    std::vector<void (*)(int)> Callbacks;
    // Object for the Map
    std::unique_ptr<CMap> MapObj;

    void SetAppIcon();

public:
    CGame(Extent GameResolution_, bool fullscreen_);
    ~CGame();

    int Execute();

    bool Init();
    bool ReCreateWindow();

    void EventHandling(SDL_Event* Event);

    void GameLoop();

    void Render();

    void RenderPresent() const;

    CMenu* RegisterMenu(std::unique_ptr<CMenu> Menu);
    bool UnregisterMenu(CMenu* Menu);
    CWindow* RegisterWindow(std::unique_ptr<CWindow> Window);
    bool UnregisterWindow(CWindow* Window);
    void RegisterCallback(void (*callback)(int));
    bool UnregisterCallback(void (*callback)(int));
    void setMapObj(std::unique_ptr<CMap> MapObj);
    CMap* getMapObj();
    void delMapObj();
    SDL_Surface* getDisplaySurface() const { return Surf_Display.get(); };
    auto getRes() const { return GameResolution; }
};

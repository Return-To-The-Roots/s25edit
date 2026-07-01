// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "CIO/CFont.h"
#include "SdlSurface.h"
#include "Texture.h"
#include <boost/filesystem/path.hpp>
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
    Texture displayTexture_;
    SDL_GLContext glContext_ = nullptr;
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
    Extent lastSetResolution_ = Extent{0, 0}; ///< Last resolution we tried to apply (to avoid infinite resize loops)

    // GL textures for splash screen and cursor
    Texture splashBg_;
    Texture cursor_;
    Texture cursorClicked_;
    Texture cross_;

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
    void RecreateDisplayResources();
    void setGLViewport();

public:
    void LoadSettings();
    void SaveSettings() const;

    CGame(Extent GameResolution_, bool fullscreen_);
    ~CGame();

    int Execute();

    bool Init();
    bool ReCreateWindow();
    void UpdateDisplaySize(const Extent& newSize);

    void EventHandling(SDL_Event* Event);

    void GameLoop();

    void Render();

    void RenderPresent();

    CMenu* RegisterMenu(std::unique_ptr<CMenu> Menu);
    bool UnregisterMenu(CMenu* Menu);
    CWindow* RegisterWindow(std::unique_ptr<CWindow> Window);
    bool UnregisterWindow(CWindow* Window);
    void RegisterCallback(void (*callback)(int));
    bool UnregisterCallback(void (*callback)(int));
    void setMapObj(std::unique_ptr<CMap> MapObj);
    CMap* getMapObj();
    void delMapObj();
    void enterEditor(const boost::filesystem::path& filepath);
    SDL_Surface* getDisplaySurface() const { return Surf_Display.get(); };
    auto getRes() const { return GameResolution; }
};

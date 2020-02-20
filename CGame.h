#ifndef _CGAME_H
#define _CGAME_H

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
        Uint16 x, y;
        bool clicked;
        struct
        {
            bool left;
            bool right;
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
    CGame();
    ~CGame();

    int Execute();

    bool Init();
    bool ReCreateWindow();

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
    SDL_Surface* getDisplaySurface() { return Surf_Display.get(); };
    auto getRes() { return GameResolution; }
};

#endif

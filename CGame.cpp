#include "CGame.h"
#include "CIO/CMenu.h"
#include "CIO/CWindow.h"
#include "CMap.h"
#include "RttrConfig.h"
#include "files.h"
#include "globals.h"
#include <boost/filesystem.hpp>
#include <boost/nowide/cstdio.hpp>
#include <iostream>
#include <limits>

namespace bfs = boost::filesystem;

//#include <vld.h>

CGame::CGame()
    : GameResolution(1024, 768), fullscreen(false), Running(true), showLoadScreen(true), Surf_Display(nullptr), Surf_DisplayGL(nullptr)
{
#ifdef _ADMINMODE
    FrameCounter = 0;
    RegisteredCallbacks = 0;
    RegisteredWindows = 0;
    RegisteredMenus = 0;
#endif

    msWait = 0;

    // mouse cursor data
    Cursor.x = 0;
    Cursor.y = 0;
    Cursor.clicked = false;
    Cursor.button.left = false;
    Cursor.button.right = false;

    for(auto& Menu : Menus)
        Menu = nullptr;
    for(auto& Window : Windows)
        Window = nullptr;
    for(auto& Callback : Callbacks)
        Callback = nullptr;
    MapObj = nullptr;
}

CGame::~CGame() {}

int CGame::Execute()
{
    if(Init() == false)
        return -1;

    SDL_Event Event;

    while(Running)
    {
        while(SDL_PollEvent(&Event))
            EventHandling(&Event);

        GameLoop();
        Render();
    }

    Cleanup();

    return 0;
}

bool CGame::RegisterMenu(CMenu* Menu)
{
    bool success = false;

    if(!Menu)
        return success;
    for(auto& i : Menus)
    {
        if(!success && !i)
        {
            i = Menu;
            i->setActive();
            success = true;
#ifdef _ADMINMODE
            RegisteredMenus++;
#endif
        } else if(i)
            i->setInactive();
    }
    return success;
}

bool CGame::UnregisterMenu(CMenu* Menu)
{
    if(!Menu)
        return false;
    for(int i = 0; i < MAXMENUS; i++)
    {
        if(Menus[i] == Menu)
        {
            for(int j = i - 1; j >= 0; j--)
            {
                if(Menus[j])
                {
                    Menus[j]->setActive();
                    break;
                }
            }
            delete Menus[i];
            Menus[i] = nullptr;
#ifdef _ADMINMODE
            RegisteredMenus--;
#endif
            return true;
        }
    }
    return false;
}

bool CGame::RegisterWindow(CWindow* Window)
{
    bool success = false;
    int highestPriority = 0;

    // first find the highest priority
    for(const auto* curWnd : Windows)
    {
        if(curWnd && curWnd->getPriority() > highestPriority)
            highestPriority = curWnd->getPriority();
    }

    if(!Window)
        return success;
    for(auto& i : Windows)
    {
        if(!success && !i)
        {
            i = Window;
            i->setActive();
            i->setPriority(highestPriority + 1);
            success = true;
#ifdef _ADMINMODE
            RegisteredWindows++;
#endif
        } else if(i)
            i->setInactive();
    }
    return success;
}

bool CGame::UnregisterWindow(CWindow* Window)
{
    if(!Window)
        return false;
    for(int i = 0; i < MAXWINDOWS; i++)
    {
        if(Windows[i] == Window)
        {
            for(int j = i - 1; j >= 0; j--)
            {
                if(Windows[j])
                {
                    Windows[j]->setActive();
                    break;
                }
            }
            delete Windows[i];
            Windows[i] = nullptr;
#ifdef _ADMINMODE
            RegisteredWindows--;
#endif
            return true;
        }
    }
    return false;
}

bool CGame::RegisterCallback(void (*callback)(int))
{
    if(!callback)
        return false;
    for(auto& Callback : Callbacks)
    {
        if(!Callback)
        {
            Callback = callback;
#ifdef _ADMINMODE
            RegisteredCallbacks++;
#endif
            return true;
        }
    }
    return false;
}

bool CGame::UnregisterCallback(void (*callback)(int))
{
    if(!callback)
        return false;
    for(auto& Callback : Callbacks)
    {
        if(Callback == callback)
        {
            Callback = nullptr;
#ifdef _ADMINMODE
            RegisteredCallbacks--;
#endif
            return true;
        }
    }
    return false;
}

void CGame::delMapObj()
{
    delete MapObj;
    MapObj = nullptr;
}

void WaitForEnter()
{
#ifndef NDEBUG

    static bool waited = false;
    if(waited)
        return;
    waited = true;
    std::cout << "\n\nPress ENTER to close this window . . ." << std::endl;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
#endif // !NDEBUG
}

bool checkWriteable(const bfs::path& folder)
{
    if(!bfs::exists(folder))
    {
        boost::system::error_code ec;
        bfs::create_directories(folder, ec);
        if(ec)
            return false;
    }
    bfs::path testFileName = folder / bfs::unique_path();
    FILE* fp = boost::nowide::fopen(testFileName.string().c_str(), "wb");
    if(!fp)
        return false;
    fclose(fp);
    bfs::remove(testFileName);
    return true;
}

#undef main
int main(int /*argc*/, char* /*argv*/ [])
{
    if(!RTTRCONFIG.Init())
    {
        std::cerr << "Failed to init program!" << std::endl;
        WaitForEnter();
        return 1;
    }

    global::gameDataFilePath = RTTRCONFIG.ExpandPath("<RTTR_GAME>");
    // Prefer application folder over user folder
    global::userMapsPath = RTTRCONFIG.ExpandPath("WORLDS");
    if(!checkWriteable(global::userMapsPath))
    {
        global::userMapsPath = RTTRCONFIG.ExpandPath(FILE_PATHS[41]);
        if(!checkWriteable(global::userMapsPath))
        {
            std::cerr << "Could not find a writable folder for maps\nCheck " << global::userMapsPath << std::endl;
            return 1;
        }
    }
    std::cout << "Expecting S2 game files in " << global::gameDataFilePath << std::endl;
    std::cout << "Maps folder set to " << global::userMapsPath << std::endl;
    boost::system::error_code ec;
    boost::filesystem::create_directories(global::userMapsPath, ec);
    if(ec)
    {
        std::cerr << "Could not create " << global::userMapsPath << ": " << ec.message() << std::endl;
        WaitForEnter();
        return 1;
    }

    try
    {
        global::s2 = new CGame;

        global::s2->Execute();
    } catch(...)
    {
        std::cerr << "Unhandled Exception" << std::endl;
        delete global::s2;
        WaitForEnter();
        return 1;
    }
    delete global::s2;

    WaitForEnter();
    return 0;
}

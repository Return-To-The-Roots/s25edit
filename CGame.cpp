// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CGame.h"
#include "CIO/CMenu.h"
#include "CIO/CWindow.h"
#include "CMap.h"
#include "RttrConfig.h"
#include "files.h"
#include "globals.h"
#include <boost/filesystem.hpp>
#include <boost/nowide/cstdio.hpp>
#include <boost/program_options.hpp>
#include <exception>
#include <iostream>
#include <limits>

namespace bfs = boost::filesystem;

//#include <vld.h>

boost::program_options::variables_map parse_cmdline_args(int argc, char* argv[]);

CGame::CGame(Extent GameResolution_, bool fullscreen_)
    : GameResolution(GameResolution_), fullscreen(fullscreen_), Running(true), showLoadScreen(true),
      lastFps("", 0, 0, FontSize::Medium)
{
    global::bmpArray.resize(MAXBOBBMP);
    global::shadowArray.resize(MAXBOBSHADOW);
    global::s2 = this;
}

CGame::~CGame()
{
    global::s2 = nullptr;
}

int CGame::Execute()
{
    if(!Init())
        return -1;

    SDL_Event Event;
    lastFps.setText("");
    lastFpsTick = SDL_GetTicks();
    lastFrameTime = SDL_GetTicks();
    framesPassedSinceLastFps = 0;

    while(Running)
    {
        while(SDL_PollEvent(&Event))
            EventHandling(&Event);

        GameLoop();
        Render();
    }

    return 0;
}

void CGame::RenderPresent() const
{
    SDL_UpdateTexture(displayTexture_.get(), nullptr, Surf_Display->pixels, Surf_Display->w * sizeof(Uint32));
    SDL_RenderClear(renderer_.get());
    SDL_RenderCopy(renderer_.get(), displayTexture_.get(), nullptr, nullptr);
    SDL_RenderPresent(renderer_.get());
}

CMenu* CGame::RegisterMenu(std::unique_ptr<CMenu> Menu)
{
    for(auto& i : Menus)
        i->setInactive();

    Menu->setActive();
    Menus.emplace_back(std::move(Menu));

    return Menus.back().get();
}

bool CGame::UnregisterMenu(CMenu* Menu)
{
    auto it = std::find_if(Menus.begin(), Menus.end(), [Menu](const auto& cur) { return cur.get() == Menu; });
    if(it == Menus.end())
        return false;
    if(it != Menus.begin())
        it[-1]->setActive();
    Menus.erase(it);
    return true;
}

CWindow* CGame::RegisterWindow(std::unique_ptr<CWindow> Window)
{
    // first find the highest priority
    const auto itHighestPriority =
      std::max_element(Windows.cbegin(), Windows.cend(),
                       [](const auto& lhs, const auto& rhs) { return lhs->getPriority() < rhs->getPriority(); });
    const int highestPriority = itHighestPriority == Windows.cend() ? 0 : (*itHighestPriority)->getPriority();

    for(auto& i : Windows)
        i->setInactive();

    Window->setActive();
    Window->setPriority(highestPriority + 1);
    Windows.emplace_back(std::move(Window));

    return Windows.back().get();
}

bool CGame::UnregisterWindow(CWindow* Window)
{
    auto it = std::find_if(Windows.begin(), Windows.end(), [Window](const auto& cur) { return cur.get() == Window; });
    if(it == Windows.end())
        return false;
    if(it != Windows.begin())
        it[-1]->setActive();
    Windows.erase(it);
    return true;
}

void CGame::RegisterCallback(void (*callback)(int))
{
    assert(callback);
    Callbacks.push_back(callback);
}

bool CGame::UnregisterCallback(void (*callback)(int))
{
    auto it = std::find(Callbacks.begin(), Callbacks.end(), callback);
    if(it == Callbacks.end())
        return false;
    Callbacks.erase(it);
    return true;
}

void CGame::setMapObj(std::unique_ptr<CMap> MapObj)
{
    this->MapObj = std::move(MapObj);
}

CMap* CGame::getMapObj()
{
    return MapObj.get();
}

void CGame::delMapObj()
{
    MapObj.reset();
}

void CGame::GameLoop()
{
    for(auto&& callback : Callbacks)
        callback(CALL_FROM_GAMELOOP);
    const auto isWaste = [](const auto& p) { return p->isWaste(); };
    auto itMenu = std::find_if(Menus.begin(), Menus.end(), isWaste);
    while(itMenu != Menus.end())
    {
        UnregisterMenu(itMenu->get());
        itMenu = std::find_if(Menus.begin(), Menus.end(), isWaste);
    }
    auto itWnd = std::find_if(Windows.begin(), Windows.end(), isWaste);
    while(itWnd != Windows.end())
    {
        UnregisterWindow(itWnd->get());
        itWnd = std::find_if(Windows.begin(), Windows.end(), isWaste);
    }
}

namespace {
void WaitForEnter()
{
#ifndef NDEBUG

    static bool waited = false;
    if(waited)
        return;
    waited = true;
    std::cout << "\n\nPress ENTER *twice* to close this window . . ." << std::endl;
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
} // namespace

#undef main
int main(int argc, char* argv[])
{
    auto programOptions = parse_cmdline_args(argc, argv);

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
        global::userMapsPath = RTTRCONFIG.ExpandPath(s25::folders::mapsOwn);
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

    std::cout << "Initializing SDL...";
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cout << "failure";
        return 1;
    }
    std::cout << "done\n";
    int result = 0;
    try
    {
        auto s2 = std::make_unique<CGame>(
          Extent(programOptions["width"].as<unsigned>(), programOptions["height"].as<unsigned>()),
          programOptions["fullscreen"].as<bool>());
        result = s2->Execute();
    } catch(...)
    {
        std::cerr << "Unhandled Exception" << std::endl;
        result = 1;
    }
    SDL_Quit();

    WaitForEnter();
    return result;
}

boost::program_options::variables_map parse_cmdline_args(int argc, char* argv[])
{
    using std::cout;
    using std::endl;
    using std::exception;
    namespace po = boost::program_options;

    po::variables_map result;

    try
    {
        po::options_description desc("Options");
        // clang-format off
        desc.add_options()
            ("help",                                                   "Show help")
            ("width",      po::value<unsigned>()->default_value(1024), "Set width")
            ("height",     po::value<unsigned>()->default_value(768),  "Set height")
            ("fullscreen", po::value<bool>()->default_value(false),    "Set fullscreen");
        // clang-format on

        po::store(po::parse_command_line(argc, argv, desc), result);
        po::notify(result);

        if(result.count("help"))
        {
            cout << desc << endl;
            exit(EXIT_SUCCESS);
        }

        cout << "Resolution set to: " << result["width"].as<unsigned>() << "x" << result["height"].as<unsigned>() << " "
             << (result["fullscreen"].as<bool>() ? "fullscreen" : "window") << "-mode" << endl;
    } catch(exception& e)
    {
        cout << "error: " << e.what() << endl;
        exit(EXIT_SUCCESS);
    } catch(...)
    {
        cout << "Exception of unknown type!" << endl;
        exit(EXIT_SUCCESS);
    }

    return result;
}

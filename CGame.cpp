// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2024 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CGame.h"
#include "RttrConfig.h"
#include "defines.h"
#include "files.h"
#include "globals.h"
#include "s25util/file_handle.h"
#include <libsiedler2/ArchivItem_Ini.h>
#include <libsiedler2/libsiedler2.h>
#include "drivers/VideoDriverWrapper.h"
#include <glad/glad.h>
#include <boost/filesystem.hpp>
#include <boost/nowide/cstdio.hpp>
#include <boost/program_options.hpp>
#include <csignal>
#include <cstdio>
#include <exception>
#include <iostream>
#include <limits>
#ifdef _WIN32
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <windows.h>
#endif

namespace bfs = boost::filesystem;

boost::program_options::variables_map parse_cmdline_args(int argc, char* argv[]);

CGame::CGame(Extent GameResolution_, bool fullscreen_)
    : GameResolution(GameResolution_), fullscreen(fullscreen_), Running(true), showLoadScreen(true)
{
    global::s2 = this;
}

CGame::~CGame()
{
    if(glContext_)
    {
        SDL_GL_DeleteContext(glContext_);
        glContext_ = nullptr;
    }
    global::s2 = nullptr;
}

int CGame::Execute()
{
    if(!Init())
        return -1;

    lastFrameTime = SDL_GetTicks();

    while(Running)
    {
        // Let the video driver poll and dispatch events to the WindowManager
        if(!VIDEODRIVER.Run())
            Running = false;

        GameLoop();
        Render();
    }

    return 0;
}

void CGame::RenderPresent()
{
    VIDEODRIVER.SwapBuffers();
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

void CGame::LoadSettings()
{
    const bfs::path settingsPath = RTTRCONFIG.ExpandPath("<RTTR_USERDATA>/s25edit.ini");
    libsiedler2::Archiv settings;
    if(libsiedler2::Load(settingsPath, settings) != 0)
        return;
    const auto* ini = dynamic_cast<const libsiedler2::ArchivItem_Ini*>(settings.find("editor"));
    if(!ini)
        return;
    GameResolution.x = ini->getValue("width", static_cast<int>(GameResolution.x));
    GameResolution.y = ini->getValue("height", static_cast<int>(GameResolution.y));
    fullscreen = ini->getValue("fullscreen", fullscreen);
}

void CGame::SaveSettings() const
{
    libsiedler2::Archiv settings;
    settings.push(std::make_unique<libsiedler2::ArchivItem_Ini>("editor"));
    auto& ini = dynamic_cast<libsiedler2::ArchivItem_Ini&>(*settings.find("editor"));
    ini.setValue("width", GameResolution.x);
    ini.setValue("height", GameResolution.y);
    ini.setValue("fullscreen", static_cast<int>(fullscreen));
    const bfs::path settingsPath = RTTRCONFIG.ExpandPath("<RTTR_USERDATA>/s25edit.ini");
    libsiedler2::Write(settingsPath, settings);
}

void CGame::GameLoop()
{
    for(auto&& callback : Callbacks)
        callback(CALL_FROM_GAMELOOP);
}

namespace {

#ifdef _WIN32
BOOL WINAPI ConsoleSignalHandler(DWORD dwCtrlType)
{
    switch(dwCtrlType)
    {
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_C_EVENT:
        {
            if(global::s2)
                global::s2->Running = false;
            return TRUE;
        }
        break;
    }
    return FALSE;
}
#else
static bool killme = false;
void ConsoleSignalHandler(int /*sig*/)
{
    if(!killme)
        std::cerr << "Do you really want to terminate the program (y/n) : ";
    else
        std::cerr << "Do you really want to kill the program (y/n) : ";

    int c = getchar();
    if(c == 'j' || c == 'y' || c == 1079565930)
    {
        if(killme)
            exit(1);

        killme = true;
        if(global::s2)
            global::s2->Running = false;
    }
}
#endif

void WaitForEnter()
{
    static bool waited = false;
    if(waited)
        return;
    waited = true;
    std::cerr << "\n\nPress ENTER to close this window . . .\n";
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

void InstallSignalHandlers()
{
#ifdef _WIN32
    SetConsoleCtrlHandler(ConsoleSignalHandler, TRUE);
#else
    struct sigaction sa;
    sa.sa_handler = ConsoleSignalHandler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, nullptr);
#endif
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
    s25util::file_handle fp(boost::nowide::fopen(testFileName.string().c_str(), "wb"));
    if(!fp)
        return false;
    fp.reset();
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
    InstallSignalHandlers();
    int result = 0;
    try
    {
        auto s2 = std::make_unique<CGame>(
          Extent(programOptions["width"].as<unsigned>(), programOptions["height"].as<unsigned>()),
          programOptions["fullscreen"].as<bool>());
        s2->LoadSettings();
        result = s2->Execute();
    } catch(...)
    {
        std::cerr << "Unhandled Exception" << std::endl;
        result = 1;
    }
    SDL_Quit();

    if(result)
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

// VideoDriverLoaderInterface callbacks
// These are called by the video driver's MessageLoop.
// We currently use our own SDL_PollEvent loop, so these are no-ops for now.
void CGame::Msg_LeftDown(MouseCoords /*mc*/) {}
void CGame::Msg_LeftUp(MouseCoords /*mc*/) {}
void CGame::Msg_RightDown(const MouseCoords& /*mc*/) {}
void CGame::Msg_RightUp(const MouseCoords& /*mc*/) {}
void CGame::Msg_MiddleDown(const MouseCoords& /*mc*/) {}
void CGame::Msg_MiddleUp(const MouseCoords& /*mc*/) {}
void CGame::Msg_WheelUp(const MouseCoords& /*mc*/) {}
void CGame::Msg_WheelDown(const MouseCoords& /*mc*/) {}
void CGame::Msg_MouseMove(const MouseCoords& /*mc*/) {}
void CGame::Msg_KeyDown(const KeyEvent& /*ke*/) {}
void CGame::WindowResized() {}


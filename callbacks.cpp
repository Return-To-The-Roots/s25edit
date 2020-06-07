#include "callbacks.h"
#include "CDebug.h"
#include "CGame.h"
#include "CIO/CButton.h"
#include "CIO/CFile.h"
#include "CIO/CFont.h"
#include "CIO/CMenu.h"
#include "CIO/CPicture.h"
#include "CIO/CSelectBox.h"
#include "CIO/CTextfield.h"
#include "CIO/CWindow.h"
#include "CMap.h"
#include "CSurface.h"
#include "globals.h"
#include "helpers/format.hpp"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <algorithm>

namespace bfs = boost::filesystem;

void callback::PleaseWait(int Param)
{
    // NOTE: This "Please wait"-window is shown until the PleaseWait-callback is called with 'WINDOW_QUIT_MESSAGE'.
    //      The window will be registered by the game. To do it the other way (create and then let it automatically
    //      destroy by the gameloop), you don't need to register the window, but register the callback.

    static CWindow* WNDWait;

    enum
    {
        WINDOWQUIT
    };

    switch(Param)
    {
        case INITIALIZING_CALL:
            if(WNDWait)
                break;
            WNDWait = global::s2->RegisterWindow(
              std::make_unique<CWindow>(PleaseWait, WINDOWQUIT, WindowPos::Center, Extent(212, 70), "Please wait"));
            // we don't register this window cause we will destroy it manually if we need
            // global::s2->RegisterCallback(PleaseWait);

            WNDWait->addText("Please wait ...", 10, 10, FontSize::Large);
            // we need to render this window NOW, cause the render loop will do it too late (when the operation
            // is done and we don't need the "Please wait"-window anymore)
            CSurface::Draw(global::s2->getDisplaySurface(), WNDWait->getSurface(), global::s2->getDisplaySurface()->w / 2 - 106,
                           global::s2->getDisplaySurface()->h / 2 - 35);
            global::s2->RenderPresent();
            break;

        case CALL_FROM_GAMELOOP: // This window gives a "Please Wait"-string, so it is shown while there is an intensive operation
            // during ONE gameloop. Therefore it is only shown DURING this ONE operation. If the next gameloop
            // appears, the operation MUST have been finished and we can destroy this window.
            if(WNDWait)
            {
                global::s2->UnregisterCallback(PleaseWait);
                WNDWait->setWaste();
                WNDWait = nullptr;
            }
            break;

        case WINDOW_QUIT_MESSAGE: // this is the global window quit message, callback is explicit called with this value, so destroy the
                                  // window
            if(WNDWait)
            {
                WNDWait->setWaste();
                WNDWait = nullptr;
            }
            break;

        default: break;
    }
}

void callback::ShowStatus(int Param)
{
    static CWindow* WND = nullptr;
    static CFont* txt = nullptr;

    enum
    {
        WINDOWQUIT,
        SHOW_SUCCESS,
        SHOW_FAILURE
    };

    switch(Param)
    {
        case INITIALIZING_CALL:
            if(WND)
                break;
            WND = global::s2->RegisterWindow(std::make_unique<CWindow>(ShowStatus, WINDOWQUIT, WindowPos::Center, Extent(250, 90), "Status",
                                                                       WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MINIMIZE | WINDOW_MOVE));
            txt = WND->addText("", 26, 20, FontSize::Large, FontColor::Yellow);
            break;
        case SHOW_SUCCESS:
            assert(txt);
            txt->setText("Operation finished successfully");
            txt->setColor(FontColor::Green);
            break;
        case SHOW_FAILURE:
            assert(txt);
            txt->setText("Operation failed! :(");
            txt->setColor(FontColor::BrightRed);
            break;

        case WINDOWQUIT:
        case MAP_QUIT:
            if(WND)
            {
                WND->setWaste();
                WND = nullptr;
            }
            break;

        default: break;
    }
}

void callback::mainmenu(int Param)
{
    static CMenu* MainMenu = nullptr;

    enum
    {
        ENDGAME = 1,
        STARTEDITOR,
        OPTIONS
    };

    switch(Param)
    {
        case INITIALIZING_CALL:
            MainMenu = global::s2->RegisterMenu(std::make_unique<CMenu>(SPLASHSCREEN_MAINMENU));
            MainMenu->addButton(mainmenu, ENDGAME, 50, 400, 200, 20, BUTTON_RED1, "Quit program");
#ifdef _ADMINMODE
            MainMenu->addButton(submenu1, INITIALIZING_CALL, 50, 200, 200, 20, BUTTON_GREY, "Submenu_1");
#endif
            MainMenu->addButton(mainmenu, STARTEDITOR, 50, 160, 200, 20, BUTTON_RED1, "Start editor");
            MainMenu->addButton(mainmenu, OPTIONS, 50, 370, 200, 20, BUTTON_GREEN2, "Options");
            break;

        case CALL_FROM_GAMELOOP: break;

        case ENDGAME:
            assert(MainMenu);
            MainMenu->setWaste();
            MainMenu = nullptr;
            global::s2->Running = false;
            break;

        case STARTEDITOR:
            assert(MainMenu);
            PleaseWait(INITIALIZING_CALL);
            global::s2->setMapObj(std::make_unique<CMap>(""));
            MainMenu->setWaste();
            MainMenu = nullptr;
            PleaseWait(WINDOW_QUIT_MESSAGE);
            break;

        case OPTIONS:
            assert(MainMenu);
            MainMenu->setWaste();
            MainMenu = nullptr;
            submenuOptions(INITIALIZING_CALL);

        default: break;
    }
}

void callback::submenuOptions(int Param)
{
    static CMenu* SubMenu = nullptr;
    static CFont* TextResolution = nullptr;
    static CButton* ButtonFullscreen = nullptr;
    std::array<char, 80> puffer;
    static CSelectBox* SelectBoxRes = nullptr;

    enum
    {
        MAINMENU = 1,
        FULLSCREEN,
        GRAPHICS_CHANGE,
        SELECTBOX_800_600,
        SELECTBOX_832_624,
        SELECTBOX_960_540,
        SELECTBOX_964_544,
        SELECTBOX_960_640,
        SELECTBOX_960_720,
        SELECTBOX_1024_576,
        SELECTBOX_1024_600,
        SELECTBOX_1072_600,
        SELECTBOX_1152_768,
        SELECTBOX_1024_768,
        SELECTBOX_1152_864,
        SELECTBOX_1152_870,
        SELECTBOX_1152_900,
        SELECTBOX_1200_800,
        SELECTBOX_1200_900,
        SELECTBOX_1280_720,
        SELECTBOX_1280_768,
        SELECTBOX_1280_800,
        SELECTBOX_1280_854,
        SELECTBOX_1360_768,
        SELECTBOX_1366_768,
        SELECTBOX_1376_768,
        SELECTBOX_1400_900,
        SELECTBOX_1440_900,
        SELECTBOX_1440_960,
        SELECTBOX_1280_960,
        SELECTBOX_1280_1024,
        SELECTBOX_1360_1024,
        SELECTBOX_1366_1024,
        SELECTBOX_1600_768,
        SELECTBOX_1600_900,
        SELECTBOX_1600_1024,
        SELECTBOX_1400_1050,
        SELECTBOX_1680_1050,
        SELECTBOX_1600_1200,
        SELECTBOX_1920_1080,
        SELECTBOX_1920_1200,
        SELECTBOX_1920_1400,
        SELECTBOX_1920_1440,
        SELECTBOX_2048_1152,
        SELECTBOX_2048_1536
    };

    switch(Param)
    {
        case INITIALIZING_CALL:
            SubMenu = global::s2->RegisterMenu(std::make_unique<CMenu>(SPLASHSCREEN_SUBMENU3));
            // add button for "back to main menu"
            SubMenu->addButton(submenuOptions, MAINMENU, (int)(global::s2->GameResolution.x / 2 - 100), 440, 200, 20, BUTTON_RED1, "back");
            // add menu title
            SubMenu->addText("Options", (int)(global::s2->GameResolution.x / 2 - 20), 10, FontSize::Large);
            // add screen resolution
            if(TextResolution)
                SubMenu->delText(TextResolution);
            sprintf(puffer.data(), "Game Resolution: %d*%d / %s", global::s2->GameResolution.x, global::s2->GameResolution.y,
                    (global::s2->fullscreen ? "Fullscreen" : "Window"));
            TextResolution = SubMenu->addText(puffer.data(), (int)(global::s2->GameResolution.x / 2 - 110), 50, FontSize::Medium);
            if(ButtonFullscreen)
                SubMenu->delButton(ButtonFullscreen);
            ButtonFullscreen = SubMenu->addButton(submenuOptions, FULLSCREEN, (int)(global::s2->GameResolution.x / 2 - 100), 190, 200, 20,
                                                  BUTTON_RED1, (global::s2->fullscreen ? "WINDOW" : "FULLSCREEN"));
            SelectBoxRes = SubMenu->addSelectBox(ButtonFullscreen->getPos() - Point16(0, 120), Extent16(200, 110), FontSize::Medium,
                                                 FontColor::Yellow, BUTTON_GREY);
            SelectBoxRes->setOption("800 x 600 (SVGA)", submenuOptions, SELECTBOX_800_600);
            SelectBoxRes->setOption("832 x 624 (Half Megapixel)", submenuOptions, SELECTBOX_832_624);
            SelectBoxRes->setOption("960 x 540 (QHD)", submenuOptions, SELECTBOX_960_540);
            SelectBoxRes->setOption("964 x 544", submenuOptions, SELECTBOX_964_544);
            SelectBoxRes->setOption("960 x 640 (DVGA)", submenuOptions, SELECTBOX_960_640);
            SelectBoxRes->setOption("960 x 720", submenuOptions, SELECTBOX_960_720);
            SelectBoxRes->setOption("1024 x 576 (WXGA)", submenuOptions, SELECTBOX_1024_576);
            SelectBoxRes->setOption("1024 x 600 (WSVGA)", submenuOptions, SELECTBOX_1024_600);
            SelectBoxRes->setOption("1072 x 600 (WSVGA)", submenuOptions, SELECTBOX_1072_600);
            SelectBoxRes->setOption("1152 x 768", submenuOptions, SELECTBOX_1152_768);
            SelectBoxRes->setOption("1024 x 768 (EVGA)", submenuOptions, SELECTBOX_1024_768);
            SelectBoxRes->setOption("1152 x 864 (XGA)", submenuOptions, SELECTBOX_1152_864);
            SelectBoxRes->setOption("1152 x 870 (XGA)", submenuOptions, SELECTBOX_1152_870);
            SelectBoxRes->setOption("1152 x 900 (XGA)", submenuOptions, SELECTBOX_1152_900);
            SelectBoxRes->setOption("1200 x 800 (DSVGA)", submenuOptions, SELECTBOX_1200_800);
            SelectBoxRes->setOption("1200 x 900 (OLPC)", submenuOptions, SELECTBOX_1200_900);
            SelectBoxRes->setOption("1280 x 720 (720p)", submenuOptions, SELECTBOX_1280_720);
            SelectBoxRes->setOption("1280 x 768 (WXGA)", submenuOptions, SELECTBOX_1280_768);
            SelectBoxRes->setOption("1280 x 800 (WXGA)", submenuOptions, SELECTBOX_1280_800);
            SelectBoxRes->setOption("1280 x 854 (WXGA)", submenuOptions, SELECTBOX_1280_854);
            SelectBoxRes->setOption("1360 x 768 (WXGA)", submenuOptions, SELECTBOX_1360_768);
            SelectBoxRes->setOption("1366 x 768 (WXGA)", submenuOptions, SELECTBOX_1366_768);
            SelectBoxRes->setOption("1376 x 768 (WXGA)", submenuOptions, SELECTBOX_1376_768);
            SelectBoxRes->setOption("1400 x 900 (WXGA+)", submenuOptions, SELECTBOX_1400_900);
            SelectBoxRes->setOption("1440 x 900 (WXGA+)", submenuOptions, SELECTBOX_1440_900);
            SelectBoxRes->setOption("1440 x 960", submenuOptions, SELECTBOX_1440_960);
            SelectBoxRes->setOption("1280 x 960 (SXGA)", submenuOptions, SELECTBOX_1280_960);
            SelectBoxRes->setOption("1280 x 1024 (SXGA)", submenuOptions, SELECTBOX_1280_1024);
            SelectBoxRes->setOption("1360 x 1024 (XGA-2)", submenuOptions, SELECTBOX_1360_1024);
            SelectBoxRes->setOption("1366 x 1024 (XGA-2)", submenuOptions, SELECTBOX_1366_1024);
            SelectBoxRes->setOption("1600 x 768 (UWXGA)", submenuOptions, SELECTBOX_1600_768);
            SelectBoxRes->setOption("1600 x 900 (WSXGA)", submenuOptions, SELECTBOX_1600_900);
            SelectBoxRes->setOption("1600 x 1024 (WSXGA)", submenuOptions, SELECTBOX_1600_1024);
            SelectBoxRes->setOption("1400 x 1050 (SXGA+)", submenuOptions, SELECTBOX_1400_1050);
            SelectBoxRes->setOption("1680 x 1050 (WSXGA+)", submenuOptions, SELECTBOX_1680_1050);
            SelectBoxRes->setOption("1600 x 1200 (UXGA)", submenuOptions, SELECTBOX_1600_1200);
            SelectBoxRes->setOption("1920 x 1080 (1080p)", submenuOptions, SELECTBOX_1920_1080);
            SelectBoxRes->setOption("1920 x 1200 (WUXGA)", submenuOptions, SELECTBOX_1920_1200);
            SelectBoxRes->setOption("1920 x 1400 (TXGA)", submenuOptions, SELECTBOX_1920_1400);
            SelectBoxRes->setOption("1920 x 1440", submenuOptions, SELECTBOX_1920_1440);
            SelectBoxRes->setOption("2048 x 1152 (QWXGA)", submenuOptions, SELECTBOX_2048_1152);
            SelectBoxRes->setOption("2048 x 1536 (SUXGA)", submenuOptions, SELECTBOX_2048_1536);
            break;

        case MAINMENU:
            assert(SubMenu);
            SubMenu->setWaste();
            TextResolution = nullptr;
            ButtonFullscreen = nullptr;
            SelectBoxRes = nullptr;
            SubMenu = nullptr;
            mainmenu(INITIALIZING_CALL);
            break;

        case FULLSCREEN:
            global::s2->fullscreen = !global::s2->fullscreen;

            submenuOptions(GRAPHICS_CHANGE);
            break;

        case GRAPHICS_CHANGE:
            assert(SubMenu);
            SubMenu->setWaste();
            TextResolution = nullptr;
            ButtonFullscreen = nullptr;
            SelectBoxRes = nullptr;
            SubMenu = nullptr;
            submenuOptions(INITIALIZING_CALL);
            break;

        case SELECTBOX_800_600:
            global::s2->GameResolution.x = 800;
            global::s2->GameResolution.y = 600;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_832_624:
            global::s2->GameResolution.x = 832;
            global::s2->GameResolution.y = 624;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_960_540:
            global::s2->GameResolution.x = 960;
            global::s2->GameResolution.y = 540;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_964_544:
            global::s2->GameResolution.x = 964;
            global::s2->GameResolution.y = 544;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_960_640:
            global::s2->GameResolution.x = 960;
            global::s2->GameResolution.y = 640;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_960_720:
            global::s2->GameResolution.x = 960;
            global::s2->GameResolution.y = 720;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1024_576:
            global::s2->GameResolution.x = 1024;
            global::s2->GameResolution.y = 576;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1024_600:
            global::s2->GameResolution.x = 1024;
            global::s2->GameResolution.y = 600;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1072_600:
            global::s2->GameResolution.x = 1072;
            global::s2->GameResolution.y = 600;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1152_768:
            global::s2->GameResolution.x = 1152;
            global::s2->GameResolution.y = 768;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1024_768:
            global::s2->GameResolution.x = 1024;
            global::s2->GameResolution.y = 768;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1152_864:
            global::s2->GameResolution.x = 1152;
            global::s2->GameResolution.y = 864;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1152_870:
            global::s2->GameResolution.x = 1152;
            global::s2->GameResolution.y = 870;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1152_900:
            global::s2->GameResolution.x = 1152;
            global::s2->GameResolution.y = 900;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1200_800:
            global::s2->GameResolution.x = 1200;
            global::s2->GameResolution.y = 800;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1200_900:
            global::s2->GameResolution.x = 1200;
            global::s2->GameResolution.y = 900;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1280_720:
            global::s2->GameResolution.x = 1280;
            global::s2->GameResolution.y = 720;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1280_768:
            global::s2->GameResolution.x = 1280;
            global::s2->GameResolution.y = 768;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1280_800:
            global::s2->GameResolution.x = 1280;
            global::s2->GameResolution.y = 800;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1280_854:
            global::s2->GameResolution.x = 1280;
            global::s2->GameResolution.y = 854;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1360_768:
            global::s2->GameResolution.x = 1360;
            global::s2->GameResolution.y = 768;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1366_768:
            global::s2->GameResolution.x = 1366;
            global::s2->GameResolution.y = 768;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1376_768:
            global::s2->GameResolution.x = 1376;
            global::s2->GameResolution.y = 768;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1400_900:
            global::s2->GameResolution.x = 1400;
            global::s2->GameResolution.y = 900;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1440_900:
            global::s2->GameResolution.x = 1440;
            global::s2->GameResolution.y = 900;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1440_960:
            global::s2->GameResolution.x = 1440;
            global::s2->GameResolution.y = 960;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1280_960:
            global::s2->GameResolution.x = 1280;
            global::s2->GameResolution.y = 960;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1280_1024:
            global::s2->GameResolution.x = 1280;
            global::s2->GameResolution.y = 1024;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1360_1024:
            global::s2->GameResolution.x = 1360;
            global::s2->GameResolution.y = 1024;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1366_1024:
            global::s2->GameResolution.x = 1366;
            global::s2->GameResolution.y = 1024;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1600_768:
            global::s2->GameResolution.x = 1600;
            global::s2->GameResolution.y = 768;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1600_900:
            global::s2->GameResolution.x = 1600;
            global::s2->GameResolution.y = 900;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1600_1024:
            global::s2->GameResolution.x = 1600;
            global::s2->GameResolution.y = 1024;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1400_1050:
            global::s2->GameResolution.x = 1400;
            global::s2->GameResolution.y = 1050;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1680_1050:
            global::s2->GameResolution.x = 1680;
            global::s2->GameResolution.y = 1050;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1600_1200:
            global::s2->GameResolution.x = 1600;
            global::s2->GameResolution.y = 1200;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1920_1080:
            global::s2->GameResolution.x = 1920;
            global::s2->GameResolution.y = 1080;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1920_1200:
            global::s2->GameResolution.x = 1920;
            global::s2->GameResolution.y = 1200;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1920_1400:
            global::s2->GameResolution.x = 1920;
            global::s2->GameResolution.y = 1400;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_1920_1440:
            global::s2->GameResolution.x = 1920;
            global::s2->GameResolution.y = 1440;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_2048_1152:
            global::s2->GameResolution.x = 2048;
            global::s2->GameResolution.y = 1152;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        case SELECTBOX_2048_1536:
            global::s2->GameResolution.x = 2048;
            global::s2->GameResolution.y = 1536;
            submenuOptions(GRAPHICS_CHANGE);
            break;

        default: break;
    }
}

// now the editor callbacks will follow

void callback::EditorHelpMenu(int Param)
{
    static CWindow* WNDHelp;

    enum
    {
        WINDOWQUIT
    };

    CSelectBox* SelectBoxHelp;
    switch(Param)
    {
        case INITIALIZING_CALL:
            if(WNDHelp)
                break;
            WNDHelp = global::s2->RegisterWindow(std::make_unique<CWindow>(EditorHelpMenu, WINDOWQUIT, WindowPos::Center, Extent(640, 380),
                                                                           "Hilfe", WINDOW_GREEN2,
                                                                           WINDOW_CLOSE | WINDOW_MOVE | WINDOW_RESIZE | WINDOW_MINIMIZE));

            SelectBoxHelp = WNDHelp->addSelectBox(Point16(0, 0), Extent16(WNDHelp->getSize() - WNDHelp->getBorderSize()), FontSize::Medium,
                                                  FontColor::Yellow, BUTTON_GREEN1);
            SelectBoxHelp->setOption("User map path: " + global::userMapsPath);
            SelectBoxHelp->setOption("");
            SelectBoxHelp->setOption(
              "Help-Menu......................................................................................................F1");
            SelectBoxHelp->setOption(
              "Window/Fullscreen........................................................................................F2");
            SelectBoxHelp->setOption("Zoom in/normal/out "
                                     "(experimental)..............................................................F5/F6/"
                                     "F7");
            SelectBoxHelp->setOption("Scroll..........................................................................................."
                                     "..................Arrow keys");
            SelectBoxHelp->setOption(
              "Cursor size 1-9 (of 11)....................................................................................1-9");
            SelectBoxHelp->setOption(
              "Make Cursor bigger/smaller........................................................................+/-");
            SelectBoxHelp->setOption(
              "Scissors-Mode...............................................................................................Ctrl");
            SelectBoxHelp->setOption(
              "Invert mode....................................................................................................Shift");
            SelectBoxHelp->setOption("(e.g. Lower altitude, remove player, lower resources)");
            SelectBoxHelp->setOption(
              "Plane mode.....................................................................................................Alt");
            SelectBoxHelp->setOption("Reduce/default/enlarge maximum height.....................................................Ins/Pos1/"
                                     "PageUp");
            SelectBoxHelp->setOption("(can't increase beyond this)");
            SelectBoxHelp->setOption("Reduce/default/enlarge minimum "
                                     "height......................................................Del/End/"
                                     "PageDown");
            SelectBoxHelp->setOption("(can't decrease below this)");
            SelectBoxHelp->setOption("Undo............................................................................................."
                                     ".......................Q");
            SelectBoxHelp->setOption("Redo............................................................................................."
                                     ".................SHIFT+Q");
            SelectBoxHelp->setOption("(just actions made with the cursor)");
            SelectBoxHelp->setOption(
              "Build help on/off.............................................................................................Space");
            SelectBoxHelp->setOption(
              "Castle-Mode....................................................................................................B");
            SelectBoxHelp->setOption("(planes the surrounding terrain");
            SelectBoxHelp->setOption(" so a castle can be build)");
            SelectBoxHelp->setOption(
              "Harbour-Mode................................................................................................H");
            SelectBoxHelp->setOption("(changes the surrounding terrain,");
            SelectBoxHelp->setOption(" so that a harbour can be build)");
            SelectBoxHelp->setOption("Convert map \"on-the-fly\"  (Greenland/Winterworld/Wasteland).................G/W/O");
            SelectBoxHelp->setOption("New/Original shadows (experimental)..........................................................P");
            SelectBoxHelp->setOption("Lock/Unlock horizontal movement................................................................F9");
            SelectBoxHelp->setOption(
              "Lock/Unlock vertical movement....................................................................F10");
            SelectBoxHelp->setOption(
              "Turn borders on/off......................................................................................F11");

            break;

        case CALL_FROM_GAMELOOP: break;

        case WINDOW_QUIT_MESSAGE: // this is the global window quit message, callback is explicit called with this value, so destroy the
                                  // window
        case WINDOWQUIT:          // this is the own window quit message of the callback
        case MAP_QUIT: // this is the global window quit message, callback is explicit called with this value, so destroy the window
            if(WNDHelp)
            {
                WNDHelp->setWaste();
                WNDHelp = nullptr;
                SelectBoxHelp = nullptr;
            }
            break;

        default: break;
    }
}

void callback::EditorMainMenu(int Param)
{
    static CWindow* WNDMain = nullptr;

    enum
    {
        LOADMENU,
        SAVEMENU,
        QUITMENU,
        WINDOWQUIT
    };

    switch(Param)
    {
        case INITIALIZING_CALL:
            if(WNDMain)
                break;
            WNDMain = global::s2->RegisterWindow(std::make_unique<CWindow>(EditorMainMenu, WINDOWQUIT, WindowPos::Center, Extent(220, 320),
                                                                           "Main menu", WINDOW_GREEN1, WINDOW_CLOSE));
            WNDMain->addButton(EditorMainMenu, LOADMENU, 8, 100, 190, 20, BUTTON_GREEN2, "Load map");
            WNDMain->addButton(EditorMainMenu, SAVEMENU, 8, 125, 190, 20, BUTTON_GREEN2, "Save map");

            WNDMain->addButton(EditorMainMenu, QUITMENU, 8, 260, 190, 20, BUTTON_GREEN2, "Leave editor");
            break;

        case WINDOWQUIT:
        case MAP_QUIT:
            if(WNDMain)
            {
                WNDMain->setWaste();
                WNDMain = nullptr;
            }
            break;

        case QUITMENU: EditorQuitMenu(INITIALIZING_CALL); break;

        case LOADMENU: EditorLoadMenu(INITIALIZING_CALL); break;

        case SAVEMENU: EditorSaveMenu(INITIALIZING_CALL); break;

        default: break;
    }
}

void callback::EditorLoadMenu(int Param)
{
    static CWindow* WNDLoad = nullptr;
    static CMap* MapObj = nullptr;
    static std::string curFilename;

    enum
    {
        LOADMAP,
        WINDOWQUIT
    };

    switch(Param)
    {
        case INITIALIZING_CALL:
        {
            if(WNDLoad)
                break;
            WNDLoad = global::s2->RegisterWindow(std::make_unique<CWindow>(EditorLoadMenu, WINDOWQUIT, WindowPos::Center, Extent(280, 320),
                                                                           "Load", WINDOW_GREEN1, WINDOW_CLOSE));
            MapObj = global::s2->getMapObj();

            auto* CB_Filename = WNDLoad->addSelectBox(Point16(10, 5), Extent16(160, 280), FontSize::Medium);
            curFilename.clear();
            for(const auto& itFile : bfs::directory_iterator(global::userMapsPath))
            {
                if(is_regular_file(itFile.status()))
                {
                    const std::string filename = itFile.path().filename().string();
                    CB_Filename->setOption(filename, [filename](int) { curFilename = filename; });
                }
            }
            WNDLoad->addButton(EditorLoadMenu, LOADMAP, 175, 140, 90, 20, BUTTON_GREY, "Load");
            WNDLoad->addButton(EditorLoadMenu, WINDOWQUIT, 175, 165, 90, 20, BUTTON_RED1, "Abort");
            break;
        }
        case WINDOWQUIT:
        case MAP_QUIT:
            if(WNDLoad)
            {
                WNDLoad->setWaste();
                WNDLoad = nullptr;
            }
            break;

        case LOADMAP:
        {
            if(curFilename.empty())
                return;
            PleaseWait(INITIALIZING_CALL);

            // we have to close the windows and initialize them again to prevent failures
            EditorCursorMenu(MAP_QUIT);
            EditorTextureMenu(MAP_QUIT);
            EditorTreeMenu(MAP_QUIT);
            EditorLandscapeMenu(MAP_QUIT);
            MinimapMenu(MAP_QUIT);
            EditorResourceMenu(MAP_QUIT);
            EditorAnimalMenu(MAP_QUIT);
            EditorPlayerMenu(MAP_QUIT);

            MapObj->destructMap();
            bfs::path filepath = bfs::path(global::userMapsPath) / curFilename;
            if(!filepath.has_extension())
                filepath.replace_extension("SWD");
            if(!bfs::exists(filepath))
                filepath.replace_extension("WLD");
            if(!bfs::exists(filepath))
                filepath.replace_extension("SWD");
            MapObj->constructMap(filepath.string());

            // we need to check which of these windows was active before
            /*
            EditorCursorMenu(INITIALIZING_CALL);
            EditorTextureMenu(INITIALIZING_CALL);
            EditorTreeMenu(INITIALIZING_CALL);
            EditorLandscapeMenu(INITIALIZING_CALL);
            MinimapMenu(INITIALIZING_CALL);
            EditorResourceMenu(INITIALIZING_CALL);
            EditorAnimalMenu(INITIALIZING_CALL);
            EditorPlayerMenu(INITIALIZING_CALL);
            */

            PleaseWait(WINDOW_QUIT_MESSAGE);
            EditorLoadMenu(WINDOWQUIT);
            break;
        }

        default: break;
    }
}

void callback::EditorSaveMenu(int Param)
{
    static CWindow* WNDSave = nullptr;
    static CTextfield* TXTF_Filename = nullptr;
    static CTextfield* TXTF_Mapname = nullptr;
    static CTextfield* TXTF_Author = nullptr;
    static CMap* MapObj = nullptr;

    enum
    {
        SAVEMAP,
        WINDOWQUIT
    };

    switch(Param)
    {
        case INITIALIZING_CALL:
            if(WNDSave)
                break;
            {
                WNDSave = global::s2->RegisterWindow(std::make_unique<CWindow>(EditorSaveMenu, WINDOWQUIT, WindowPos::Center,
                                                                               Extent(280, 200), "Save", WINDOW_GREEN1, WINDOW_CLOSE));
                MapObj = global::s2->getMapObj();

                WNDSave->addText("Filename", 100, 2, FontSize::Small);
                TXTF_Filename = WNDSave->addTextfield(10, 13, 21, 1);
                auto const filePath = MapObj->getFilename().empty() ? "MyMap" : MapObj->getFilename();
                TXTF_Filename->setText(bfs::path(filePath).filename().string());
                WNDSave->addText("Mapname", 98, 38, FontSize::Small);
                TXTF_Mapname = WNDSave->addTextfield(10, 50, 19, 1);
                TXTF_Mapname->setText(MapObj->getMapname());
                WNDSave->addText("Author", 110, 75, FontSize::Medium);
                TXTF_Author = WNDSave->addTextfield(10, 87, 19, 1);
                TXTF_Author->setText(MapObj->getAuthor());
                WNDSave->addButton(EditorSaveMenu, SAVEMAP, 170, 120, 90, 20, BUTTON_GREY, "Save");
                WNDSave->addButton(EditorSaveMenu, WINDOWQUIT, 170, 145, 90, 20, BUTTON_RED1, "Abort");
                break;
            }
        case WINDOWQUIT:
        case MAP_QUIT:
            if(WNDSave)
            {
                WNDSave->setWaste();
                WNDSave = nullptr;
            }
            TXTF_Filename = nullptr;
            TXTF_Mapname = nullptr;
            TXTF_Author = nullptr;
            break;

        case SAVEMAP:
        {
            PleaseWait(INITIALIZING_CALL);

            MapObj->setMapname(TXTF_Mapname->getText());
            MapObj->setAuthor(TXTF_Author->getText());
            MapObj->setFilename(TXTF_Filename->getText());
            bfs::path filepath = bfs::path(global::userMapsPath) / TXTF_Filename->getText();
            if(!filepath.has_extension())
                filepath.replace_extension("SWD");
            bool result = CFile::save_file(filepath.string(), WLD, MapObj->getMap());

            ShowStatus(INITIALIZING_CALL);
            ShowStatus(result ? 1 : 2);

            PleaseWait(WINDOW_QUIT_MESSAGE);
            EditorSaveMenu(WINDOWQUIT);
            break;
        }

        default: break;
    }
}

void callback::EditorQuitMenu(int Param)
{
    static CWindow* WNDBackToMainMenu = nullptr;

    enum
    {
        BACKTOMAIN = 1,
        NOTBACKTOMAIN,
        WINDOWQUIT
    };

    switch(Param)
    {
        case INITIALIZING_CALL:
            if(WNDBackToMainMenu)
                break;
            WNDBackToMainMenu = global::s2->RegisterWindow(
              std::make_unique<CWindow>(EditorQuitMenu, WINDOWQUIT, WindowPos::Center, Extent(212, 110), "Exit?"));
            WNDBackToMainMenu->addButton(EditorQuitMenu, BACKTOMAIN, 0, 0, 100, 80, BUTTON_GREEN2, nullptr, PICTURE_SMALL_TICK);
            WNDBackToMainMenu->addButton(EditorQuitMenu, NOTBACKTOMAIN, 100, 0, 100, 80, BUTTON_RED1, nullptr, PICTURE_SMALL_CROSS);
            break;

        case BACKTOMAIN:
            if(global::s2->getMapObj())
                global::s2->delMapObj();
            WNDBackToMainMenu->setWaste();
            WNDBackToMainMenu = nullptr;
            // now call all EditorMenu callbacks (from the menubar) with MAP_QUIT
            EditorHelpMenu(MAP_QUIT);
            EditorMainMenu(MAP_QUIT);
            EditorLoadMenu(MAP_QUIT);
            EditorSaveMenu(MAP_QUIT);
            EditorTextureMenu(MAP_QUIT);
            EditorTreeMenu(MAP_QUIT);
            EditorLandscapeMenu(MAP_QUIT);
            MinimapMenu(MAP_QUIT);
            EditorCursorMenu(MAP_QUIT);
            EditorResourceMenu(MAP_QUIT);
            EditorAnimalMenu(MAP_QUIT);
            EditorPlayerMenu(MAP_QUIT);
            EditorCreateMenu(MAP_QUIT);
            // go to main menu
            mainmenu(INITIALIZING_CALL);
            break;

        case NOTBACKTOMAIN:
            if(WNDBackToMainMenu)
            {
                WNDBackToMainMenu->setWaste();
                WNDBackToMainMenu = nullptr;
            }
            break;

        default: break;
    }
}

void callback::EditorTextureMenu(int Param)
{
    static CWindow* WNDTexture = nullptr;
    static CMap* MapObj = nullptr;
    static bobMAP* map = nullptr;
    static int textureIndex = 0;
    static int lastContent = 0x00;
    static Position Pos{0, 0};

    enum
    {
        WINDOWQUIT,
        PICSNOW,
        PICSTEPPE,
        PICSWAMP,
        PICFLOWER,
        PICMINING1,
        PICMINING2,
        PICMINING3,
        PICMINING4,
        PICSTEPPE_MEADOW1,
        PICMEADOW1,
        PICMEADOW2,
        PICMEADOW3,
        PICSTEPPE_MEADOW2,
        PICMINING_MEADOW,
        PICWATER,
        PICLAVA,
        PICMEADOW_MIXED
    };

    if(Param != INITIALIZING_CALL && Param != MAP_QUIT)
    {
        assert(MapObj);
        assert(map);
    }

    switch(Param)
    {
        case INITIALIZING_CALL:
            if(WNDTexture)
                break;
            WNDTexture =
              global::s2->RegisterWindow(std::make_unique<CWindow>(EditorTextureMenu, WINDOWQUIT, Pos, Extent(220, 133), "Terrain",
                                                                   WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MINIMIZE | WINDOW_MOVE));
            MapObj = global::s2->getMapObj();
            map = MapObj->getMap();
            switch(map->type)
            {
                case MAP_GREENLAND: textureIndex = PICTURE_GREENLAND_TEXTURE_SNOW; break;
                case MAP_WASTELAND: textureIndex = PICTURE_WASTELAND_TEXTURE_SNOW; break;
                case MAP_WINTERLAND: textureIndex = PICTURE_WINTERLAND_TEXTURE_SNOW; break;
                default: textureIndex = PICTURE_GREENLAND_TEXTURE_SNOW; break;
            }
            MapObj->setMode(EDITOR_MODE_TEXTURE);
            MapObj->setModeContent(TRIANGLE_TEXTURE_SNOW);

            WNDTexture->addPicture(EditorTextureMenu, PICSNOW, 2, 2, textureIndex++);
            WNDTexture->addPicture(EditorTextureMenu, PICSTEPPE, 36, 2, textureIndex++);
            WNDTexture->addPicture(EditorTextureMenu, PICSWAMP, 70, 2, textureIndex++);
            WNDTexture->addPicture(EditorTextureMenu, PICFLOWER, 104, 2, textureIndex++);
            WNDTexture->addPicture(EditorTextureMenu, PICMINING1, 138, 2, textureIndex++);
            WNDTexture->addPicture(EditorTextureMenu, PICMINING2, 172, 2, textureIndex++);
            WNDTexture->addPicture(EditorTextureMenu, PICMINING3, 2, 36, textureIndex++);
            WNDTexture->addPicture(EditorTextureMenu, PICMINING4, 36, 36, textureIndex++);
            WNDTexture->addPicture(EditorTextureMenu, PICSTEPPE_MEADOW1, 70, 36, textureIndex++);
            WNDTexture->addPicture(EditorTextureMenu, PICMEADOW1, 104, 36, textureIndex++);
            WNDTexture->addPicture(EditorTextureMenu, PICMEADOW2, 138, 36, textureIndex++);
            WNDTexture->addPicture(EditorTextureMenu, PICMEADOW3, 172, 36, textureIndex++);
            WNDTexture->addPicture(EditorTextureMenu, PICSTEPPE_MEADOW2, 2, 70, textureIndex++);
            WNDTexture->addPicture(EditorTextureMenu, PICMINING_MEADOW, 36, 70, textureIndex++);
            WNDTexture->addPicture(EditorTextureMenu, PICWATER, 70, 70, textureIndex++);
            WNDTexture->addPicture(EditorTextureMenu, PICLAVA, 104, 70, textureIndex++);
            if(map->type != MAP_WASTELAND)
                WNDTexture->addPicture(EditorTextureMenu, PICMEADOW_MIXED, 138, 70, textureIndex);
            break;

        case PICSNOW: MapObj->setModeContent(TRIANGLE_TEXTURE_SNOW); break;
        case PICSTEPPE: MapObj->setModeContent(TRIANGLE_TEXTURE_STEPPE); break;
        case PICSWAMP: MapObj->setModeContent(TRIANGLE_TEXTURE_SWAMP); break;
        case PICFLOWER: MapObj->setModeContent(TRIANGLE_TEXTURE_FLOWER); break;
        case PICMINING1: MapObj->setModeContent(TRIANGLE_TEXTURE_MINING1); break;
        case PICMINING2: MapObj->setModeContent(TRIANGLE_TEXTURE_MINING2); break;
        case PICMINING3: MapObj->setModeContent(TRIANGLE_TEXTURE_MINING3); break;
        case PICMINING4: MapObj->setModeContent(TRIANGLE_TEXTURE_MINING4); break;
        case PICSTEPPE_MEADOW1: MapObj->setModeContent(TRIANGLE_TEXTURE_STEPPE_MEADOW1); break;
        case PICMEADOW1: MapObj->setModeContent(TRIANGLE_TEXTURE_MEADOW1); break;
        case PICMEADOW2: MapObj->setModeContent(TRIANGLE_TEXTURE_MEADOW2); break;
        case PICMEADOW3: MapObj->setModeContent(TRIANGLE_TEXTURE_MEADOW3); break;
        case PICSTEPPE_MEADOW2: MapObj->setModeContent(TRIANGLE_TEXTURE_STEPPE_MEADOW2); break;
        case PICMINING_MEADOW: MapObj->setModeContent(TRIANGLE_TEXTURE_MINING_MEADOW); break;
        case PICWATER: MapObj->setModeContent(TRIANGLE_TEXTURE_WATER); break;
        case PICLAVA: MapObj->setModeContent(TRIANGLE_TEXTURE_LAVA); break;
        case PICMEADOW_MIXED: MapObj->setModeContent(TRIANGLE_TEXTURE_MEADOW_MIXED); break;

        case WINDOW_CLICKED_CALL:
            if(MapObj)
            {
                MapObj->setMode(EDITOR_MODE_TEXTURE);
                MapObj->setModeContent(lastContent);
            }
            break;

        case WINDOWQUIT:
            if(WNDTexture)
            {
                Pos = WNDTexture->getPos();
                WNDTexture->setWaste();
                WNDTexture = nullptr;
            }
            MapObj->setMode(EDITOR_MODE_HEIGHT_RAISE);
            MapObj->setModeContent(0x00);
            lastContent = 0x00;
            MapObj = nullptr;
            map = nullptr;
            textureIndex = 0;
            break;

        case MAP_QUIT:
            // we do the same like in case WINDOWQUIT, but we won't setMode(EDITOR_MODE_HEIGHT_RAISE), cause map is dead
            if(WNDTexture)
            {
                Pos = WNDTexture->getPos();
                WNDTexture->setWaste();
                WNDTexture = nullptr;
            }
            lastContent = 0x00;
            MapObj = nullptr;
            map = nullptr;
            textureIndex = 0;
            break;

        default: break;
    }
    if(MapObj)
        lastContent = MapObj->getModeContent();
}

void callback::EditorTreeMenu(int Param)
{
    static CWindow* WNDTree = nullptr;
    static CMap* MapObj = nullptr;
    static bobMAP* map = nullptr;
    static int lastContent = 0x00;
    static int lastContent2 = 0x00;
    static Position Pos{230, 0};

    enum
    {
        WINDOWQUIT,
        PICPINE,
        PICBIRCH,
        PICOAK,
        PICPALM1,
        PICPALM2,
        PICPINEAPPLE,
        PICCYPRESS,
        PICCHERRY,
        PICFIR,
        PICSPIDER,
        PICFLAPHAT,
        PICWOOD_MIXED,
        PICPALM_MIXED
    };

    if(Param != INITIALIZING_CALL && Param != MAP_QUIT)
        assert(WNDTree && MapObj && map);

    switch(Param)
    {
        case INITIALIZING_CALL:
            if(WNDTree)
                break;
            WNDTree = global::s2->RegisterWindow(std::make_unique<CWindow>(EditorTreeMenu, WINDOWQUIT, Pos, Extent(148, 140), "Trees",
                                                                           WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MINIMIZE | WINDOW_MOVE));
            MapObj = global::s2->getMapObj();
            map = MapObj->getMap();
            switch(map->type)
            {
                case MAP_GREENLAND:
                    WNDTree->addPicture(EditorTreeMenu, PICPINE, 2, 2, PICTURE_TREE_PINE);
                    WNDTree->addPicture(EditorTreeMenu, PICBIRCH, 36, 2, PICTURE_TREE_BIRCH);
                    WNDTree->addPicture(EditorTreeMenu, PICOAK, 70, 2, PICTURE_TREE_OAK);
                    WNDTree->addPicture(EditorTreeMenu, PICPALM1, 104, 2, PICTURE_TREE_PALM1);
                    WNDTree->addPicture(EditorTreeMenu, PICPALM2, 2, 36, PICTURE_TREE_PALM2);
                    WNDTree->addPicture(EditorTreeMenu, PICPINEAPPLE, 36, 36, PICTURE_TREE_PINEAPPLE);
                    WNDTree->addPicture(EditorTreeMenu, PICCYPRESS, 70, 36, PICTURE_TREE_CYPRESS);
                    WNDTree->addPicture(EditorTreeMenu, PICCHERRY, 104, 36, PICTURE_TREE_CHERRY);
                    WNDTree->addPicture(EditorTreeMenu, PICFIR, 2, 72, PICTURE_TREE_FIR);
                    WNDTree->addPicture(EditorTreeMenu, PICWOOD_MIXED, 36, 70, PICTURE_TREE_WOOD_MIXED);
                    WNDTree->addPicture(EditorTreeMenu, PICPALM_MIXED, 70, 70, PICTURE_TREE_PALM_MIXED);
                    break;
                case MAP_WASTELAND:
                    WNDTree->addPicture(EditorTreeMenu, PICFLAPHAT, 2, 2, PICTURE_TREE_FLAPHAT);
                    WNDTree->addPicture(EditorTreeMenu, PICSPIDER, 36, 2, PICTURE_TREE_SPIDER);
                    WNDTree->addPicture(EditorTreeMenu, PICPINEAPPLE, 70, 2, PICTURE_TREE_PINEAPPLE);
                    WNDTree->addPicture(EditorTreeMenu, PICCHERRY, 104, 2, PICTURE_TREE_CHERRY);
                    break;
                case MAP_WINTERLAND:
                    WNDTree->addPicture(EditorTreeMenu, PICPINE, 2, 2, PICTURE_TREE_PINE);
                    WNDTree->addPicture(EditorTreeMenu, PICBIRCH, 36, 2, PICTURE_TREE_BIRCH);
                    WNDTree->addPicture(EditorTreeMenu, PICCYPRESS, 70, 2, PICTURE_TREE_CYPRESS);
                    WNDTree->addPicture(EditorTreeMenu, PICFIR, 104, 2, PICTURE_TREE_FIR);
                    WNDTree->addPicture(EditorTreeMenu, PICWOOD_MIXED, 2, 36, PICTURE_TREE_WOOD_MIXED);
                    break;
                default: // should not happen
                    break;
            }
            MapObj->setMode(EDITOR_MODE_TREE);
            MapObj->setModeContent(0x30);
            MapObj->setModeContent2(0xC4);
            lastContent = 0x30;
            lastContent2 = 0xC4;
            break;

        case PICPINE:
            MapObj->setModeContent(0x30);
            MapObj->setModeContent2(0xC4);
            lastContent = 0x30;
            lastContent2 = 0xC4;
            break;
        case PICBIRCH:
            MapObj->setModeContent(0x70);
            MapObj->setModeContent2(0xC4);
            lastContent = 0x70;
            lastContent2 = 0xC4;
            break;
        case PICOAK:
            MapObj->setModeContent(0xB0);
            MapObj->setModeContent2(0xC4);
            lastContent = 0xB0;
            lastContent2 = 0xC4;
            break;
        case PICPALM1:
            MapObj->setModeContent(0xF0);
            MapObj->setModeContent2(0xC4);
            lastContent = 0xF0;
            lastContent2 = 0xC4;
            break;
        case PICPALM2:
            MapObj->setModeContent(0x30);
            MapObj->setModeContent2(0xC5);
            lastContent = 0x30;
            lastContent2 = 0xC5;
            break;
        case PICPINEAPPLE:
            MapObj->setModeContent(0x70);
            MapObj->setModeContent2(0xC5);
            lastContent = 0x70;
            lastContent2 = 0xC5;
            break;
        case PICCYPRESS:
            MapObj->setModeContent(0xB0);
            MapObj->setModeContent2(0xC5);
            lastContent = 0xB0;
            lastContent2 = 0xC5;
            break;
        case PICCHERRY:
            MapObj->setModeContent(0xF0);
            MapObj->setModeContent2(0xC5);
            lastContent = 0xF0;
            lastContent2 = 0xC5;
            break;
        case PICFIR:
            MapObj->setModeContent(0x30);
            MapObj->setModeContent2(0xC6);
            lastContent = 0x30;
            lastContent2 = 0xC6;
            break;
        case PICFLAPHAT:
            MapObj->setModeContent(0x70);
            MapObj->setModeContent2(0xC4);
            lastContent = 0x70;
            lastContent2 = 0xC4;
            break;
        case PICSPIDER:
            MapObj->setModeContent(0x30);
            MapObj->setModeContent2(0xC4);
            lastContent = 0x30;
            lastContent2 = 0xC4;
            break;
        case PICWOOD_MIXED:
            MapObj->setModeContent(0xFF);
            MapObj->setModeContent2(0xC4);
            lastContent = 0xFF;
            lastContent2 = 0xC4;
            break;
        case PICPALM_MIXED:
            MapObj->setModeContent(0xFF);
            MapObj->setModeContent2(0xC5);
            lastContent = 0xFF;
            lastContent2 = 0xC5;
            break;

        case WINDOW_CLICKED_CALL:
            if(MapObj)
            {
                MapObj->setMode(EDITOR_MODE_TREE);
                MapObj->setModeContent(lastContent);
                MapObj->setModeContent2(lastContent2);
            }
            break;

        case WINDOWQUIT:
            if(WNDTree)
            {
                Pos = WNDTree->getPos();
                WNDTree->setWaste();
                WNDTree = nullptr;
            }
            MapObj->setMode(EDITOR_MODE_HEIGHT_RAISE);
            MapObj->setModeContent(0x00);
            MapObj->setModeContent2(0x00);
            lastContent = 0x00;
            lastContent2 = 0x00;
            MapObj = nullptr;
            map = nullptr;
            break;

        case MAP_QUIT:
            // we do the same like in case WINDOWQUIT, but we won't setMode(EDITOR_MODE_HEIGHT_RAISE), cause map is dead
            if(WNDTree)
            {
                Pos = WNDTree->getPos();
                WNDTree->setWaste();
                WNDTree = nullptr;
            }
            lastContent = 0x00;
            lastContent2 = 0x00;
            MapObj = nullptr;
            map = nullptr;
            break;

        default: break;
    }
}

void callback::EditorResourceMenu(int Param)
{
    static CWindow* WNDResource = nullptr;
    static CMap* MapObj = nullptr;
    static int lastContent = 0x00;
    static Position Pos{0, 140};

    enum
    {
        WINDOWQUIT,
        PICGOLD,
        PICORE,
        PICCOAL,
        PICGRANITE
    };

    if(Param != INITIALIZING_CALL && Param != MAP_QUIT)
        assert(WNDResource && MapObj);

    switch(Param)
    {
        case INITIALIZING_CALL:
            if(WNDResource)
                break;
            WNDResource =
              global::s2->RegisterWindow(std::make_unique<CWindow>(EditorResourceMenu, WINDOWQUIT, Pos, Extent(148, 55), "Resources",
                                                                   WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MINIMIZE | WINDOW_MOVE));
            MapObj = global::s2->getMapObj();

            WNDResource->addPicture(EditorResourceMenu, PICGOLD, 2, 2, PICTURE_RESOURCE_GOLD);
            WNDResource->addPicture(EditorResourceMenu, PICORE, 36, 2, PICTURE_RESOURCE_ORE);
            WNDResource->addPicture(EditorResourceMenu, PICCOAL, 70, 2, PICTURE_RESOURCE_COAL);
            WNDResource->addPicture(EditorResourceMenu, PICGRANITE, 104, 2, PICTURE_RESOURCE_GRANITE);

            MapObj->setMode(EDITOR_MODE_RESOURCE_RAISE);
            MapObj->setModeContent(0x51);
            lastContent = 0x51;
            break;

        case PICGOLD:
            MapObj->setModeContent(0x51);
            lastContent = 0x51;
            break;
        case PICORE:
            MapObj->setModeContent(0x49);
            lastContent = 0x49;
            break;
        case PICCOAL:
            MapObj->setModeContent(0x41);
            lastContent = 0x41;
            break;
        case PICGRANITE:
            MapObj->setModeContent(0x59);
            lastContent = 0x59;
            break;

        case WINDOW_CLICKED_CALL:
            if(MapObj)
            {
                MapObj->setMode(EDITOR_MODE_RESOURCE_RAISE);
                MapObj->setModeContent(lastContent);
            }
            break;

        case WINDOWQUIT:
            if(WNDResource)
            {
                Pos = WNDResource->getPos();
                WNDResource->setWaste();
                WNDResource = nullptr;
            }
            MapObj->setMode(EDITOR_MODE_HEIGHT_RAISE);
            MapObj->setModeContent(0x00);
            lastContent = 0x00;
            MapObj = nullptr;
            break;

        case MAP_QUIT:
            // we do the same like in case WINDOWQUIT, but we won't setMode(EDITOR_MODE_HEIGHT_RAISE), cause map is dead
            if(WNDResource)
            {
                Pos = WNDResource->getPos();
                WNDResource->setWaste();
                WNDResource = nullptr;
            }
            lastContent = 0x00;
            MapObj = nullptr;
            break;

        default: break;
    }
}

void callback::EditorLandscapeMenu(int Param)
{
    static CWindow* WNDLandscape = nullptr;
    static CMap* MapObj = nullptr;
    static bobMAP* map = nullptr;
    static int lastContent = 0x00;
    static int lastContent2 = 0x00;
    static Position Pos{390, 0};

    enum
    {
        WINDOWQUIT,
        PICGRANITE,
        PICTREEDEAD,
        PICSTONE,
        PICCACTUS,
        PICPEBBLE,
        PICBUSH,
        PICSHRUB,
        PICBONE,
        PICMUSHROOM,
        PICSTALAGMITE,
        PICFLOWERS
    };

    if(Param != INITIALIZING_CALL && Param != MAP_QUIT)
        assert(WNDLandscape && MapObj && map);

    switch(Param)
    {
        case INITIALIZING_CALL:
            if(WNDLandscape)
                break;
            WNDLandscape =
              global::s2->RegisterWindow(std::make_unique<CWindow>(EditorLandscapeMenu, WINDOWQUIT, Pos, Extent(112, 174), "Landscape",
                                                                   WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MINIMIZE | WINDOW_MOVE));
            MapObj = global::s2->getMapObj();
            map = MapObj->getMap();
            switch(map->type)
            {
                case MAP_GREENLAND:
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICGRANITE, 2, 2, PICTURE_LANDSCAPE_GRANITE);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICTREEDEAD, 36, 2, PICTURE_LANDSCAPE_TREE_DEAD);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICSTONE, 70, 2, PICTURE_LANDSCAPE_STONE);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICCACTUS, 2, 36, PICTURE_LANDSCAPE_CACTUS);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICPEBBLE, 36, 36, PICTURE_LANDSCAPE_PEBBLE);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICBUSH, 70, 36, PICTURE_LANDSCAPE_BUSH);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICSHRUB, 2, 70, PICTURE_LANDSCAPE_SHRUB);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICBONE, 36, 70, PICTURE_LANDSCAPE_BONE);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICMUSHROOM, 70, 70, PICTURE_LANDSCAPE_MUSHROOM);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICFLOWERS, 5, 107, MAPPIC_FLOWERS);
                    break;
                case MAP_WASTELAND:
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICGRANITE, 2, 2, PICTURE_LANDSCAPE_GRANITE);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICTREEDEAD, 36, 2, PICTURE_LANDSCAPE_TREE_DEAD);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICSTONE, 70, 2, PICTURE_LANDSCAPE_STONE);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICSTALAGMITE, 2, 36, PICTURE_LANDSCAPE_STALAGMITE);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICPEBBLE, 36, 36, PICTURE_LANDSCAPE_PEBBLE);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICBUSH, 70, 36, PICTURE_LANDSCAPE_BUSH);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICSHRUB, 2, 70, PICTURE_LANDSCAPE_SHRUB);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICBONE, 36, 70, PICTURE_LANDSCAPE_BONE);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICMUSHROOM, 70, 70, PICTURE_LANDSCAPE_MUSHROOM);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICFLOWERS, 5, 107, MAPPIC_FLOWERS);
                    break;
                case MAP_WINTERLAND:
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICGRANITE, 2, 2, PICTURE_LANDSCAPE_GRANITE_WINTER);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICTREEDEAD, 36, 2, PICTURE_LANDSCAPE_TREE_DEAD_WINTER);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICSTONE, 70, 2, PICTURE_LANDSCAPE_STONE_WINTER);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICPEBBLE, 2, 36, PICTURE_LANDSCAPE_PEBBLE_WINTER);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICBONE, 36, 36, PICTURE_LANDSCAPE_BONE_WINTER);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICMUSHROOM, 70, 36, PICTURE_LANDSCAPE_MUSHROOM_WINTER);
                    WNDLandscape->addPicture(EditorLandscapeMenu, PICFLOWERS, 73, 73, MAPPIC_FLOWERS);
                    break;
                default: // should not happen
                    break;
            }
            MapObj->setMode(EDITOR_MODE_LANDSCAPE);
            MapObj->setModeContent(0x01);
            MapObj->setModeContent2(0xCC);
            lastContent = 0x01;
            lastContent2 = 0xCC;
            break;

        case PICGRANITE:
            MapObj->setModeContent(0x01);
            MapObj->setModeContent2(0xCC);
            lastContent = 0x01;
            lastContent2 = 0xCC;
            break;
        case PICTREEDEAD:
            MapObj->setModeContent(0x05);
            MapObj->setModeContent2(0xC8);
            lastContent = 0x05;
            lastContent2 = 0xC8;
            break;
        case PICSTONE:
            MapObj->setModeContent(0x02);
            MapObj->setModeContent2(0xC8);
            lastContent = 0x02;
            lastContent2 = 0xC8;
            break;
        case PICCACTUS:
            MapObj->setModeContent(0x0C);
            MapObj->setModeContent2(0xC8);
            lastContent = 0x0C;
            lastContent2 = 0xC8;
            break;
        case PICPEBBLE:
            MapObj->setModeContent(0x25);
            MapObj->setModeContent2(0xC8);
            lastContent = 0x25;
            lastContent2 = 0xC8;
            break;
        case PICBUSH:
            MapObj->setModeContent(0x10);
            MapObj->setModeContent2(0xC8);
            lastContent = 0x10;
            lastContent2 = 0xC8;
            break;
        case PICSHRUB:
            MapObj->setModeContent(0x0E);
            MapObj->setModeContent2(0xC8);
            lastContent = 0x0E;
            lastContent2 = 0xC8;
            break;
        case PICBONE:
            MapObj->setModeContent(0x07);
            MapObj->setModeContent2(0xC8);
            lastContent = 0x07;
            lastContent2 = 0xC8;
            break;
        case PICMUSHROOM:
            MapObj->setModeContent(0x00);
            MapObj->setModeContent2(0xC8);
            lastContent = 0x00;
            lastContent2 = 0xC8;
            break;
        case PICSTALAGMITE:
            MapObj->setModeContent(0x18);
            MapObj->setModeContent2(0xC8);
            lastContent = 0x18;
            lastContent2 = 0xC8;
            break;
        case PICFLOWERS:
            MapObj->setModeContent(0x09);
            MapObj->setModeContent2(0xC8);
            lastContent = 0x09;
            lastContent2 = 0xC8;
            break;

        case WINDOW_CLICKED_CALL:
            if(MapObj)
            {
                MapObj->setMode(EDITOR_MODE_LANDSCAPE);
                MapObj->setModeContent(lastContent);
                MapObj->setModeContent2(lastContent2);
            }
            break;

        case WINDOWQUIT:
            if(WNDLandscape)
            {
                Pos = WNDLandscape->getPos();
                WNDLandscape->setWaste();
                WNDLandscape = nullptr;
            }
            MapObj->setMode(EDITOR_MODE_HEIGHT_RAISE);
            MapObj->setModeContent(0x00);
            MapObj->setModeContent2(0x00);
            lastContent = 0x00;
            lastContent2 = 0x00;
            MapObj = nullptr;
            map = nullptr;
            break;

        case MAP_QUIT:
            // we do the same like in case WINDOWQUIT, but we won't setMode(EDITOR_MODE_HEIGHT_RAISE), cause map is dead
            if(WNDLandscape)
            {
                Pos = WNDLandscape->getPos();
                WNDLandscape->setWaste();
                WNDLandscape = nullptr;
            }
            lastContent = 0x00;
            lastContent2 = 0x00;
            MapObj = nullptr;
            map = nullptr;
            break;

        default: break;
    }
}

void callback::EditorAnimalMenu(int Param)
{
    static CWindow* WNDAnimal = nullptr;
    static CMap* MapObj = nullptr;
    static int lastContent = 0x00;
    static Position Pos{510, 0};

    enum
    {
        WINDOWQUIT,
        PICRABBIT,
        PICFOX,
        PICSTAG,
        PICROE,
        PICDUCK,
        PICSHEEP
    };

    if(Param != INITIALIZING_CALL && Param != MAP_QUIT)
        assert(WNDAnimal && MapObj);

    switch(Param)
    {
        case INITIALIZING_CALL:
            if(WNDAnimal)
                break;
            WNDAnimal = global::s2->RegisterWindow(std::make_unique<CWindow>(EditorAnimalMenu, WINDOWQUIT, Pos, Extent(116, 106), "Animals",
                                                                             WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MINIMIZE | WINDOW_MOVE));
            WNDAnimal->addPicture(EditorAnimalMenu, PICRABBIT, 2, 2, PICTURE_ANIMAL_RABBIT);
            WNDAnimal->addPicture(EditorAnimalMenu, PICFOX, 36, 2, PICTURE_ANIMAL_FOX);
            WNDAnimal->addPicture(EditorAnimalMenu, PICSTAG, 70, 2, PICTURE_ANIMAL_STAG);
            WNDAnimal->addPicture(EditorAnimalMenu, PICROE, 2, 36, PICTURE_ANIMAL_ROE);
            WNDAnimal->addPicture(EditorAnimalMenu, PICDUCK, 36, 36, PICTURE_ANIMAL_DUCK);
            WNDAnimal->addPicture(EditorAnimalMenu, PICSHEEP, 70, 36, PICTURE_ANIMAL_SHEEP);

            MapObj = global::s2->getMapObj();
            MapObj->setMode(EDITOR_MODE_ANIMAL);
            MapObj->setModeContent(0x01);
            lastContent = 0x01;
            break;

        case PICRABBIT:
            MapObj->setModeContent(0x01);
            lastContent = 0x01;
            break;
        case PICFOX:
            MapObj->setModeContent(0x02);
            lastContent = 0x02;
            break;
        case PICSTAG:
            MapObj->setModeContent(0x03);
            lastContent = 0x03;
            break;
        case PICROE:
            MapObj->setModeContent(0x04);
            lastContent = 0x04;
            break;
        case PICDUCK:
            MapObj->setModeContent(0x05);
            lastContent = 0x05;
            break;
        case PICSHEEP:
            MapObj->setModeContent(0x06);
            lastContent = 0x06;
            break;

        case WINDOW_CLICKED_CALL:
            if(MapObj)
            {
                MapObj->setMode(EDITOR_MODE_ANIMAL);
                MapObj->setModeContent(lastContent);
            }
            break;

        case WINDOWQUIT:
            if(WNDAnimal)
            {
                Pos = WNDAnimal->getPos();
                WNDAnimal->setWaste();
                WNDAnimal = nullptr;
            }
            MapObj->setMode(EDITOR_MODE_HEIGHT_RAISE);
            MapObj->setModeContent(0x00);
            lastContent = 0x00;
            MapObj = nullptr;
            break;

        case MAP_QUIT:
            // we do the same like in case WINDOWQUIT, but we won't setMode(EDITOR_MODE_HEIGHT_RAISE), cause map is dead
            if(WNDAnimal)
            {
                Pos = WNDAnimal->getPos();
                WNDAnimal->setWaste();
                WNDAnimal = nullptr;
            }
            lastContent = 0x00;
            MapObj = nullptr;
            break;

        default: break;
    }
}

void callback::EditorPlayerMenu(int Param)
{
    static CWindow* WNDPlayer = nullptr;
    static CMap* MapObj = nullptr;
    static int PlayerNumber = 0x00;
    static CFont* PlayerNumberText = nullptr;
    std::array<char, 30> puffer;
    static DisplayRectangle tempRect;
    static Uint16* PlayerHQx = nullptr;
    static Uint16* PlayerHQy = nullptr;
    static Position Pos{0, 200};

    enum
    {
        PLAYER_REDUCE = 0,
        PLAYER_RAISE,
        GOTO_PLAYER,
        WINDOWQUIT
    };

    if(Param != INITIALIZING_CALL && Param != MAP_QUIT)
        assert(WNDPlayer && MapObj);

    switch(Param)
    {
        case INITIALIZING_CALL:
            if(WNDPlayer)
                break;
            WNDPlayer = global::s2->RegisterWindow(std::make_unique<CWindow>(EditorPlayerMenu, WINDOWQUIT, Pos, Extent(100, 80), "Players",
                                                                             WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MINIMIZE | WINDOW_MOVE));
            MapObj = global::s2->getMapObj();
            tempRect = MapObj->getDisplayRect();
            PlayerHQx = MapObj->getPlayerHQx().data();
            PlayerHQy = MapObj->getPlayerHQy().data();

            MapObj->setMode(EDITOR_MODE_FLAG);
            MapObj->setModeContent(PlayerNumber);

            WNDPlayer->addButton(EditorPlayerMenu, PLAYER_REDUCE, 0, 0, 20, 20, BUTTON_GREY, "-");
            sprintf(puffer.data(), "%d", PlayerNumber + 1);
            PlayerNumberText = WNDPlayer->addText(puffer.data(), 26, 4, FontSize::Large, FontColor::Orange);
            WNDPlayer->addButton(EditorPlayerMenu, PLAYER_RAISE, 40, 0, 20, 20, BUTTON_GREY, "+");
            WNDPlayer->addButton(EditorPlayerMenu, GOTO_PLAYER, 0, 20, 60, 20, BUTTON_GREY, "Go to");
            break;

        case PLAYER_REDUCE:
            if(PlayerNumber > 0)
            {
                PlayerNumber--;
                MapObj->setModeContent(PlayerNumber);
                WNDPlayer->delText(PlayerNumberText);
                sprintf(puffer.data(), "%d", PlayerNumber + 1);
                PlayerNumberText = WNDPlayer->addText(puffer.data(), 26, 4, FontSize::Large, FontColor::Orange);
            }
            break;

        case PLAYER_RAISE:
            if(PlayerNumber < MAXPLAYERS - 1)
            {
                PlayerNumber++;
                MapObj->setModeContent(PlayerNumber);
                WNDPlayer->delText(PlayerNumberText);
                sprintf(puffer.data(), "%d", PlayerNumber + 1);
                PlayerNumberText = WNDPlayer->addText(puffer.data(), 26, 4, FontSize::Large, FontColor::Orange);
            }
            break;

        case GOTO_PLAYER: // test if player exists on map
            if(PlayerHQx[PlayerNumber] != 0xFFFF && PlayerHQy[PlayerNumber] != 0xFFFF)
            {
                tempRect = MapObj->getDisplayRect();
                tempRect.setOrigin(Position(PlayerHQx[PlayerNumber], PlayerHQy[PlayerNumber]) * Position(TRIANGLE_WIDTH, TRIANGLE_HEIGHT)
                                   - tempRect.getSize() / 2);
                MapObj->setDisplayRect(tempRect);
            }
            break;

        case WINDOW_CLICKED_CALL:
            if(MapObj)
            {
                MapObj->setMode(EDITOR_MODE_FLAG);
                MapObj->setModeContent(PlayerNumber);
            }
            break;

        case WINDOWQUIT:
            if(WNDPlayer)
            {
                Pos = WNDPlayer->getPos();
                WNDPlayer->setWaste();
                WNDPlayer = nullptr;
            }
            MapObj->setMode(EDITOR_MODE_HEIGHT_RAISE);
            MapObj->setModeContent(0x00);
            MapObj->setModeContent2(0x00);
            MapObj = nullptr;
            PlayerNumber = 0x01;
            PlayerNumberText = nullptr;
            PlayerHQx = nullptr;
            PlayerHQy = nullptr;
            break;

        case MAP_QUIT:
            // we do the same like in case WINDOWQUIT, but we won't setMode(EDITOR_MODE_HEIGHT_RAISE), cause map is dead
            if(WNDPlayer)
            {
                Pos = WNDPlayer->getPos();
                WNDPlayer->setWaste();
                WNDPlayer = nullptr;
            }
            MapObj = nullptr;
            PlayerNumber = 0x01;
            PlayerNumberText = nullptr;
            PlayerHQx = nullptr;
            PlayerHQy = nullptr;
            break;

        default: break;
    }
}

void callback::EditorCursorMenu(int Param)
{
    static CWindow* WNDCursor = nullptr;
    static CMap* MapObj = nullptr;
    static int trianglePictureArrowUp = -1;
    static int trianglePictureArrowDown = -1;
    static int trianglePictureRandom = -1;
    static CButton* CursorModeButton = nullptr;
    static CButton* CursorRandomButton = nullptr;
    static Position Pos{0, 0};

    enum
    {
        WINDOWQUIT,
        TRIANGLE,
        CURSORMODE,
        CURSORRANDOM
    };

    if(Param != INITIALIZING_CALL && Param != MAP_QUIT)
        assert(WNDCursor && MapObj);

    switch(Param)
    {
        case INITIALIZING_CALL:
            if(WNDCursor)
                break;
            WNDCursor = global::s2->RegisterWindow(std::make_unique<CWindow>(EditorCursorMenu, WINDOWQUIT, Pos, Extent(210, 130), "Cursor",
                                                                             WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MINIMIZE | WINDOW_MOVE));
            MapObj = global::s2->getMapObj();

            CursorModeButton = WNDCursor->addButton(EditorCursorMenu, CURSORMODE, 2, 2, 96, 32, BUTTON_GREY, "Hexagon");
            CursorRandomButton =
              WNDCursor->addButton(EditorCursorMenu, CURSORRANDOM, 2, 34, 196, 32, BUTTON_GREY, "Cursor-Activity: static");
            WNDCursor->addButton(EditorCursorMenu, TRIANGLE, 2, 66, 32, 32, BUTTON_GREY, nullptr);
            trianglePictureArrowUp = WNDCursor->addStaticPicture(8, 74, CURSOR_SYMBOL_ARROW_UP);
            trianglePictureArrowDown = WNDCursor->addStaticPicture(17, 77, CURSOR_SYMBOL_ARROW_DOWN);
            if(MapObj)
            {
                MapObj->setVertexFillRSU(true);
                MapObj->setVertexFillUSD(true);
                MapObj->setVertexFillRandom(false);
                MapObj->setHexagonMode(true);
                MapObj->setVertexActivityRandom(false);
            }
            break;

        case TRIANGLE:
            if(trianglePictureArrowUp != -1 && trianglePictureArrowDown != -1)
            {
                // both arrows are shown, so set to random
                // delete arrow up
                WNDCursor->delStaticPicture(trianglePictureArrowUp);
                trianglePictureArrowUp = -1;
                // delete arrow down
                WNDCursor->delStaticPicture(trianglePictureArrowDown);
                trianglePictureArrowDown = -1;
                // add random if necessary
                if(trianglePictureRandom == -1)
                    trianglePictureRandom = WNDCursor->addStaticPicture(14, 76, FONT14_SPACE + 31 * 7 + 5); // Interrogation point
                MapObj->setVertexFillRSU(false);
                MapObj->setVertexFillUSD(false);
                MapObj->setVertexFillRandom(true);
            } else if(trianglePictureArrowUp == -1 && trianglePictureRandom == -1)
            {
                // only arrow down is shown, so upgrade to both arrows
                // add arrow up
                trianglePictureArrowUp = WNDCursor->addStaticPicture(8, 74, CURSOR_SYMBOL_ARROW_UP);
                // add arrow down if necessary
                if(trianglePictureArrowDown == -1)
                    trianglePictureArrowDown = WNDCursor->addStaticPicture(17, 77, CURSOR_SYMBOL_ARROW_DOWN);
                MapObj->setVertexFillRSU(true);
                MapObj->setVertexFillUSD(true);
                MapObj->setVertexFillRandom(false);
            } else if(trianglePictureArrowDown == -1 && trianglePictureRandom == -1)
            {
                // only arrow up is shown, so delete arrow up and add arrow down
                // delete arrow up if necessary
                if(trianglePictureArrowUp != -1)
                {
                    WNDCursor->delStaticPicture(trianglePictureArrowUp);
                    trianglePictureArrowUp = -1;
                }
                trianglePictureArrowDown = WNDCursor->addStaticPicture(17, 77, CURSOR_SYMBOL_ARROW_DOWN);
                MapObj->setVertexFillRSU(false);
                MapObj->setVertexFillUSD(true);
                MapObj->setVertexFillRandom(false);
            } else
            {
                // the interrogation point is shown, so set to arrow up
                WNDCursor->delStaticPicture(trianglePictureRandom);
                trianglePictureRandom = -1;
                // add arrow up if necessary
                if(trianglePictureArrowUp == -1)
                    trianglePictureArrowUp = WNDCursor->addStaticPicture(8, 74, CURSOR_SYMBOL_ARROW_UP);
                // delete arrow down if necessary
                if(trianglePictureArrowDown != -1)
                {
                    WNDCursor->delStaticPicture(trianglePictureArrowDown);
                    trianglePictureArrowDown = -1;
                }
                MapObj->setVertexFillRSU(true);
                MapObj->setVertexFillUSD(false);
                MapObj->setVertexFillRandom(false);
            }
            break;

        case CURSORMODE:
            if(CursorModeButton)
            {
                WNDCursor->delButton(CursorModeButton);
                CursorModeButton = nullptr;
            }
            if(MapObj->getHexagonMode())
            {
                CursorModeButton = WNDCursor->addButton(EditorCursorMenu, CURSORMODE, 2, 2, 96, 32, BUTTON_GREY, "Square");
                MapObj->setHexagonMode(false);
            } else
            {
                CursorModeButton = WNDCursor->addButton(EditorCursorMenu, CURSORMODE, 2, 2, 96, 32, BUTTON_GREY, "Hexagon");
                MapObj->setHexagonMode(true);
            }
            break;
        case CURSORRANDOM:
            if(CursorRandomButton)
            {
                WNDCursor->delButton(CursorRandomButton);
                CursorRandomButton = nullptr;
            }
            if(MapObj->getVertexActivityRandom())
            {
                CursorRandomButton =
                  WNDCursor->addButton(EditorCursorMenu, CURSORRANDOM, 2, 34, 196, 32, BUTTON_GREY, "Cursor-Activity: static");
                MapObj->setVertexActivityRandom(false);
            } else
            {
                CursorRandomButton =
                  WNDCursor->addButton(EditorCursorMenu, CURSORRANDOM, 2, 34, 196, 32, BUTTON_GREY, "Cursor-Activity: random");
                MapObj->setVertexActivityRandom(true);
            }
            break;

        case WINDOW_CLICKED_CALL: break;

        case WINDOWQUIT:
        case MAP_QUIT:
            // we do the same like in case WINDOWQUIT
            if(WNDCursor)
            {
                Pos = WNDCursor->getPos();
                WNDCursor->setWaste();
                WNDCursor = nullptr;
            }
            MapObj = nullptr;
            trianglePictureArrowUp = -1;
            trianglePictureArrowDown = -1;
            trianglePictureRandom = -1;
            CursorModeButton = nullptr;
            CursorRandomButton = nullptr;
            break;

        default: break;
    }
}

//"create world" menu
void callback::EditorCreateMenu(int Param)
{
    static CWindow* WNDCreate = nullptr;
    static CMap* MapObj = nullptr;
    static CFont* TextWidth = nullptr;
    static int width = 32;
    static CFont* TextHeight = nullptr;
    static int height = 32;
    static CButton* ButtonLandscape = nullptr;
    static int LandscapeType = 0; // 0 = Greenland, 1 = Wasteland, 2 = Winterland
    static int PicTextureIndex = -1;
    static int PicTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_SNOW;
    static int texture = TRIANGLE_TEXTURE_SNOW;
    static int PicBorderTextureIndex = -1;
    static int PicBorderTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_SNOW;
    static CFont* TextBorder = nullptr;
    static int border = 0;
    static int border_texture = TRIANGLE_TEXTURE_SNOW;
    static std::array<char, 5> puffer;
    static Position Pos(global::s2->GameResolution.x / 2 - 125, global::s2->GameResolution.y / 2 - 175);

    enum
    {
        REDUCE_WIDTH_128,
        REDUCE_WIDTH_16,
        REDUCE_WIDTH_2,
        RAISE_WIDTH_2,
        RAISE_WIDTH_16,
        RAISE_WIDTH_128,
        REDUCE_HEIGHT_128,
        REDUCE_HEIGHT_16,
        REDUCE_HEIGHT_2,
        RAISE_HEIGHT_2,
        RAISE_HEIGHT_16,
        RAISE_HEIGHT_128,
        CHANGE_LANDSCAPE,
        TEXTURE_PREVIOUS,
        TEXTURE_NEXT,
        BORDER_TEXTURE_PREVIOUS,
        BORDER_TEXTURE_NEXT,
        REDUCE_BORDER,
        RAISE_BORDER,
        CREATE_WORLD,
        WINDOWQUIT
    };

    if(Param != INITIALIZING_CALL && Param != MAP_QUIT)
    {
        assert(WNDCreate);
        assert(MapObj);
    }

    switch(Param)
    {
        case INITIALIZING_CALL:
            if(WNDCreate)
                break;
            WNDCreate =
              global::s2->RegisterWindow(std::make_unique<CWindow>(EditorCreateMenu, WINDOWQUIT, Pos, Extent(250, 350), "Create world",
                                                                   WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MOVE | WINDOW_MINIMIZE));
            MapObj = global::s2->getMapObj();

            WNDCreate->addText("Width", 95, 4, FontSize::Small, FontColor::Yellow);
            WNDCreate->addButton(EditorCreateMenu, REDUCE_WIDTH_128, 0, 15, 35, 20, BUTTON_GREY, "128<-");
            WNDCreate->addButton(EditorCreateMenu, REDUCE_WIDTH_16, 35, 15, 35, 20, BUTTON_GREY, "16<-");
            WNDCreate->addButton(EditorCreateMenu, REDUCE_WIDTH_2, 70, 15, 25, 20, BUTTON_GREY, "2<-");
            sprintf(puffer.data(), "%d", width);
            TextWidth = WNDCreate->addText(puffer.data(), 105, 17, FontSize::Large, FontColor::Yellow);
            TextWidth->setText(puffer.data());
            WNDCreate->addButton(EditorCreateMenu, RAISE_WIDTH_2, 143, 15, 25, 20, BUTTON_GREY, "->2");
            WNDCreate->addButton(EditorCreateMenu, RAISE_WIDTH_16, 168, 15, 35, 20, BUTTON_GREY, "->16");
            WNDCreate->addButton(EditorCreateMenu, RAISE_WIDTH_128, 203, 15, 35, 20, BUTTON_GREY, "->128");

            WNDCreate->addText("Height", 100, 40, FontSize::Small, FontColor::Yellow);
            WNDCreate->addButton(EditorCreateMenu, REDUCE_HEIGHT_128, 0, 49, 35, 20, BUTTON_GREY, "128<-");
            WNDCreate->addButton(EditorCreateMenu, REDUCE_HEIGHT_16, 35, 49, 35, 20, BUTTON_GREY, "16<-");
            WNDCreate->addButton(EditorCreateMenu, REDUCE_HEIGHT_2, 70, 49, 25, 20, BUTTON_GREY, "2<-");
            sprintf(puffer.data(), "%d", height);
            TextHeight = WNDCreate->addText(puffer.data(), 105, 51, FontSize::Large, FontColor::Yellow);
            TextHeight->setText(puffer.data());
            WNDCreate->addButton(EditorCreateMenu, RAISE_HEIGHT_2, 143, 49, 25, 20, BUTTON_GREY, "->2");
            WNDCreate->addButton(EditorCreateMenu, RAISE_HEIGHT_16, 168, 49, 35, 20, BUTTON_GREY, "->16");
            WNDCreate->addButton(EditorCreateMenu, RAISE_HEIGHT_128, 203, 49, 35, 20, BUTTON_GREY, "->128");

            WNDCreate->addText("Landscape", 85, 80, FontSize::Small, FontColor::Yellow);
            ButtonLandscape = WNDCreate->addButton(EditorCreateMenu, CHANGE_LANDSCAPE, 64, 93, 110, 20, BUTTON_GREY,
                                                   (LandscapeType == 0 ? "Greenland" : (LandscapeType == 1 ? "Wasteland" : "Winterworld")));

            WNDCreate->addText("Main area", 82, 120, FontSize::Small, FontColor::Yellow);
            WNDCreate->addButton(EditorCreateMenu, TEXTURE_PREVIOUS, 45, 139, 35, 20, BUTTON_GREY, "-");
            PicTextureIndex = WNDCreate->addStaticPicture(102, 133, PicTextureIndexGlobal);
            WNDCreate->addButton(EditorCreateMenu, TEXTURE_NEXT, 158, 139, 35, 20, BUTTON_GREY, "+");

            WNDCreate->addText("Border size", 103, 175, FontSize::Small, FontColor::Yellow);
            WNDCreate->addButton(EditorCreateMenu, REDUCE_BORDER, 45, 186, 35, 20, BUTTON_GREY, "-");
            sprintf(puffer.data(), "%d", border);
            TextBorder = WNDCreate->addText(puffer.data(), 112, 188, FontSize::Large, FontColor::Yellow);
            WNDCreate->addButton(EditorCreateMenu, RAISE_BORDER, 158, 186, 35, 20, BUTTON_GREY, "+");

            WNDCreate->addText("Border area", 65, 215, FontSize::Small, FontColor::Yellow);
            WNDCreate->addButton(EditorCreateMenu, BORDER_TEXTURE_PREVIOUS, 45, 234, 35, 20, BUTTON_GREY, "-");
            PicBorderTextureIndex = WNDCreate->addStaticPicture(102, 228, PicBorderTextureIndexGlobal);
            WNDCreate->addButton(EditorCreateMenu, BORDER_TEXTURE_NEXT, 158, 234, 35, 20, BUTTON_GREY, "+");

            WNDCreate->addButton(EditorCreateMenu, CREATE_WORLD, 44, 275, 150, 40, BUTTON_GREY, "Create world");
            break;

        case CALL_FROM_GAMELOOP: break;

        case REDUCE_WIDTH_128:
            if(width - 128 >= 32)
                width -= 128;
            else
                width = 32;
            WNDCreate->delText(TextWidth);
            sprintf(puffer.data(), "%d", width);
            TextWidth = WNDCreate->addText(puffer.data(), 105, 17, FontSize::Large, FontColor::Yellow);
            break;
        case REDUCE_WIDTH_16:
            if(width - 16 >= 32)
                width -= 16;
            else
                width = 32;
            WNDCreate->delText(TextWidth);
            sprintf(puffer.data(), "%d", width);
            TextWidth = WNDCreate->addText(puffer.data(), 105, 17, FontSize::Large, FontColor::Yellow);
            break;
        case REDUCE_WIDTH_2:
            if(width - 2 >= 32)
                width -= 2;
            else
                width = 32;
            WNDCreate->delText(TextWidth);
            sprintf(puffer.data(), "%d", width);
            TextWidth = WNDCreate->addText(puffer.data(), 105, 17, FontSize::Large, FontColor::Yellow);
            break;
        case RAISE_WIDTH_2:
            if(width + 2 <= MAXMAPWIDTH)
                width += 2;
            else
                width = MAXMAPWIDTH;
            WNDCreate->delText(TextWidth);
            sprintf(puffer.data(), "%d", width);
            TextWidth = WNDCreate->addText(puffer.data(), 105, 17, FontSize::Large, FontColor::Yellow);
            break;
        case RAISE_WIDTH_16:
            if(width + 16 <= MAXMAPWIDTH)
                width += 16;
            else
                width = MAXMAPWIDTH;
            WNDCreate->delText(TextWidth);
            sprintf(puffer.data(), "%d", width);
            TextWidth = WNDCreate->addText(puffer.data(), 105, 17, FontSize::Large, FontColor::Yellow);
            break;
        case RAISE_WIDTH_128:
            if(width + 128 <= MAXMAPWIDTH)
                width += 128;
            else
                width = MAXMAPWIDTH;
            WNDCreate->delText(TextWidth);
            sprintf(puffer.data(), "%d", width);
            TextWidth = WNDCreate->addText(puffer.data(), 105, 17, FontSize::Large, FontColor::Yellow);
            break;
        case REDUCE_HEIGHT_128:
            if(height - 128 >= 32)
                height -= 128;
            else
                height = 32;
            WNDCreate->delText(TextHeight);
            sprintf(puffer.data(), "%d", height);
            TextHeight = WNDCreate->addText(puffer.data(), 105, 51, FontSize::Large, FontColor::Yellow);
            break;
        case REDUCE_HEIGHT_16:
            if(height - 16 >= 32)
                height -= 16;
            else
                height = 32;
            WNDCreate->delText(TextHeight);
            sprintf(puffer.data(), "%d", height);
            TextHeight = WNDCreate->addText(puffer.data(), 105, 51, FontSize::Large, FontColor::Yellow);
            break;
        case REDUCE_HEIGHT_2:
            if(height - 2 >= 32)
                height -= 2;
            else
                height = 32;
            WNDCreate->delText(TextHeight);
            sprintf(puffer.data(), "%d", height);
            TextHeight = WNDCreate->addText(puffer.data(), 105, 51, FontSize::Large, FontColor::Yellow);
            break;
        case RAISE_HEIGHT_2:
            if(height + 2 <= MAXMAPHEIGHT)
                height += 2;
            else
                height = MAXMAPHEIGHT;
            WNDCreate->delText(TextHeight);
            sprintf(puffer.data(), "%d", height);
            TextHeight = WNDCreate->addText(puffer.data(), 105, 51, FontSize::Large, FontColor::Yellow);
            break;
        case RAISE_HEIGHT_16:
            if(height + 16 <= MAXMAPHEIGHT)
                height += 16;
            else
                height = MAXMAPHEIGHT;
            WNDCreate->delText(TextHeight);
            sprintf(puffer.data(), "%d", height);
            TextHeight = WNDCreate->addText(puffer.data(), 105, 51, FontSize::Large, FontColor::Yellow);
            break;
        case RAISE_HEIGHT_128:
            if(height + 128 <= MAXMAPHEIGHT)
                height += 128;
            else
                height = MAXMAPHEIGHT;
            WNDCreate->delText(TextHeight);
            sprintf(puffer.data(), "%d", height);
            TextHeight = WNDCreate->addText(puffer.data(), 105, 51, FontSize::Large, FontColor::Yellow);
            break;

        case CHANGE_LANDSCAPE:
            LandscapeType++;
            if(LandscapeType > 2)
                LandscapeType = 0;
            WNDCreate->delButton(ButtonLandscape);
            ButtonLandscape = WNDCreate->addButton(EditorCreateMenu, CHANGE_LANDSCAPE, 64, 93, 110, 20, BUTTON_GREY,
                                                   (LandscapeType == 0 ? "Greenland" : (LandscapeType == 1 ? "Wasteland" : "Winterworld")));
            switch(LandscapeType)
            {
                case 0:
                    PicTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_SNOW;
                    PicBorderTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_SNOW;
                    break;
                case 1:
                    PicTextureIndexGlobal = PICTURE_WASTELAND_TEXTURE_SNOW;
                    PicBorderTextureIndexGlobal = PICTURE_WASTELAND_TEXTURE_SNOW;
                    break;
                case 2:
                    PicTextureIndexGlobal = PICTURE_WINTERLAND_TEXTURE_SNOW;
                    PicBorderTextureIndexGlobal = PICTURE_WINTERLAND_TEXTURE_SNOW;
                    break;
                default: break;
            }
            WNDCreate->delStaticPicture(PicTextureIndex);
            WNDCreate->delStaticPicture(PicBorderTextureIndex);
            PicTextureIndex = WNDCreate->addStaticPicture(102, 133, PicTextureIndexGlobal);
            PicBorderTextureIndex = WNDCreate->addStaticPicture(102, 228, PicBorderTextureIndexGlobal);
            break;

        case TEXTURE_PREVIOUS:
            PicTextureIndexGlobal--;
            switch(LandscapeType)
            {
                case 0:
                    if(PicTextureIndexGlobal < PICTURE_GREENLAND_TEXTURE_SNOW)
                        PicTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_SNOW;
                    break;
                case 1:
                    if(PicTextureIndexGlobal < PICTURE_WASTELAND_TEXTURE_SNOW)
                        PicTextureIndexGlobal = PICTURE_WASTELAND_TEXTURE_SNOW;
                    break;
                case 2:
                    if(PicTextureIndexGlobal < PICTURE_WINTERLAND_TEXTURE_SNOW)
                        PicTextureIndexGlobal = PICTURE_WINTERLAND_TEXTURE_SNOW;
                    break;
                default: break;
            }
            if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_SNOW || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_SNOW
               || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_SNOW)
            {
                texture = TRIANGLE_TEXTURE_SNOW;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE)
            {
                texture = TRIANGLE_TEXTURE_STEPPE;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_SWAMP || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_SWAMP
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_SWAMP)
            {
                texture = TRIANGLE_TEXTURE_SWAMP;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_FLOWER || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_FLOWER
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_FLOWER)
            {
                texture = TRIANGLE_TEXTURE_FLOWER;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING1
                      || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING1
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING1)
            {
                texture = TRIANGLE_TEXTURE_MINING1;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING2
                      || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING2
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING2)
            {
                texture = TRIANGLE_TEXTURE_MINING2;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING3
                      || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING3
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING3)
            {
                texture = TRIANGLE_TEXTURE_MINING3;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING4
                      || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING4
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING4)
            {
                texture = TRIANGLE_TEXTURE_MINING4;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW1
                      || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE_MEADOW1
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE_MEADOW1)
            {
                texture = TRIANGLE_TEXTURE_STEPPE_MEADOW1;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW1
                      || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW1
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW1)
            {
                texture = TRIANGLE_TEXTURE_MEADOW1;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW2
                      || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW2
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW2)
            {
                texture = TRIANGLE_TEXTURE_MEADOW2;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW3
                      || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW3
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW3)
            {
                texture = TRIANGLE_TEXTURE_MEADOW3;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW2
                      || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE_MEADOW2
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE_MEADOW2)
            {
                texture = TRIANGLE_TEXTURE_STEPPE_MEADOW2;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING_MEADOW
                      || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING_MEADOW
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING_MEADOW)
            {
                texture = TRIANGLE_TEXTURE_MINING_MEADOW;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_WATER || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_WATER
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_WATER)
            {
                texture = TRIANGLE_TEXTURE_WATER;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_LAVA || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_LAVA
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_LAVA)
            {
                texture = TRIANGLE_TEXTURE_LAVA;
            }
            WNDCreate->delStaticPicture(PicTextureIndex);
            PicTextureIndex = WNDCreate->addStaticPicture(102, 133, PicTextureIndexGlobal);
            break;
        case TEXTURE_NEXT:
            PicTextureIndexGlobal++;
            switch(LandscapeType)
            {
                case 0:
                    if(PicTextureIndexGlobal > PICTURE_GREENLAND_TEXTURE_LAVA)
                        PicTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_LAVA;
                    break;
                case 1:
                    if(PicTextureIndexGlobal > PICTURE_WASTELAND_TEXTURE_LAVA)
                        PicTextureIndexGlobal = PICTURE_WASTELAND_TEXTURE_LAVA;
                    break;
                case 2:
                    if(PicTextureIndexGlobal > PICTURE_WINTERLAND_TEXTURE_LAVA)
                        PicTextureIndexGlobal = PICTURE_WINTERLAND_TEXTURE_LAVA;
                    break;
                default: break;
            }
            if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_SNOW || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_SNOW
               || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_SNOW)
            {
                texture = TRIANGLE_TEXTURE_SNOW;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE)
            {
                texture = TRIANGLE_TEXTURE_STEPPE;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_SWAMP || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_SWAMP
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_SWAMP)
            {
                texture = TRIANGLE_TEXTURE_SWAMP;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_FLOWER || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_FLOWER
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_FLOWER)
            {
                texture = TRIANGLE_TEXTURE_FLOWER;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING1
                      || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING1
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING1)
            {
                texture = TRIANGLE_TEXTURE_MINING1;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING2
                      || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING2
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING2)
            {
                texture = TRIANGLE_TEXTURE_MINING2;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING3
                      || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING3
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING3)
            {
                texture = TRIANGLE_TEXTURE_MINING3;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING4
                      || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING4
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING4)
            {
                texture = TRIANGLE_TEXTURE_MINING4;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW1
                      || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE_MEADOW1
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE_MEADOW1)
            {
                texture = TRIANGLE_TEXTURE_STEPPE_MEADOW1;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW1
                      || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW1
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW1)
            {
                texture = TRIANGLE_TEXTURE_MEADOW1;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW2
                      || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW2
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW2)
            {
                texture = TRIANGLE_TEXTURE_MEADOW2;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW3
                      || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW3
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW3)
            {
                texture = TRIANGLE_TEXTURE_MEADOW3;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW2
                      || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE_MEADOW2
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE_MEADOW2)
            {
                texture = TRIANGLE_TEXTURE_STEPPE_MEADOW2;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING_MEADOW
                      || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING_MEADOW
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING_MEADOW)
            {
                texture = TRIANGLE_TEXTURE_MINING_MEADOW;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_WATER || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_WATER
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_WATER)
            {
                texture = TRIANGLE_TEXTURE_WATER;
            } else if(PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_LAVA || PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_LAVA
                      || PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_LAVA)
            {
                texture = TRIANGLE_TEXTURE_LAVA;
            }
            WNDCreate->delStaticPicture(PicTextureIndex);
            PicTextureIndex = WNDCreate->addStaticPicture(102, 133, PicTextureIndexGlobal);
            break;

        case REDUCE_BORDER:
            if(border - 1 >= 0)
                border -= 1;
            else
                border = 0;
            WNDCreate->delText(TextBorder);
            sprintf(puffer.data(), "%d", border);
            TextBorder = WNDCreate->addText(puffer.data(), 112, 188, FontSize::Large, FontColor::Yellow);
            break;
        case RAISE_BORDER:
            if(border + 1 <= 12)
                border += 1;
            else
                border = 12;
            WNDCreate->delText(TextBorder);
            sprintf(puffer.data(), "%d", border);
            TextBorder = WNDCreate->addText(puffer.data(), 112, 188, FontSize::Large, FontColor::Yellow);
            break;

        case BORDER_TEXTURE_PREVIOUS:
            PicBorderTextureIndexGlobal--;
            switch(LandscapeType)
            {
                case 0:
                    if(PicBorderTextureIndexGlobal < PICTURE_GREENLAND_TEXTURE_SNOW)
                        PicBorderTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_SNOW;
                    break;
                case 1:
                    if(PicBorderTextureIndexGlobal < PICTURE_WASTELAND_TEXTURE_SNOW)
                        PicBorderTextureIndexGlobal = PICTURE_WASTELAND_TEXTURE_SNOW;
                    break;
                case 2:
                    if(PicBorderTextureIndexGlobal < PICTURE_WINTERLAND_TEXTURE_SNOW)
                        PicBorderTextureIndexGlobal = PICTURE_WINTERLAND_TEXTURE_SNOW;
                    break;
                default: break;
            }
            if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_SNOW
               || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_SNOW
               || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_SNOW)
            {
                border_texture = TRIANGLE_TEXTURE_SNOW;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE)
            {
                border_texture = TRIANGLE_TEXTURE_STEPPE;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_SWAMP
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_SWAMP
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_SWAMP)
            {
                border_texture = TRIANGLE_TEXTURE_SWAMP;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_FLOWER
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_FLOWER
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_FLOWER)
            {
                border_texture = TRIANGLE_TEXTURE_FLOWER;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING1
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING1
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING1)
            {
                border_texture = TRIANGLE_TEXTURE_MINING1;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING2
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING2
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING2)
            {
                border_texture = TRIANGLE_TEXTURE_MINING2;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING3
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING3
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING3)
            {
                border_texture = TRIANGLE_TEXTURE_MINING3;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING4
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING4
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING4)
            {
                border_texture = TRIANGLE_TEXTURE_MINING4;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW1
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE_MEADOW1
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE_MEADOW1)
            {
                border_texture = TRIANGLE_TEXTURE_STEPPE_MEADOW1;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW1
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW1
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW1)
            {
                border_texture = TRIANGLE_TEXTURE_MEADOW1;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW2
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW2
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW2)
            {
                border_texture = TRIANGLE_TEXTURE_MEADOW2;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW3
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW3
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW3)
            {
                border_texture = TRIANGLE_TEXTURE_MEADOW3;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW2
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE_MEADOW2
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE_MEADOW2)
            {
                border_texture = TRIANGLE_TEXTURE_STEPPE_MEADOW2;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING_MEADOW
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING_MEADOW
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING_MEADOW)
            {
                border_texture = TRIANGLE_TEXTURE_MINING_MEADOW;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_WATER
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_WATER
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_WATER)
            {
                border_texture = TRIANGLE_TEXTURE_WATER;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_LAVA
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_LAVA
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_LAVA)
            {
                border_texture = TRIANGLE_TEXTURE_LAVA;
            }
            WNDCreate->delStaticPicture(PicBorderTextureIndex);
            PicBorderTextureIndex = WNDCreate->addStaticPicture(102, 228, PicBorderTextureIndexGlobal);
            break;
        case BORDER_TEXTURE_NEXT:
            PicBorderTextureIndexGlobal++;
            switch(LandscapeType)
            {
                case 0:
                    if(PicBorderTextureIndexGlobal > PICTURE_GREENLAND_TEXTURE_LAVA)
                        PicBorderTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_LAVA;
                    break;
                case 1:
                    if(PicBorderTextureIndexGlobal > PICTURE_WASTELAND_TEXTURE_LAVA)
                        PicBorderTextureIndexGlobal = PICTURE_WASTELAND_TEXTURE_LAVA;
                    break;
                case 2:
                    if(PicBorderTextureIndexGlobal > PICTURE_WINTERLAND_TEXTURE_LAVA)
                        PicBorderTextureIndexGlobal = PICTURE_WINTERLAND_TEXTURE_LAVA;
                    break;
                default: break;
            }
            if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_SNOW
               || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_SNOW
               || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_SNOW)
            {
                border_texture = TRIANGLE_TEXTURE_SNOW;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE)
            {
                border_texture = TRIANGLE_TEXTURE_STEPPE;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_SWAMP
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_SWAMP
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_SWAMP)
            {
                border_texture = TRIANGLE_TEXTURE_SWAMP;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_FLOWER
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_FLOWER
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_FLOWER)
            {
                border_texture = TRIANGLE_TEXTURE_FLOWER;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING1
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING1
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING1)
            {
                border_texture = TRIANGLE_TEXTURE_MINING1;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING2
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING2
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING2)
            {
                border_texture = TRIANGLE_TEXTURE_MINING2;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING3
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING3
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING3)
            {
                border_texture = TRIANGLE_TEXTURE_MINING3;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING4
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING4
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING4)
            {
                border_texture = TRIANGLE_TEXTURE_MINING4;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW1
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE_MEADOW1
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE_MEADOW1)
            {
                border_texture = TRIANGLE_TEXTURE_STEPPE_MEADOW1;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW1
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW1
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW1)
            {
                border_texture = TRIANGLE_TEXTURE_MEADOW1;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW2
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW2
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW2)
            {
                border_texture = TRIANGLE_TEXTURE_MEADOW2;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW3
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW3
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW3)
            {
                border_texture = TRIANGLE_TEXTURE_MEADOW3;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW2
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE_MEADOW2
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE_MEADOW2)
            {
                border_texture = TRIANGLE_TEXTURE_STEPPE_MEADOW2;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING_MEADOW
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING_MEADOW
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING_MEADOW)
            {
                border_texture = TRIANGLE_TEXTURE_MINING_MEADOW;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_WATER
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_WATER
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_WATER)
            {
                border_texture = TRIANGLE_TEXTURE_WATER;
            } else if(PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_LAVA
                      || PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_LAVA
                      || PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_LAVA)
            {
                border_texture = TRIANGLE_TEXTURE_LAVA;
            }
            WNDCreate->delStaticPicture(PicBorderTextureIndex);
            PicBorderTextureIndex = WNDCreate->addStaticPicture(102, 228, PicBorderTextureIndexGlobal);
            break;

        case CREATE_WORLD:
            PleaseWait(INITIALIZING_CALL);

            // we have to close the windows and initialize them again to prevent failures
            EditorCursorMenu(MAP_QUIT);
            EditorTextureMenu(MAP_QUIT);
            EditorTreeMenu(MAP_QUIT);
            EditorLandscapeMenu(MAP_QUIT);
            MinimapMenu(MAP_QUIT);
            EditorResourceMenu(MAP_QUIT);
            EditorAnimalMenu(MAP_QUIT);
            EditorPlayerMenu(MAP_QUIT);

            MapObj->destructMap();
            MapObj->constructMap("", width, height, MapType(LandscapeType), TriangleTerrainType(texture), border, border_texture);

            // we need to check which of these windows was active before
            /*
            EditorCursorMenu(INITIALIZING_CALL);
            EditorTextureMenu(INITIALIZING_CALL);
            EditorTreeMenu(INITIALIZING_CALL);
            EditorLandscapeMenu(INITIALIZING_CALL);
            MinimapMenu(INITIALIZING_CALL);
            EditorResourceMenu(INITIALIZING_CALL);
            EditorAnimalMenu(INITIALIZING_CALL);
            EditorPlayerMenu(INITIALIZING_CALL);
            */

            PleaseWait(WINDOW_QUIT_MESSAGE);
            break;

        case MAP_QUIT:
        case WINDOWQUIT:
            if(WNDCreate)
            {
                Pos = WNDCreate->getPos();
                WNDCreate->setWaste();
                WNDCreate = nullptr;
            }
            MapObj = nullptr;
            TextWidth = nullptr;
            width = 32;
            TextHeight = nullptr;
            height = 32;
            ButtonLandscape = nullptr;
            LandscapeType = 0;
            PicTextureIndex = -1;
            PicTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_SNOW;
            texture = TRIANGLE_TEXTURE_SNOW;
            PicBorderTextureIndex = -1;
            PicBorderTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_SNOW;
            TextBorder = nullptr;
            border = 0;
            border_texture = TRIANGLE_TEXTURE_SNOW;
            break;

        default: break;
    }
}

void callback::MinimapMenu(int Param)
{
    static CWindow* WNDMinimap = nullptr;
    static CMap* MapObj = nullptr;
    static SDL_Surface* WndSurface = nullptr;
    static int scaleNum = 1;
    // only in case INITIALIZING_CALL needed to create the window
    int width;
    int height;

    enum
    {
        WINDOWQUIT
    };

    if(Param != INITIALIZING_CALL && Param != MAP_QUIT)
        assert(WNDMinimap && MapObj);

    switch(Param)
    {
        case INITIALIZING_CALL:
            if(WNDMinimap)
                break;
            {
                // this variables are needed to reduce the size of minimap-windows of big maps
                MapObj = global::s2->getMapObj();
                bobMAP* map = MapObj->getMap();
                scaleNum = std::max(std::max(map->width / 256, map->height / 256), 1);

                width = map->width / scaleNum;
                height = map->height / scaleNum;
                //--> 12px is width of left and right window frame and 30px is height of the upper and lower window frame
                if((global::s2->getDisplaySurface()->w - 12 < width) || (global::s2->getDisplaySurface()->h - 30 < height))
                    break;
                WNDMinimap = global::s2->RegisterWindow(std::make_unique<CWindow>(MinimapMenu, WINDOWQUIT, WindowPos::Center,
                                                                                  Extent(width + 12, height + 30), "Overview",
                                                                                  WINDOW_NOTHING, WINDOW_CLOSE | WINDOW_MOVE));
                global::s2->RegisterCallback(MinimapMenu);
                WndSurface = WNDMinimap->getSurface();
            }
            break;

        case CALL_FROM_GAMELOOP:
            if(MapObj && WndSurface)
                MapObj->drawMinimap(WndSurface);
            break;

        case WINDOW_CLICKED_CALL:
            if(MapObj)
            {
                Position mouse;
                if(SDL_GetMouseState(&mouse.x, &mouse.y) & SDL_BUTTON(1))
                {
                    if(mouse.x > (WNDMinimap->getX() + 6) && mouse.x < (WNDMinimap->getX() + WNDMinimap->getW() - 6)
                       && mouse.y > (WNDMinimap->getY() + 20) && mouse.y < (WNDMinimap->getY() + WNDMinimap->getH() - 10))
                    {
                        DisplayRectangle displayRect = MapObj->getDisplayRect();
                        displayRect.setOrigin(
                          (mouse - WNDMinimap->getRect().getOrigin() - Position(6, 20)
                           - Position(global::bmpArray[MAPPIC_ARROWCROSS_ORANGE].nx, global::bmpArray[MAPPIC_ARROWCROSS_ORANGE].ny))
                          * Position(TRIANGLE_WIDTH, TRIANGLE_HEIGHT) * scaleNum);
                        MapObj->setDisplayRect(displayRect);
                    }
                }
            }
            break;

        case WINDOWQUIT:
        case MAP_QUIT:
            if(WNDMinimap)
            {
                WNDMinimap->setWaste();
                WNDMinimap = nullptr;
            }
            MapObj = nullptr;
            WndSurface = nullptr;
            global::s2->UnregisterCallback(MinimapMenu);
            break;

        default: break;
    }
}

#ifdef _ADMINMODE
// the debugger is an object and a friend class of all other classes
// debugger-function only will construct a new debugger and if debugger-function gets a window-quit-message
// then the debugger-function will destruct the object
void callback::debugger(int Param)
{
    static CDebug* Debugger = nullptr;

    switch(Param)
    {
        case INITIALIZING_CALL:
            if(Debugger)
                break;
            Debugger = new CDebug(debugger, DEBUGGER_QUIT);
            break;

        case DEBUGGER_QUIT:
            delete Debugger;
            Debugger = nullptr;
            break;

        default:
            if(Debugger)
                Debugger->sendParam(Param);
            break;
    }
}

// this is the picture-viewer
void callback::viewer(int Param)
{
    static CWindow* WNDViewer = nullptr;
    static int index = 0;
    static int PicInWndIndex = -1;
    static CFont* PicInfosText = nullptr;

    enum
    {
        BACKWARD_1,
        BACKWARD_10,
        BACKWARD_100,
        FORWARD_1,
        FORWARD_10,
        FORWARD_100,
        WINDOWQUIT
    };
    if(Param != INITIALIZING_CALL && Param != MAP_QUIT)
    {
        assert(WNDViewer);
    }

    switch(Param)
    {
        case INITIALIZING_CALL:
            if(WNDViewer)
                break;
            WNDViewer = global::s2->RegisterWindow(std::make_unique<CWindow>(viewer, WINDOWQUIT, Position(0, 0), Extent(250, 140), "Viewer",
                                                                             WINDOW_GREEN1,
                                                                             WINDOW_CLOSE | WINDOW_MOVE | WINDOW_RESIZE | WINDOW_MINIMIZE));
            global::s2->RegisterCallback(viewer);
            WNDViewer->addButton(viewer, BACKWARD_100, 0, 0, 35, 20, BUTTON_GREY, "100<-");
            WNDViewer->addButton(viewer, BACKWARD_10, 35, 0, 35, 20, BUTTON_GREY, "10<-");
            WNDViewer->addButton(viewer, BACKWARD_1, 70, 0, 35, 20, BUTTON_GREY, "1<-");
            WNDViewer->addButton(viewer, FORWARD_1, 105, 0, 35, 20, BUTTON_GREY, "->1");
            WNDViewer->addButton(viewer, FORWARD_10, 140, 0, 35, 20, BUTTON_GREY, "->10");
            WNDViewer->addButton(viewer, FORWARD_100, 175, 0, 35, 20, BUTTON_GREY, "->100");
            break;

        case CALL_FROM_GAMELOOP:
            if(PicInWndIndex >= 0)
                WNDViewer->delStaticPicture(PicInWndIndex);
            PicInWndIndex = WNDViewer->addStaticPicture(5, 30, index);

            if(PicInfosText)
            {
                if(WNDViewer->delText(PicInfosText))
                    PicInfosText = nullptr;
            }
            if(!PicInfosText)
            {
                const auto infos = helpers::format("index=%d, w=%d, h=%d, nx=%d, ny=%d", index, global::bmpArray[index].w,
                                                   global::bmpArray[index].h, global::bmpArray[index].nx, global::bmpArray[index].ny);
                PicInfosText = WNDViewer->addText(infos, 220, 3, 14, FontColor::Red);
            }

            break;

        case BACKWARD_100:
            if(index - 100 >= 0)
                index -= 100;
            else
                index = 0;
            break;
        case BACKWARD_10:
            if(index - 10 >= 0)
                index -= 10;
            else
                index = 0;
            break;
        case BACKWARD_1:
            if(index - 1 >= 0)
                index -= 1;
            else
                index = 0;
            break;
        case FORWARD_1:
            if(index < MAXBOBBMP - 1)
                index++;
            break;
        case FORWARD_10:
            if(index + 10 < MAXBOBBMP - 1)
                index += 10;
            break;
        case FORWARD_100:
            if(index + 100 < MAXBOBBMP - 1)
                index += 100;
            break;

        case WINDOWQUIT:
            if(WNDViewer)
            {
                WNDViewer->setWaste();
                WNDViewer = nullptr;
                global::s2->UnregisterCallback(viewer);
                index = 0;
                PicInWndIndex = -1;
            }
            break;

        default: break;
    }
}

// this is a submenu for testing
void callback::submenu1(int Param)
{
    static CMenu* SubMenu = nullptr;
    static CButton* greatMoon = nullptr;
    static CFont* greatMoonText = nullptr;
    static CFont* counterText = nullptr;
    static CPicture* picObject = nullptr;
    static int counter = 0;
    static CWindow* testWindow = nullptr;
    static CWindow* testWindow2 = nullptr;
    static CPicture* testWindowPicture = nullptr;
    static CFont* testWindowText = nullptr;
    static CFont* testWindowText2 = nullptr;
    static CTextfield* testTextfield = nullptr;
    static CFont* TextFrom_testTextfield = nullptr;
    static CTextfield* testTextfield_testWindow = nullptr;
    static CSelectBox* testSelectBox = nullptr;

    static int picIndex = -1;

    // if this is the first time the function is called
    if(Param == INITIALIZING_CALL)
        global::s2->RegisterCallback(submenu1);

    enum
    {
        MAINMENU = 1,
        GREATMOON,
        SMALLMOON,
        TOOSMALL,
        CREATEWINDOW,
        GREATMOONENTRY,
        GREATMOONLEAVE,
        PICOBJECT,
        PICOBJECTENTRY,
        PICOBJECTLEAVE,
        TESTWINDOWPICTURE,
        TESTWINDOWPICTUREENTRY,
        TESTWINDOWPICTURELEAVE,
        TESTWINDOWQUITMESSAGE,
        TESTWINDOW2QUITMESSAGE,
        SELECTBOX_OPTION1,
        SELECTBOX_OPTION2,
        SELECTBOX_OPTION3
    };

    if(Param != INITIALIZING_CALL && Param != MAP_QUIT)
    {
        assert(SubMenu);
    }

    switch(Param)
    {
        case INITIALIZING_CALL:
            SubMenu = global::s2->RegisterMenu(std::make_unique<CMenu>(SPLASHSCREEN_SUBMENU1));
            SubMenu->addButton(submenu1, MAINMENU, 400, 440, 200, 20, BUTTON_RED1, "back");
            greatMoon = SubMenu->addButton(submenu1, GREATMOON, 100, 100, 200, 200, BUTTON_STONE, nullptr, MOON);
            greatMoon->setMotionParams(GREATMOONENTRY, GREATMOONLEAVE);
            SubMenu->addButton(submenu1, SMALLMOON, 100, 350, global::bmpArray[MOON].w, global::bmpArray[MOON].h, BUTTON_STONE, nullptr,
                               MOON);
            SubMenu->addButton(submenu1, TOOSMALL, 100, 400, global::bmpArray[MOON].w - 1, global::bmpArray[MOON].h - 1, BUTTON_STONE,
                               nullptr, MOON);
            SubMenu->addButton(submenu1, CREATEWINDOW, 500, 10, 130, 30, BUTTON_GREEN1, "Create window");
            picObject = SubMenu->addPicture(submenu1, PICOBJECT, 200, 30, MIS0BOBS_SHIP);
            picObject->setMotionParams(PICOBJECTENTRY, PICOBJECTLEAVE);
            // text block with \n
            SubMenu->addText("\nTextblock:\n\nNeue Zeile\nNoch eine neue Zeile", 400, 200, 14);
            testTextfield = SubMenu->addTextfield(400, 300, 10, 3);
            testSelectBox = SubMenu->addSelectBox(Point16(500, 500), Extent16(300, 200));
            testSelectBox->setOption("Erste Option", submenu1, SELECTBOX_OPTION1);
            testSelectBox->setOption("Zweite Option", submenu1, SELECTBOX_OPTION2);
            testSelectBox->setOption("Dritte Option", submenu1, SELECTBOX_OPTION3);
            break;

        case MAINMENU:
            SubMenu->setWaste();
            SubMenu = nullptr;
            greatMoon = nullptr;
            greatMoonText = nullptr;
            counterText = nullptr;
            testWindowPicture = nullptr;
            testWindowText = nullptr;
            testWindowText2 = nullptr;
            testTextfield = nullptr;
            TextFrom_testTextfield = nullptr;
            testTextfield_testWindow = nullptr;
            testSelectBox = nullptr;
            global::s2->UnregisterCallback(submenu1);
            if(testWindow)
            {
                testWindow->setWaste();
                testWindow = nullptr;
            }
            if(testWindow2)
            {
                testWindow2->setWaste();
                testWindow2 = nullptr;
            }
            picIndex = -1;
            break;

        case GREATMOON:
            SubMenu->addText("Title!", 300, 10, 14);
            SubMenu->addText(helpers::format("Window X: %d Window Y: %d", global::s2->GameResolution.x, global::s2->GameResolution.y), 10,
                             10, 14);
            break;

        case SMALLMOON:
            SubMenu->delButton(greatMoon);
            SubMenu->delStaticPicture(picIndex);
            picIndex = -1;
            break;

        case TOOSMALL:
            if(picIndex == -1)
                picIndex = SubMenu->addStaticPicture(0, 0, MAINFRAME_640_480);
            break;

        case CREATEWINDOW:
            if(!testWindow)
            {
                testWindow = global::s2->RegisterWindow(
                  std::make_unique<CWindow>(submenu1, TESTWINDOWQUITMESSAGE, Position(5, 5), Extent(350, 240), "Window", WINDOW_GREEN1,
                                            WINDOW_CLOSE | WINDOW_MOVE | WINDOW_MINIMIZE | WINDOW_RESIZE));
                testWindow->addText("Text inside the window", 10, 10, 14);
                testWindow->addButton(submenu1, -10, 150, 100, 210, 30, BUTTON_GREEN2, "Button inside the window");
                testWindowPicture = testWindow->addPicture(submenu1, TESTWINDOWPICTURE, 10, 60, MIS2BOBS_FORTRESS);
                testWindowPicture->setMotionParams(TESTWINDOWPICTUREENTRY, TESTWINDOWPICTURELEAVE);
                testTextfield_testWindow = testWindow->addTextfield(130, 30, 10, 3, 14, FontColor::Red, BUTTON_GREY, true);
                testTextfield_testWindow->setText(
                  "This is a very long test text in order to destroy the text field completely once and for all");
            }
            if(!testWindow2)
            {
                testWindow2 = global::s2->RegisterWindow(
                  std::make_unique<CWindow>(submenu1, TESTWINDOW2QUITMESSAGE, Position(200, 5), Extent(350, 240), "Another Window",
                                            WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MOVE | WINDOW_MINIMIZE | WINDOW_RESIZE));
                testWindow2->addText("Text inside the window", 50, 40, 9);
                testWindow2->addButton(submenu1, -10, 100, 100, 100, 20, BUTTON_GREEN2, "Button");
            }
            break;

        case GREATMOONENTRY:
            if(!greatMoonText)
                greatMoonText = SubMenu->addText("Test-Text", 100, 10, 14);
            break;

        case GREATMOONLEAVE:
            if(greatMoonText)
            {
                SubMenu->delText(greatMoonText);
                greatMoonText = nullptr;
            }
            break;

        case PICOBJECT:
            if(greatMoon)
            {
                SubMenu->delButton(greatMoon);
                greatMoon = nullptr;
            }
            break;

        case PICOBJECTENTRY:
            if(!greatMoonText)
                greatMoonText = SubMenu->addText("Test-Text", 100, 10, 14);
            break;

        case PICOBJECTLEAVE:
            if(greatMoonText)
            {
                SubMenu->delText(greatMoonText);
                greatMoonText = nullptr;
            }
            break;

        case TESTWINDOWPICTURE:
            assert(testWindow);
            if(!testWindowText)
                testWindowText = testWindow->addText("Clicked on castle", 10, 200, 11);
            else
            {
                testWindow->delText(testWindowText);
                testWindowText = nullptr;
            }
            break;

        case TESTWINDOWPICTUREENTRY:
            assert(testWindow);
            if(testWindowText2)
            {
                testWindow->delText(testWindowText2);
                testWindowText2 = nullptr;
            }
            testWindowText2 = testWindow->addText("Bildbereich betreten", 10, 220, 11);
            break;

        case TESTWINDOWPICTURELEAVE:
            assert(testWindow);
            if(testWindowText2)
            {
                testWindow->delText(testWindowText2);
                testWindowText2 = nullptr;
            }
            testWindowText2 = testWindow->addText("Bildbereich verlassen", 10, 220, 11);
            break;

        case TESTWINDOWQUITMESSAGE:
            assert(testWindow);
            testWindow->setWaste();
            testWindow = nullptr;
            break;

        case TESTWINDOW2QUITMESSAGE:
            assert(testWindow2);
            testWindow2->setWaste();
            testWindow2 = nullptr;
            break;

        case CALL_FROM_GAMELOOP:
            if(counter % 10 == 0)
            {
                if(counterText)
                {
                    if(SubMenu->delText(counterText))
                        counterText = nullptr;
                }
                if(!counterText)
                {
                    counterText = SubMenu->addText(helpers::format("counter: %d", counter), 100, 20, 9);
                }

                if(TextFrom_testTextfield)
                {
                    SubMenu->delText(TextFrom_testTextfield);
                    TextFrom_testTextfield = nullptr;
                }
                TextFrom_testTextfield = SubMenu->addText("Der Text im Textfeld lautet: " + testTextfield->getText(), 200, 400, 14);
            }
            counter++;
            break;

        default: break;
    }
}
#endif

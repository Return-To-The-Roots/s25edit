#include "callbacks.h"

void callback::PleaseWait(int Param)
{
    //NOTE: This "Please wait"-window is shown until the PleaseWait-callback is called with 'WINDOW_QUIT_MESSAGE'.
    //      The window will be registered by the game. To do it the other way (create and then let it automatically
    //      destroy by the gameloop), you don't need to register the window, but register the callback.

    static CWindow *WNDWait;

    enum
    {
        WINDOWQUIT
    };

    switch (Param)
    {
        case INITIALIZING_CALL: if (WNDWait != NULL)
                                    break;
                                WNDWait = new CWindow(PleaseWait, WINDOWQUIT, global::s2->getDisplaySurface()->w/2-106, global::s2->getDisplaySurface()->h/2-35, 212, 70, "Bitte warten");
                                if (global::s2->RegisterWindow(WNDWait))
                                {
                                    //we don't register this window cause we will destroy it manually if we need
                                    //global::s2->RegisterCallback(PleaseWait);

                                    WNDWait->addText("Bitte warten ...", 10, 10, 14);
                                    //we need to render this window NOW, cause the render loop will do it too late (when the operation
                                    //is done and we don't need the "Please wait"-window anymore)
                                    CSurface::Draw(global::s2->getDisplaySurface(), WNDWait->getSurface(), global::s2->getDisplaySurface()->w/2-106, global::s2->getDisplaySurface()->h/2-35);
                                    SDL_Flip(global::s2->getDisplaySurface());
                                }
                                else
                                {
                                    delete WNDWait;
                                    WNDWait = NULL;
                                    return;
                                }
                                break;

        case CALL_FROM_GAMELOOP:    //This window gives a "Please Wait"-string, so it is shown while there is an intensive operation
                                    //during ONE gameloop. Therefore it is only shown DURING this ONE operation. If the next gameloop
                                    //appears, the operation MUST have been finished and we can destroy this window.
                                    if (WNDWait != NULL)
                                    {
                                        global::s2->UnregisterCallback(PleaseWait);
                                        WNDWait->setWaste();
                                        WNDWait = NULL;
                                    }
                                    break;

        case WINDOW_QUIT_MESSAGE:   //this is the global window quit message, callback is explicit called with this value, so destroy the window
                                    if (WNDWait != NULL)
                                    {
                                        WNDWait->setWaste();
                                        WNDWait = NULL;
                                    }
                                    break;

        default:                break;
    }
}

void callback::mainmenu(int Param)
{
    static CMenu *MainMenu = NULL;

    enum
    {
        ENDGAME = 1,
        STARTEDITOR
    };

    switch (Param)
    {
        case INITIALIZING_CALL:
                    MainMenu = new CMenu(SPLASHSCREEN_MAINMENU);
                    if (!global::s2->RegisterMenu(MainMenu))
                    {
                        delete MainMenu;
                        MainMenu = NULL;
                        return;
                    }
                    MainMenu->addButton(mainmenu, ENDGAME, 50, 400, 200, 20, BUTTON_RED1, "Programm Verlassen");
                    #ifdef _ADMINMODE
                        MainMenu->addButton(submenu1, INITIALIZING_CALL, 50, 200, 200, 20, BUTTON_GREY, "Submenu_1");
                    #endif
                    MainMenu->addButton(mainmenu, STARTEDITOR, 50, 160, 200, 20, BUTTON_RED1, "Editor starten");
                    MainMenu->addButton(submenuOptions, INITIALIZING_CALL, 50, 370, 200, 20, BUTTON_GREEN2, "Optionen");
                    break;

        case CALL_FROM_GAMELOOP:
                    break;

        case ENDGAME:
                    MainMenu->setWaste();
                    MainMenu = NULL;
                    global::s2->Running = false;
                    break;

        case STARTEDITOR:
                    PleaseWait(INITIALIZING_CALL);
                    global::s2->setMapObj(new CMap(NULL));
                    MainMenu->setWaste();
                    MainMenu = NULL;
                    PleaseWait(WINDOW_QUIT_MESSAGE);
                    break;

        default:    break;
    }
}

void callback::submenuOptions(int Param)
{
    static CMenu *SubMenu = NULL;
    static CFont *TextResolution = NULL;
    static CButton *ButtonFullscreen = NULL;
    char puffer[80];
    char puffer2[80];
    const SDL_VideoInfo* videoinfo = SDL_GetVideoInfo();

    enum
    {
        MAINMENU = 1,
        RES_640,
        RES_800,
        RES_1024,
        RES_1280,
        RES_1680,
        RES_1280_720,
        FULLSCREEN
    };

    switch (Param)
    {
        case INITIALIZING_CALL:
                SubMenu = new CMenu(SPLASHSCREEN_SUBMENU3);
                if (!global::s2->RegisterMenu(SubMenu))
                {
                    delete SubMenu;
                    SubMenu = NULL;
                    return;
                }
                //add button for "back to main menu"
                SubMenu->addButton(submenuOptions, MAINMENU, (int)(global::s2->MenuResolutionX/2-100), 440, 200, 20, BUTTON_RED1, "zurück");
                //add menu title
                SubMenu->addText("Optionen", (int)(global::s2->MenuResolutionX/2-20), 10, 14);
                //add screen resolution
                if (TextResolution != NULL)
                    SubMenu->delText(TextResolution);
                sprintf(puffer, "Game Resolution: %d*%d / %s", global::s2->GameResolutionX, global::s2->GameResolutionY, (global::s2->fullscreen ? "Fullscreen" : "Window"));
                TextResolution = SubMenu->addText(puffer, (int)(global::s2->MenuResolutionX/2-110), 50, 11);
                //add buttons for resolutions
                SubMenu->addButton(submenuOptions, RES_640, (int)(global::s2->MenuResolutionX/2-100), 70, 200, 20, BUTTON_RED1, "640*480");
                SubMenu->addButton(submenuOptions, RES_800, (int)(global::s2->MenuResolutionX/2-100), 90, 200, 20, BUTTON_RED1, "800*600");
                SubMenu->addButton(submenuOptions, RES_1024, (int)(global::s2->MenuResolutionX/2-100), 110, 200, 20, BUTTON_RED1, "1024*768");
                SubMenu->addButton(submenuOptions, RES_1280_720, (int)(global::s2->MenuResolutionX/2-100), 130, 200, 20, BUTTON_RED1, "1280*720");
                SubMenu->addButton(submenuOptions, RES_1280, (int)(global::s2->MenuResolutionX/2-100), 150, 200, 20, BUTTON_RED1, "1280*1024");
                SubMenu->addButton(submenuOptions, RES_1680, (int)(global::s2->MenuResolutionX/2-100), 170, 200, 20, BUTTON_RED1, "1680*1050");
                if (ButtonFullscreen != NULL)
                    SubMenu->delButton(ButtonFullscreen);
                ButtonFullscreen = SubMenu->addButton(submenuOptions, FULLSCREEN, (int)(global::s2->MenuResolutionX/2-100), 190, 200, 20, BUTTON_RED1, (global::s2->fullscreen ? "WINDOW" : "FULLSCREEN"));
                //add video driver name
                SDL_VideoDriverName(puffer, 80);
                sprintf(puffer2, "Video-Treiber: %s", puffer);
                SubMenu->addText(puffer2, (int)(global::s2->MenuResolutionX/2-70), 195, 11);
                //add video memory
                sprintf(puffer, "Grafik-Speicher: %d MB", videoinfo->video_mem);
                SubMenu->addText(puffer, (int)(global::s2->MenuResolutionX/2-70), 210, 11);
                break;

        case MAINMENU:  SubMenu->setWaste();
                        SubMenu = NULL;
                        break;
        case RES_640:   global::s2->GameResolutionX = 640;
                        global::s2->GameResolutionY = 480;
                        if (TextResolution != NULL)
                            SubMenu->delText(TextResolution);
                        sprintf(puffer, "Game Resolution: %d*%d / %s", global::s2->GameResolutionX, global::s2->GameResolutionY, (global::s2->fullscreen ? "Fullscreen" : "Window"));
                        TextResolution = SubMenu->addText(puffer, (int)(global::s2->MenuResolutionX/2-110), 50, 11);
                        break;
        case RES_800:   global::s2->GameResolutionX = 800;
                        global::s2->GameResolutionY = 600;
                        if (TextResolution != NULL)
                            SubMenu->delText(TextResolution);
                        sprintf(puffer, "Game Resolution: %d*%d / %s", global::s2->GameResolutionX, global::s2->GameResolutionY, (global::s2->fullscreen ? "Fullscreen" : "Window"));
                        TextResolution = SubMenu->addText(puffer, (int)(global::s2->MenuResolutionX/2-110), 50, 11);
                        break;
        case RES_1024:  global::s2->GameResolutionX = 1024;
                        global::s2->GameResolutionY = 768;
                        if (TextResolution != NULL)
                            SubMenu->delText(TextResolution);
                        sprintf(puffer, "Game Resolution: %d*%d / %s", global::s2->GameResolutionX, global::s2->GameResolutionY, (global::s2->fullscreen ? "Fullscreen" : "Window"));
                        TextResolution = SubMenu->addText(puffer, (int)(global::s2->MenuResolutionX/2-110), 50, 11);
                        break;
        case RES_1280_720:  global::s2->GameResolutionX = 1280;
                            global::s2->GameResolutionY = 720;
                            if (TextResolution != NULL)
                                SubMenu->delText(TextResolution);
                            sprintf(puffer, "Game Resolution: %d*%d / %s", global::s2->GameResolutionX, global::s2->GameResolutionY, (global::s2->fullscreen ? "Fullscreen" : "Window"));
                            TextResolution = SubMenu->addText(puffer, (int)(global::s2->MenuResolutionX/2-110), 50, 11);
                            break;
        case RES_1280:  global::s2->GameResolutionX = 1280;
                        global::s2->GameResolutionY = 1024;
                        if (TextResolution != NULL)
                            SubMenu->delText(TextResolution);
                        sprintf(puffer, "Game Resolution: %d*%d / %s", global::s2->GameResolutionX, global::s2->GameResolutionY, (global::s2->fullscreen ? "Fullscreen" : "Window"));
                        TextResolution = SubMenu->addText(puffer, (int)(global::s2->MenuResolutionX/2-110), 50, 11);
                        break;
        case RES_1680:  global::s2->GameResolutionX = 1680;
                        global::s2->GameResolutionY = 1050;
                        if (TextResolution != NULL)
                            SubMenu->delText(TextResolution);
                        sprintf(puffer, "Game Resolution: %d*%d / %s", global::s2->GameResolutionX, global::s2->GameResolutionY, (global::s2->fullscreen ? "Fullscreen" : "Window"));
                        TextResolution = SubMenu->addText(puffer, (int)(global::s2->MenuResolutionX/2-110), 50, 11);
                        break;
        case FULLSCREEN:if (global::s2->fullscreen)
                            global::s2->fullscreen = false;
                        else
                            global::s2->fullscreen = true;
                        if (TextResolution != NULL)
                            SubMenu->delText(TextResolution);
                        sprintf(puffer, "Game Resolution: %d*%d / %s", global::s2->GameResolutionX, global::s2->GameResolutionY, (global::s2->fullscreen ? "Fullscreen" : "Window"));
                        TextResolution = SubMenu->addText(puffer, (int)(global::s2->MenuResolutionX/2-110), 50, 11);
                        if (ButtonFullscreen != NULL)
                            SubMenu->delButton(ButtonFullscreen);
                        ButtonFullscreen = SubMenu->addButton(submenuOptions, FULLSCREEN, (int)(global::s2->MenuResolutionX/2-100), 170, 200, 20, BUTTON_RED1, (global::s2->fullscreen ? "WINDOW" : "FULLSCREEN"));
                        break;

        default:    break;
    }
}

#ifdef _EDITORMODE
//now the editor callbacks will follow (editor mode)

void callback::EditorHelpMenu(int Param)
{
    //NOTE: This "Please wait"-window is shown until the PleaseWait-callback is called with 'WINDOW_QUIT_MESSAGE'.
    //      The window will be registered by the game. To do it the other way (create and then let it automatically
    //      destroy by the gameloop), you don't need to register the window, but register the callback.

    static CWindow *WNDHelp;

    enum
    {
        WINDOWQUIT
    };

    switch (Param)
    {
        case INITIALIZING_CALL: if (WNDHelp != NULL)
                                    break;
                                WNDHelp = new CWindow(EditorHelpMenu, WINDOWQUIT, global::s2->getDisplaySurface()->w/2-320, global::s2->getDisplaySurface()->h/2-240, 640, 480, "Hilfe", WINDOW_GREEN2, WINDOW_CLOSE | WINDOW_MOVE | WINDOW_RESIZE | WINDOW_MINIMIZE);
                                if (global::s2->RegisterWindow(WNDHelp))
                                {
                                    //we don't register this window cause we will destroy it manually if we need
                                    //global::s2->RegisterCallback(PleaseWait);

                                    WNDHelp->addText(   "Hilfe-Menu........................................................................................................F1\n"
                                                        "Fenster/Vollbild...............................................................................................F2\n"
                                                        "Zoom in/normal/out (experimentell)................................................................F5/F6/F7\n"
                                                        "Scrollen............................................................................................................Pfeiltasten\n"
                                                        "Cursorgrösse 1-9 (von 11).................................................................................1-9\n"
                                                        "Cursor vergrössern/verkleinern..................................................................+/-\n"
                                                        "Scheren-Modus................................................................................................Strg\n"
                                                        "Modus umkehren..............................................................................................Shift\n"
                                                        "(bspw. Höhe senken, Spieler entfernen, Ressourcen senken)\n"
                                                        "Planiermodus....................................................................................................Alt\n"
                                                        "Maximal-Höhe senken/standard/erhöhen......................................................Einf/Pos1/BildAuf\n"
                                                        "(über dies kann dann nicht erhöht werden)\n"
                                                        "Minimal-Höhe senken/standard/erhöhen.......................................................Entf/Ende/BildAb\n"
                                                        "(unter dies kann dann nicht gesenkt werden)\n"
                                                        "Rückgängig.......................................................................................................Q\n"
                                                        "(nur Aktionen, die mit dem Cursor durchgeführt wurden)\n"
                                                        "Bauhilfe an/aus................................................................................................Leertaste\n"
                                                        "Schloss-Modus..................................................................................................B\n"
                                                        "(das umliegende Gelände wird so geebnet,\n"
                                                        " dass ein grosses Haus gebaut werden kann)\n"
                                                        "Hafen-Modus.....................................................................................................H\n"
                                                        "(das umliegende Gelände wird so verändert,\n"
                                                        " dass ein Hafen gebaut werden kann)\n"
                                                        "Map \"on-the-fly\" konvertieren (Grün-/Winter-/Ödland).................................G/W/O\n"
                                                        "Neue/Originale Schattierung (experimentell).................................................P\n"
                                                        "Horizontale Bewegung sperren/entsperren...................................................F9\n"
                                                        "Vertikale Bewegung sperren/entsperren......................................................F10\n"
                                                        "Ränder an-/abschalten....................................................................................F11\n"
                                                    , 10, 10, 11);
                                }
                                else
                                {
                                    delete WNDHelp;
                                    WNDHelp = NULL;
                                    return;
                                }
                                break;

        case CALL_FROM_GAMELOOP:    break;

        case WINDOW_QUIT_MESSAGE:   //this is the global window quit message, callback is explicit called with this value, so destroy the window
                                    if (WNDHelp != NULL)
                                    {
                                        WNDHelp->setWaste();
                                        WNDHelp = NULL;
                                    }
                                    break;

        case WINDOWQUIT:            //this is the own window quit message of the callback
                                    if (WNDHelp != NULL)
                                    {
                                        WNDHelp->setWaste();
                                        WNDHelp = NULL;
                                    }
                                    break;

        case MAP_QUIT:              //this is the global window quit message, callback is explicit called with this value, so destroy the window
                                    if (WNDHelp != NULL)
                                    {
                                        WNDHelp->setWaste();
                                        WNDHelp = NULL;
                                    }
                                    break;

        default:                break;
    }
}

void callback::EditorMainMenu(int Param)
{
    static CWindow *WNDMain = NULL;

    enum
    {
        LOADMENU,
        SAVEMENU,
        QUITMENU,
        WINDOWQUIT
    };

    switch (Param)
    {
        case INITIALIZING_CALL:
                    if (WNDMain != NULL)
                        break;
                    WNDMain = new CWindow(EditorMainMenu, WINDOWQUIT, global::s2->GameResolutionX/2-110, global::s2->GameResolutionY/2-160, 220, 320, "Hauptmenu", WINDOW_GREEN1, WINDOW_CLOSE);
                    if (global::s2->RegisterWindow(WNDMain))
                    {
                        WNDMain->addButton(EditorMainMenu, LOADMENU, 8, 100, 190, 20, BUTTON_GREEN2, "Karte laden");
                        WNDMain->addButton(EditorMainMenu, SAVEMENU, 8, 125, 190, 20, BUTTON_GREEN2, "Karte speichern");

                        WNDMain->addButton(EditorMainMenu, QUITMENU, 8, 260, 190, 20, BUTTON_GREEN2, "Editor verlassen");
                    }
                    else
                    {
                        delete WNDMain;
                        WNDMain = NULL;
                        return;
                    }
                    break;

        case WINDOWQUIT:
                if (WNDMain != NULL)
                {
                    WNDMain->setWaste();
                    WNDMain = NULL;
                }
                break;

        case MAP_QUIT:
                if (WNDMain != NULL)
                {
                    WNDMain->setWaste();
                    WNDMain = NULL;
                }
                break;

        case QUITMENU:
                    EditorQuitMenu(INITIALIZING_CALL);
                    break;

        case LOADMENU:
                    EditorLoadMenu(INITIALIZING_CALL);
                    break;

        case SAVEMENU:
                    EditorSaveMenu(INITIALIZING_CALL);
                    break;

        default:    break;
    }
}

void callback::EditorLoadMenu(int Param)
{
    static CWindow *WNDLoad = NULL;
    static CTextfield *TXTF_Filename = NULL;
    static CMap* MapObj = NULL;

    enum
    {
        LOADMAP,
        WINDOWQUIT
    };

    switch (Param)
    {
        case INITIALIZING_CALL:
                    if (WNDLoad != NULL)
                        break;
                    WNDLoad = new CWindow(EditorLoadMenu, WINDOWQUIT, global::s2->GameResolutionX/2-140, global::s2->GameResolutionY/2-45, 280, 120, "Laden", WINDOW_GREEN1, WINDOW_CLOSE);
                    if (global::s2->RegisterWindow(WNDLoad))
                    {
                        MapObj = global::s2->getMapObj();

                        TXTF_Filename = WNDLoad->addTextfield(10, 10, 21, 1);
                        TXTF_Filename->setText("WORLDS/");
                        WNDLoad->addButton(EditorLoadMenu, LOADMAP, 170, 40, 90, 20, BUTTON_GREY, "Laden");
                        WNDLoad->addButton(EditorLoadMenu, WINDOWQUIT, 170, 65, 90, 20, BUTTON_RED1, "Abbrechen");
                    }
                    else
                    {
                        delete WNDLoad;
                        WNDLoad = NULL;
                        return;
                    }
                    break;

        case WINDOWQUIT:
                if (WNDLoad != NULL)
                {
                    WNDLoad->setWaste();
                    WNDLoad = NULL;
                }
                TXTF_Filename = NULL;
                break;

        case MAP_QUIT:
                if (WNDLoad != NULL)
                {
                    WNDLoad->setWaste();
                    WNDLoad = NULL;
                }
                TXTF_Filename = NULL;
                break;

        case LOADMAP:
                    PleaseWait(INITIALIZING_CALL);

                    //we have to close the windows and initialize them again to prevent failures
                    EditorCursorMenu(MAP_QUIT);
                    EditorTextureMenu(MAP_QUIT);
                    EditorTreeMenu(MAP_QUIT);
                    EditorLandscapeMenu(MAP_QUIT);
                    MinimapMenu(MAP_QUIT);
                    EditorResourceMenu(MAP_QUIT);
                    EditorAnimalMenu(MAP_QUIT);
                    EditorPlayerMenu(MAP_QUIT);

                    MapObj->destructMap();
                    MapObj->constructMap((char*)TXTF_Filename->getText());

                    //we need to check which of these windows was active before
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

        default:    break;
    }
}

void callback::EditorSaveMenu(int Param)
{
    static CWindow *WNDSave = NULL;
    static CTextfield *TXTF_Filename = NULL;
    static CTextfield *TXTF_Mapname = NULL;
    static CTextfield *TXTF_Author = NULL;
    static CMap* MapObj = NULL;

    enum
    {
        SAVEMAP,
        WINDOWQUIT
    };

    switch (Param)
    {
        case INITIALIZING_CALL:
                    if (WNDSave != NULL)
                        break;
                    WNDSave = new CWindow(EditorSaveMenu, WINDOWQUIT, global::s2->GameResolutionX/2-140, global::s2->GameResolutionY/2-100, 280, 200, "Speichern", WINDOW_GREEN1, WINDOW_CLOSE);
                    if (global::s2->RegisterWindow(WNDSave))
                    {
                        MapObj = global::s2->getMapObj();

                        WNDSave->addText("Dateiname", 100, 2, 9);
                        TXTF_Filename = WNDSave->addTextfield(10, 13, 21, 1);
                        TXTF_Filename->setText("WORLDS/");
                        WNDSave->addText("Kartenname", 98, 38, 9);
                        TXTF_Mapname = WNDSave->addTextfield(10, 50, 19, 1);
                        TXTF_Mapname->setText(MapObj->getMapname());
                        WNDSave->addText("Author", 110, 75, 9);
                        TXTF_Author = WNDSave->addTextfield(10, 87, 19, 1);
                        TXTF_Author->setText(MapObj->getAuthor());
                        WNDSave->addButton(EditorSaveMenu, SAVEMAP, 170, 120, 90, 20, BUTTON_GREY, "Speichern");
                        WNDSave->addButton(EditorSaveMenu, WINDOWQUIT, 170, 145, 90, 20, BUTTON_RED1, "Abbrechen");
                    }
                    else
                    {
                        delete WNDSave;
                        WNDSave = NULL;
                        return;
                    }
                    break;

        case WINDOWQUIT:
                if (WNDSave != NULL)
                {
                    WNDSave->setWaste();
                    WNDSave = NULL;
                }
                TXTF_Filename = NULL;
                TXTF_Mapname = NULL;
                TXTF_Author = NULL;
                break;

        case MAP_QUIT:
                if (WNDSave != NULL)
                {
                    WNDSave->setWaste();
                    WNDSave = NULL;
                }
                TXTF_Filename = NULL;
                TXTF_Mapname = NULL;
                TXTF_Author = NULL;
                break;

        case SAVEMAP:
                    PleaseWait(INITIALIZING_CALL);

                    MapObj->setMapname((char*)TXTF_Mapname->getText());
                    MapObj->setAuthor((char*)TXTF_Author->getText());
                    CFile::save_file((char*)TXTF_Filename->getText(), WLD, MapObj->getMap());

                    PleaseWait(WINDOW_QUIT_MESSAGE);
                    EditorSaveMenu(WINDOWQUIT);
                    break;

        default:    break;
    }
}

void callback::EditorQuitMenu(int Param)
{
    static CWindow *WNDBackToMainMenu = NULL;

    enum
    {
        BACKTOMAIN = 1,
        NOTBACKTOMAIN,
        WINDOWQUIT
    };

    switch (Param)
    {
        case INITIALIZING_CALL:
                    if (WNDBackToMainMenu != NULL)
                        break;
                    WNDBackToMainMenu = new CWindow(EditorQuitMenu, WINDOWQUIT, global::s2->GameResolutionX/2-106, global::s2->GameResolutionY/2-55, 212, 110, "Beenden?");
                    if (global::s2->RegisterWindow(WNDBackToMainMenu))
                    {
                        WNDBackToMainMenu->addButton(EditorQuitMenu, BACKTOMAIN, 0, 0, 100, 80, BUTTON_GREEN2, NULL, PICTURE_SMALL_TICK);
                        WNDBackToMainMenu->addButton(EditorQuitMenu, NOTBACKTOMAIN, 100, 0, 100, 80, BUTTON_RED1, NULL, PICTURE_SMALL_CROSS);
                    }
                    else
                    {
                        delete WNDBackToMainMenu;
                        WNDBackToMainMenu = NULL;
                        return;
                    }
                    break;

        case BACKTOMAIN:
                    if (global::s2->getMapObj() != NULL)
                        global::s2->delMapObj();
                    WNDBackToMainMenu->setWaste();
                    WNDBackToMainMenu = NULL;
                    //now call all EditorMenu callbacks (from the menubar) with MAP_QUIT
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
                    //go to main menu
                    mainmenu(INITIALIZING_CALL);
                    break;

        case NOTBACKTOMAIN:
                    if (WNDBackToMainMenu != NULL)
                    {
                        WNDBackToMainMenu->setWaste();
                        WNDBackToMainMenu = NULL;
                    }
                    break;

        default:    break;
    }
}

void callback::EditorTextureMenu(int Param)
{
    static CWindow *WNDTexture = NULL;
    static CMap* MapObj = NULL;
    static bobMAP *map = NULL;
    static int textureIndex = 0;
    static int harbourPictureCross = 0; //this have to be -1 if we use the harbour button
    static int lastContent = 0x00;
    static int PosX = 0, PosY = 0;

    enum
    {
        WINDOWQUIT,
        HARBOUR,
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

    switch (Param)
    {
        case INITIALIZING_CALL:
                    if (WNDTexture != NULL)
                        break;
                    WNDTexture = new CWindow(EditorTextureMenu, WINDOWQUIT, PosX, PosY, 220, 133, "Terrain", WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MINIMIZE | WINDOW_MOVE);
                    if (global::s2->RegisterWindow(WNDTexture))
                    {
                        MapObj = global::s2->getMapObj();
                        map = MapObj->getMap();
                        switch (map->type)
                        {
                            case MAP_GREENLAND: textureIndex = PICTURE_GREENLAND_TEXTURE_SNOW;
                                                break;
                            case MAP_WASTELAND: textureIndex = PICTURE_WASTELAND_TEXTURE_SNOW;
                                                break;
                            case MAP_WINTERLAND: textureIndex = PICTURE_WINTERLAND_TEXTURE_SNOW;
                                                break;
                            default:            textureIndex = PICTURE_GREENLAND_TEXTURE_SNOW;
                                                break;
                        }
                        MapObj->setMode(EDITOR_MODE_TEXTURE);
                        MapObj->setModeContent(TRIANGLE_TEXTURE_SNOW);
                        lastContent = TRIANGLE_TEXTURE_SNOW;

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
                        if (map->type != MAP_WASTELAND)
                            WNDTexture->addPicture(EditorTextureMenu, PICMEADOW_MIXED, 138, 70, textureIndex);

                        //WNDTexture->addButton(EditorTextureMenu, HARBOUR, 172, 70, 32, 32, BUTTON_GREY, NULL, MAPPIC_HOUSE_HARBOUR);
                        //harbourPictureCross = WNDTexture->addStaticPicture(185, 80, PICTURE_SMALL_CROSS);
                    }
                    else
                    {
                        delete WNDTexture;
                        WNDTexture = NULL;
                        return;
                    }
                    break;

        case HARBOUR:           //harbour mode is active
                                if (harbourPictureCross == -1)
                                {
                                    if (   MapObj->getModeContent() == TRIANGLE_TEXTURE_STEPPE_MEADOW1_HARBOUR || MapObj->getModeContent() == TRIANGLE_TEXTURE_MEADOW1_HARBOUR || MapObj->getModeContent() == TRIANGLE_TEXTURE_MEADOW2_HARBOUR
                                        || MapObj->getModeContent() == TRIANGLE_TEXTURE_MEADOW3_HARBOUR || MapObj->getModeContent() == TRIANGLE_TEXTURE_STEPPE_MEADOW2_HARBOUR || MapObj->getModeContent() == TRIANGLE_TEXTURE_FLOWER_HARBOUR
                                        || MapObj->getModeContent() == TRIANGLE_TEXTURE_MINING_MEADOW_HARBOUR || MapObj->getModeContent() == TRIANGLE_TEXTURE_MEADOW_MIXED_HARBOUR)
                                    {
                                        harbourPictureCross = WNDTexture->addStaticPicture(185, 80, PICTURE_SMALL_CROSS);
                                        MapObj->setModeContent(MapObj->getModeContent()-0x40);
                                        lastContent = MapObj->getModeContent();
                                    }
                                }
                                //harbour mode is inactive
                                else
                                {
                                    if (   MapObj->getModeContent() == TRIANGLE_TEXTURE_STEPPE_MEADOW1 || MapObj->getModeContent() == TRIANGLE_TEXTURE_MEADOW1 || MapObj->getModeContent() == TRIANGLE_TEXTURE_MEADOW2
                                        || MapObj->getModeContent() == TRIANGLE_TEXTURE_MEADOW3 || MapObj->getModeContent() == TRIANGLE_TEXTURE_STEPPE_MEADOW2 || MapObj->getModeContent() == TRIANGLE_TEXTURE_FLOWER
                                        || MapObj->getModeContent() == TRIANGLE_TEXTURE_MINING_MEADOW || MapObj->getModeContent() == TRIANGLE_TEXTURE_MEADOW_MIXED)
                                    {
                                        WNDTexture->delStaticPicture(harbourPictureCross);
                                        harbourPictureCross = -1;
                                        MapObj->setModeContent(MapObj->getModeContent()+0x40);
                                        lastContent = MapObj->getModeContent();
                                    }
                                }
                                break;
        case PICSNOW:           MapObj->setModeContent(TRIANGLE_TEXTURE_SNOW);
                                lastContent = TRIANGLE_TEXTURE_SNOW;
                                if (harbourPictureCross == -1)
                                    harbourPictureCross = WNDTexture->addStaticPicture(185, 80, PICTURE_SMALL_CROSS);
                                break;
        case PICSTEPPE:         MapObj->setModeContent(TRIANGLE_TEXTURE_STEPPE);
                                lastContent = TRIANGLE_TEXTURE_STEPPE;
                                if (harbourPictureCross == -1)
                                    harbourPictureCross = WNDTexture->addStaticPicture(185, 80, PICTURE_SMALL_CROSS);
                                break;
        case PICSWAMP:          MapObj->setModeContent(TRIANGLE_TEXTURE_SWAMP);
                                lastContent = TRIANGLE_TEXTURE_SWAMP;
                                if (harbourPictureCross == -1)
                                    harbourPictureCross = WNDTexture->addStaticPicture(185, 80, PICTURE_SMALL_CROSS);
                                break;
        case PICFLOWER:         if (harbourPictureCross == -1)
                                {
                                    MapObj->setModeContent(TRIANGLE_TEXTURE_FLOWER_HARBOUR);
                                    lastContent = TRIANGLE_TEXTURE_FLOWER_HARBOUR;
                                }
                                else
                                {
                                    MapObj->setModeContent(TRIANGLE_TEXTURE_FLOWER);
                                    lastContent = TRIANGLE_TEXTURE_FLOWER;
                                }
                                break;
        case PICMINING1:        MapObj->setModeContent(TRIANGLE_TEXTURE_MINING1);
                                lastContent = TRIANGLE_TEXTURE_MINING1;
                                if (harbourPictureCross == -1)
                                    harbourPictureCross = WNDTexture->addStaticPicture(185, 80, PICTURE_SMALL_CROSS);
                                break;
        case PICMINING2:        MapObj->setModeContent(TRIANGLE_TEXTURE_MINING2);
                                lastContent = TRIANGLE_TEXTURE_MINING2;
                                if (harbourPictureCross == -1)
                                    harbourPictureCross = WNDTexture->addStaticPicture(185, 80, PICTURE_SMALL_CROSS);
                                break;
        case PICMINING3:        MapObj->setModeContent(TRIANGLE_TEXTURE_MINING3);
                                lastContent = TRIANGLE_TEXTURE_MINING3;
                                if (harbourPictureCross == -1)
                                    harbourPictureCross = WNDTexture->addStaticPicture(185, 80, PICTURE_SMALL_CROSS);
                                break;
        case PICMINING4:        MapObj->setModeContent(TRIANGLE_TEXTURE_MINING4);
                                lastContent = TRIANGLE_TEXTURE_MINING4;
                                if (harbourPictureCross == -1)
                                    harbourPictureCross = WNDTexture->addStaticPicture(185, 80, PICTURE_SMALL_CROSS);
                                break;
        case PICSTEPPE_MEADOW1: if (harbourPictureCross == -1)
                                {
                                    MapObj->setModeContent(TRIANGLE_TEXTURE_STEPPE_MEADOW1_HARBOUR);
                                    lastContent = TRIANGLE_TEXTURE_STEPPE_MEADOW1_HARBOUR;
                                }
                                else
                                {
                                    MapObj->setModeContent(TRIANGLE_TEXTURE_STEPPE_MEADOW1);
                                    lastContent = TRIANGLE_TEXTURE_STEPPE_MEADOW1;
                                }
                                break;
        case PICMEADOW1:        if (harbourPictureCross == -1)
                                {
                                    MapObj->setModeContent(TRIANGLE_TEXTURE_MEADOW1_HARBOUR);
                                    lastContent = TRIANGLE_TEXTURE_MEADOW1_HARBOUR;
                                }
                                else
                                {
                                    MapObj->setModeContent(TRIANGLE_TEXTURE_MEADOW1);
                                    lastContent = TRIANGLE_TEXTURE_MEADOW1;
                                }
                                break;
        case PICMEADOW2:        if (harbourPictureCross == -1)
                                {
                                    MapObj->setModeContent(TRIANGLE_TEXTURE_MEADOW2_HARBOUR);
                                    lastContent = TRIANGLE_TEXTURE_MEADOW2_HARBOUR;
                                }
                                else
                                {
                                    MapObj->setModeContent(TRIANGLE_TEXTURE_MEADOW2);
                                    lastContent = TRIANGLE_TEXTURE_MEADOW2;
                                }
                                break;
        case PICMEADOW3:        if (harbourPictureCross == -1)
                                {
                                    MapObj->setModeContent(TRIANGLE_TEXTURE_MEADOW3_HARBOUR);
                                    lastContent = TRIANGLE_TEXTURE_MEADOW3_HARBOUR;
                                }
                                else
                                {
                                    MapObj->setModeContent(TRIANGLE_TEXTURE_MEADOW3);
                                    lastContent = TRIANGLE_TEXTURE_MEADOW3;
                                }
                                break;
        case PICSTEPPE_MEADOW2: if (harbourPictureCross == -1)
                                {
                                    MapObj->setModeContent(TRIANGLE_TEXTURE_STEPPE_MEADOW2_HARBOUR);
                                    lastContent = TRIANGLE_TEXTURE_STEPPE_MEADOW2_HARBOUR;
                                }
                                else
                                {
                                    MapObj->setModeContent(TRIANGLE_TEXTURE_STEPPE_MEADOW2);
                                    lastContent = TRIANGLE_TEXTURE_STEPPE_MEADOW2;
                                }
                                break;
        case PICMINING_MEADOW:  if (harbourPictureCross == -1)
                                {
                                    MapObj->setModeContent(TRIANGLE_TEXTURE_MINING_MEADOW_HARBOUR);
                                    lastContent = TRIANGLE_TEXTURE_MINING_MEADOW_HARBOUR;
                                }
                                else
                                {
                                    MapObj->setModeContent(TRIANGLE_TEXTURE_MINING_MEADOW);
                                    lastContent = TRIANGLE_TEXTURE_MINING_MEADOW;
                                }
                                break;
        case PICWATER:          MapObj->setModeContent(TRIANGLE_TEXTURE_WATER);
                                lastContent = TRIANGLE_TEXTURE_WATER;
                                if (harbourPictureCross == -1)
                                    harbourPictureCross = WNDTexture->addStaticPicture(185, 80, PICTURE_SMALL_CROSS);
                                break;
        case PICLAVA:           MapObj->setModeContent(TRIANGLE_TEXTURE_LAVA);
                                lastContent = TRIANGLE_TEXTURE_LAVA;
                                if (harbourPictureCross == -1)
                                    harbourPictureCross = WNDTexture->addStaticPicture(185, 80, PICTURE_SMALL_CROSS);
                                break;
        case PICMEADOW_MIXED:   if (harbourPictureCross == -1)
                                {
                                    MapObj->setModeContent(TRIANGLE_TEXTURE_MEADOW_MIXED_HARBOUR);
                                    lastContent = TRIANGLE_TEXTURE_MEADOW_MIXED_HARBOUR;
                                }
                                else
                                {
                                    MapObj->setModeContent(TRIANGLE_TEXTURE_MEADOW_MIXED);
                                    lastContent = TRIANGLE_TEXTURE_MEADOW_MIXED;
                                }
                                break;

        case WINDOW_CLICKED_CALL:   if (MapObj != NULL)
                                    {
                                        MapObj->setMode(EDITOR_MODE_TEXTURE);
                                        MapObj->setModeContent(lastContent);
                                    }
                                    break;

        case WINDOWQUIT:
                    if (WNDTexture != NULL)
                    {
                        PosX = WNDTexture->getX();
                        PosY = WNDTexture->getY();
                        WNDTexture->setWaste();
                        WNDTexture = NULL;
                    }
                    MapObj->setMode(EDITOR_MODE_HEIGHT_RAISE);
                    MapObj->setModeContent(0x00);
                    lastContent = 0x00;
                    MapObj = NULL;
                    map = NULL;
                    textureIndex = 0;
                    harbourPictureCross = 0; //this have to be -1 if we use the harbour button
                    break;

        case MAP_QUIT:
                    //we do the same like in case WINDOWQUIT, but we won't setMode(EDITOR_MODE_HEIGHT_RAISE), cause map is dead
                    if (WNDTexture != NULL)
                    {
                        PosX = WNDTexture->getX();
                        PosY = WNDTexture->getY();
                        WNDTexture->setWaste();
                        WNDTexture = NULL;
                    }
                    lastContent = 0x00;
                    MapObj = NULL;
                    map = NULL;
                    textureIndex = 0;
                    harbourPictureCross = 0; //this have to be -1 if we use the harbour button
                    break;

        default:    break;
    }
}

void callback::EditorTreeMenu(int Param)
{
    static CWindow *WNDTree = NULL;
    static CMap* MapObj = NULL;
    static bobMAP *map = NULL;
    static int lastContent = 0x00;
    static int lastContent2 = 0x00;
    static int PosX = 230, PosY = 0;

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

    switch (Param)
    {
        case INITIALIZING_CALL:
                    if (WNDTree != NULL)
                        break;
                    WNDTree = new CWindow(EditorTreeMenu, WINDOWQUIT, PosX, PosY, 148, 140, "Bäume", WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MINIMIZE | WINDOW_MOVE);
                    if (global::s2->RegisterWindow(WNDTree))
                    {
                        MapObj = global::s2->getMapObj();
                        map = MapObj->getMap();
                        switch (map->type)
                        {
                            case MAP_GREENLAND: WNDTree->addPicture(EditorTreeMenu, PICPINE, 2, 2, PICTURE_TREE_PINE);
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
                            case MAP_WASTELAND: WNDTree->addPicture(EditorTreeMenu, PICFLAPHAT, 2, 2, PICTURE_TREE_FLAPHAT);
                                                WNDTree->addPicture(EditorTreeMenu, PICSPIDER, 36, 2, PICTURE_TREE_SPIDER);
                                                WNDTree->addPicture(EditorTreeMenu, PICPINEAPPLE, 70, 2, PICTURE_TREE_PINEAPPLE);
                                                WNDTree->addPicture(EditorTreeMenu, PICCHERRY, 104, 2, PICTURE_TREE_CHERRY);
                                                break;
                            case MAP_WINTERLAND:WNDTree->addPicture(EditorTreeMenu, PICPINE, 2, 2, PICTURE_TREE_PINE);
                                                WNDTree->addPicture(EditorTreeMenu, PICBIRCH, 36, 2, PICTURE_TREE_BIRCH);
                                                WNDTree->addPicture(EditorTreeMenu, PICCYPRESS, 70, 2, PICTURE_TREE_CYPRESS);
                                                WNDTree->addPicture(EditorTreeMenu, PICFIR, 104, 2, PICTURE_TREE_FIR);
                                                WNDTree->addPicture(EditorTreeMenu, PICWOOD_MIXED, 2, 36, PICTURE_TREE_WOOD_MIXED);
                                                break;
                            default:            //should not happen
                                                break;
                        }
                        MapObj->setMode(EDITOR_MODE_TREE);
                        MapObj->setModeContent(0x30);
                        MapObj->setModeContent2(0xC4);
                        lastContent = 0x30;
                        lastContent2 = 0xC4;
                    }
                    else
                    {
                        delete WNDTree;
                        WNDTree = NULL;
                        return;
                    }
                    break;

        case PICPINE:           MapObj->setModeContent(0x30);
                                MapObj->setModeContent2(0xC4);
                                lastContent = 0x30;
                                lastContent2 = 0xC4;
                                break;
        case PICBIRCH:          MapObj->setModeContent(0x70);
                                MapObj->setModeContent2(0xC4);
                                lastContent = 0x70;
                                lastContent2 = 0xC4;
                                break;
        case PICOAK:            MapObj->setModeContent(0xB0);
                                MapObj->setModeContent2(0xC4);
                                lastContent = 0xB0;
                                lastContent2 = 0xC4;
                                break;
        case PICPALM1:          MapObj->setModeContent(0xF0);
                                MapObj->setModeContent2(0xC4);
                                lastContent = 0xF0;
                                lastContent2 = 0xC4;
                                break;
        case PICPALM2:          MapObj->setModeContent(0x30);
                                MapObj->setModeContent2(0xC5);
                                lastContent = 0x30;
                                lastContent2 = 0xC5;
                                break;
        case PICPINEAPPLE:      MapObj->setModeContent(0x70);
                                MapObj->setModeContent2(0xC5);
                                lastContent = 0x70;
                                lastContent2 = 0xC5;
                                break;
        case PICCYPRESS:        MapObj->setModeContent(0xB0);
                                MapObj->setModeContent2(0xC5);
                                lastContent = 0xB0;
                                lastContent2 = 0xC5;
                                break;
        case PICCHERRY:         MapObj->setModeContent(0xF0);
                                MapObj->setModeContent2(0xC5);
                                lastContent = 0xF0;
                                lastContent2 = 0xC5;
                                break;
        case PICFIR:            MapObj->setModeContent(0x30);
                                MapObj->setModeContent2(0xC6);
                                lastContent = 0x30;
                                lastContent2 = 0xC6;
                                break;
        case PICFLAPHAT:        MapObj->setModeContent(0x70);
                                MapObj->setModeContent2(0xC4);
                                lastContent = 0x70;
                                lastContent2 = 0xC4;
                                break;
        case PICSPIDER:         MapObj->setModeContent(0x30);
                                MapObj->setModeContent2(0xC4);
                                lastContent = 0x30;
                                lastContent2 = 0xC4;
                                break;
        case PICWOOD_MIXED:     MapObj->setModeContent(0xFF);
                                MapObj->setModeContent2(0xC4);
                                lastContent = 0xFF;
                                lastContent2 = 0xC4;
                                break;
        case PICPALM_MIXED:     MapObj->setModeContent(0xFF);
                                MapObj->setModeContent2(0xC5);
                                lastContent = 0xFF;
                                lastContent2 = 0xC5;
                                break;

        case WINDOW_CLICKED_CALL:   if (MapObj != NULL)
                                    {
                                        MapObj->setMode(EDITOR_MODE_TREE);
                                        MapObj->setModeContent(lastContent);
                                        MapObj->setModeContent2(lastContent2);
                                    }
                                    break;

        case WINDOWQUIT:
                    if (WNDTree != NULL)
                    {
                        PosX = WNDTree->getX();
                        PosY = WNDTree->getY();
                        WNDTree->setWaste();
                        WNDTree = NULL;
                    }
                    MapObj->setMode(EDITOR_MODE_HEIGHT_RAISE);
                    MapObj->setModeContent(0x00);
                    MapObj->setModeContent2(0x00);
                    lastContent = 0x00;
                    lastContent2 = 0x00;
                    MapObj = NULL;
                    map = NULL;
                    break;

        case MAP_QUIT:
                    //we do the same like in case WINDOWQUIT, but we won't setMode(EDITOR_MODE_HEIGHT_RAISE), cause map is dead
                    if (WNDTree != NULL)
                    {
                        PosX = WNDTree->getX();
                        PosY = WNDTree->getY();
                        WNDTree->setWaste();
                        WNDTree = NULL;
                    }
                    lastContent = 0x00;
                    lastContent2 = 0x00;
                    MapObj = NULL;
                    map = NULL;
                    break;

        default:    break;
    }
}

void callback::EditorResourceMenu(int Param)
{
    static CWindow *WNDResource = NULL;
    static CMap* MapObj = NULL;
    static int lastContent = 0x00;
    static int PosX = 0, PosY = 140;

    enum
    {
        WINDOWQUIT,
        PICGOLD,
        PICORE,
        PICCOAL,
        PICGRANITE
    };

    switch (Param)
    {
        case INITIALIZING_CALL:
                    if (WNDResource != NULL)
                        break;
                    WNDResource = new CWindow(EditorResourceMenu, WINDOWQUIT, PosX, PosY, 148, 55, "Rohstoffe", WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MINIMIZE | WINDOW_MOVE);
                    if (global::s2->RegisterWindow(WNDResource))
                    {
                        MapObj = global::s2->getMapObj();

                        WNDResource->addPicture(EditorResourceMenu, PICGOLD, 2, 2, PICTURE_RESOURCE_GOLD);
                        WNDResource->addPicture(EditorResourceMenu, PICORE, 36, 2, PICTURE_RESOURCE_ORE);
                        WNDResource->addPicture(EditorResourceMenu, PICCOAL, 70, 2, PICTURE_RESOURCE_COAL);
                        WNDResource->addPicture(EditorResourceMenu, PICGRANITE, 104, 2, PICTURE_RESOURCE_GRANITE);

                        MapObj->setMode(EDITOR_MODE_RESOURCE_RAISE);
                        MapObj->setModeContent(0x51);
                        lastContent = 0x51;
                    }
                    else
                    {
                        delete WNDResource;
                        WNDResource = NULL;
                        return;
                    }
                    break;

        case PICGOLD:           MapObj->setModeContent(0x51);
                                lastContent = 0x51;
                                break;
        case PICORE:            MapObj->setModeContent(0x49);
                                lastContent = 0x49;
                                break;
        case PICCOAL:           MapObj->setModeContent(0x41);
                                lastContent = 0x41;
                                break;
        case PICGRANITE:        MapObj->setModeContent(0x59);
                                lastContent = 0x59;
                                break;

        case WINDOW_CLICKED_CALL:   if (MapObj != NULL)
                                    {
                                        MapObj->setMode(EDITOR_MODE_RESOURCE_RAISE);
                                        MapObj->setModeContent(lastContent);
                                    }
                                    break;

        case WINDOWQUIT:
                    if (WNDResource != NULL)
                    {
                        PosX = WNDResource->getX();
                        PosY = WNDResource->getY();
                        WNDResource->setWaste();
                        WNDResource = NULL;
                    }
                    MapObj->setMode(EDITOR_MODE_HEIGHT_RAISE);
                    MapObj->setModeContent(0x00);
                    lastContent = 0x00;
                    MapObj = NULL;
                    break;

        case MAP_QUIT:
                    //we do the same like in case WINDOWQUIT, but we won't setMode(EDITOR_MODE_HEIGHT_RAISE), cause map is dead
                    if (WNDResource != NULL)
                    {
                        PosX = WNDResource->getX();
                        PosY = WNDResource->getY();
                        WNDResource->setWaste();
                        WNDResource = NULL;
                    }
                    lastContent = 0x00;
                    MapObj = NULL;
                    break;

        default:    break;
    }
}

void callback::EditorLandscapeMenu(int Param)
{
    static CWindow *WNDLandscape = NULL;
    static CMap* MapObj = NULL;
    static bobMAP *map = NULL;
    static int lastContent = 0x00;
    static int lastContent2 = 0x00;
    static int PosX = 390, PosY = 0;

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

    switch (Param)
    {
        case INITIALIZING_CALL:
                    if (WNDLandscape != NULL)
                        break;
                    WNDLandscape = new CWindow(EditorLandscapeMenu, WINDOWQUIT, PosX, PosY, 112, 174, "Landschaft", WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MINIMIZE | WINDOW_MOVE);
                    if (global::s2->RegisterWindow(WNDLandscape))
                    {
                        MapObj = global::s2->getMapObj();
                        map = MapObj->getMap();
                        switch (map->type)
                        {
                            case MAP_GREENLAND: WNDLandscape->addPicture(EditorLandscapeMenu, PICGRANITE, 2, 2, PICTURE_LANDSCAPE_GRANITE);
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
                            case MAP_WASTELAND: WNDLandscape->addPicture(EditorLandscapeMenu, PICGRANITE, 2, 2, PICTURE_LANDSCAPE_GRANITE);
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
                            case MAP_WINTERLAND:WNDLandscape->addPicture(EditorLandscapeMenu, PICGRANITE, 2, 2, PICTURE_LANDSCAPE_GRANITE_WINTER);
                                                WNDLandscape->addPicture(EditorLandscapeMenu, PICTREEDEAD, 36, 2, PICTURE_LANDSCAPE_TREE_DEAD_WINTER);
                                                WNDLandscape->addPicture(EditorLandscapeMenu, PICSTONE, 70, 2, PICTURE_LANDSCAPE_STONE_WINTER);
                                                WNDLandscape->addPicture(EditorLandscapeMenu, PICPEBBLE, 2, 36, PICTURE_LANDSCAPE_PEBBLE_WINTER);
                                                WNDLandscape->addPicture(EditorLandscapeMenu, PICBONE, 36, 36, PICTURE_LANDSCAPE_BONE_WINTER);
                                                WNDLandscape->addPicture(EditorLandscapeMenu, PICMUSHROOM, 70, 36, PICTURE_LANDSCAPE_MUSHROOM_WINTER);
                                                WNDLandscape->addPicture(EditorLandscapeMenu, PICFLOWERS, 73, 73, MAPPIC_FLOWERS);
                                                break;
                            default:            //should not happen
                                                break;
                        }
                        MapObj->setMode(EDITOR_MODE_LANDSCAPE);
                        MapObj->setModeContent(0x01);
                        MapObj->setModeContent2(0xCC);
                        lastContent = 0x01;
                        lastContent2 = 0xCC;
                    }
                    else
                    {
                        delete WNDLandscape;
                        WNDLandscape = NULL;
                        return;
                    }
                    break;

        case PICGRANITE:        MapObj->setModeContent(0x01);
                                MapObj->setModeContent2(0xCC);
                                lastContent = 0x01;
                                lastContent2 = 0xCC;
                                break;
        case PICTREEDEAD:       MapObj->setModeContent(0x05);
                                MapObj->setModeContent2(0xC8);
                                lastContent = 0x05;
                                lastContent2 = 0xC8;
                                break;
        case PICSTONE:          MapObj->setModeContent(0x02);
                                MapObj->setModeContent2(0xC8);
                                lastContent = 0x02;
                                lastContent2 = 0xC8;
                                break;
        case PICCACTUS:         MapObj->setModeContent(0x0C);
                                MapObj->setModeContent2(0xC8);
                                lastContent = 0x0C;
                                lastContent2 = 0xC8;
                                break;
        case PICPEBBLE:         MapObj->setModeContent(0x25);
                                MapObj->setModeContent2(0xC8);
                                lastContent = 0x25;
                                lastContent2 = 0xC8;
                                break;
        case PICBUSH:           MapObj->setModeContent(0x10);
                                MapObj->setModeContent2(0xC8);
                                lastContent = 0x10;
                                lastContent2 = 0xC8;
                                break;
        case PICSHRUB:          MapObj->setModeContent(0x0E);
                                MapObj->setModeContent2(0xC8);
                                lastContent = 0x0E;
                                lastContent2 = 0xC8;
                                break;
        case PICBONE:           MapObj->setModeContent(0x07);
                                MapObj->setModeContent2(0xC8);
                                lastContent = 0x07;
                                lastContent2 = 0xC8;
                                break;
        case PICMUSHROOM:       MapObj->setModeContent(0x00);
                                MapObj->setModeContent2(0xC8);
                                lastContent = 0x00;
                                lastContent2 = 0xC8;
                                break;
        case PICSTALAGMITE:     MapObj->setModeContent(0x18);
                                MapObj->setModeContent2(0xC8);
                                lastContent = 0x18;
                                lastContent2 = 0xC8;
                                break;
        case PICFLOWERS:        MapObj->setModeContent(0x09);
                                MapObj->setModeContent2(0xC8);
                                lastContent = 0x09;
                                lastContent2 = 0xC8;
                                break;

        case WINDOW_CLICKED_CALL:   if (MapObj != NULL)
                                    {
                                        MapObj->setMode(EDITOR_MODE_LANDSCAPE);
                                        MapObj->setModeContent(lastContent);
                                        MapObj->setModeContent2(lastContent2);
                                    }
                                    break;

        case WINDOWQUIT:
                    if (WNDLandscape != NULL)
                    {
                        PosX = WNDLandscape->getX();
                        PosY = WNDLandscape->getY();
                        WNDLandscape->setWaste();
                        WNDLandscape = NULL;
                    }
                    MapObj->setMode(EDITOR_MODE_HEIGHT_RAISE);
                    MapObj->setModeContent(0x00);
                    MapObj->setModeContent2(0x00);
                    lastContent = 0x00;
                    lastContent2 = 0x00;
                    MapObj = NULL;
                    map = NULL;
                    break;

        case MAP_QUIT:
                    //we do the same like in case WINDOWQUIT, but we won't setMode(EDITOR_MODE_HEIGHT_RAISE), cause map is dead
                    if (WNDLandscape != NULL)
                    {
                        PosX = WNDLandscape->getX();
                        PosY = WNDLandscape->getY();
                        WNDLandscape->setWaste();
                        WNDLandscape = NULL;
                    }
                    lastContent = 0x00;
                    lastContent2 = 0x00;
                    MapObj = NULL;
                    map = NULL;
                    break;

        default:    break;
    }
}

void callback::EditorAnimalMenu(int Param)
{
    static CWindow *WNDAnimal = NULL;
    static CMap* MapObj = NULL;
    static int lastContent = 0x00;
    static int PosX = 510, PosY = 0;

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

    switch (Param)
    {
        case INITIALIZING_CALL:
                    if (WNDAnimal != NULL)
                        break;
                    WNDAnimal = new CWindow(EditorAnimalMenu, WINDOWQUIT, PosX, PosY, 116, 106, "Tiere", WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MINIMIZE | WINDOW_MOVE);
                    if (global::s2->RegisterWindow(WNDAnimal))
                    {
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
                    }
                    else
                    {
                        delete WNDAnimal;
                        WNDAnimal = NULL;
                        return;
                    }
                    break;

        case PICRABBIT:         MapObj->setModeContent(0x01);
                                lastContent = 0x01;
                                break;
        case PICFOX:            MapObj->setModeContent(0x02);
                                lastContent = 0x02;
                                break;
        case PICSTAG:           MapObj->setModeContent(0x03);
                                lastContent = 0x03;
                                break;
        case PICROE:            MapObj->setModeContent(0x04);
                                lastContent = 0x04;
                                break;
        case PICDUCK:           MapObj->setModeContent(0x05);
                                lastContent = 0x05;
                                break;
        case PICSHEEP:          MapObj->setModeContent(0x06);
                                lastContent = 0x06;
                                break;

        case WINDOW_CLICKED_CALL:   if (MapObj != NULL)
                                    {
                                        MapObj->setMode(EDITOR_MODE_ANIMAL);
                                        MapObj->setModeContent(lastContent);
                                    }
                                    break;

        case WINDOWQUIT:
                    if (WNDAnimal != NULL)
                    {
                        PosX = WNDAnimal->getX();
                        PosY = WNDAnimal->getY();
                        WNDAnimal->setWaste();
                        WNDAnimal = NULL;
                    }
                    MapObj->setMode(EDITOR_MODE_HEIGHT_RAISE);
                    MapObj->setModeContent(0x00);
                    lastContent = 0x00;
                    MapObj = NULL;
                    break;

        case MAP_QUIT:
                    //we do the same like in case WINDOWQUIT, but we won't setMode(EDITOR_MODE_HEIGHT_RAISE), cause map is dead
                    if (WNDAnimal != NULL)
                    {
                        PosX = WNDAnimal->getX();
                        PosY = WNDAnimal->getY();
                        WNDAnimal->setWaste();
                        WNDAnimal = NULL;
                    }
                    lastContent = 0x00;
                    MapObj = NULL;
                    break;

        default:    break;
    }
}

void callback::EditorPlayerMenu(int Param)
{
    static CWindow *WNDPlayer = NULL;
    static CMap* MapObj = NULL;
    static bobMAP *map = NULL;
    static int PlayerNumber = 0x00;
    static CFont *PlayerNumberText = NULL;
    char puffer[30];
    static DisplayRectangle tempRect;
    static Uint16* PlayerHQx = NULL;
    static Uint16* PlayerHQy = NULL;
    static int PosX = 0, PosY = 200;

    enum
    {
        PLAYER_REDUCE = 0,
        PLAYER_RAISE,
        GOTO_PLAYER,
        WINDOWQUIT
    };

    switch (Param)
    {
        case INITIALIZING_CALL:
                    if (WNDPlayer != NULL)
                        break;
                    WNDPlayer = new CWindow(EditorPlayerMenu, WINDOWQUIT, PosX, PosY, 100, 80, "Spieler", WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MINIMIZE | WINDOW_MOVE);
                    if (global::s2->RegisterWindow(WNDPlayer))
                    {
                        MapObj = global::s2->getMapObj();
                        map = MapObj->getMap();
                        tempRect = MapObj->getDisplayRect();
                        PlayerHQx = MapObj->getPlayerHQx();
                        PlayerHQy = MapObj->getPlayerHQy();

                        MapObj->setMode(EDITOR_MODE_FLAG);
                        MapObj->setModeContent(PlayerNumber);

                        WNDPlayer->addButton(EditorPlayerMenu, PLAYER_REDUCE, 0, 0, 20, 20, BUTTON_GREY, "-");
                        sprintf(puffer, "%d", PlayerNumber+1);
                        PlayerNumberText = WNDPlayer->addText(puffer, 26, 4, 14, FONT_ORANGE);
                        WNDPlayer->addButton(EditorPlayerMenu, PLAYER_RAISE, 40, 0, 20, 20, BUTTON_GREY, "+");
                        WNDPlayer->addButton(EditorPlayerMenu, GOTO_PLAYER, 0, 20, 60, 20, BUTTON_GREY, "Gehe zu");
                    }
                    else
                    {
                        delete WNDPlayer;
                        WNDPlayer = NULL;
                        return;
                    }
                    break;

        case PLAYER_REDUCE:         if (PlayerNumber > 0)
                                    {
                                        PlayerNumber--;
                                        MapObj->setModeContent(PlayerNumber);
                                        WNDPlayer->delText(PlayerNumberText);
                                        sprintf(puffer, "%d", PlayerNumber+1);
                                        PlayerNumberText = WNDPlayer->addText(puffer, 26, 4, 14, FONT_ORANGE);
                                    }
                                    break;

        case PLAYER_RAISE:          if (PlayerNumber < MAXPLAYERS-1)
                                    {
                                        PlayerNumber++;
                                        MapObj->setModeContent(PlayerNumber);
                                        WNDPlayer->delText(PlayerNumberText);
                                        sprintf(puffer, "%d", PlayerNumber+1);
                                        PlayerNumberText = WNDPlayer->addText(puffer, 26, 4, 14, FONT_ORANGE);
                                    }
                                    break;

        case GOTO_PLAYER:           //test if player exists on map
                                    if (PlayerHQx[PlayerNumber] != 0xFFFF && PlayerHQy[PlayerNumber] != 0xFFFF)
                                    {
                                        tempRect = MapObj->getDisplayRect();
                                        tempRect.x = PlayerHQx[PlayerNumber]*TRIANGLE_WIDTH - tempRect.w/2;
                                        tempRect.y = PlayerHQy[PlayerNumber]*TRIANGLE_HEIGHT - tempRect.h/2;
                                        MapObj->setDisplayRect(tempRect);
                                    }
                                    break;

        case WINDOW_CLICKED_CALL:   if (MapObj != NULL)
                                    {
                                        MapObj->setMode(EDITOR_MODE_FLAG);
                                        MapObj->setModeContent(PlayerNumber);
                                    }
                                    break;

        case WINDOWQUIT:
                    if (WNDPlayer != NULL)
                    {
                        PosX = WNDPlayer->getX();
                        PosY = WNDPlayer->getY();
                        WNDPlayer->setWaste();
                        WNDPlayer = NULL;
                    }
                    MapObj->setMode(EDITOR_MODE_HEIGHT_RAISE);
                    MapObj->setModeContent(0x00);
                    MapObj->setModeContent2(0x00);
                    MapObj = NULL;
                    map = NULL;
                    PlayerNumber = 0x01;
                    PlayerNumberText = NULL;
                    PlayerHQx = NULL;
                    PlayerHQy = NULL;
                    break;

        case MAP_QUIT:
                    //we do the same like in case WINDOWQUIT, but we won't setMode(EDITOR_MODE_HEIGHT_RAISE), cause map is dead
                    if (WNDPlayer != NULL)
                    {
                        PosX = WNDPlayer->getX();
                        PosY = WNDPlayer->getY();
                        WNDPlayer->setWaste();
                        WNDPlayer = NULL;
                    }
                    MapObj = NULL;
                    map = NULL;
                    PlayerNumber = 0x01;
                    PlayerNumberText = NULL;
                    PlayerHQx = NULL;
                    PlayerHQy = NULL;
                    break;

        default:    break;
    }
}

void callback::EditorCursorMenu(int Param)
{
    static CWindow *WNDCursor = NULL;
    static CMap* MapObj = NULL;
    static bobMAP *map = NULL;
    static int trianglePictureArrowUp = -1;
    static int trianglePictureArrowDown = -1;
    static int trianglePictureRandom = -1;
    static CButton *CursorModeButton = NULL;
    static CButton *CursorRandomButton = NULL;
    static int PosX = 0, PosY = 0;

    enum
    {
        WINDOWQUIT,
        TRIANGLE,
        CURSORMODE,
        CURSORRANDOM
    };

    switch (Param)
    {
        case INITIALIZING_CALL:
                    if (WNDCursor != NULL)
                        break;
                    WNDCursor = new CWindow(EditorCursorMenu, WINDOWQUIT, PosX, PosY, 210, 130, "Cursor", WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MINIMIZE | WINDOW_MOVE);
                    if (global::s2->RegisterWindow(WNDCursor))
                    {
                        MapObj = global::s2->getMapObj();
                        map = MapObj->getMap();

                        WNDCursor->addButton(EditorCursorMenu, TRIANGLE, 2, 66, 32, 32, BUTTON_GREY, NULL);
                        trianglePictureArrowUp = WNDCursor->addStaticPicture(8, 74, CURSOR_SYMBOL_ARROW_UP);
                        trianglePictureArrowDown = WNDCursor->addStaticPicture(17, 77, CURSOR_SYMBOL_ARROW_DOWN);
                        CursorModeButton = WNDCursor->addButton(EditorCursorMenu, CURSORMODE, 2, 2, 96, 32, BUTTON_GREY, "Hexagon");
                        CursorRandomButton = WNDCursor->addButton(EditorCursorMenu, CURSORRANDOM, 2, 34, 196, 32, BUTTON_GREY, "Cursor-Aktivität: statisch");
                        if (MapObj != NULL)
                        {
                            MapObj->setVertexFillRSU(true);
                            MapObj->setVertexFillUSD(true);
                            MapObj->setVertexFillRandom(false);
                            MapObj->setHexagonMode(true);
                            MapObj->setVertexActivityRandom(false);
                        }
                    }
                    else
                    {
                        delete WNDCursor;
                        WNDCursor = NULL;
                        return;
                    }
                    break;

        case TRIANGLE:          if (trianglePictureArrowUp != -1 && trianglePictureArrowDown != -1)
                                {
                                    //both arrows are shown, so set to random
                                    //delete arrow up
                                    WNDCursor->delStaticPicture(trianglePictureArrowUp);
                                    trianglePictureArrowUp = -1;
                                    //delete arrow down
                                    WNDCursor->delStaticPicture(trianglePictureArrowDown);
                                    trianglePictureArrowDown = -1;
                                    //add random if necessary
                                    if (trianglePictureRandom == -1)
                                        trianglePictureRandom = WNDCursor->addStaticPicture(14, 76, FONT14_SPACE + 31*7+5); //Interrogation point
                                    MapObj->setVertexFillRSU(false);
                                    MapObj->setVertexFillUSD(false);
                                    MapObj->setVertexFillRandom(true);
                                }
                                else if (trianglePictureArrowUp == -1 && trianglePictureRandom == -1)
                                {
                                    //only arrow down is shown, so upgrade to both arrows
                                    //add arrow up
                                    trianglePictureArrowUp = WNDCursor->addStaticPicture(8, 74, CURSOR_SYMBOL_ARROW_UP);
                                    //add arrow down if necessary
                                    if (trianglePictureArrowDown == -1)
                                        trianglePictureArrowDown = WNDCursor->addStaticPicture(17, 77, CURSOR_SYMBOL_ARROW_DOWN);
                                    MapObj->setVertexFillRSU(true);
                                    MapObj->setVertexFillUSD(true);
                                    MapObj->setVertexFillRandom(false);
                                }
                                else if (trianglePictureArrowDown == -1 && trianglePictureRandom == -1)
                                {
                                    //only arrow up is shown, so delete arrow up and add arrow down
                                    //delete arrow up if necessary
                                    if (trianglePictureArrowUp != -1)
                                    {
                                        WNDCursor->delStaticPicture(trianglePictureArrowUp);
                                        trianglePictureArrowUp = -1;
                                    }
                                    trianglePictureArrowDown = WNDCursor->addStaticPicture(17, 77, CURSOR_SYMBOL_ARROW_DOWN);
                                    MapObj->setVertexFillRSU(false);
                                    MapObj->setVertexFillUSD(true);
                                    MapObj->setVertexFillRandom(false);
                                }
                                else
                                {
                                    //the interrogation point is shown, so set to arrow up
                                    WNDCursor->delStaticPicture(trianglePictureRandom);
                                    trianglePictureRandom = -1;
                                    //add arrow up if necessary
                                    if (trianglePictureArrowUp == -1)
                                        trianglePictureArrowUp = WNDCursor->addStaticPicture(8, 74, CURSOR_SYMBOL_ARROW_UP);
                                    //delete arrow down if necessary
                                    if (trianglePictureArrowDown != -1)
                                    {
                                        WNDCursor->delStaticPicture(trianglePictureArrowDown);
                                        trianglePictureArrowDown = -1;
                                    }
                                    MapObj->setVertexFillRSU(true);
                                    MapObj->setVertexFillUSD(false);
                                    MapObj->setVertexFillRandom(false);
                                }
                                break;

        case CURSORMODE:    if (CursorModeButton != NULL)
                            {
                                WNDCursor->delButton(CursorModeButton);
                                CursorModeButton = NULL;
                            }
                            if (MapObj->getHexagonMode())
                            {
                                CursorModeButton = WNDCursor->addButton(EditorCursorMenu, CURSORMODE, 2, 2, 96, 32, BUTTON_GREY, "Quadrat");
                                MapObj->setHexagonMode(false);
                            }
                            else
                            {
                                CursorModeButton = WNDCursor->addButton(EditorCursorMenu, CURSORMODE, 2, 2, 96, 32, BUTTON_GREY, "Hexagon");
                                MapObj->setHexagonMode(true);
                            }
                            break;
        case CURSORRANDOM:  if (CursorRandomButton != NULL)
                            {
                                WNDCursor->delButton(CursorRandomButton);
                                CursorRandomButton = NULL;
                            }
                            if (MapObj->getVertexActivityRandom())
                            {
                                CursorRandomButton = WNDCursor->addButton(EditorCursorMenu, CURSORRANDOM, 2, 34, 196, 32, BUTTON_GREY, "Cursor-Aktivität: statisch");
                                MapObj->setVertexActivityRandom(false);
                            }
                            else
                            {
                                CursorRandomButton = WNDCursor->addButton(EditorCursorMenu, CURSORRANDOM, 2, 34, 196, 32, BUTTON_GREY, "Cursor-Aktivität: zufällig");
                                MapObj->setVertexActivityRandom(true);
                            }
                            break;

        case WINDOW_CLICKED_CALL:   break;

        case WINDOWQUIT:
                    if (WNDCursor != NULL)
                    {
                        PosX = WNDCursor->getX();
                        PosY = WNDCursor->getY();
                        WNDCursor->setWaste();
                        WNDCursor = NULL;
                    }
                    MapObj = NULL;
                    map = NULL;
                    trianglePictureArrowUp = -1;
                    trianglePictureArrowDown = -1;
                    trianglePictureRandom = -1;
                    CursorModeButton = NULL;
                    CursorRandomButton = NULL;
                    break;

        case MAP_QUIT:
                    //we do the same like in case WINDOWQUIT
                    if (WNDCursor != NULL)
                    {
                        PosX = WNDCursor->getX();
                        PosY = WNDCursor->getY();
                        WNDCursor->setWaste();
                        WNDCursor = NULL;
                    }
                    MapObj = NULL;
                    map = NULL;
                    trianglePictureArrowUp = -1;
                    trianglePictureArrowDown = -1;
                    trianglePictureRandom = -1;
                    CursorModeButton = NULL;
                    CursorRandomButton = NULL;
                    break;

        default:    break;
    }
}

//"create world" menu
void callback::EditorCreateMenu(int Param)
{
    static CWindow *WNDCreate = NULL;
    static CMap* MapObj = NULL;
    static CFont *TextWidth = NULL;
    static int width = 32;
    static CFont *TextHeight = NULL;
    static int height = 32;
    static CButton *ButtonLandscape = NULL;
    static int LandscapeType = 0; //0 = Greenland, 1 = Wasteland, 2 = Winterland
    static int PicTextureIndex = -1;
    static int PicTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_SNOW;
    static int texture = TRIANGLE_TEXTURE_SNOW;
    static int PicBorderTextureIndex = -1;
    static int PicBorderTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_SNOW;
    static CFont *TextBorder = NULL;
    static int border = 0;
    static int border_texture = TRIANGLE_TEXTURE_SNOW;
    static char puffer[5];
    static int PosX = global::s2->GameResolutionX/2-125, PosY = global::s2->GameResolutionY/2-175;

    enum
    {
        REDUCE_WIDTH_128,
        REDUCE_WIDTH_12,
        RAISE_WIDTH_12,
        RAISE_WIDTH_128,
        REDUCE_HEIGHT_128,
        REDUCE_HEIGHT_12,
        RAISE_HEIGHT_12,
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

    switch (Param)
    {
        case INITIALIZING_CALL:
                    if (WNDCreate != NULL)
                        break;
                    WNDCreate = new CWindow(EditorCreateMenu, WINDOWQUIT, PosX, PosY, 250, 350, "Welt erschaffen", WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MOVE | WINDOW_MINIMIZE);
                    if (global::s2->RegisterWindow(WNDCreate))
                    {
                        MapObj = global::s2->getMapObj();

                        WNDCreate->addText("Breite", 95, 4, 9, FONT_YELLOW);
                        WNDCreate->addButton(EditorCreateMenu, REDUCE_WIDTH_128, 10, 15, 35, 20, BUTTON_GREY, "128<-");
                        WNDCreate->addButton(EditorCreateMenu, REDUCE_WIDTH_12, 45, 15, 35, 20, BUTTON_GREY, "12<-");
                        sprintf(puffer, "%d", width);
                        TextWidth = WNDCreate->addText(puffer, 105, 17, 14, FONT_YELLOW);
                        WNDCreate->addButton(EditorCreateMenu, RAISE_WIDTH_12, 158, 15, 35, 20, BUTTON_GREY, "->12");
                        WNDCreate->addButton(EditorCreateMenu, RAISE_WIDTH_128, 193, 15, 35, 20, BUTTON_GREY, "->128");

                        WNDCreate->addText("Höhe", 100, 40, 9, FONT_YELLOW);
                        WNDCreate->addButton(EditorCreateMenu, REDUCE_HEIGHT_128, 10, 49, 35, 20, BUTTON_GREY, "128<-");
                        WNDCreate->addButton(EditorCreateMenu, REDUCE_HEIGHT_12, 45, 49, 35, 20, BUTTON_GREY, "12<-");
                        sprintf(puffer, "%d", height);
                        TextHeight = WNDCreate->addText(puffer, 105, 51, 14, FONT_YELLOW);
                        WNDCreate->addButton(EditorCreateMenu, RAISE_HEIGHT_12, 158, 49, 35, 20, BUTTON_GREY, "->12");
                        WNDCreate->addButton(EditorCreateMenu, RAISE_HEIGHT_128, 193, 49, 35, 20, BUTTON_GREY, "->128");

                        WNDCreate->addText("Landschaft", 85, 80, 9, FONT_YELLOW);
                        ButtonLandscape = WNDCreate->addButton(EditorCreateMenu, CHANGE_LANDSCAPE, 64, 93, 110, 20, BUTTON_GREY, (LandscapeType == 0 ? "Grünland" : (LandscapeType == 1 ? "Ödland" : "Winterwelt")));

                        WNDCreate->addText("Grundfläche", 82, 120, 9, FONT_YELLOW);
                        WNDCreate->addButton(EditorCreateMenu, TEXTURE_PREVIOUS, 45, 139, 35, 20, BUTTON_GREY, "-");
                        PicTextureIndex = WNDCreate->addStaticPicture(102, 133, PicTextureIndexGlobal);
                        WNDCreate->addButton(EditorCreateMenu, TEXTURE_NEXT, 158, 139, 35, 20, BUTTON_GREY, "+");

                        WNDCreate->addText("Rand", 103, 175, 9, FONT_YELLOW);
                        WNDCreate->addButton(EditorCreateMenu, REDUCE_BORDER, 45, 186, 35, 20, BUTTON_GREY, "-");
                        sprintf(puffer, "%d", border);
                        TextBorder = WNDCreate->addText(puffer, 112, 188, 14, FONT_YELLOW);
                        WNDCreate->addButton(EditorCreateMenu, RAISE_BORDER, 158, 186, 35, 20, BUTTON_GREY, "+");

                        WNDCreate->addText("Rand-Grundfläche", 65, 215, 9, FONT_YELLOW);
                        WNDCreate->addButton(EditorCreateMenu, BORDER_TEXTURE_PREVIOUS,  45, 234, 35, 20, BUTTON_GREY, "-");
                        PicBorderTextureIndex = WNDCreate->addStaticPicture(102, 228, PicBorderTextureIndexGlobal);
                        WNDCreate->addButton(EditorCreateMenu, BORDER_TEXTURE_NEXT, 158, 234, 35, 20, BUTTON_GREY, "+");

                        WNDCreate->addButton(EditorCreateMenu, CREATE_WORLD, 44, 275, 150, 40, BUTTON_GREY, "Welt erschaffen");
                    }
                    else
                    {
                        delete WNDCreate;
                        WNDCreate = NULL;
                        return;
                    }
                    break;

        case CALL_FROM_GAMELOOP:
                    break;

        case REDUCE_WIDTH_128:
                    if (width - 128 >= 32)
                        width -= 128;
                    else
                        width = 32;
                    WNDCreate->delText(TextWidth);
                    sprintf(puffer, "%d", width);
                    TextWidth = WNDCreate->addText(puffer, 105, 17, 14, FONT_YELLOW);
                    break;
        case REDUCE_WIDTH_12:
                    if (width - 12 >= 32)
                        width -= 12;
                    else
                        width = 32;
                    WNDCreate->delText(TextWidth);
                    sprintf(puffer, "%d", width);
                    TextWidth = WNDCreate->addText(puffer, 105, 17, 14, FONT_YELLOW);
                    break;
        case RAISE_WIDTH_12:
                    if (width + 12 <= MAXMAPWIDTH)
                        width += 12;
                    else
                        width = MAXMAPWIDTH;
                    WNDCreate->delText(TextWidth);
                    sprintf(puffer, "%d", width);
                    TextWidth = WNDCreate->addText(puffer, 105, 17, 14, FONT_YELLOW);
                    break;
        case RAISE_WIDTH_128:
                    if (width + 128 <= MAXMAPWIDTH)
                        width += 128;
                    else
                        width = MAXMAPWIDTH;
                    WNDCreate->delText(TextWidth);
                    sprintf(puffer, "%d", width);
                    TextWidth = WNDCreate->addText(puffer, 105, 17, 14, FONT_YELLOW);
                    break;
        case REDUCE_HEIGHT_128:
                    if (height - 128 >= 32)
                        height -= 128;
                    else
                        height = 32;
                    WNDCreate->delText(TextHeight);
                    sprintf(puffer, "%d", height);
                    TextHeight = WNDCreate->addText(puffer, 105, 51, 14, FONT_YELLOW);
                    break;
        case REDUCE_HEIGHT_12:
                    if (height - 12 >= 32)
                        height -= 12;
                    else
                        height = 32;
                    WNDCreate->delText(TextHeight);
                    sprintf(puffer, "%d", height);
                    TextHeight = WNDCreate->addText(puffer, 105, 51, 14, FONT_YELLOW);
                    break;
        case RAISE_HEIGHT_12:
                    if (height + 12 <= MAXMAPHEIGHT)
                        height += 12;
                    else
                        height = MAXMAPHEIGHT;
                    WNDCreate->delText(TextHeight);
                    sprintf(puffer, "%d", height);
                    TextHeight = WNDCreate->addText(puffer, 105, 51, 14, FONT_YELLOW);
                    break;
        case RAISE_HEIGHT_128:
                    if (height + 128 <= MAXMAPHEIGHT)
                        height += 128;
                    else
                        height = MAXMAPHEIGHT;
                    WNDCreate->delText(TextHeight);
                    sprintf(puffer, "%d", height);
                    TextHeight = WNDCreate->addText(puffer, 105, 51, 14, FONT_YELLOW);
                    break;

        case CHANGE_LANDSCAPE:
                    LandscapeType++;
                    if (LandscapeType > 2)
                        LandscapeType = 0;
                    WNDCreate->delButton(ButtonLandscape);
                    ButtonLandscape = WNDCreate->addButton(EditorCreateMenu, CHANGE_LANDSCAPE, 64, 93, 110, 20, BUTTON_GREY, (LandscapeType == 0 ? "Grünland" : (LandscapeType == 1 ? "Ödland" : "Winterwelt")));
                    switch (LandscapeType)
                    {
                        case 0: PicTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_SNOW;
                                PicBorderTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_SNOW;
                                break;
                        case 1: PicTextureIndexGlobal = PICTURE_WASTELAND_TEXTURE_SNOW;
                                PicBorderTextureIndexGlobal = PICTURE_WASTELAND_TEXTURE_SNOW;
                                break;
                        case 2: PicTextureIndexGlobal = PICTURE_WINTERLAND_TEXTURE_SNOW;
                                PicBorderTextureIndexGlobal = PICTURE_WINTERLAND_TEXTURE_SNOW;
                                break;
                        default:break;
                    }
                    WNDCreate->delStaticPicture(PicTextureIndex);
                    WNDCreate->delStaticPicture(PicBorderTextureIndex);
                    PicTextureIndex = WNDCreate->addStaticPicture(102, 133, PicTextureIndexGlobal);
                    PicBorderTextureIndex = WNDCreate->addStaticPicture(102, 228, PicBorderTextureIndexGlobal);
                    break;

        case TEXTURE_PREVIOUS:
                    PicTextureIndexGlobal--;
                    switch (LandscapeType)
                    {
                        case 0: if (PicTextureIndexGlobal < PICTURE_GREENLAND_TEXTURE_SNOW)
                                    PicTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_SNOW;
                                break;
                        case 1: if (PicTextureIndexGlobal < PICTURE_WASTELAND_TEXTURE_SNOW)
                                    PicTextureIndexGlobal = PICTURE_WASTELAND_TEXTURE_SNOW;
                                break;
                        case 2: if (PicTextureIndexGlobal < PICTURE_WINTERLAND_TEXTURE_SNOW)
                                    PicTextureIndexGlobal = PICTURE_WINTERLAND_TEXTURE_SNOW;
                                break;
                        default:break;
                    }
                    if      (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_SNOW
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_SNOW
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_SNOW
                            )
                    {
                        texture = TRIANGLE_TEXTURE_SNOW;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE
                            )
                    {
                        texture = TRIANGLE_TEXTURE_STEPPE;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_SWAMP
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_SWAMP
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_SWAMP
                            )
                    {
                        texture = TRIANGLE_TEXTURE_SWAMP;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_FLOWER
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_FLOWER
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_FLOWER
                            )
                    {
                        texture = TRIANGLE_TEXTURE_FLOWER;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING1
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING1
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING1
                            )
                    {
                        texture = TRIANGLE_TEXTURE_MINING1;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING2
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING2
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING2
                            )
                    {
                        texture = TRIANGLE_TEXTURE_MINING2;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING3
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING3
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING3
                            )
                    {
                        texture = TRIANGLE_TEXTURE_MINING3;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING4
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING4
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING4
                            )
                    {
                        texture = TRIANGLE_TEXTURE_MINING4;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW1
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE_MEADOW1
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE_MEADOW1
                            )
                    {
                        texture = TRIANGLE_TEXTURE_STEPPE_MEADOW1;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW1
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW1
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW1
                            )
                    {
                        texture = TRIANGLE_TEXTURE_MEADOW1;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW2
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW2
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW2
                            )
                    {
                        texture = TRIANGLE_TEXTURE_MEADOW2;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW3
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW3
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW3
                            )
                    {
                        texture = TRIANGLE_TEXTURE_MEADOW3;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW2
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE_MEADOW2
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE_MEADOW2
                            )
                    {
                        texture = TRIANGLE_TEXTURE_STEPPE_MEADOW2;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING_MEADOW
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING_MEADOW
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING_MEADOW
                            )
                    {
                        texture = TRIANGLE_TEXTURE_MINING_MEADOW;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_WATER
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_WATER
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_WATER
                            )
                    {
                        texture = TRIANGLE_TEXTURE_WATER;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_LAVA
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_LAVA
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_LAVA
                            )
                    {
                        texture = TRIANGLE_TEXTURE_LAVA;
                    }
                    WNDCreate->delStaticPicture(PicTextureIndex);
                    PicTextureIndex = WNDCreate->addStaticPicture(102, 133, PicTextureIndexGlobal);
                    break;
        case TEXTURE_NEXT:
                    PicTextureIndexGlobal++;
                    switch (LandscapeType)
                    {
                        case 0: if (PicTextureIndexGlobal > PICTURE_GREENLAND_TEXTURE_LAVA)
                                    PicTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_LAVA;
                                break;
                        case 1: if (PicTextureIndexGlobal > PICTURE_WASTELAND_TEXTURE_LAVA)
                                    PicTextureIndexGlobal = PICTURE_WASTELAND_TEXTURE_LAVA;
                                break;
                        case 2: if (PicTextureIndexGlobal > PICTURE_WINTERLAND_TEXTURE_LAVA)
                                    PicTextureIndexGlobal = PICTURE_WINTERLAND_TEXTURE_LAVA;
                                break;
                        default:break;
                    }
                    if      (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_SNOW
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_SNOW
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_SNOW
                            )
                    {
                        texture = TRIANGLE_TEXTURE_SNOW;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE
                            )
                    {
                        texture = TRIANGLE_TEXTURE_STEPPE;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_SWAMP
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_SWAMP
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_SWAMP
                            )
                    {
                        texture = TRIANGLE_TEXTURE_SWAMP;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_FLOWER
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_FLOWER
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_FLOWER
                            )
                    {
                        texture = TRIANGLE_TEXTURE_FLOWER;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING1
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING1
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING1
                            )
                    {
                        texture = TRIANGLE_TEXTURE_MINING1;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING2
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING2
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING2
                            )
                    {
                        texture = TRIANGLE_TEXTURE_MINING2;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING3
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING3
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING3
                            )
                    {
                        texture = TRIANGLE_TEXTURE_MINING3;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING4
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING4
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING4
                            )
                    {
                        texture = TRIANGLE_TEXTURE_MINING4;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW1
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE_MEADOW1
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE_MEADOW1
                            )
                    {
                        texture = TRIANGLE_TEXTURE_STEPPE_MEADOW1;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW1
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW1
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW1
                            )
                    {
                        texture = TRIANGLE_TEXTURE_MEADOW1;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW2
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW2
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW2
                            )
                    {
                        texture = TRIANGLE_TEXTURE_MEADOW2;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW3
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW3
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW3
                            )
                    {
                        texture = TRIANGLE_TEXTURE_MEADOW3;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW2
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE_MEADOW2
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE_MEADOW2
                            )
                    {
                        texture = TRIANGLE_TEXTURE_STEPPE_MEADOW2;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING_MEADOW
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING_MEADOW
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING_MEADOW
                            )
                    {
                        texture = TRIANGLE_TEXTURE_MINING_MEADOW;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_WATER
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_WATER
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_WATER
                            )
                    {
                        texture = TRIANGLE_TEXTURE_WATER;
                    }
                    else if (   PicTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_LAVA
                            ||  PicTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_LAVA
                            ||  PicTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_LAVA
                            )
                    {
                        texture = TRIANGLE_TEXTURE_LAVA;
                    }
                    WNDCreate->delStaticPicture(PicTextureIndex);
                    PicTextureIndex = WNDCreate->addStaticPicture(102, 133, PicTextureIndexGlobal);
                    break;

        case REDUCE_BORDER:
                    if (border - 1 >= 0)
                        border -= 1;
                    else
                        border = 0;
                    WNDCreate->delText(TextBorder);
                    sprintf(puffer, "%d", border);
                    TextBorder = WNDCreate->addText(puffer, 112, 188, 14, FONT_YELLOW);
                    break;
        case RAISE_BORDER:
                    if (border + 1 <= 12)
                        border += 1;
                    else
                        border = 12;
                    WNDCreate->delText(TextBorder);
                    sprintf(puffer, "%d", border);
                    TextBorder = WNDCreate->addText(puffer, 112, 188, 14, FONT_YELLOW);
                    break;

        case BORDER_TEXTURE_PREVIOUS:
                    PicBorderTextureIndexGlobal--;
                    switch (LandscapeType)
                    {
                        case 0: if (PicBorderTextureIndexGlobal < PICTURE_GREENLAND_TEXTURE_SNOW)
                                    PicBorderTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_SNOW;
                                break;
                        case 1: if (PicBorderTextureIndexGlobal < PICTURE_WASTELAND_TEXTURE_SNOW)
                                    PicBorderTextureIndexGlobal = PICTURE_WASTELAND_TEXTURE_SNOW;
                                break;
                        case 2: if (PicBorderTextureIndexGlobal < PICTURE_WINTERLAND_TEXTURE_SNOW)
                                    PicBorderTextureIndexGlobal = PICTURE_WINTERLAND_TEXTURE_SNOW;
                                break;
                        default:break;
                    }
                    if      (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_SNOW
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_SNOW
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_SNOW
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_SNOW;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_STEPPE;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_SWAMP
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_SWAMP
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_SWAMP
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_SWAMP;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_FLOWER
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_FLOWER
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_FLOWER
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_FLOWER;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING1
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING1
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING1
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_MINING1;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING2
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING2
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING2
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_MINING2;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING3
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING3
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING3
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_MINING3;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING4
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING4
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING4
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_MINING4;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW1
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE_MEADOW1
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE_MEADOW1
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_STEPPE_MEADOW1;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW1
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW1
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW1
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_MEADOW1;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW2
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW2
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW2
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_MEADOW2;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW3
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW3
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW3
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_MEADOW3;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW2
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE_MEADOW2
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE_MEADOW2
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_STEPPE_MEADOW2;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING_MEADOW
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING_MEADOW
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING_MEADOW
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_MINING_MEADOW;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_WATER
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_WATER
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_WATER
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_WATER;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_LAVA
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_LAVA
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_LAVA
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_LAVA;
                    }
                    WNDCreate->delStaticPicture(PicBorderTextureIndex);
                    PicBorderTextureIndex = WNDCreate->addStaticPicture(102, 228, PicBorderTextureIndexGlobal);
                    break;
        case BORDER_TEXTURE_NEXT:
                    PicBorderTextureIndexGlobal++;
                    switch (LandscapeType)
                    {
                        case 0: if (PicBorderTextureIndexGlobal > PICTURE_GREENLAND_TEXTURE_LAVA)
                                    PicBorderTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_LAVA;
                                break;
                        case 1: if (PicBorderTextureIndexGlobal > PICTURE_WASTELAND_TEXTURE_LAVA)
                                    PicBorderTextureIndexGlobal = PICTURE_WASTELAND_TEXTURE_LAVA;
                                break;
                        case 2: if (PicBorderTextureIndexGlobal > PICTURE_WINTERLAND_TEXTURE_LAVA)
                                    PicBorderTextureIndexGlobal = PICTURE_WINTERLAND_TEXTURE_LAVA;
                                break;
                        default:break;
                    }
                    if      (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_SNOW
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_SNOW
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_SNOW
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_SNOW;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_STEPPE;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_SWAMP
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_SWAMP
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_SWAMP
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_SWAMP;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_FLOWER
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_FLOWER
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_FLOWER
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_FLOWER;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING1
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING1
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING1
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_MINING1;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING2
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING2
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING2
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_MINING2;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING3
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING3
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING3
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_MINING3;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING4
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING4
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING4
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_MINING4;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW1
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE_MEADOW1
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE_MEADOW1
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_STEPPE_MEADOW1;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW1
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW1
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW1
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_MEADOW1;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW2
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW2
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW2
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_MEADOW2;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MEADOW3
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MEADOW3
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MEADOW3
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_MEADOW3;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW2
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_STEPPE_MEADOW2
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_STEPPE_MEADOW2
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_STEPPE_MEADOW2;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_MINING_MEADOW
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_MINING_MEADOW
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_MINING_MEADOW
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_MINING_MEADOW;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_WATER
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_WATER
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_WATER
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_WATER;
                    }
                    else if (   PicBorderTextureIndexGlobal == PICTURE_GREENLAND_TEXTURE_LAVA
                            ||  PicBorderTextureIndexGlobal == PICTURE_WASTELAND_TEXTURE_LAVA
                            ||  PicBorderTextureIndexGlobal == PICTURE_WINTERLAND_TEXTURE_LAVA
                            )
                    {
                        border_texture = TRIANGLE_TEXTURE_LAVA;
                    }
                    WNDCreate->delStaticPicture(PicBorderTextureIndex);
                    PicBorderTextureIndex = WNDCreate->addStaticPicture(102, 228, PicBorderTextureIndexGlobal);
                    break;

        case CREATE_WORLD:
                    PleaseWait(INITIALIZING_CALL);

                    //we have to close the windows and initialize them again to prevent failures
                    EditorCursorMenu(MAP_QUIT);
                    EditorTextureMenu(MAP_QUIT);
                    EditorTreeMenu(MAP_QUIT);
                    EditorLandscapeMenu(MAP_QUIT);
                    MinimapMenu(MAP_QUIT);
                    EditorResourceMenu(MAP_QUIT);
                    EditorAnimalMenu(MAP_QUIT);
                    EditorPlayerMenu(MAP_QUIT);

                    MapObj->destructMap();
                    MapObj->constructMap(NULL, width, height, LandscapeType, texture, border, border_texture);

                    //we need to check which of these windows was active before
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
                    if (WNDCreate != NULL)
                    {
                        PosX = WNDCreate->getX();
                        PosY = WNDCreate->getY();
                        WNDCreate->setWaste();
                        WNDCreate = NULL;
                    }
                    MapObj = NULL;
                    TextWidth = NULL;
                    width = 32;
                    TextHeight = NULL;
                    height = 32;
                    ButtonLandscape = NULL;
                    LandscapeType = 0;
                    PicTextureIndex = -1;
                    PicTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_SNOW;
                    texture = TRIANGLE_TEXTURE_SNOW;
                    PicBorderTextureIndex = -1;
                    PicBorderTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_SNOW;
                    TextBorder = NULL;
                    border = 0;
                    border_texture = TRIANGLE_TEXTURE_SNOW;
                    break;

        case WINDOWQUIT:
                    if (WNDCreate != NULL)
                    {
                        PosX = WNDCreate->getX();
                        PosY = WNDCreate->getY();
                        WNDCreate->setWaste();
                        WNDCreate = NULL;
                    }
                    MapObj = NULL;
                    TextWidth = NULL;
                    width = 32;
                    TextHeight = NULL;
                    height = 32;
                    ButtonLandscape = NULL;
                    LandscapeType = 0;
                    PicTextureIndex = -1;
                    PicTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_SNOW;
                    texture = TRIANGLE_TEXTURE_SNOW;
                    PicBorderTextureIndex = -1;
                    PicBorderTextureIndexGlobal = PICTURE_GREENLAND_TEXTURE_SNOW;
                    TextBorder = NULL;
                    border = 0;
                    border_texture = TRIANGLE_TEXTURE_SNOW;
                    break;

        default:    break;
    }
}

#else
//now the 4 game callbacks from the menubar will follow

void callback::GameMenu(int Param)
{
    static CWindow *WNDBackToMainMenu = NULL;

    enum
    {
        BACKTOMAIN = 1,
        WINDOWQUIT
    };

    switch (Param)
    {
        case INITIALIZING_CALL:
                    if (WNDBackToMainMenu != NULL)
                        break;
                    WNDBackToMainMenu = new CWindow(GameMenu, WINDOWQUIT, global::s2->GameResolutionX/2-125, global::s2->GameResolutionY/2-60, 250, 140, "Zurueck zum Hauptmenu?", WINDOW_GREEN1, WINDOW_CLOSE);
                    if (global::s2->RegisterWindow(WNDBackToMainMenu))
                    {
                        WNDBackToMainMenu->addButton(GameMenu, BACKTOMAIN, 20, 20, 200, 80, BUTTON_RED2, "Ja");
                    }
                    else
                    {
                        delete WNDBackToMainMenu;
                        WNDBackToMainMenu = NULL;
                        return;
                    }
                    break;

        case BACKTOMAIN:
                    if (global::s2->getMapObj() != NULL)
                        global::s2->delMapObj();
                    WNDBackToMainMenu->setWaste();
                    WNDBackToMainMenu = NULL;
                    mainmenu(INITIALIZING_CALL);
                    break;

        case WINDOWQUIT:
                    if (WNDBackToMainMenu != NULL)
                    {
                        WNDBackToMainMenu->setWaste();
                        WNDBackToMainMenu = NULL;
                    }
                    break;

        default:    break;
    }
}
#endif

void callback::MinimapMenu(int Param)
{
    static CWindow *WNDMinimap = NULL;
    static CMap* MapObj = NULL;
    static SDL_Surface *WndSurface = NULL;
    static int num_x = 1, num_y = 1;
    //only in case INITIALIZING_CALL needed to create the window
    int width;
    int height;

    enum
    {
        WINDOWQUIT
    };

    switch (Param)
    {
        case INITIALIZING_CALL:
                    if (WNDMinimap != NULL)
                        break;

                    //this variables are needed to reduce the size of minimap-windows of big maps
                    num_x = (global::s2->getMapObj()->getMap()->width > 256 ? global::s2->getMapObj()->getMap()->width/256 : 1);
                    num_y = (global::s2->getMapObj()->getMap()->height > 256 ? global::s2->getMapObj()->getMap()->height/256 : 1);

                    //make sure the minimap has the same proportions as the "real" map, so scale the same rate
                    num_x = (num_x > num_y ? num_x : num_y);
                    num_y = (num_x > num_y ? num_x : num_y);

                    width = global::s2->getMapObj()->getMap()->width/num_x;
                    height = global::s2->getMapObj()->getMap()->height/num_y;
                    //--> 12px is width of left and right window frame and 30px is height of the upper and lower window frame
                    if ( (global::s2->getDisplaySurface()->w-12 < width) || (global::s2->getDisplaySurface()->h-30 < height) )
                        break;
                    WNDMinimap = new CWindow(MinimapMenu, WINDOWQUIT, global::s2->GameResolutionX/2-width/2-6, global::s2->GameResolutionY/2-height/2-15, width+12, height+30, "Übersicht", WINDOW_NOTHING, WINDOW_CLOSE | WINDOW_MOVE);
                    if (global::s2->RegisterWindow(WNDMinimap) && global::s2->RegisterCallback(MinimapMenu))
                    {
                        WndSurface = WNDMinimap->getSurface();
                        MapObj = global::s2->getMapObj();
                    }
                    else
                    {
                        delete WNDMinimap;
                        WNDMinimap = NULL;
                        return;
                    }
                    break;

        case CALL_FROM_GAMELOOP:
                    if (MapObj != NULL && WndSurface != NULL)
                        MapObj->drawMinimap(WndSurface);
                    break;

        case WINDOW_CLICKED_CALL:
                    if (MapObj != NULL)
                    {
                        int MouseX, MouseY;
                        if (SDL_GetMouseState(&MouseX, &MouseY)&SDL_BUTTON(1))
                        {
                            if (    MouseX > (WNDMinimap->getX() + 6) && MouseX < (WNDMinimap->getX() + WNDMinimap->getW() - 6)
                                &&  MouseY > (WNDMinimap->getY() + 20) && MouseY < (WNDMinimap->getY() + WNDMinimap->getH() - 10)
                               )
                            {
                                DisplayRectangle displayRect = MapObj->getDisplayRect();
                                displayRect.x = (MouseX - WNDMinimap->getX() - 6 - global::bmpArray[MAPPIC_ARROWCROSS_ORANGE].nx)*TRIANGLE_WIDTH*num_x;
                                displayRect.y = (MouseY - WNDMinimap->getY() - 20 - global::bmpArray[MAPPIC_ARROWCROSS_ORANGE].ny)*TRIANGLE_HEIGHT*num_y;
                                MapObj->setDisplayRect(displayRect);
                            }
                        }
                    }
                    break;

        case WINDOWQUIT:
                    if (WNDMinimap != NULL)
                    {
                        WNDMinimap->setWaste();
                        WNDMinimap = NULL;
                    }
                    MapObj = NULL;
                    WndSurface = NULL;
                    global::s2->UnregisterCallback(MinimapMenu);
                    break;

        case MAP_QUIT:
                    //we do the same like in case WINDOWQUIT
                    if (WNDMinimap != NULL)
                    {
                        WNDMinimap->setWaste();
                        WNDMinimap = NULL;
                    }
                    MapObj = NULL;
                    WndSurface = NULL;
                    global::s2->UnregisterCallback(MinimapMenu);
                    break;

        default:    break;
    }
}


#ifdef _ADMINMODE
//the debugger is an object and a friend class of all other classes
//debugger-function only will construct a new debugger and if debugger-function gets a window-quit-message
//then the debugger-function will destruct the object
void callback::debugger(int Param)
{
    static CDebug* Debugger = NULL;

    switch (Param)
    {
        case INITIALIZING_CALL: if (Debugger != NULL)
                                    break;
                                Debugger = new CDebug(debugger, DEBUGGER_QUIT);
                                break;

        case DEBUGGER_QUIT:     delete Debugger;
                                Debugger = NULL;
                                break;

        default:                if (Debugger != NULL)
                                    Debugger->sendParam(Param);
                                break;
    }
}

//this is the picture-viewer
void callback::viewer(int Param)
{
    static CWindow *WNDViewer = NULL;
    static int index = 0;
    static int PicInWndIndex = -1;
    static char PicInfos[50];
    static CFont *PicInfosText = NULL;

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

    switch (Param)
    {
        case INITIALIZING_CALL:
                    if (WNDViewer != NULL)
                        break;
                    WNDViewer = new CWindow(viewer, WINDOWQUIT, 0, 0, 250, 140, "Viewer", WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MOVE | WINDOW_RESIZE | WINDOW_MINIMIZE);
                    if (global::s2->RegisterWindow(WNDViewer))
                    {
                        global::s2->RegisterCallback(viewer);
                        WNDViewer->addButton(viewer, BACKWARD_100, 0, 0, 35, 20, BUTTON_GREY, "100<-");
                        WNDViewer->addButton(viewer, BACKWARD_10,  35, 0, 35, 20, BUTTON_GREY, "10<-");
                        WNDViewer->addButton(viewer, BACKWARD_1,   70, 0, 35, 20, BUTTON_GREY, "1<-");
                        WNDViewer->addButton(viewer, FORWARD_1,    105, 0, 35, 20, BUTTON_GREY, "->1");
                        WNDViewer->addButton(viewer, FORWARD_10,   140, 0, 35, 20, BUTTON_GREY, "->10");
                        WNDViewer->addButton(viewer, FORWARD_100,  175, 0, 35, 20, BUTTON_GREY, "->100");
                    }
                    else
                    {
                        delete WNDViewer;
                        WNDViewer = NULL;
                        return;
                    }
                    break;

        case CALL_FROM_GAMELOOP:
                    if (PicInWndIndex >= 0)
                        WNDViewer->delStaticPicture(PicInWndIndex);
                    PicInWndIndex = WNDViewer->addStaticPicture(5, 30, index);

                    if (PicInfosText != NULL)
                    {
                        WNDViewer->delText(PicInfosText);
                        PicInfosText = NULL;
                    }
                    if (PicInfosText == NULL)
                    {
                        sprintf(PicInfos, "index=%d, w=%d, h=%d, nx=%d, ny=%d", index, global::bmpArray[index].w, global::bmpArray[index].h, global::bmpArray[index].nx, global::bmpArray[index].ny);
                        PicInfosText = WNDViewer->addText(PicInfos, 220, 3, 14, FONT_RED);
                    }

                    break;

        case BACKWARD_100:  if (index-100 >= 0)
                                index -= 100;
                            else
                                index = 0;
                            break;
        case BACKWARD_10:   if (index-10 >= 0)
                                index -= 10;
                            else
                                index = 0;
                            break;
        case BACKWARD_1:    if (index-1 >= 0)
                                index -= 1;
                            else
                                index = 0;
                            break;
        case FORWARD_1:     index++;
                            break;
        case FORWARD_10:    index += 10;
                            break;
        case FORWARD_100:   index += 100;
                            break;

        case WINDOWQUIT:
                    if (WNDViewer != NULL)
                    {
                        WNDViewer->setWaste();
                        WNDViewer = NULL;
                        global::s2->UnregisterCallback(viewer);
                        index = 0;
                        PicInWndIndex = -1;
                    }
                    break;

        default:    break;
    }
}

//this is a submenu for testing
void callback::submenu1(int Param)
{
    static CMenu *SubMenu = NULL;
    static CButton *toMain = NULL;
    static CButton *greatMoon = NULL;
    static CButton *smallMoon = NULL;
    static CButton *toosmall = NULL;
    static CButton *createWindow = NULL;
    static CFont *ueberschrift = NULL;
    static CFont *resolution = NULL;
    static CFont *greatMoonText = NULL;
    static CFont *counterText = NULL;
    static CPicture *picObject = NULL;
    static int counter = 0;
    static CWindow *testWindow = NULL;
    static CWindow *testWindow2 = NULL;
    static CPicture *testWindowPicture = NULL;
    static CFont *testWindowText = NULL;
    static CFont *testWindowText2 = NULL;
    static CTextfield *testTextfield = NULL;
    static CFont *TextFrom_testTextfield = NULL;
    static CTextfield *testTextfield_testWindow = NULL;

    static int picIndex = -1;
    char puffer[80];

    //if this is the first time the function is called
    if (Param == INITIALIZING_CALL)
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
        TESTWINDOW2QUITMESSAGE
    };

    switch (Param)
    {
        case INITIALIZING_CALL:
                SubMenu = new CMenu(SPLASHSCREEN_SUBMENU1);
                if (!global::s2->RegisterMenu(SubMenu))
                {
                    delete SubMenu;
                    SubMenu = NULL;
                    return;
                }
                toMain = SubMenu->addButton(submenu1, MAINMENU, 400, 440, 200, 20, BUTTON_RED1, "zurück");
                greatMoon = SubMenu->addButton(submenu1, GREATMOON, 100, 100, 200, 200, BUTTON_STONE, NULL, MOON);
                greatMoon->setMotionParams(GREATMOONENTRY, GREATMOONLEAVE);
                smallMoon = SubMenu->addButton(submenu1, SMALLMOON, 100, 350, global::bmpArray[MOON].w, global::bmpArray[MOON].h, BUTTON_STONE, NULL, MOON);
                toosmall = SubMenu->addButton(submenu1, TOOSMALL, 100, 400, global::bmpArray[MOON].w-1, global::bmpArray[MOON].h-1, BUTTON_STONE, NULL, MOON);
                createWindow = SubMenu->addButton(submenu1, CREATEWINDOW, 500, 10, 130, 30, BUTTON_GREEN1, "Fenster erzeugen");
                picObject = SubMenu->addPicture(submenu1, PICOBJECT, 200, 30, MIS0BOBS_SHIP);
                picObject->setMotionParams(PICOBJECTENTRY, PICOBJECTLEAVE);
                //text block with \n
                sprintf(puffer, "\nTextblock:\n\nNeue Zeile\nNoch eine neue Zeile");
                SubMenu->addText(puffer, 400, 200, 14);
                testTextfield = SubMenu->addTextfield(400, 300, 10, 3);
                break;

        case MAINMENU:  SubMenu->setWaste();
                        SubMenu = NULL;
                        toMain = NULL;
                        greatMoon = NULL;
                        smallMoon = NULL;
                        toosmall = NULL;
                        createWindow = NULL;
                        ueberschrift = NULL;
                        resolution = NULL;
                        greatMoonText = NULL;
                        counterText = NULL;
                        testWindowPicture = NULL;
                        testWindowText = NULL;
                        testWindowText2 = NULL;
                        testTextfield = NULL;
                        TextFrom_testTextfield = NULL;
                        testTextfield_testWindow = NULL;
                        global::s2->UnregisterCallback(submenu1);
                        if (testWindow != NULL)
                        {
                            testWindow->setWaste();
                            testWindow = NULL;
                        }
                        if (testWindow2 != NULL)
                        {
                            testWindow2->setWaste();
                            testWindow2 = NULL;
                        }
                        picIndex = -1;
                        break;

        case GREATMOON: ueberschrift = SubMenu->addText("Überschrift!", 300, 10, 14);
                        sprintf(puffer, "Fenster X: %d Fenster Y: %d", global::s2->MenuResolutionX, global::s2->MenuResolutionY);
                        resolution = SubMenu->addText(puffer, 10, 10, 14);
                        break;

        case SMALLMOON: SubMenu->delButton(greatMoon);
                        SubMenu->delStaticPicture(picIndex);
                        picIndex = -1;
                        break;

        case TOOSMALL:  if (picIndex == -1)
                            picIndex = SubMenu->addStaticPicture(0, 0, MAINFRAME_640_480);
                        break;

        case CREATEWINDOW:  if (testWindow == NULL)
                            {
                                testWindow = new CWindow(submenu1, TESTWINDOWQUITMESSAGE, 5, 5, 350, 240, "Fenster", WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MOVE | WINDOW_MINIMIZE | WINDOW_RESIZE);
                                if (global::s2->RegisterWindow(testWindow))
                                {
                                    testWindow->addText("Text innerhalb des Fensters", 10, 10, 14);
                                    testWindow->addButton(submenu1, -10, 150, 100, 210, 30, BUTTON_GREEN2, "Button innerhalb des Fensters");
                                    testWindowPicture = testWindow->addPicture(submenu1, TESTWINDOWPICTURE, 10, 60, MIS2BOBS_FORTRESS);
                                    testWindowPicture->setMotionParams(TESTWINDOWPICTUREENTRY, TESTWINDOWPICTURELEAVE);
                                    testTextfield_testWindow = testWindow->addTextfield(130, 30, 10, 3, 14, FONT_RED, BUTTON_GREY, true);
                                    testTextfield_testWindow->setText("Die ist ein sehr langer Testtext in der Hoffnung, das Textfeld ein für alle mal zu sprengen");
                                }
                                else
                                {
                                    delete testWindow;
                                    testWindow = NULL;
                                    return;
                                }
                            }
                            if (testWindow2 == NULL)
                            {
                                testWindow2 = new CWindow(submenu1, TESTWINDOW2QUITMESSAGE, 200, 5, 350, 240, "Noch ein Fenster", WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MOVE | WINDOW_MINIMIZE | WINDOW_RESIZE);
                                if (global::s2->RegisterWindow(testWindow2))
                                {
                                    testWindow2->addText("Text innerhalb des Fensters", 50, 40, 9);
                                    testWindow2->addButton(submenu1, -10, 100, 100, 100, 20, BUTTON_GREEN2, "Button");
                                }
                                else
                                {
                                    delete testWindow2;
                                    testWindow2 = NULL;
                                    return;
                                }
                            }
                            break;

        case GREATMOONENTRY:    if (greatMoonText == NULL)
                                    greatMoonText = SubMenu->addText("Test-Textöäüß", 100, 10, 14);
                                break;

        case GREATMOONLEAVE:    if (greatMoonText != NULL)
                                {
                                    SubMenu->delText(greatMoonText);
                                    greatMoonText = NULL;
                                }
                                break;

        case PICOBJECT: if (greatMoon != NULL)
                        {
                            SubMenu->delButton(greatMoon);
                            greatMoon = NULL;
                        }
                        break;

        case PICOBJECTENTRY:    if (greatMoonText == NULL)
                                    greatMoonText = SubMenu->addText("Test-Textöäüß", 100, 10, 14);
                                break;

        case PICOBJECTLEAVE:    if (greatMoonText != NULL)
                                {
                                    SubMenu->delText(greatMoonText);
                                    greatMoonText = NULL;
                                }
                                break;

        case TESTWINDOWPICTURE: if (testWindowText == NULL)
                                    testWindowText = testWindow->addText("Auf Festung geklickt", 10, 200, 11);
                                else
                                {
                                    testWindow->delText(testWindowText);
                                    testWindowText = NULL;
                                }
                                break;

        case TESTWINDOWPICTUREENTRY:    if (testWindowText2 != NULL)
                                        {
                                            testWindow->delText(testWindowText2);
                                            testWindowText2 = NULL;
                                        }
                                        testWindowText2 = testWindow->addText("Bildbereich betreten", 10, 220, 11);
                                        break;

        case TESTWINDOWPICTURELEAVE:    if (testWindowText2 != NULL)
                                        {
                                            testWindow->delText(testWindowText2);
                                            testWindowText2 = NULL;
                                        }
                                        testWindowText2 = testWindow->addText("Bildbereich verlassen", 10, 220, 11);
                                        break;

        case TESTWINDOWQUITMESSAGE:     testWindow->setWaste();
                                        testWindow = NULL;
                                        break;

        case TESTWINDOW2QUITMESSAGE:    testWindow2->setWaste();
                                        testWindow2 = NULL;
                                        break;

        case CALL_FROM_GAMELOOP:    if (counter % 10 == 0)
                                    {
                                        if (counterText != NULL)
                                        {
                                            SubMenu->delText(counterText);
                                            counterText = NULL;
                                        }
                                        if (counterText == NULL)
                                        {
                                            sprintf(puffer, "zaehler: %d", counter);
                                            counterText = SubMenu->addText(puffer, 100, 20, 9);
                                        }

                                        if (TextFrom_testTextfield != NULL)
                                        {
                                            SubMenu->delText(TextFrom_testTextfield);
                                            TextFrom_testTextfield = NULL;
                                        }
                                        sprintf(puffer, "Der Text im Textfeld lautet: %s", testTextfield->getText());
                                        TextFrom_testTextfield = SubMenu->addText(puffer, 200, 400, 14);
                                    }
                                    counter++;
                                    break;

        default:    break;
    }
}
#endif

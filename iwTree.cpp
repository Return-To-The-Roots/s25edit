// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iwTree.h"
#include "CGame.h"
#include "CIO/CWindow.h"
#include "CMap.h"
#include "globals.h"
#include <cassert>
#include <memory>

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
            WNDTree = global::s2->RegisterWindow(
              std::make_unique<CWindow>(EditorTreeMenu, WINDOWQUIT, Pos, Extent(148, 140), "Trees", WINDOW_GREEN1,
                                        WINDOW_CLOSE | WINDOW_MINIMIZE | WINDOW_MOVE));
            MapObj = global::s2->getMapObj();
            map = MapObj->getMap();
            switch(map->type)
            {
                case MAP_GREENLAND:
                    WNDTree->addPicture(EditorTreeMenu, PICPINE, Position(2, 2), PICTURE_TREE_PINE);
                    WNDTree->addPicture(EditorTreeMenu, PICBIRCH, Position(36, 2), PICTURE_TREE_BIRCH);
                    WNDTree->addPicture(EditorTreeMenu, PICOAK, Position(70, 2), PICTURE_TREE_OAK);
                    WNDTree->addPicture(EditorTreeMenu, PICPALM1, Position(104, 2), PICTURE_TREE_PALM1);
                    WNDTree->addPicture(EditorTreeMenu, PICPALM2, Position(2, 36), PICTURE_TREE_PALM2);
                    WNDTree->addPicture(EditorTreeMenu, PICPINEAPPLE, Position(36, 36), PICTURE_TREE_PINEAPPLE);
                    WNDTree->addPicture(EditorTreeMenu, PICCYPRESS, Position(70, 36), PICTURE_TREE_CYPRESS);
                    WNDTree->addPicture(EditorTreeMenu, PICCHERRY, Position(104, 36), PICTURE_TREE_CHERRY);
                    WNDTree->addPicture(EditorTreeMenu, PICFIR, Position(2, 72), PICTURE_TREE_FIR);
                    WNDTree->addPicture(EditorTreeMenu, PICWOOD_MIXED, Position(36, 70), PICTURE_TREE_WOOD_MIXED);
                    WNDTree->addPicture(EditorTreeMenu, PICPALM_MIXED, Position(70, 70), PICTURE_TREE_PALM_MIXED);
                    break;
                case MAP_WASTELAND:
                    WNDTree->addPicture(EditorTreeMenu, PICFLAPHAT, Position(2, 2), PICTURE_TREE_FLAPHAT);
                    WNDTree->addPicture(EditorTreeMenu, PICSPIDER, Position(36, 2), PICTURE_TREE_SPIDER);
                    WNDTree->addPicture(EditorTreeMenu, PICPINEAPPLE, Position(70, 2), PICTURE_TREE_PINEAPPLE);
                    WNDTree->addPicture(EditorTreeMenu, PICCHERRY, Position(104, 2), PICTURE_TREE_CHERRY);
                    break;
                case MAP_WINTERLAND:
                    WNDTree->addPicture(EditorTreeMenu, PICPINE, Position(2, 2), PICTURE_TREE_PINE);
                    WNDTree->addPicture(EditorTreeMenu, PICBIRCH, Position(36, 2), PICTURE_TREE_BIRCH);
                    WNDTree->addPicture(EditorTreeMenu, PICCYPRESS, Position(70, 2), PICTURE_TREE_CYPRESS);
                    WNDTree->addPicture(EditorTreeMenu, PICFIR, Position(104, 2), PICTURE_TREE_FIR);
                    WNDTree->addPicture(EditorTreeMenu, PICWOOD_MIXED, Position(2, 36), PICTURE_TREE_WOOD_MIXED);
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

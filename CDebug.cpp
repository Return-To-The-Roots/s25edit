// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CDebug.h"
#include "CGame.h"
#include "CIO/CFont.h"
#include "CIO/CWindow.h"
#include "CMap.h"
#include "defines.h"
#include "globals.h"
#include "helpers/format.hpp"
#include <cmath>

#ifdef _ADMINMODE

CDebug::CDebug(void dbgCallback(int), int quitParam)
{
    dbgWnd = global::s2->RegisterWindow(
      std::make_unique<CWindow>(dbgCallback, quitParam, Position(0, 0), Extent(540, 130), "Debugger", WINDOW_GREEN1,
                                WINDOW_CLOSE | WINDOW_MOVE | WINDOW_MINIMIZE | WINDOW_RESIZE));
    dbgWnd->addText("Debugger started", 0, 0, fontsize);
    this->dbgCallback_ = dbgCallback;
    FrameCounterText = nullptr;
    FramesPerSecText = nullptr;
    msWaitText = nullptr;
    RegisteredMenusText = nullptr;
    RegisteredWindowsText = nullptr;
    RegisteredCallbacksText = nullptr;
    DisplayRectText = nullptr;
    MouseText = nullptr;
    MapNameText = nullptr;
    MapSizeText = nullptr;
    MapAuthorText = nullptr;
    MapTypeText = nullptr;
    MapPlayerText = nullptr;
    VertexText = nullptr;
    VertexDataText = nullptr;
    VertexVectorText = nullptr;
    FlatVectorText = nullptr;
    rsuTextureText = nullptr;
    usdTextureText = nullptr;
    roadText = nullptr;
    objectTypeText = nullptr;
    objectInfoText = nullptr;
    animalText = nullptr;
    unknown1Text = nullptr;
    buildText = nullptr;
    unknown2Text = nullptr;
    unknown3Text = nullptr;
    resourceText = nullptr;
    shadingText = nullptr;
    unknown5Text = nullptr;
    editorModeText = nullptr;
    fontsize = FontSize::Small;
    MapObj = global::s2->MapObj.get();
    map = nullptr;
    global::s2->RegisterCallback(dbgCallback);

    // add buttons for in-/decrementing msWait
    dbgWnd->addButton(dbgCallback, DECREMENT_MSWAIT, 75, 30, 15, 15, BUTTON_GREY, "-");
    dbgWnd->addButton(dbgCallback, SETZERO_MSWAIT, 90, 30, 15, 15, BUTTON_GREY, "0");
    dbgWnd->addButton(dbgCallback, INCREMENT_MSWAIT, 105, 30, 15, 15, BUTTON_GREY, "+");

    // we draw a vertical line to separate map data on the right side from things on the left side
    dbgWnd->addText("#", 240, 10, fontsize);
    dbgWnd->addText("#", 240, 20, fontsize);
    dbgWnd->addText("#", 240, 30, fontsize);
    dbgWnd->addText("#", 240, 40, fontsize);
    dbgWnd->addText("#", 240, 50, fontsize);
    dbgWnd->addText("#", 240, 60, fontsize);
    dbgWnd->addText("#", 240, 70, fontsize);
    dbgWnd->addText("#", 240, 80, fontsize);
    dbgWnd->addText("#", 240, 90, fontsize);
    dbgWnd->addText("#", 240, 100, fontsize);
    dbgWnd->addText("#", 240, 110, fontsize);
    dbgWnd->addText("#", 240, 120, fontsize);
    dbgWnd->addText("#", 240, 130, fontsize);
    dbgWnd->addText("#", 240, 140, fontsize);
    dbgWnd->addText("#", 240, 150, fontsize);
    dbgWnd->addText("#", 240, 160, fontsize);
    dbgWnd->addText("#", 240, 170, fontsize);
    dbgWnd->addText("#", 240, 180, fontsize);
    dbgWnd->addText("#", 240, 190, fontsize);
    dbgWnd->addText("#", 240, 200, fontsize);
    dbgWnd->addText("#", 240, 210, fontsize);
    dbgWnd->addText("#", 240, 220, fontsize);
}

CDebug::~CDebug()
{
    global::s2->UnregisterCallback(dbgCallback_);
    dbgWnd->setWaste();
}

void CDebug::sendParam(int Param)
{
    switch(Param)
    {
        case CALL_FROM_GAMELOOP: actualizeData(); break;

        case INCREMENT_MSWAIT: global::s2->msWait++; break;

        case DECREMENT_MSWAIT:
            if(global::s2->msWait > 0)
                global::s2->msWait--;
            break;

        case SETZERO_MSWAIT: global::s2->msWait = 0; break;

        default: break;
    }
}

void CDebug::actualizeData()
{
    if(!FrameCounterText)
        FrameCounterText = dbgWnd->addText("", 0, 10, fontsize);
    // write new FrameCounterText and draw it
    FrameCounterText->setText("Actual Frame: " + std::to_string(global::s2->FrameCounter));

    // Frames per Second
    static Uint32 tmpFrameCtr = 0, tmpTickCtr = SDL_GetTicks();
    if(tmpFrameCtr == 10)
    {
        // write new FramesPerSecText and draw it
        if(!FramesPerSecText)
            FramesPerSecText = dbgWnd->addText("", 0, 20, fontsize);
        FramesPerSecText->setText(
          helpers::format("Frames per Sec: %.2f", tmpFrameCtr / (((float)SDL_GetTicks() - tmpTickCtr) / 1000)));
        // set new values
        tmpFrameCtr = 0;
        tmpTickCtr = SDL_GetTicks();
    } else
        tmpFrameCtr++;

    // del msWaitText before drawing new
    if(!msWaitText)
        msWaitText = dbgWnd->addText("", 0, 35, fontsize);
    // write new msWaitText and draw it
    msWaitText->setText("Wait: " + std::to_string(global::s2->msWait));

    // del MouseText before drawing new
    if(!MouseText)
        MouseText = dbgWnd->addText("", 0, 50, fontsize);
    // write new MouseText and draw it
    const char* clickedStr =
      global::s2->Cursor.clicked ?
        (global::s2->Cursor.button.left ? "LMB clicked" :
                                          (global::s2->Cursor.button.right ? "RMB clicked" : "clicked")) :
        "unclicked";
    MouseText->setText(
      helpers::format("Mouse: x=%d y=%d %s", global::s2->Cursor.pos.x, global::s2->Cursor.pos.y, clickedStr));

    // del RegisteredMenusText before drawing new
    if(!RegisteredMenusText)
        RegisteredMenusText = dbgWnd->addText("", 0, 60, fontsize);
    // write new RegisteredMenusText and draw it
    RegisteredMenusText->setText(helpers::format("Registered Menus: %d", global::s2->Menus.size()));

    // del RegisteredWindowsText before drawing new
    if(!RegisteredWindowsText)
        RegisteredWindowsText = dbgWnd->addText("", 0, 70, fontsize);
    // write new RegisteredWindowsText and draw it
    RegisteredWindowsText->setText(helpers::format("Registered Windows: %d", global::s2->Windows.size()));

    // del RegisteredCallbacksText before drawing new
    if(!RegisteredCallbacksText)
        RegisteredCallbacksText = dbgWnd->addText("", 0, 80, fontsize);
    // write new RegisteredCallbacksText and draw it
    RegisteredCallbacksText->setText(helpers::format("Registered Callbacks: %d", global::s2->Callbacks.size()));

    if(!DisplayRectText)
        DisplayRectText = dbgWnd->addText("", 0, 90, fontsize);
    auto const displayRect = MapObj->getDisplayRect();
    DisplayRectText->setText(helpers::format("DisplayRect: (%d,%d)->(%d,%d)\n= size(%d, %d)", displayRect.left,
                                             displayRect.top, displayRect.right, displayRect.bottom,
                                             displayRect.getSize().x, displayRect.getSize().y));

    // we will now write the map data if a map is active
    MapObj = global::s2->MapObj.get();
    if(MapObj)
    {
        map = MapObj->getMap();
        const MapNode& vertex = map->getVertex(MapObj->Vertex_);

        if(!MapNameText)
            MapNameText = dbgWnd->addText("", 260, 10, fontsize);
        MapNameText->setText("Map Name: " + map->getName());
        if(!MapSizeText)
            MapSizeText = dbgWnd->addText("", 260, 20, fontsize);
        MapSizeText->setText(helpers::format("Width: %d  Height: %d", map->width, map->height));
        if(!MapAuthorText)
            MapAuthorText = dbgWnd->addText("", 260, 30, fontsize);
        MapAuthorText->setText("Author: " + map->getAuthor());
        if(!MapTypeText)
            MapTypeText = dbgWnd->addText("", 260, 40, fontsize);
        const char* ltStr =
          map->type == MAP_GREENLAND ?
            "Greenland" :
            (map->type == MAP_WASTELAND ? "Wasteland" : (map->type == MAP_WINTERLAND ? "Winterland" : "Unknown"));
        MapTypeText->setText(helpers::format("Type: %d (%s)", map->type, ltStr));
        if(!MapPlayerText)
            MapPlayerText = dbgWnd->addText("", 260, 50, fontsize);
        MapPlayerText->setText(helpers::format("Player: %d", map->player));
        if(!VertexText)
            VertexText = dbgWnd->addText("", 260, 60, fontsize);
        VertexText->setText(helpers::format("Vertex: %d, %d", MapObj->Vertex_.x, MapObj->Vertex_.y));
        if(!VertexDataText)
            VertexDataText = dbgWnd->addText("", 260, 70, fontsize);
        VertexDataText->setText(helpers::format("Vertex Data: x=%d, y=%d, z=%d i=%.2f h=%#04x", vertex.x, vertex.y,
                                                vertex.z, ((float)vertex.i) / pow(2, 16), vertex.h));
        if(!VertexVectorText)
            VertexVectorText = dbgWnd->addText("", 260, 80, fontsize);
        VertexVectorText->setText(helpers::format("Vertex Vector: (%.2f, %.2f, %.2f)", vertex.normVector.x,
                                                  vertex.normVector.y, vertex.normVector.z));
        if(!FlatVectorText)
            FlatVectorText = dbgWnd->addText("", 260, 90, fontsize);
        FlatVectorText->setText(helpers::format("Flat Vector: (%.2f, %.2f, %.2f)", vertex.flatVector.x,
                                                vertex.flatVector.y, vertex.flatVector.z));
        if(!rsuTextureText)
            rsuTextureText = dbgWnd->addText("", 260, 100, fontsize);
        rsuTextureText->setText(helpers::format("RSU-Texture: %#04x", vertex.rsuTexture));
        if(!usdTextureText)
            usdTextureText = dbgWnd->addText("", 260, 110, fontsize);
        usdTextureText->setText(helpers::format("USD-Texture: %#04x", vertex.usdTexture));
        if(!roadText)
            roadText = dbgWnd->addText("", 260, 120, fontsize);
        roadText->setText(helpers::format("road: %#04x", vertex.road));
        if(!objectTypeText)
            objectTypeText = dbgWnd->addText("", 260, 130, fontsize);
        objectTypeText->setText(helpers::format("objectType: %#04x", vertex.objectType));
        if(!objectInfoText)
            objectInfoText = dbgWnd->addText("", 260, 140, fontsize);
        objectInfoText->setText(helpers::format("objectInfo: %#04x", vertex.objectInfo));
        if(!animalText)
            animalText = dbgWnd->addText("", 260, 150, fontsize);
        animalText->setText(helpers::format("animal: %#04x", vertex.animal));
        if(!unknown1Text)
            unknown1Text = dbgWnd->addText("", 260, 160, fontsize);
        unknown1Text->setText(helpers::format("unknown1: %#04x", vertex.unknown1));
        if(!buildText)
            buildText = dbgWnd->addText("", 260, 170, fontsize);
        buildText->setText(helpers::format("build: %#04x", vertex.build));
        if(!unknown2Text)
            unknown2Text = dbgWnd->addText("", 260, 180, fontsize);
        unknown2Text->setText(helpers::format("unknown2: %#04x", vertex.unknown2));
        if(!unknown3Text)
            unknown3Text = dbgWnd->addText("", 260, 190, fontsize);
        unknown3Text->setText(helpers::format("unknown3: %#04x", vertex.unknown3));
        if(!resourceText)
            resourceText = dbgWnd->addText("", 260, 200, fontsize);
        resourceText->setText(helpers::format("resource: %#04x", vertex.resource));
        if(!shadingText)
            shadingText = dbgWnd->addText("", 260, 210, fontsize);
        shadingText->setText(helpers::format("shading: %#04x", vertex.shading));
        if(!unknown5Text)
            unknown5Text = dbgWnd->addText("", 260, 220, fontsize);
        unknown5Text->setText(helpers::format("unknown5: %#04x", vertex.unknown5));
        if(!editorModeText)
            editorModeText = dbgWnd->addText("", 260, 230, fontsize);
        editorModeText->setText(helpers::format("Editor --> Mode: %d Content: %#04x Content2: %#04x", MapObj->mode,
                                                MapObj->modeContent, MapObj->modeContent2));
    } else
    {
        if(!MapNameText)
            dbgWnd->addText("", 260, 10, fontsize);
        // write new MapNameText and draw it
        MapNameText->setText("No Map loaded!");
        if(MapSizeText)
        {
            dbgWnd->delText(MapSizeText);
            MapSizeText = nullptr;
        }
        if(MapAuthorText)
        {
            dbgWnd->delText(MapAuthorText);
            MapAuthorText = nullptr;
        }
        if(MapTypeText)
        {
            dbgWnd->delText(MapTypeText);
            MapTypeText = nullptr;
        }
        if(MapPlayerText)
        {
            dbgWnd->delText(MapPlayerText);
            MapPlayerText = nullptr;
        }
        if(VertexText)
        {
            dbgWnd->delText(VertexText);
            VertexText = nullptr;
        }
        if(VertexDataText)
        {
            dbgWnd->delText(VertexDataText);
            VertexDataText = nullptr;
        }
        if(VertexVectorText)
        {
            dbgWnd->delText(VertexVectorText);
            VertexVectorText = nullptr;
        }
        if(FlatVectorText)
        {
            dbgWnd->delText(FlatVectorText);
            FlatVectorText = nullptr;
        }
        if(rsuTextureText)
        {
            dbgWnd->delText(rsuTextureText);
            rsuTextureText = nullptr;
        }
        if(usdTextureText)
        {
            dbgWnd->delText(usdTextureText);
            usdTextureText = nullptr;
        }
        if(roadText)
        {
            dbgWnd->delText(roadText);
            roadText = nullptr;
        }
        if(objectTypeText)
        {
            dbgWnd->delText(objectTypeText);
            objectTypeText = nullptr;
        }
        if(objectInfoText)
        {
            dbgWnd->delText(objectInfoText);
            objectInfoText = nullptr;
        }
        if(animalText)
        {
            dbgWnd->delText(animalText);
            animalText = nullptr;
        }
        if(unknown1Text)
        {
            dbgWnd->delText(unknown1Text);
            unknown1Text = nullptr;
        }
        if(buildText)
        {
            dbgWnd->delText(buildText);
            buildText = nullptr;
        }
        if(unknown2Text)
        {
            dbgWnd->delText(unknown2Text);
            unknown2Text = nullptr;
        }
        if(unknown3Text)
        {
            dbgWnd->delText(unknown3Text);
            unknown3Text = nullptr;
        }
        if(resourceText)
        {
            dbgWnd->delText(resourceText);
            resourceText = nullptr;
        }
        if(shadingText)
        {
            dbgWnd->delText(shadingText);
            shadingText = nullptr;
        }
        if(unknown5Text)
        {
            dbgWnd->delText(unknown5Text);
            unknown5Text = nullptr;
        }
        if(editorModeText)
        {
            dbgWnd->delText(editorModeText);
            editorModeText = nullptr;
        }
    }
    dbgWnd->setDirty();
}

#endif

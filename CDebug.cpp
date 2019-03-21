#include "CDebug.h"
#include "CGame.h"
#include "CIO/CFont.h"
#include "CIO/CWindow.h"
#include "CMap.h"
#include "globals.h"

#ifdef _ADMINMODE

CDebug::CDebug(void dbgCallback(int), int quitParam)
{
    dbgWnd = new CWindow(dbgCallback, quitParam, 0, 0, 540, 130, "Debugger", WINDOW_GREEN1,
                         WINDOW_CLOSE | WINDOW_MOVE | WINDOW_MINIMIZE | WINDOW_RESIZE);
    global::s2->RegisterWindow(dbgWnd);
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
    fontsize = 9;
    MapObj = global::s2->MapObj;
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
    // some puffers to write texts with sprintf()
    std::array<char, 100> puffer1;
    if(!FrameCounterText)
        FrameCounterText = dbgWnd->addText("", 0, 10, fontsize);
    // write new FrameCounterText and draw it
    sprintf(puffer1, "Actual Frame:    %lu", global::s2->FrameCounter);
    FrameCounterText->setText(puffer1);

    // Frames per Second
    static Uint32 tmpFrameCtr = 0, tmpTickCtr = SDL_GetTicks();
    if(tmpFrameCtr == 10)
    {
        // write new FramesPerSecText and draw it
        sprintf(puffer1, "Frames per Sec: %.2f", tmpFrameCtr / (((float)SDL_GetTicks() - tmpTickCtr) / 1000));
        if(!FramesPerSecText)
            FramesPerSecText = dbgWnd->addText("", 0, 20, fontsize);
        FramesPerSecText->setText(puffer1);
        // set new values
        tmpFrameCtr = 0;
        tmpTickCtr = SDL_GetTicks();
    } else
        tmpFrameCtr++;

    // del msWaitText before drawing new
    if(!msWaitText)
        msWaitText = dbgWnd->addText("", 0, 35, fontsize);
    // write new msWaitText and draw it
    sprintf(puffer1, "Wait: %ums", global::s2->msWait);
    msWaitText->setText(puffer1);

    // del MouseText before drawing new
    if(!MouseText)
        MouseText = dbgWnd->addText("", 0, 50, fontsize);
    // write new MouseText and draw it
    sprintf(puffer1, "Mouse: x=%d y=%d %s", global::s2->Cursor.x, global::s2->Cursor.y,
            (global::s2->Cursor.clicked ?
               (global::s2->Cursor.button.left ? "LMB clicked" : (global::s2->Cursor.button.right ? "RMB clicked" : "clicked")) :
               "unclicked"));
    MouseText->setText(puffer1);

    // del RegisteredMenusText before drawing new
    if(!RegisteredMenusText)
        RegisteredMenusText = dbgWnd->addText("", 0, 60, fontsize);
    // write new RegisteredMenusText and draw it
    sprintf(puffer1, "Registered Menus: %d (max. %d)", global::s2->RegisteredMenus, MAXMENUS);
    RegisteredMenusText->setText(puffer1);

    // del RegisteredWindowsText before drawing new
    if(!RegisteredWindowsText)
        RegisteredWindowsText = dbgWnd->addText("", 0, 70, fontsize);
    // write new RegisteredWindowsText and draw it
    sprintf(puffer1, "Registered Windows: %d (max. %d)", global::s2->RegisteredWindows, MAXWINDOWS);
    RegisteredWindowsText->setText(puffer1);

    // del RegisteredCallbacksText before drawing new
    if(!RegisteredCallbacksText)
        RegisteredCallbacksText = dbgWnd->addText("", 0, 80, fontsize);
    // write new RegisteredCallbacksText and draw it
    sprintf(puffer1, "Registered Callbacks: %d (max. %d)", global::s2->RegisteredCallbacks, MAXCALLBACKS);
    RegisteredCallbacksText->setText(puffer1);

    if(!DisplayRectText)
        DisplayRectText = dbgWnd->addText("", 0, 90, fontsize);
    auto const displayRect = MapObj->getDisplayRect();
    sprintf(puffer1, "DisplayRect: (%d,%d)->(%d,%d)\n= size(%d, %d)", displayRect.left, displayRect.top, displayRect.right,
            displayRect.bottom, displayRect.getSize().x, displayRect.getSize().y);
    DisplayRectText->setText(puffer1);

    // we will now write the map data if a map is active
    MapObj = global::s2->MapObj;
    if(MapObj)
    {
        map = MapObj->map;
        const MapNode& vertex = map->getVertex(MapObj->VertexX_, MapObj->VertexY_);

        if(!MapNameText)
            MapNameText = dbgWnd->addText("", 260, 10, fontsize);
        sprintf(puffer1, "Map Name: %s", map->name);
        MapNameText->setText(puffer1);
        if(!MapSizeText)
            MapSizeText = dbgWnd->addText("", 260, 20, fontsize);
        sprintf(puffer1, "Width: %d  Height: %d", map->width, map->height);
        MapSizeText->setText(puffer1);
        if(!MapAuthorText)
            MapAuthorText = dbgWnd->addText("", 260, 30, fontsize);
        sprintf(puffer1, "Author: %s", map->author);
        MapAuthorText->setText(puffer1);
        if(!MapTypeText)
            MapTypeText = dbgWnd->addText("", 260, 40, fontsize);
        sprintf(puffer1, "Type: %d (%s)", map->type,
                (map->type == MAP_GREENLAND ?
                   "Greenland" :
                   (map->type == MAP_WASTELAND ? "Wasteland" : (map->type == MAP_WINTERLAND ? "Winterland" : "Unknown"))));
        MapTypeText->setText(puffer1);
        if(!MapPlayerText)
            MapPlayerText = dbgWnd->addText("", 260, 50, fontsize);
        sprintf(puffer1, "Player: %d", map->player);
        MapPlayerText->setText(puffer1);
        if(!VertexText)
            VertexText = dbgWnd->addText("", 260, 60, fontsize);
        sprintf(puffer1, "Vertex: %d, %d", MapObj->VertexX_, MapObj->VertexY_);
        VertexText->setText(puffer1);
        if(!VertexDataText)
            VertexDataText = dbgWnd->addText("", 260, 70, fontsize);
        sprintf(puffer1, "Vertex Data: x=%d, y=%d, z=%d i=%.2f h=%#04x", vertex.x, vertex.y, vertex.z, ((float)vertex.i) / pow(2, 16),
                vertex.h);
        VertexDataText->setText(puffer1);
        if(!VertexVectorText)
            VertexVectorText = dbgWnd->addText("", 260, 80, fontsize);
        sprintf(puffer1, "Vertex Vector: (%.2f, %.2f, %.2f)", vertex.normVector.x, vertex.normVector.y, vertex.normVector.z);
        VertexVectorText->setText(puffer1);
        if(!FlatVectorText)
            FlatVectorText = dbgWnd->addText("", 260, 90, fontsize);
        sprintf(puffer1, "Flat Vector: (%.2f, %.2f, %.2f)", vertex.flatVector.x, vertex.flatVector.y, vertex.flatVector.z);
        FlatVectorText->setText(puffer1);
        if(!rsuTextureText)
            rsuTextureText = dbgWnd->addText("", 260, 100, fontsize);
        sprintf(puffer1, "RSU-Texture: %#04x", vertex.rsuTexture);
        rsuTextureText->setText(puffer1);
        if(!usdTextureText)
            usdTextureText = dbgWnd->addText("", 260, 110, fontsize);
        sprintf(puffer1, "USD-Texture: %#04x", vertex.usdTexture);
        usdTextureText->setText(puffer1);
        if(!roadText)
            roadText = dbgWnd->addText("", 260, 120, fontsize);
        sprintf(puffer1, "road: %#04x", vertex.road);
        roadText->setText(puffer1);
        if(!objectTypeText)
            objectTypeText = dbgWnd->addText("", 260, 130, fontsize);
        sprintf(puffer1, "objectType: %#04x", vertex.objectType);
        objectTypeText->setText(puffer1);
        if(!objectInfoText)
            objectInfoText = dbgWnd->addText("", 260, 140, fontsize);
        sprintf(puffer1, "objectInfo: %#04x", vertex.objectInfo);
        objectInfoText->setText(puffer1);
        if(!animalText)
            animalText = dbgWnd->addText("", 260, 150, fontsize);
        sprintf(puffer1, "animal: %#04x", vertex.animal);
        animalText->setText(puffer1);
        if(!unknown1Text)
            unknown1Text = dbgWnd->addText("", 260, 160, fontsize);
        sprintf(puffer1, "unknown1: %#04x", vertex.unknown1);
        unknown1Text->setText(puffer1);
        if(!buildText)
            buildText = dbgWnd->addText("", 260, 170, fontsize);
        sprintf(puffer1, "build: %#04x", vertex.build);
        buildText->setText(puffer1);
        if(!unknown2Text)
            unknown2Text = dbgWnd->addText("", 260, 180, fontsize);
        sprintf(puffer1, "unknown2: %#04x", vertex.unknown2);
        unknown2Text->setText(puffer1);
        if(!unknown3Text)
            unknown3Text = dbgWnd->addText("", 260, 190, fontsize);
        sprintf(puffer1, "unknown3: %#04x", vertex.unknown3);
        unknown3Text->setText(puffer1);
        if(!resourceText)
            resourceText = dbgWnd->addText("", 260, 200, fontsize);
        sprintf(puffer1, "resource: %#04x", vertex.resource);
        resourceText->setText(puffer1);
        if(!shadingText)
            shadingText = dbgWnd->addText("", 260, 210, fontsize);
        sprintf(puffer1, "shading: %#04x", vertex.shading);
        shadingText->setText(puffer1);
        if(!unknown5Text)
            unknown5Text = dbgWnd->addText("", 260, 220, fontsize);
        sprintf(puffer1, "unknown5: %#04x", vertex.unknown5);
        unknown5Text->setText(puffer1);
        if(!editorModeText)
            editorModeText = dbgWnd->addText("", 260, 230, fontsize);
        sprintf(puffer1, "Editor --> Mode: %d Content: %#04x Content2: %#04x", MapObj->mode, MapObj->modeContent, MapObj->modeContent2);
        editorModeText->setText(puffer1);
    } else
    {
        if(!MapNameText)
            dbgWnd->addText("", 260, 10, fontsize);
        // write new MapNameText and draw it
        sprintf(puffer1, "No Map loaded!");
        MapNameText->setText(puffer1);
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

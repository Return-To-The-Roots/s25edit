#include "CDebug.h"

#ifdef _ADMINMODE

CDebug::CDebug(void dbgCallback(int), int quitParam)
{
    dbgWnd = new CWindow(dbgCallback, quitParam, 0, 0, 540, 150, "Debugger", WINDOW_GREEN1, WINDOW_CLOSE | WINDOW_MOVE | WINDOW_MINIMIZE | WINDOW_RESIZE);
    global::s2->RegisterWindow(dbgWnd);
    dbgWnd->addText("Debugger started", 0, 0, fontsize);
    this->dbgCallback = dbgCallback;
    FrameCounterText = NULL;
    FramesPerSecText = NULL;
    msWaitText = NULL;
    RegisteredMenusText = NULL;
    RegisteredWindowsText = NULL;
    RegisteredCallbacksText = NULL;
    MouseText = NULL;
    MapNameText = NULL;
    MapSizeText = NULL;
    MapAuthorText = NULL;
    MapTypeText = NULL;
    MapPlayerText = NULL;
    VertexText = NULL;
    VertexDataText = NULL;
    VertexVectorText = NULL;
    FlatVectorText = NULL;
    rsuTextureText = NULL;
    usdTextureText = NULL;
    ShadingButton = NULL;
    fontsize = 9;
    MapObj = global::s2->Map;
    map = NULL;
    global::s2->RegisterCallback(dbgCallback);

    //add buttons for in-/decrementing msWait
    dbgWnd->addButton(dbgCallback, DECREMENT_MSWAIT, 75, 30, 15, 15, BUTTON_GREY, "-");
    dbgWnd->addButton(dbgCallback, SETZERO_MSWAIT, 90, 30, 15, 15, BUTTON_GREY, "0");
    dbgWnd->addButton(dbgCallback, INCREMENT_MSWAIT, 105, 30, 15, 15, BUTTON_GREY, "+");

    //we draw a vertical line to separate map data on the right side from things on the left side
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
}

CDebug::~CDebug()
{
    global::s2->UnregisterCallback(dbgCallback);
    dbgWnd->setWaste();
}

void CDebug::sendParam(int Param)
{
    switch (Param)
    {
        case CALL_FROM_GAMELOOP:    actualizeData();
                                    break;

        case INCREMENT_MSWAIT:      global::s2->msWait++;
                                    break;

        case DECREMENT_MSWAIT:      if (global::s2->msWait > 0)
                                        global::s2->msWait--;
                                    break;

        case SETZERO_MSWAIT:        global::s2->msWait = 0;
                                    break;

        case CHANGE_SHADING:        if (CSurface::gouraud)
                                        CSurface::gouraud = false;
                                    else
                                        CSurface::gouraud = true;
                                    if (MapObj != NULL)
                                        MapObj->needSurface = true;
                                    if (ShadingButton != NULL)
                                    {
                                        dbgWnd->delButton(ShadingButton);
                                        ShadingButton = NULL;
                                    }
                                    break;

        default:                    break;
    }
}

void CDebug::actualizeData(void)
{
    //del FrameCounterText before drawing new
    if (FrameCounterText != NULL)
        dbgWnd->delText(FrameCounterText);
    //write new FrameCounterText and draw it
    sprintf(puffer1, "Actual Frame:    %ld", global::s2->FrameCounter);
    FrameCounterText = dbgWnd->addText(puffer1, 0, 10, fontsize);

    //Frames per Second
    static float tmpFrameCtr = 0, tmpTickCtr = (float)SDL_GetTicks();
    if (tmpFrameCtr == 10)
    {
        //del FramesPerSecText before drawing new
        if (FramesPerSecText != NULL)
            dbgWnd->delText(FramesPerSecText);
        //write new FramesPerSecText and draw it
        sprintf(puffer1, "Frames per Sec: %.2f", tmpFrameCtr/(((float)SDL_GetTicks() - tmpTickCtr)/1000));
        FramesPerSecText = dbgWnd->addText(puffer1, 0, 20, fontsize);
        //set new values
        tmpFrameCtr = 0;
        tmpTickCtr = (float)SDL_GetTicks();
    }
    else
        tmpFrameCtr++;

    //del msWaitText before drawing new
    if (msWaitText != NULL)
        dbgWnd->delText(msWaitText);
    //write new msWaitText and draw it
    sprintf(puffer1, "Wait: %dms", global::s2->msWait);
    msWaitText = dbgWnd->addText(puffer1, 0, 35, fontsize);

    //del MouseText before drawing new
    if (MouseText != NULL)
        dbgWnd->delText(MouseText);
    //write new MouseText and draw it
    sprintf(puffer1, "Mouse: x=%d y=%d %s", global::s2->Cursor.x, global::s2->Cursor.y, (global::s2->Cursor.clicked ? (global::s2->Cursor.button.left ? "LMB clicked" : (global::s2->Cursor.button.right ? "RMB clicked" : "clicked")) : "unclicked"));
    MouseText = dbgWnd->addText(puffer1, 0, 50, fontsize);

    //del RegisteredMenusText before drawing new
    if (RegisteredMenusText != NULL)
        dbgWnd->delText(RegisteredMenusText);
    //write new RegisteredMenusText and draw it
    sprintf(puffer1, "Registered Menus: %d (max. %d)", global::s2->RegisteredMenus, MAXMENUS);
    RegisteredMenusText = dbgWnd->addText(puffer1, 0, 60, fontsize);

    //del RegisteredWindowsText before drawing new
    if (RegisteredWindowsText != NULL)
        dbgWnd->delText(RegisteredWindowsText);
    //write new RegisteredWindowsText and draw it
    sprintf(puffer1, "Registered Windows: %d (max. %d)", global::s2->RegisteredWindows, MAXWINDOWS);
    RegisteredWindowsText = dbgWnd->addText(puffer1, 0, 70, fontsize);

    //del RegisteredCallbacksText before drawing new
    if (RegisteredCallbacksText != NULL)
        dbgWnd->delText(RegisteredCallbacksText);
    //write new RegisteredCallbacksText and draw it
    sprintf(puffer1, "Registered Callbacks: %d (max. %d)", global::s2->RegisteredCallbacks, MAXCALLBACKS);
    RegisteredCallbacksText = dbgWnd->addText(puffer1, 0, 80, fontsize);

    //we will now write the map data if a map is active
    MapObj = global::s2->Map;
    if (MapObj != NULL)
    {
        map = MapObj->map;

        if (MapNameText != NULL)
            dbgWnd->delText(MapNameText);
        sprintf(puffer1, "Map Name: %s", map->name);
        MapNameText = dbgWnd->addText(puffer1, 260, 10, fontsize);
        if (MapSizeText != NULL)
            dbgWnd->delText(MapSizeText);
        sprintf(puffer1, "Width: %d  Height: %d", map->width, map->height);
        MapSizeText = dbgWnd->addText(puffer1, 260, 20, fontsize);
        if (MapAuthorText != NULL)
            dbgWnd->delText(MapAuthorText);
        sprintf(puffer1, "Author: %s", map->author);
        MapAuthorText = dbgWnd->addText(puffer1, 260, 30, fontsize);
        if (MapTypeText != NULL)
            dbgWnd->delText(MapTypeText);
        sprintf(puffer1, "Type: %d (%s)", map->type, (map->type == MAP_GREENLAND ? "Greenland" : (map->type == MAP_WASTELAND ? "Wasteland" : (map->type == MAP_WINTERLAND ? "Winterland" : "Unknown"))));
        MapTypeText = dbgWnd->addText(puffer1, 260, 40, fontsize);
        if (MapPlayerText != NULL)
            dbgWnd->delText(MapPlayerText);
        sprintf(puffer1, "Player: %d", map->player);
        MapPlayerText = dbgWnd->addText(puffer1, 260, 50, fontsize);
        if (VertexText != NULL)
            dbgWnd->delText(VertexText);
        sprintf(puffer1, "Vertex: %d, %d", MapObj->VertexX, MapObj->VertexY);
        VertexText = dbgWnd->addText(puffer1, 260, 60, fontsize);
        if (VertexDataText != NULL)
            dbgWnd->delText(VertexDataText);
        sprintf(puffer1, "Vertex Data: x=%ld, y=%ld, z=%d i=%.2f", map->vertex[MapObj->VertexY*map->width+MapObj->VertexX].x, map->vertex[MapObj->VertexY*map->width+MapObj->VertexX].y, map->vertex[MapObj->VertexY*map->width+MapObj->VertexX].z, ((float)map->vertex[MapObj->VertexY*map->width+MapObj->VertexX].i)/pow(2,16));
        VertexDataText = dbgWnd->addText(puffer1, 260, 70, fontsize);
        if (VertexVectorText != NULL)
            dbgWnd->delText(VertexVectorText);
        sprintf(puffer1, "Vertex Vector: (%.2f, %.2f, %.2f)", map->vertex[MapObj->VertexY*map->width+MapObj->VertexX].normVector.x, map->vertex[MapObj->VertexY*map->width+MapObj->VertexX].normVector.y, map->vertex[MapObj->VertexY*map->width+MapObj->VertexX].normVector.z);
        VertexVectorText = dbgWnd->addText(puffer1, 260, 80, fontsize);
        if (FlatVectorText != NULL)
            dbgWnd->delText(FlatVectorText);
        sprintf(puffer1, "Flat Vector: (%.2f, %.2f, %.2f)", map->vertex[MapObj->VertexY*map->width+MapObj->VertexX].flatVector.x, map->vertex[MapObj->VertexY*map->width+MapObj->VertexX].flatVector.y, map->vertex[MapObj->VertexY*map->width+MapObj->VertexX].flatVector.z);
        FlatVectorText = dbgWnd->addText(puffer1, 260, 90, fontsize);
        if (rsuTextureText != NULL)
            dbgWnd->delText(rsuTextureText);
        sprintf(puffer1, "RSU-Texture: %#04x", map->vertex[MapObj->VertexY*map->width+MapObj->VertexX].rsuTexture);
        rsuTextureText = dbgWnd->addText(puffer1, 260, 100, fontsize);
        if (usdTextureText != NULL)
            dbgWnd->delText(usdTextureText);
        sprintf(puffer1, "USD-Texture: %#04x", map->vertex[MapObj->VertexY*map->width+MapObj->VertexX].usdTexture);
        usdTextureText = dbgWnd->addText(puffer1, 260, 110, fontsize);

        if (ShadingButton == NULL)
        {
            if (CSurface::gouraud)
                ShadingButton = dbgWnd->addButton(dbgCallback, CHANGE_SHADING, 410, 40, 80, 20, BUTTON_GREY, "Flat");
            else
                ShadingButton = dbgWnd->addButton(dbgCallback, CHANGE_SHADING, 410, 40, 80, 20, BUTTON_GREY, "Gouraud");
        }
    }
    else
    {
        //del MapNameText before drawing new
        if (MapNameText != NULL)
            dbgWnd->delText(MapNameText);
        //write new MapNameText and draw it
        sprintf(puffer1, "No Map loaded!");
        MapNameText = dbgWnd->addText(puffer1, 260, 10, fontsize);
        if (MapSizeText != NULL)
        {
            dbgWnd->delText(MapSizeText);
            MapSizeText = NULL;
        }
        if (MapAuthorText != NULL)
        {
            dbgWnd->delText(MapAuthorText);
            MapAuthorText = NULL;
        }
        if (MapTypeText != NULL)
        {
            dbgWnd->delText(MapTypeText);
            MapTypeText = NULL;
        }
        if (MapPlayerText != NULL)
        {
            dbgWnd->delText(MapPlayerText);
            MapPlayerText = NULL;
        }
        if (VertexText != NULL)
        {
            dbgWnd->delText(VertexText);
            VertexText = NULL;
        }
        if (VertexDataText != NULL)
        {
            dbgWnd->delText(VertexDataText);
            VertexDataText = NULL;
        }
        if (VertexVectorText != NULL)
        {
            dbgWnd->delText(VertexVectorText);
            VertexVectorText = NULL;
        }
        if (FlatVectorText != NULL)
        {
            dbgWnd->delText(FlatVectorText);
            FlatVectorText = NULL;
        }
        if (rsuTextureText != NULL)
        {
            dbgWnd->delText(rsuTextureText);
            rsuTextureText = NULL;
        }
        if (usdTextureText != NULL)
        {
            dbgWnd->delText(usdTextureText);
            usdTextureText = NULL;
        }
        if (ShadingButton != NULL)
        {
            dbgWnd->delButton(ShadingButton);
            ShadingButton = NULL;
        }
    }
}

#endif

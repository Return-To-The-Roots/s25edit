#include "CMap.h"

CMap::CMap(char *filename)
{
    constructMap(filename);
}

CMap::~CMap()
{
    destructMap();
}

void CMap::constructMap(char *filename, int width, int height, int type, int texture, int border, int border_texture)
{
    map = NULL;
    Surf_Map = NULL;
    Surf_RightMenubar = NULL;
    displayRect.x = 0;
    displayRect.y = 0;
    displayRect.w = global::s2->GameResolutionX;
    displayRect.h = global::s2->GameResolutionY;

    if (filename != NULL)
        map = (bobMAP*)CFile::open_file(filename, WLD); //TODO: open_file(filename, SWD); if really necessary

    if (map == NULL)
        map = generateMap(width, height, type, texture, border, border_texture);


    //load the right MAP0x.LST for all pictures
    loadMapPics();

    CSurface::get_nodeVectors(map);
    #ifdef _EDITORMODE
    //for safety recalculate build and shadow data and test if fishes and water is correct
    for (int i = 0; i < map->height; i++)
    {
        for (int j = 0; j < map->width; j++)
        {
            modifyBuild(j, i);
            modifyShading(j, i);
            modifyResource(j, i);
        }
    }
    #endif
    needSurface = true;
    active = true;
    VertexX = 10;
    VertexY = 10;
    BuildHelp = false;
    MouseBlitX = correctMouseBlitX(VertexX, VertexY);
    MouseBlitY = correctMouseBlitY(VertexX, VertexY);
    ChangeSection = 1;
    lastChangeSection = ChangeSection;
    ChangeSectionHexagonMode = true;
    VertexFillRSU = true;
    VertexFillUSD = true;
    VertexFillRandom = false;
    VertexActivityRandom = false;
    //calculate the maximum number of vertices for cursor
    VertexCounter = 0;
    for (int i = -MAX_CHANGE_SECTION; i <= MAX_CHANGE_SECTION; i++)
    {
        if (abs(i)%2 == 0)
            for (int j = -MAX_CHANGE_SECTION; j <= MAX_CHANGE_SECTION; j++)
                VertexCounter++;
        else
            for (int j = -MAX_CHANGE_SECTION; j <= MAX_CHANGE_SECTION-1; j++)
                VertexCounter++;
    }
    Vertices = (struct cursorPoint*)malloc(VertexCounter*sizeof(struct cursorPoint));
    calculateVertices();
    setupVerticesActivity();
    mode = EDITOR_MODE_HEIGHT_RAISE;
    lastMode = EDITOR_MODE_HEIGHT_RAISE;
    modeContent = 0x00;
    modeContent2 = 0x00;
    modify = false;
    MaxRaiseHeight = 0x3C;
    MinReduceHeight = 0x00;
    saveCurrentVertices = false;
    if ( (CurrPtr_savedVertices = (struct savedVertices*)malloc(sizeof(struct savedVertices))) != NULL)
    {
        CurrPtr_savedVertices->empty = true;
        CurrPtr_savedVertices->prev = NULL;
        CurrPtr_savedVertices->next = NULL;
    }

    //we count the players, cause the original editor writes number of players to header no matter if they are set or not
    int CountPlayers = 0;
    //now for internal reasons save all players to a new array, also players with number greater than 7
    //initalize the internal array
    for (int i = 0; i < MAXPLAYERS; i++)
    {
        PlayerHQx[i] = 0xFFFF;
        PlayerHQy[i] = 0xFFFF;
    }
    //find out player positions
    for (int y = 0; y < map->height; y++)
    {
        for (int x = 0; x < map->width; x++)
        {
            if (map->vertex[y*map->width+x].objectInfo == 0x80)
            {
                CountPlayers++;
                //objectType is the number of the player
                if (map->vertex[y*map->width+x].objectType < MAXPLAYERS)
                {
                    PlayerHQx[map->vertex[y*map->width+x].objectType] = x;
                    PlayerHQy[map->vertex[y*map->width+x].objectType] = y;

                    //for compatibility with original settlers 2 save the first 7 player positions to the map header
                    //NOTE: this is already done by map loading, but to prevent inconsistence we do it again this way
                    if (map->vertex[y*map->width+x].objectType < 0x07)
                    {
                        map->HQx[map->vertex[y*map->width+x].objectType] = x;
                        map->HQy[map->vertex[y*map->width+x].objectType] = y;
                    }
                }
            }
        }
    }
    map->player = CountPlayers;
}
void CMap::destructMap(void)
{
    //free all surfaces that MAP0x.LST needed
    unloadMapPics();
    //free concatenated list for "undo" and "do"
    if (CurrPtr_savedVertices != NULL)
    {
        //go to the end
        while (CurrPtr_savedVertices->next != NULL)
        {
            CurrPtr_savedVertices = CurrPtr_savedVertices->next;
        }
        //and now free all pointers from behind
        while (CurrPtr_savedVertices->prev != NULL)
        {
            CurrPtr_savedVertices = CurrPtr_savedVertices->prev;
            free(CurrPtr_savedVertices->next);
        }
        free(CurrPtr_savedVertices);
    }
    //free the map surface
    SDL_FreeSurface(Surf_Map);
    //free the surface of the right menubar
    SDL_FreeSurface(Surf_RightMenubar);
    //free vertex array
    free(Vertices);
    //free vertex memory
    free(map->vertex);
    //free map structure memory
    free(map);
}

bobMAP* CMap::generateMap(int width, int height, int type, int texture, int border, int border_texture)
{
    bobMAP *myMap = (bobMAP*)malloc(sizeof(bobMAP));
    Uint8 heightFactor;

    strcpy(myMap->name, "Ohne Namen");
    myMap->width = width;
    myMap->width_pixel = myMap->width*TRIANGLE_WIDTH;
    myMap->height = height;
    myMap->height_pixel = myMap->height*TRIANGLE_HEIGHT;
    myMap->type = type;
    myMap->player = 0;
    strcpy(myMap->author, "Niemand");
    for (int i = 0; i < 7; i++)
    {
        myMap->HQx[i] = 0xFFFF;
        myMap->HQy[i] = 0xFFFF;
    }

    if ( (myMap->vertex = (struct point*) malloc(sizeof(struct point)*myMap->width*myMap->height)) == NULL )
    {
        free(myMap);
        return NULL;
    }

    int a;
    int b = 0;
    for (int j = 0; j < myMap->height; j++)
    {
        if (j%2 == 0)
            a = TRIANGLE_WIDTH/2;
        else
            a = TRIANGLE_WIDTH;

        for (int i = 0; i < myMap->width; i++)
        {
            myMap->vertex[j*myMap->width+i].VertexX = i;
            myMap->vertex[j*myMap->width+i].VertexY = j;
            heightFactor = 0x0A;
            myMap->vertex[j*myMap->width+i].h = heightFactor;
            myMap->vertex[j*myMap->width+i].x = a;
            myMap->vertex[j*myMap->width+i].y = b + (-TRIANGLE_INCREASE)*(heightFactor - 0x0A);
            myMap->vertex[j*myMap->width+i].z = TRIANGLE_INCREASE*(heightFactor - 0x0A);
            a += TRIANGLE_WIDTH;

            if ( (j < border || myMap->height-j <= border) || (i < border || myMap->width-i <= border) )
            {
                myMap->vertex[j*myMap->width+i].rsuTexture = border_texture;
                myMap->vertex[j*myMap->width+i].usdTexture = border_texture;
            }
            else
            {
                myMap->vertex[j*myMap->width+i].rsuTexture = texture;
                myMap->vertex[j*myMap->width+i].usdTexture = texture;
            }

            //initialize all other blocks -- outcommented blocks are recalculated at map load
            myMap->vertex[j*myMap->width+i].road = 0x00;
            myMap->vertex[j*myMap->width+i].objectType = 0x00;
            myMap->vertex[j*myMap->width+i].objectInfo = 0x00;
            myMap->vertex[j*myMap->width+i].animal = 0x00;
            myMap->vertex[j*myMap->width+i].unknown1 = 0x00;
            //myMap->vertex[j*myMap->width+i].build = 0x00;
            myMap->vertex[j*myMap->width+i].unknown2 = 0x07;
            myMap->vertex[j*myMap->width+i].unknown3 = 0x00;
            //myMap->vertex[j*myMap->width+i].resource = 0x00;
            //myMap->vertex[j*myMap->width+i].shading = 0x00;
            myMap->vertex[j*myMap->width+i].unknown5 = 0x00;
        }
        b += TRIANGLE_HEIGHT;
    }

    return myMap;
}

void CMap::loadMapPics(void)
{
    char outputString1[47], outputString2[34];
    char picFile[17];
    switch (map->type)
    {
        case 0:     strcpy(outputString1, "\nLoading palette from file: /DATA/MAP00.LST...");
                    strcpy(outputString2, "\nLoading file: /DATA/MAP00.LST...");
                    strcpy(picFile, "./DATA/MAP00.LST");
                    break;
        case 1:     strcpy(outputString1, "\nLoading palette from file: /DATA/MAP01.LST...");
                    strcpy(outputString2, "\nLoading file: /DATA/MAP01.LST...");
                    strcpy(picFile, "./DATA/MAP01.LST");
                    break;
        case 2:     strcpy(outputString1, "\nLoading palette from file: /DATA/MAP02.LST...");
                    strcpy(outputString2, "\nLoading file: /DATA/MAP02.LST...");
                    strcpy(picFile, "./DATA/MAP02.LST");
                    break;
        default:    strcpy(outputString1, "\nLoading palette from file: /DATA/MAP00.LST...");
                    strcpy(outputString2, "\nLoading file: /DATA/MAP00.LST...");
                    strcpy(picFile, "./DATA/MAP00.LST");
                    break;
    }
    //load only the palette at this time from MAP0x.LST
    std::cout << outputString1;
    if ( CFile::open_file(picFile, LST, true) == false )
    {
        std::cout << "failure";
    }
    //set the right palette
    CFile::set_palActual(CFile::get_palArray()-1);
    std::cout << outputString2;
    if ( CFile::open_file(picFile, LST) == false )
    {
        std::cout << "failure";
    }
    //set back palette
    //CFile::set_palActual(CFile::get_palArray());
    //std::cout << "\nLoading file: /DATA/MBOB/ROM_BOBS.LST...";
    //if ( CFile::open_file("./DATA/MBOB/ROM_BOBS.LST", LST) == false )
    //{
    //    std::cout << "failure";
    //}
    //set back palette
    CFile::set_palActual(CFile::get_palArray());
}

void CMap::unloadMapPics(void)
{
    for (int i = MAPPIC_ARROWCROSS_YELLOW; i <= MAPPIC_LAST_ENTRY; i++)
    {
        SDL_FreeSurface(global::bmpArray[i].surface);
        global::bmpArray[i].surface = NULL;
    }
    //set back bmpArray-pointer, cause MAP0x.LST is no longer needed
    CFile::set_bmpArray(global::bmpArray+MAPPIC_ARROWCROSS_YELLOW);
}

void CMap::setMouseData(SDL_MouseMotionEvent motion)
{
#ifdef _WIN32
    //following code important for blitting the right field of the map
    static bool warping = false;
    //is right mouse button pressed?
    if (SDL_GetMouseState(NULL,NULL)&SDL_BUTTON(3))
    {
        //this whole "warping-thing" is to prevent cursor-moving WITHIN the window while user moves over the map
        if (warping == false)
        {
            displayRect.x += motion.xrel;
            displayRect.y += motion.yrel;
            warping = true;
            SDL_WarpMouse(motion.x-motion.xrel, motion.y-motion.yrel);
        }
        else
            warping = false;

        //reset coords of displayRects when end of map is reached
        if (displayRect.x >= map->width*TRIANGLE_WIDTH)
            displayRect.x = 0;
        else if (displayRect.x <= -displayRect.w)
            displayRect.x = map->width*TRIANGLE_WIDTH - displayRect.w;

        if (displayRect.y >= map->height*TRIANGLE_HEIGHT)
            displayRect.y = 0;
        else if (displayRect.y <= -displayRect.h)
            displayRect.y = map->height*TRIANGLE_HEIGHT - displayRect.h;
    }
#else
    if (SDL_GetMouseState(NULL,NULL)&SDL_BUTTON(3))
    {
        displayRect.x += motion.xrel;
        displayRect.y += motion.yrel;

        //reset coords of displayRects when end of map is reached
        if (displayRect.x >= map->width*TRIANGLE_WIDTH)
            displayRect.x = 0;
        else if (displayRect.x <= -displayRect.w)
            displayRect.x = map->width*TRIANGLE_WIDTH - displayRect.w;

        if (displayRect.y >= map->height*TRIANGLE_HEIGHT)
            displayRect.y = 0;
        else if (displayRect.y <= -displayRect.h)
            displayRect.y = map->height*TRIANGLE_HEIGHT - displayRect.h;
    }
#endif

    saveVertex(motion.x, motion.y, motion.state);
}

void CMap::setMouseData(SDL_MouseButtonEvent button)
{
    if (button.state == SDL_PRESSED)
    {
        #ifdef _EDITORMODE
        //find out if user clicked on one of the game menu pictures
        //we start with lower menubar
        if (button.button == SDL_BUTTON_LEFT && button.x >= (displayRect.w/2-236) && button.x <= (displayRect.w/2-199)
                                                  && button.y >= (displayRect.h-35) && button.y <= (displayRect.h-3)
           )
        {
            //the height-mode picture was clicked
            mode = EDITOR_MODE_HEIGHT_RAISE;
            return;
        }
        else if (button.button == SDL_BUTTON_LEFT && button.x >= (displayRect.w/2-199) && button.x <= (displayRect.w/2-162)
                                                  && button.y >= (displayRect.h-35) && button.y <= (displayRect.h-3)
           )
        {
            //the texture-mode picture was clicked
            mode = EDITOR_MODE_TEXTURE;
            callback::EditorTextureMenu(INITIALIZING_CALL);
            return;
        }
        else if (button.button == SDL_BUTTON_LEFT && button.x >= (displayRect.w/2-162) && button.x <= (displayRect.w/2-125)
                                                  && button.y >= (displayRect.h-35) && button.y <= (displayRect.h-3)
           )
        {
            //the tree-mode picture was clicked
            mode = EDITOR_MODE_TREE;
            callback::EditorTreeMenu(INITIALIZING_CALL);
            return;
        }
        else if (button.button == SDL_BUTTON_LEFT && button.x >= (displayRect.w/2-125) && button.x <= (displayRect.w/2-88)
                                                  && button.y >= (displayRect.h-35) && button.y <= (displayRect.h-3)
           )
        {
            //the resource-mode picture was clicked
            mode = EDITOR_MODE_RESOURCE_RAISE;
            callback::EditorResourceMenu(INITIALIZING_CALL);
            return;
        }
        else if (button.button == SDL_BUTTON_LEFT && button.x >= (displayRect.w/2-88) && button.x <= (displayRect.w/2-51)
                                                  && button.y >= (displayRect.h-35) && button.y <= (displayRect.h-3)
           )
        {
            //the landscape-mode picture was clicked
            mode = EDITOR_MODE_LANDSCAPE;
            callback::EditorLandscapeMenu(INITIALIZING_CALL);
            return;
        }
        else if (button.button == SDL_BUTTON_LEFT && button.x >= (displayRect.w/2-51) && button.x <= (displayRect.w/2-14)
                                                  && button.y >= (displayRect.h-35) && button.y <= (displayRect.h-3)
           )
        {
            //the animal-mode picture was clicked
            mode = EDITOR_MODE_ANIMAL;
            callback::EditorAnimalMenu(INITIALIZING_CALL);
            return;
        }
        else if (button.button == SDL_BUTTON_LEFT && button.x >= (displayRect.w/2-14) && button.x <= (displayRect.w/2+23)
                                                  && button.y >= (displayRect.h-35) && button.y <= (displayRect.h-3)
           )
        {
            //the player-mode picture was clicked
            mode = EDITOR_MODE_FLAG;
            ChangeSection = 0;
            setupVerticesActivity();
            callback::EditorPlayerMenu(INITIALIZING_CALL);
            return;
        }
        else if (button.button == SDL_BUTTON_LEFT && button.x >= (displayRect.w/2+96) && button.x <= (displayRect.w/2+133)
                                                  && button.y >= (displayRect.h-35) && button.y <= (displayRect.h-3)
           )
        {
            //the build-help picture was clicked
            BuildHelp = !BuildHelp;
            return;
        }
        else if (button.button == SDL_BUTTON_LEFT && button.x >= (displayRect.w/2+131) && button.x <= (displayRect.w/2+168)
                                                  && button.y >= (displayRect.h-35) && button.y <= (displayRect.h-3)
           )
        {
            //the minimap picture was clicked
            callback::MinimapMenu(INITIALIZING_CALL);
            return;
        }
        else if (button.button == SDL_BUTTON_LEFT && button.x >= (displayRect.w/2+166) && button.x <= (displayRect.w/2+203)
                                                  && button.y >= (displayRect.h-35) && button.y <= (displayRect.h-3)
           )
        {
            //the create-world picture was clicked
            callback::EditorCreateMenu(INITIALIZING_CALL);
            return;
        }
        else if (button.button == SDL_BUTTON_LEFT && button.x >= (displayRect.w/2+203) && button.x <= (displayRect.w/2+240)
                                             && button.y >= (displayRect.h-35) && button.y <= (displayRect.h-3)
           )
        {
            //the editor-main-menu picture was clicked
            callback::EditorQuitMenu(INITIALIZING_CALL); //"quit" menu is temporary, later this will be "main" menu
            return;
        }
        //now we check the right menubar
        else if (button.button == SDL_BUTTON_LEFT && button.x >= (displayRect.w-37) && button.x <= (displayRect.w)
                                                  && button.y >= (displayRect.h/2+200) && button.y <= (displayRect.h/2+237)
           )
        {
            //the temproray save-map picture was clicked
            callback::PleaseWait(INITIALIZING_CALL);
            //for safety recalculate build and shadow data --> is done on map load, so it's not necessary again
            //for (int i = 0; i < map->height; i++)
            //{
            //    for (int j = 0; j < map->width; j++)
            //    {
            //        modifyBuild(j, i);
            //        modifyShading(j, i);
            //    }
            //}
            CFile::save_file("./WORLDS/NEW_MAP.SWD", SWD, map);
            callback::PleaseWait(WINDOW_QUIT_MESSAGE);
            return;
        }
        else if (button.button == SDL_BUTTON_LEFT && button.x >= (displayRect.w-37) && button.x <= (displayRect.w)
                                                  && button.y >= (displayRect.h/2-239) && button.y <= (displayRect.h/2-202)
           )
        {
            //the cursor picture was clicked
            callback::EditorCursorMenu(INITIALIZING_CALL);
            return;
        }
        else
        {
            //no picture was clicked

            //touch vertex data
            if (button.button == SDL_BUTTON_LEFT)
            {
                modify = true;
                saveCurrentVertices = true;
            }
        }
        #else
        //find out if user clicked on one of the game menu pictures
        if (button.button == SDL_BUTTON_LEFT && button.x >= (displayRect.w/2-74) && button.x <= (displayRect.w/2-37)
                                             && button.y >= (displayRect.h-37) && button.y <= (displayRect.h-4)
           )
        {
            //the first picture was clicked
            callback::GameMenu(INITIALIZING_CALL);
        }
        #endif
    }
    else if (button.state == SDL_RELEASED)
    {
        #ifdef _EDITORMODE
        //stop touching vertex data
        if (button.button == SDL_BUTTON_LEFT)
            modify = false;
        #else

        #endif
    }
}

void CMap::setKeyboardData(SDL_KeyboardEvent key)
{
    if (key.type == SDL_KEYDOWN)
    {
        if (key.keysym.sym == SDLK_LSHIFT && mode == EDITOR_MODE_HEIGHT_RAISE)
            mode = EDITOR_MODE_HEIGHT_REDUCE;
        else if (key.keysym.sym == SDLK_LALT && mode == EDITOR_MODE_HEIGHT_RAISE)
            mode = EDITOR_MODE_HEIGHT_PLANE;
        else if (key.keysym.sym == SDLK_e && (mode == EDITOR_MODE_HEIGHT_RAISE || mode == EDITOR_MODE_HEIGHT_REDUCE))
        {
            if (MaxRaiseHeight > 0x00)
                MaxRaiseHeight--;
        }
        else if (key.keysym.sym == SDLK_r && (mode == EDITOR_MODE_HEIGHT_RAISE || mode == EDITOR_MODE_HEIGHT_REDUCE))
        {
            if (MaxRaiseHeight < 0x3C)
                MaxRaiseHeight++;
        }
        else if (key.keysym.sym == SDLK_s && (mode == EDITOR_MODE_HEIGHT_RAISE || mode == EDITOR_MODE_HEIGHT_REDUCE))
        {
            if (MinReduceHeight > 0x00)
                MinReduceHeight--;
        }
        else if (key.keysym.sym == SDLK_d && (mode == EDITOR_MODE_HEIGHT_RAISE || mode == EDITOR_MODE_HEIGHT_REDUCE))
        {
            if (MinReduceHeight < 0x3C)
                MinReduceHeight++;
        }
        else if (key.keysym.sym == SDLK_LSHIFT && mode == EDITOR_MODE_RESOURCE_RAISE)
            mode = EDITOR_MODE_RESOURCE_REDUCE;
        else if (key.keysym.sym == SDLK_LSHIFT && mode == EDITOR_MODE_FLAG)
            mode = EDITOR_MODE_FLAG_DELETE;
        else if (key.keysym.sym == SDLK_LCTRL)
        {
            lastMode = mode;
            mode = EDITOR_MODE_CUT;
        }
        else if (key.keysym.sym == SDLK_b && mode != EDITOR_MODE_HEIGHT_MAKE_BIG_HOUSE && mode != EDITOR_MODE_TEXTURE_MAKE_HARBOUR)
        {
            lastMode = mode;
            lastChangeSection = ChangeSection;
            ChangeSection = 0;
            setupVerticesActivity();
            mode = EDITOR_MODE_HEIGHT_MAKE_BIG_HOUSE;
        }
        else if (key.keysym.sym == SDLK_h && mode != EDITOR_MODE_HEIGHT_MAKE_BIG_HOUSE && mode != EDITOR_MODE_TEXTURE_MAKE_HARBOUR)
        {
            lastMode = mode;
            lastChangeSection = ChangeSection;
            ChangeSection = 0;
            setupVerticesActivity();
            mode = EDITOR_MODE_TEXTURE_MAKE_HARBOUR;
        }
        else if (key.keysym.sym == SDLK_KP_PLUS)
        {
            if (ChangeSection < MAX_CHANGE_SECTION)
            {
                ChangeSection++;
                setupVerticesActivity();
            }
        }
        else if (key.keysym.sym == SDLK_KP_MINUS)
        {
            if (ChangeSection > 0)
            {
                ChangeSection--;
                setupVerticesActivity();
            }
        }
        else if (key.keysym.sym == SDLK_1 || key.keysym.sym == SDLK_KP1)
        {
            ChangeSection = 0;
            setupVerticesActivity();
        }
        else if (key.keysym.sym == SDLK_2 || key.keysym.sym == SDLK_KP2)
        {
            ChangeSection = 1;
            setupVerticesActivity();
        }
        else if (key.keysym.sym == SDLK_3 || key.keysym.sym == SDLK_KP3)
        {
            ChangeSection = 2;
            setupVerticesActivity();
        }
        else if (key.keysym.sym == SDLK_4 || key.keysym.sym == SDLK_KP4)
        {
            ChangeSection = 3;
            setupVerticesActivity();
        }
        else if (key.keysym.sym == SDLK_5 || key.keysym.sym == SDLK_KP5)
        {
            ChangeSection = 4;
            setupVerticesActivity();
        }
        else if (key.keysym.sym == SDLK_6 || key.keysym.sym == SDLK_KP6)
        {
            ChangeSection = 5;
            setupVerticesActivity();
        }
        else if (key.keysym.sym == SDLK_7 || key.keysym.sym == SDLK_KP7)
        {
            ChangeSection = 6;
            setupVerticesActivity();
        }
        else if (key.keysym.sym == SDLK_8 || key.keysym.sym == SDLK_KP8)
        {
            ChangeSection = 7;
            setupVerticesActivity();
        }
        else if (key.keysym.sym == SDLK_9 || key.keysym.sym == SDLK_KP9)
        {
            ChangeSection = 8;
            setupVerticesActivity();
        }

        else if (key.keysym.sym == SDLK_SPACE)
        {
            BuildHelp = !BuildHelp;
        }
        else if (key.keysym.sym == SDLK_q)
        {
            if (!saveCurrentVertices)
            {
                if (CurrPtr_savedVertices != NULL && CurrPtr_savedVertices->prev != NULL)
                {
                    CurrPtr_savedVertices = CurrPtr_savedVertices->prev;
                    //if (CurrPtr_savedVertices->next->empty && CurrPtr_savedVertices->prev != NULL)
                    //    CurrPtr_savedVertices = CurrPtr_savedVertices->prev;
                    for (int i = CurrPtr_savedVertices->VertexX-MAX_CHANGE_SECTION-10-2, k = 0; i <= CurrPtr_savedVertices->VertexX+MAX_CHANGE_SECTION+10+2; i++, k++)
                    {

                        for (int j = CurrPtr_savedVertices->VertexY-MAX_CHANGE_SECTION-10-2, l = 0; j <= CurrPtr_savedVertices->VertexY+MAX_CHANGE_SECTION+10+2; j++, l++)
                        {
                            int m = i;
                            if (m < 0)  m += map->width;
                            else if (m >= map->width) m -= map->width;
                            int n = j;
                            if (n < 0)  n += map->height;
                            else if (n >= map->height) n -= map->height;
                            memcpy(&(map->vertex[n*map->width+m]), &(CurrPtr_savedVertices->PointsArroundVertex[l*((MAX_CHANGE_SECTION+10+2)*2+1)+k]), sizeof(struct point));
                        }
                    }
                }
            }
        }
        /*else if (key.keysym.sym == SDLK_w)
        {
            if (!saveCurrentVertices)
            {
                if (CurrPtr_savedVertices != NULL)
                {
                    if (CurrPtr_savedVertices->next != NULL)
                        CurrPtr_savedVertices = CurrPtr_savedVertices->next;
                    if (!CurrPtr_savedVertices->empty)
                    {
                        for (int i = CurrPtr_savedVertices->VertexX-MAX_CHANGE_SECTION-10-2, k = 0; i <= CurrPtr_savedVertices->VertexX+MAX_CHANGE_SECTION+10+2; i++, k++)
                        {

                            for (int j = CurrPtr_savedVertices->VertexY-MAX_CHANGE_SECTION-10-2, l = 0; j <= CurrPtr_savedVertices->VertexY+MAX_CHANGE_SECTION+10+2; j++, l++)
                            {
                                int m = i;
                                if (m < 0)  m += map->width;
                                else if (m >= map->width) m -= map->width;
                                int n = j;
                                if (n < 0)  n += map->height;
                                else if (n >= map->height) n -= map->height;
                                memcpy(&(map->vertex[n*map->width+m]), &(CurrPtr_savedVertices->PointsArroundVertex[l*((MAX_CHANGE_SECTION+10+2)*2+1)+k]), sizeof(struct point));
                            }
                        }
                    }
                }
            }
        }*/
        else if (key.keysym.sym == SDLK_UP || key.keysym.sym == SDLK_DOWN || key.keysym.sym == SDLK_LEFT || key.keysym.sym == SDLK_RIGHT)
        {
            //move displayRect
            displayRect.x += (key.keysym.sym == SDLK_LEFT ? -100 : (key.keysym.sym == SDLK_RIGHT ? 100 : 0));
            displayRect.y += (key.keysym.sym == SDLK_UP ? -100 : (key.keysym.sym == SDLK_DOWN ? 100 : 0));

            //reset coords of displayRects when end of map is reached
            if (displayRect.x >= map->width*TRIANGLE_WIDTH)
                displayRect.x = 0;
            else if (displayRect.x <= -displayRect.w)
                displayRect.x = map->width*TRIANGLE_WIDTH - displayRect.w;

            if (displayRect.y >= map->height*TRIANGLE_HEIGHT)
                displayRect.y = 0;
            else if (displayRect.y <= -displayRect.h)
                displayRect.y = map->height*TRIANGLE_HEIGHT - displayRect.h;
        }
        //convert map to greenland
        else if (key.keysym.sym == SDLK_g)
        {
            callback::PleaseWait(INITIALIZING_CALL);

            //we have to close the windows and initialize them again to prevent failures
            callback::EditorCursorMenu(MAP_QUIT);
            callback::EditorTextureMenu(MAP_QUIT);
            callback::EditorTreeMenu(MAP_QUIT);
            callback::EditorLandscapeMenu(MAP_QUIT);
            callback::MinimapMenu(MAP_QUIT);
            callback::EditorResourceMenu(MAP_QUIT);
            callback::EditorAnimalMenu(MAP_QUIT);
            callback::EditorPlayerMenu(MAP_QUIT);

            map->type = 0;
            unloadMapPics();
            loadMapPics();

            callback::PleaseWait(WINDOW_QUIT_MESSAGE);
        }
        //convert map to wasteland
        else if (key.keysym.sym == SDLK_o)
        {
            callback::PleaseWait(INITIALIZING_CALL);

            //we have to close the windows and initialize them again to prevent failures
            callback::EditorCursorMenu(MAP_QUIT);
            callback::EditorTextureMenu(MAP_QUIT);
            callback::EditorTreeMenu(MAP_QUIT);
            callback::EditorLandscapeMenu(MAP_QUIT);
            callback::MinimapMenu(MAP_QUIT);
            callback::EditorResourceMenu(MAP_QUIT);
            callback::EditorAnimalMenu(MAP_QUIT);
            callback::EditorPlayerMenu(MAP_QUIT);

            map->type = 1;
            unloadMapPics();
            loadMapPics();

            callback::PleaseWait(WINDOW_QUIT_MESSAGE);
        }
        //convert map to winterland
        else if (key.keysym.sym == SDLK_w)
        {
            callback::PleaseWait(INITIALIZING_CALL);

            //we have to close the windows and initialize them again to prevent failures
            callback::EditorCursorMenu(MAP_QUIT);
            callback::EditorTextureMenu(MAP_QUIT);
            callback::EditorTreeMenu(MAP_QUIT);
            callback::EditorLandscapeMenu(MAP_QUIT);
            callback::MinimapMenu(MAP_QUIT);
            callback::EditorResourceMenu(MAP_QUIT);
            callback::EditorAnimalMenu(MAP_QUIT);
            callback::EditorPlayerMenu(MAP_QUIT);

            map->type = 2;
            unloadMapPics();
            loadMapPics();

            callback::PleaseWait(WINDOW_QUIT_MESSAGE);
        }
    }
    else if (key.type == SDL_KEYUP)
    {
        //user probably released EDITOR_MODE_HEIGHT_REDUCE
        if (key.keysym.sym == SDLK_LSHIFT && mode == EDITOR_MODE_HEIGHT_REDUCE)
            mode = EDITOR_MODE_HEIGHT_RAISE;
        //user probably released EDITOR_MODE_HEIGHT_PLANE
        else if (key.keysym.sym == SDLK_LALT && mode == EDITOR_MODE_HEIGHT_PLANE)
            mode = EDITOR_MODE_HEIGHT_RAISE;
        //user probably released EDITOR_MODE_RESOURCE_REDUCE
        else if (key.keysym.sym == SDLK_LSHIFT && mode == EDITOR_MODE_RESOURCE_REDUCE)
            mode = EDITOR_MODE_RESOURCE_RAISE;
        //user probably released EDITOR_MODE_FLAG_DELETE
        else if (key.keysym.sym == SDLK_LSHIFT && mode == EDITOR_MODE_FLAG_DELETE)
            mode = EDITOR_MODE_FLAG;
        //user probably released EDITOR_MODE_CUT
        else if (key.keysym.sym == SDLK_LCTRL)
            mode = lastMode;
        //user probably released EDITOR_MODE_HEIGHT_MAKE_BIG_HOUSE
        else if (key.keysym.sym == SDLK_b)
        {
            mode = lastMode;
            ChangeSection = lastChangeSection;
            setupVerticesActivity();
        }
        //user probably released EDITOR_MODE_TEXTURE_MAKE_HARBOUR
        else if (key.keysym.sym == SDLK_h)
        {
            mode = lastMode;
            ChangeSection = lastChangeSection;
            setupVerticesActivity();
        }
    }
}

void CMap::saveVertex(Uint16 MouseX, Uint16 MouseY, Uint8 MouseState)
{
    //if user raises or reduces the height of a vertex, don't let the cursor jump to another vertex
    //if ( (MouseState == SDL_PRESSED) && (mode == EDITOR_MODE_HEIGHT_RAISE || mode == EDITOR_MODE_HEIGHT_REDUCE) )
        //return;

    int X = 0, Xeven = 0, Xuneven = 0;
    int Y = 0, MousePosY = 0;

    //get X
    //following out commented lines are the correct ones, but for tolerance (to prevent to early jumps of the cursor) we substract "TRIANGLE_WIDTH/2"
    //Xeven = (MouseX + displayRect.x) / TRIANGLE_WIDTH;
    Xeven = (MouseX + displayRect.x - TRIANGLE_WIDTH/2) / TRIANGLE_WIDTH;
    if (Xeven < 0)
        Xeven += (map->width);
    else if (Xeven > map->width-1)
        Xeven -= (map->width-1);
    //Xuneven = (MouseX + displayRect.x + TRIANGLE_WIDTH/2) / TRIANGLE_WIDTH;
    Xuneven = (MouseX + displayRect.x) / TRIANGLE_WIDTH;
    if (Xuneven < 0)
        Xuneven += (map->width-1);
    else if (Xuneven > map->width-1)
        Xuneven -= (map->width);

    MousePosY = MouseY + displayRect.y;
    //correct mouse position Y if displayRect is outside map edges
    if (MousePosY < 0)
        MousePosY += map->height_pixel;
    else if (MousePosY > map->height_pixel)
        MousePosY = MouseY - (map->height_pixel - displayRect.y);

    //get Y
    for (int j = 0; j < map->height; j++)
    {
        if (j%2 == 0)
        {
            //substract "TRIANGLE_HEIGHT/2" is for tolerance, we did the same for X
            if ((MousePosY - TRIANGLE_HEIGHT/2) > map->vertex[j*map->width+Xeven].y)
                Y++;
            else
            {
                X = Xuneven;
                break;
            }
        }
        else
        {
            if ((MousePosY - TRIANGLE_HEIGHT/2) > map->vertex[j*map->width+Xuneven].y)
                Y++;
            else
            {
                X = Xeven;
                break;
            }
        }
    }
    if (Y < 0)
        Y += (map->height-1);
    else if (Y > map->height-1)
        Y -= (map->height-1);

    VertexX = X;
    VertexY = Y;

    MouseBlitX = correctMouseBlitX(VertexX, VertexY);
    MouseBlitY = correctMouseBlitY(VertexX, VertexY);

    calculateVertices();
}

int CMap::correctMouseBlitX(int VertexX, int VertexY)
{
    int MouseBlitX = map->vertex[VertexY*map->width+VertexX].x;
    if (MouseBlitX < displayRect.x)
        MouseBlitX += map->width_pixel;
    else if (MouseBlitX > (displayRect.x+displayRect.w))
        MouseBlitX -= map->width_pixel;
    MouseBlitX -= displayRect.x;

    return MouseBlitX;
}
int CMap::correctMouseBlitY(int VertexX, int VertexY)
{
    int MouseBlitY = map->vertex[VertexY*map->width+VertexX].y;
    if (MouseBlitY < displayRect.y)
        MouseBlitY += map->height_pixel;
    else if (MouseBlitY > (displayRect.y+displayRect.h))
        MouseBlitY -= map->height_pixel;
    MouseBlitY -= displayRect.y;

    return MouseBlitY;
}

bool CMap::render(void)
{
    char textBuffer[100];

    //check if gameresolution has been changed
    if (displayRect.w != global::s2->GameResolutionX || displayRect.h != global::s2->GameResolutionY)
    {
        displayRect.w = global::s2->GameResolutionX;
        displayRect.h = global::s2->GameResolutionY;
        needSurface = true;
    }

    //if we need a new surface
    if (needSurface)
    {
        SDL_FreeSurface(Surf_Map);
        Surf_Map = NULL;
        if ( (Surf_Map = SDL_CreateRGBSurface(SDL_SWSURFACE, displayRect.w, displayRect.h, 32, 0, 0, 0, 0)) == NULL )
            return false;
        needSurface = false;
    }
    //else
        //clear the surface before drawing new (in normal case not needed)
        //SDL_FillRect( Surf_Map, NULL, SDL_MapRGB(Surf_Map->format,0,0,0) );

    //touch vertex data if user modifies it
    if (modify)
        modifyVertex();

    //if (map->vertex != NULL)
        CSurface::DrawTriangleField(Surf_Map, displayRect, map);


    //draw pictures to cursor position
#ifdef _EDITORMODE
    int symbol_index, symbol_index2 = -1;
    switch (mode)
    {
        case EDITOR_MODE_CUT:                   symbol_index = CURSOR_SYMBOL_SCISSORS;
                                                break;
        case EDITOR_MODE_TREE:                  symbol_index = CURSOR_SYMBOL_TREE;
                                                break;
        case EDITOR_MODE_HEIGHT_RAISE:          symbol_index = CURSOR_SYMBOL_ARROW_UP;
                                                break;
        case EDITOR_MODE_HEIGHT_REDUCE:         symbol_index = CURSOR_SYMBOL_ARROW_DOWN;
                                                break;
        case EDITOR_MODE_HEIGHT_PLANE:          symbol_index = CURSOR_SYMBOL_ARROW_UP;
                                                symbol_index2 = CURSOR_SYMBOL_ARROW_DOWN;
                                                break;
        case EDITOR_MODE_HEIGHT_MAKE_BIG_HOUSE: symbol_index = MAPPIC_ARROWCROSS_RED_HOUSE_BIG;
                                                break;
        case EDITOR_MODE_TEXTURE:               symbol_index = CURSOR_SYMBOL_TEXTURE;
                                                break;
        case EDITOR_MODE_TEXTURE_MAKE_HARBOUR:  symbol_index = MAPPIC_ARROWCROSS_RED_HOUSE_HARBOUR;
                                                break;
        case EDITOR_MODE_LANDSCAPE:             symbol_index = CURSOR_SYMBOL_LANDSCAPE;
                                                break;
        case EDITOR_MODE_FLAG:                  symbol_index = CURSOR_SYMBOL_FLAG;
                                                break;
        case EDITOR_MODE_FLAG_DELETE:           symbol_index = CURSOR_SYMBOL_FLAG;
                                                break;
        case EDITOR_MODE_RESOURCE_REDUCE:       symbol_index = CURSOR_SYMBOL_PICKAXE_MINUS;
                                                break;
        case EDITOR_MODE_RESOURCE_RAISE:        symbol_index = CURSOR_SYMBOL_PICKAXE_PLUS;
                                                break;
        case EDITOR_MODE_ANIMAL:                symbol_index = CURSOR_SYMBOL_ANIMAL;
                                                break;
        default:                                symbol_index = CURSOR_SYMBOL_ARROW_UP;
                                                break;
    }
    for (int i = 0; i < VertexCounter; i++)
    {
        if (Vertices[i].active)
        {
            CSurface::Draw(Surf_Map, global::bmpArray[symbol_index].surface, Vertices[i].blit_x-10, Vertices[i].blit_y-10);
            if (symbol_index2 >= 0)
                CSurface::Draw(Surf_Map, global::bmpArray[symbol_index2].surface, Vertices[i].blit_x, Vertices[i].blit_y-7);
        }
    }

    //text for x and y of vertex (shown in upper left corner)
    sprintf(textBuffer, "%d    %d", VertexX, VertexY);
    CFont::writeText(Surf_Map, textBuffer, 20, 20);
    //text for MinReduceHeight and MaxRaiseHeight
    sprintf(textBuffer, "min. Höhe: %#04x/0x3C  max. Höhe: %#04x/0x3C  NormalNull: 0x0A", MinReduceHeight, MaxRaiseHeight);
    CFont::writeText(Surf_Map, textBuffer, 100, 20);
#else
    CSurface::Draw(Surf_Map, global::bmpArray[CIRCLE_FLAT_GREY].surface, MouseBlitX-10, MouseBlitY-10);
#endif


    //draw the frame
    if (displayRect.w == 640 && displayRect.h == 480)
        CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_640_480].surface, 0, 0);
    else if (displayRect.w == 800 && displayRect.h == 600)
        CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_800_600].surface, 0, 0);
    else if (displayRect.w == 1024 && displayRect.h == 768)
        CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_1024_768].surface, 0, 0);
    else if (displayRect.w == 1280 && displayRect.h == 1024)
    {
        CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_LEFT_1280_1024].surface, 0, 0);
        CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_RIGHT_1280_1024].surface, 640, 0);
    }
    else
    {
        int x=150, y=150;
        //draw the corners
        CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_640_480].surface, 0, 0, 0, 0, 150, 150);
        CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_640_480].surface, 0, displayRect.h-150, 0, 480-150, 150, 150);
        CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_640_480].surface, displayRect.w-150, 0, 640-150, 0, 150, 150);
        CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_640_480].surface, displayRect.w-150, displayRect.h-150, 640-150, 480-150, 150, 150);
        //draw the edges
        while (x < displayRect.w-150)
        {
            CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_640_480].surface, x, 0, 150, 0, 150, 12);
            CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_640_480].surface, x, displayRect.h-12, 150, 0, 150, 12);
            x+=150;
        }
        while (y < displayRect.h-150)
        {
            CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_640_480].surface, 0, y, 0, 150, 12, 150);
            CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_640_480].surface, displayRect.w-12, y, 0, 150, 12, 150);
            y+=150;
        }
    }

    //draw the statues at the frame
    CSurface::Draw(Surf_Map, global::bmpArray[STATUE_UP_LEFT].surface, 12, 12);
    CSurface::Draw(Surf_Map, global::bmpArray[STATUE_UP_RIGHT].surface, displayRect.w-global::bmpArray[STATUE_UP_RIGHT].w-12, 12);
    CSurface::Draw(Surf_Map, global::bmpArray[STATUE_DOWN_LEFT].surface, 12, displayRect.h-global::bmpArray[STATUE_DOWN_LEFT].h-12);
    CSurface::Draw(Surf_Map, global::bmpArray[STATUE_DOWN_RIGHT].surface, displayRect.w-global::bmpArray[STATUE_DOWN_RIGHT].w-12, displayRect.h-global::bmpArray[STATUE_DOWN_RIGHT].h-12);

    //lower menubar
    //draw lower menubar
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR].surface, displayRect.w/2-global::bmpArray[MENUBAR].w/2, displayRect.h-global::bmpArray[MENUBAR].h);

    //draw pictures to lower menubar
#ifdef _EDITORMODE
    //backgrounds
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w/2-236, displayRect.h-36, 0, 0, 37, 32);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w/2-199, displayRect.h-36, 0, 0, 37, 32);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w/2-162, displayRect.h-36, 0, 0, 37, 32);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w/2-125, displayRect.h-36, 0, 0, 37, 32);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w/2-88, displayRect.h-36, 0, 0, 37, 32);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w/2-51, displayRect.h-36, 0, 0, 37, 32);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w/2-14, displayRect.h-36, 0, 0, 37, 32);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w/2+92, displayRect.h-36, 0, 0, 37, 32);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w/2+129, displayRect.h-36, 0, 0, 37, 32);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w/2+166, displayRect.h-36, 0, 0, 37, 32);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w/2+203, displayRect.h-36, 0, 0, 37, 32);
    //pictures
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_HEIGHT].surface, displayRect.w/2-232, displayRect.h-35);
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_TEXTURE].surface, displayRect.w/2-195, displayRect.h-35);
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_TREE].surface, displayRect.w/2-158, displayRect.h-37);
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_RESOURCE].surface, displayRect.w/2-121, displayRect.h-32);
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_LANDSCAPE].surface, displayRect.w/2-84, displayRect.h-37);
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_ANIMAL].surface, displayRect.w/2-48, displayRect.h-36);
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_PLAYER].surface, displayRect.w/2-10, displayRect.h-34);

    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_BUILDHELP].surface, displayRect.w/2+96, displayRect.h-35);
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_MINIMAP].surface, displayRect.w/2+131, displayRect.h-37);
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_NEWWORLD].surface, displayRect.w/2+166, displayRect.h-37);
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_COMPUTER].surface, displayRect.w/2+207, displayRect.h-35);
#else

#endif

#ifdef _EDITORMODE
    //right menubar
    //do we need a surface?
    if (Surf_RightMenubar == NULL)
    {
        //we permute width and height, cause we want to rotate the menubar 90 degrees
        if ( (Surf_RightMenubar = SDL_CreateRGBSurface(SDL_SWSURFACE, global::bmpArray[MENUBAR].h, global::bmpArray[MENUBAR].w, 8, 0, 0, 0, 0)) != NULL )
        {
            SDL_SetPalette(Surf_RightMenubar, SDL_LOGPAL, global::palArray[PAL_RESOURCE].colors, 0, 256);
            SDL_SetColorKey(Surf_RightMenubar, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(Surf_RightMenubar->format, 0, 0, 0));
            CSurface::Draw(Surf_RightMenubar, global::bmpArray[MENUBAR].surface, 0, 0, 270);
        }
    }
    //draw right menubar (remember permutation of width and height)
    CSurface::Draw(Surf_Map, Surf_RightMenubar, displayRect.w-global::bmpArray[MENUBAR].h, displayRect.h/2-global::bmpArray[MENUBAR].w/2);

    //draw pictures to right menubar
    //backgrounds
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w-36, displayRect.h/2-239, 0, 0, 32, 37);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w-36, displayRect.h/2-202, 0, 0, 32, 37);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w-36, displayRect.h/2-165, 0, 0, 32, 37);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w-36, displayRect.h/2-128, 0, 0, 32, 37);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w-36, displayRect.h/2-22, 0, 0, 32, 37);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w-36, displayRect.h/2+15, 0, 0, 32, 37);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w-36, displayRect.h/2+52, 0, 0, 32, 37);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w-36, displayRect.h/2+89, 0, 0, 32, 37);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w-36, displayRect.h/2+126, 0, 0, 32, 37);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w-36, displayRect.h/2+163, 0, 0, 32, 37);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, displayRect.w-36, displayRect.h/2+200, 0, 0, 32, 37);
    //pictures
    //temprorary to save a map
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_BUGKILL].surface, displayRect.w-37, displayRect.h/2+202);
    //four cursor menu pictures
    CSurface::Draw(Surf_Map, global::bmpArray[CURSOR_SYMBOL_ARROW_UP].surface, displayRect.w-33, displayRect.h/2-237);
    CSurface::Draw(Surf_Map, global::bmpArray[CURSOR_SYMBOL_ARROW_DOWN].surface, displayRect.w-20, displayRect.h/2-235);
    CSurface::Draw(Surf_Map, global::bmpArray[CURSOR_SYMBOL_ARROW_DOWN].surface, displayRect.w-33, displayRect.h/2-220);
    CSurface::Draw(Surf_Map, global::bmpArray[CURSOR_SYMBOL_ARROW_UP].surface, displayRect.w-20, displayRect.h/2-220);

#endif

    return true;
}

void CMap::drawMinimap(SDL_Surface *Window)
{
    Uint32 *pixel;
    Uint32 *row;

    Uint8 r8,g8,b8;
    Sint16 r,g,b;

    //this variables are needed to reduce the size of minimap-windows of big maps
    int num_x = (map->width > 256 ? map->width/256 : 1);
    int num_y = (map->height > 256 ? map->height/256 : 1);

    //if (Window->w < map->width || Window->h < map->height)
        //return;

    for (int y = 0; y < map->height; y++)
    {
        if (y%num_y != 0)
            continue;

        row = (Uint32 *)Window->pixels + (y+20)*Window->pitch/4;
        for (int x = 0; x < map->width; x++)
        {
            if (x%num_x != 0)
                continue;

            switch (map->vertex[y*map->width+x].rsuTexture)
            {
                case TRIANGLE_TEXTURE_STEPPE_MEADOW1:   r = (map->type == 0x00 ? 100 : (map->type == 0x01 ? 68 : 160));
                                                        g = (map->type == 0x00 ? 144 : (map->type == 0x01 ? 72 : 172));
                                                        b = (map->type == 0x00 ? 20 : (map->type == 0x01 ? 80 : 204));
                                                        break;
                case TRIANGLE_TEXTURE_MINING1:          r = (map->type == 0x00 ? 156 : (map->type == 0x01 ? 112 : 84));
                                                        g = (map->type == 0x00 ? 128 : (map->type == 0x01 ? 108 : 88));
                                                        b = (map->type == 0x00 ? 88 : (map->type == 0x01 ? 84 : 108));
                                                        break;
                case TRIANGLE_TEXTURE_SNOW:             r = (map->type == 0x00 ? 180 : (map->type == 0x01 ? 132 : 0));
                                                        g = (map->type == 0x00 ? 192 : (map->type == 0x01 ? 0 : 48));
                                                        b = (map->type == 0x00 ? 200 : (map->type == 0x01 ? 0: 104));
                                                        break;
                case TRIANGLE_TEXTURE_SWAMP:            r = (map->type == 0x00 ? 100 : (map->type == 0x01 ? 0 : 0));
                                                        g = (map->type == 0x00 ? 144 : (map->type == 0x01 ? 24 : 40));
                                                        b = (map->type == 0x00 ? 20 : (map->type == 0x01 ? 32 : 108));
                                                        break;
                case TRIANGLE_TEXTURE_STEPPE:           r = (map->type == 0x00 ? 192 : (map->type == 0x01 ? 156 : 0));
                                                        g = (map->type == 0x00 ? 156 : (map->type == 0x01 ? 124 : 112));
                                                        b = (map->type == 0x00 ? 124 : (map->type == 0x01 ? 100 : 176));
                                                        break;
                case TRIANGLE_TEXTURE_WATER:            r = (map->type == 0x00 ? 16 : (map->type == 0x01 ? 68 : 0));
                                                        g = (map->type == 0x00 ? 56 : (map->type == 0x01 ? 68 : 48));
                                                        b = (map->type == 0x00 ? 164 : (map->type == 0x01 ? 44 : 104));
                                                        break;
                case TRIANGLE_TEXTURE_MEADOW1:          r = (map->type == 0x00 ? 72 : (map->type == 0x01 ? 92 : 176));
                                                        g = (map->type == 0x00 ? 120 : (map->type == 0x01 ? 88 : 164));
                                                        b = (map->type == 0x00 ? 12 : (map->type == 0x01 ? 64 : 148));
                                                        break;
                case TRIANGLE_TEXTURE_MEADOW2:          r = (map->type == 0x00 ? 100 : (map->type == 0x01 ? 100 : 180));
                                                        g = (map->type == 0x00 ? 144 : (map->type == 0x01 ? 96 : 184));
                                                        b = (map->type == 0x00 ? 20 : (map->type == 0x01 ? 72 : 180));
                                                        break;
                case TRIANGLE_TEXTURE_MEADOW3:          r = (map->type == 0x00 ? 64 : (map->type == 0x01 ? 100 : 160));
                                                        g = (map->type == 0x00 ? 112 : (map->type == 0x01 ? 96 : 172));
                                                        b = (map->type == 0x00 ? 8 : (map->type == 0x01 ? 72 : 204));
                                                        break;
                case TRIANGLE_TEXTURE_MINING2:          r = (map->type == 0x00 ? 156 : (map->type == 0x01 ? 112 : 96));
                                                        g = (map->type == 0x00 ? 128 : (map->type == 0x01 ? 100 : 96));
                                                        b = (map->type == 0x00 ? 88 : (map->type == 0x01 ? 84 : 124));
                                                        break;
                case TRIANGLE_TEXTURE_MINING3:          r = (map->type == 0x00 ? 156 : (map->type == 0x01 ? 104 : 104));
                                                        g = (map->type == 0x00 ? 128 : (map->type == 0x01 ? 76 : 108));
                                                        b = (map->type == 0x00 ? 88 : (map->type == 0x01 ? 36 : 140));
                                                        break;
                case TRIANGLE_TEXTURE_MINING4:          r = (map->type == 0x00 ? 140 : (map->type == 0x01 ? 104 : 104));
                                                        g = (map->type == 0x00 ? 112 : (map->type == 0x01 ? 76 : 108));
                                                        b = (map->type == 0x00 ? 72 : (map->type == 0x01 ? 36 : 140));
                                                        break;
                case TRIANGLE_TEXTURE_STEPPE_MEADOW2:   r = (map->type == 0x00 ? 136 : (map->type == 0x01 ? 112 : 100));
                                                        g = (map->type == 0x00 ? 176 : (map->type == 0x01 ? 108 : 144));
                                                        b = (map->type == 0x00 ? 40 : (map->type == 0x01 ? 84 : 20));
                                                        break;
                case TRIANGLE_TEXTURE_FLOWER:           r = (map->type == 0x00 ? 72 : (map->type == 0x01 ? 68 : 124));
                                                        g = (map->type == 0x00 ? 120 : (map->type == 0x01 ? 72 : 132));
                                                        b = (map->type == 0x00 ? 12 : (map->type == 0x01 ? 80 : 172));
                                                        break;
                case TRIANGLE_TEXTURE_LAVA:             r = (map->type == 0x00 ? 192 : (map->type == 0x01 ? 128 : 144));
                                                        g = (map->type == 0x00 ? 32 : (map->type == 0x01 ? 20 : 44));
                                                        b = (map->type == 0x00 ? 32 : (map->type == 0x01 ? 0 : 4));
                                                        break;
                case TRIANGLE_TEXTURE_MINING_MEADOW:    r = (map->type == 0x00 ? 156 : (map->type == 0x01 ? 0 : 148));
                                                        g = (map->type == 0x00 ? 128 : (map->type == 0x01 ? 24 : 160));
                                                        b = (map->type == 0x00 ? 88 : (map->type == 0x01 ? 32 : 192));
                                                        break;
                default:                                //color grey
                                                        r = 128;
                                                        g = 128;
                                                        b = 128;
                                                        break;
            }

            row = (Uint32 *)Window->pixels + (y/num_y+20)*Window->pitch/4;
            //+6 because of the left window frame
            pixel = row + x/num_x + 6;

            r = ( (r*map->vertex[y*map->width+x].i) >>16 );
            g = ( (g*map->vertex[y*map->width+x].i) >>16 );
            b = ( (b*map->vertex[y*map->width+x].i) >>16 );
            r8 = (Uint8)(r > 255 ? 255 : (r < 0 ? 0 : r));
            g8 = (Uint8)(g > 255 ? 255 : (g < 0 ? 0 : g));
            b8 = (Uint8)(b > 255 ? 255 : (b < 0 ? 0 : b));
            *pixel = ( (r8 << Window->format->Rshift) + (g8 << Window->format->Gshift) + (b8 << Window->format->Bshift) );
        }
    }

#ifdef _EDITORMODE
    //draw the player flags
    char playerNumber[2];
    for (int i = 0; i < MAXPLAYERS; i++)
    {
        if (PlayerHQx[i] != 0xFFFF && PlayerHQx[i] != 0xFFFF)
        {
            //draw flag
            //%7 cause in the original game there are only 7 players and 7 different flags
            CSurface::Draw(Window, global::bmpArray[FLAG_BLUE_DARK + i % 7].surface, 6+PlayerHQx[i]/num_x-global::bmpArray[FLAG_BLUE_DARK + i % 7].nx, 20+PlayerHQy[i]/num_y-global::bmpArray[FLAG_BLUE_DARK + i % 7].ny);
            //write player number
            sprintf(playerNumber, "%d", i+1);
            CFont::writeText(Window, playerNumber, 6+PlayerHQx[i]/num_x, 20+PlayerHQy[i]/num_y, 9, FONT_MINTGREEN);
        }

    }
#endif

    //draw the arrow --> 6px is width of left window frame and 20px is the height of the upper window frame
    CSurface::Draw(Window, global::bmpArray[MAPPIC_ARROWCROSS_ORANGE].surface, 6+(displayRect.x+displayRect.w/2)/TRIANGLE_WIDTH/num_x-global::bmpArray[MAPPIC_ARROWCROSS_ORANGE].nx, 20+(displayRect.y+displayRect.h/2)/TRIANGLE_HEIGHT/num_y-global::bmpArray[MAPPIC_ARROWCROSS_ORANGE].ny);
}

void CMap::modifyVertex(void)
{
    static Uint32 TimeOfLastModification = SDL_GetTicks();

    if ( (SDL_GetTicks() - TimeOfLastModification) < 5 )
        return;
    else
        TimeOfLastModification = SDL_GetTicks();

    //save vertices for "undo" and "do"
    if (saveCurrentVertices)
    {
        if (CurrPtr_savedVertices != NULL)
        {
            CurrPtr_savedVertices->empty = false;
            CurrPtr_savedVertices->VertexX = VertexX;
            CurrPtr_savedVertices->VertexY = VertexY;
            for (int i = VertexX-MAX_CHANGE_SECTION-10-2, k = 0; i <= VertexX+MAX_CHANGE_SECTION+10+2; i++, k++)
            {

                for (int j = VertexY-MAX_CHANGE_SECTION-10-2, l = 0; j <= VertexY+MAX_CHANGE_SECTION+10+2; j++, l++)
                {
                    //i und j muessen wegen den mapraendern noch korrigiert werden!
                    int m = i;
                    if (m < 0)  m += map->width;
                    else if (m >= map->width) m -= map->width;
                    int n = j;
                    if (n < 0)  n += map->height;
                    else if (n >= map->height) n -= map->height;
                    //printf("\n X=%d Y=%d i=%d j=%d k=%d l=%d m=%d n=%d", VertexX, VertexY, i, j, k, l, m, n);
                    memcpy(&(CurrPtr_savedVertices->PointsArroundVertex[l*((MAX_CHANGE_SECTION+10+2)*2+1)+k]), &(map->vertex[n*map->width+m]), sizeof(struct point));
                    //CurrPtr_savedVertices->PointsArroundVertex[l*map->width+k] = map->vertex[n*map->width+m];
                }
            }
            if (CurrPtr_savedVertices->next == NULL)
            {
                if ( (CurrPtr_savedVertices->next = (struct savedVertices*)malloc(sizeof(struct savedVertices))) != NULL )
                {
                    CurrPtr_savedVertices->next->empty = true;
                    CurrPtr_savedVertices->next->prev = CurrPtr_savedVertices;
                    CurrPtr_savedVertices->next->next = NULL;
                    CurrPtr_savedVertices = CurrPtr_savedVertices->next;
                }
            }
            else
                CurrPtr_savedVertices = CurrPtr_savedVertices->next;
        }
        saveCurrentVertices = false;
    }

    if (mode == EDITOR_MODE_HEIGHT_RAISE)
    {
        for (int i = 0; i < VertexCounter; i++)
            if (Vertices[i].active)
                modifyHeightRaise(Vertices[i].x, Vertices[i].y);
    }
    else if (mode == EDITOR_MODE_HEIGHT_REDUCE)
    {
        for (int i = 0; i < VertexCounter; i++)
            if (Vertices[i].active)
                modifyHeightReduce(Vertices[i].x, Vertices[i].y);
    }
    else if (mode == EDITOR_MODE_HEIGHT_PLANE)
    {
        //calculate height average over all vertices
        int h_sum = 0;
        int h_count = 0;
        Uint8 h_avg = 0x00;

        for (int i = 0; i < VertexCounter; i++)
        {
            if (Vertices[i].active)
            {
                h_sum += map->vertex[Vertices[i].y*map->width+Vertices[i].x].h;
                h_count++;
            }
        }

        h_avg = h_sum / h_count;

        for (int i = 0; i < VertexCounter; i++)
            if (Vertices[i].active)
                modifyHeightPlane(Vertices[i].x, Vertices[i].y, h_avg);
    }
    else if (mode == EDITOR_MODE_HEIGHT_MAKE_BIG_HOUSE)
    {
        modifyHeightMakeBigHouse(VertexX, VertexY);
    }
    else if (mode == EDITOR_MODE_TEXTURE_MAKE_HARBOUR)
    {
        modifyHeightMakeBigHouse(VertexX, VertexY);
        modifyTextureMakeHarbour(VertexX, VertexY);
    }
    //at this time we need a modeContent to set
    else if (mode == EDITOR_MODE_CUT)
    {
        for (int i = 0; i < VertexCounter; i++)
        {
            if (Vertices[i].active)
            {
                modifyObject(Vertices[i].x, Vertices[i].y);
                modifyAnimal(Vertices[i].x, Vertices[i].y);
            }
        }
    }
    else if (mode == EDITOR_MODE_TEXTURE)
    {
        for (int i = 0; i < VertexCounter; i++)
            if (Vertices[i].active)
                modifyTexture(Vertices[i].x, Vertices[i].y, Vertices[i].fill_rsu, Vertices[i].fill_usd);
    }
    else if (mode == EDITOR_MODE_TREE)
    {
        for (int i = 0; i < VertexCounter; i++)
            if (Vertices[i].active)
                modifyObject(Vertices[i].x, Vertices[i].y);
    }
    else if (mode == EDITOR_MODE_LANDSCAPE)
    {
        for (int i = 0; i < VertexCounter; i++)
            if (Vertices[i].active)
                modifyObject(Vertices[i].x, Vertices[i].y);
    }
    else if (mode == EDITOR_MODE_RESOURCE_RAISE || mode == EDITOR_MODE_RESOURCE_REDUCE)
    {
        for (int i = 0; i < VertexCounter; i++)
            if (Vertices[i].active)
                modifyResource(Vertices[i].x, Vertices[i].y);
    }
    else if (mode == EDITOR_MODE_ANIMAL)
    {
        for (int i = 0; i < VertexCounter; i++)
            if (Vertices[i].active)
                modifyAnimal(Vertices[i].x, Vertices[i].y);
    }
    else if (mode == EDITOR_MODE_FLAG || mode == EDITOR_MODE_FLAG_DELETE)
    {
        modifyPlayer(VertexX, VertexY);
    }
}

void CMap::modifyHeightRaise(int VertexX, int VertexY)
{
    //vertex count for the points
    int X, Y;
    struct point *tempP = &map->vertex[VertexY*map->width+VertexX];
    //this is to setup the building depending on the vertices around
    struct cursorPoint tempVertices[19];
    calculateVerticesAround(tempVertices, VertexX, VertexY, 2);

    bool even = false;
    if (VertexY%2 == 0)
        even = true;

    //DO IT
    if (tempP->z >= TRIANGLE_INCREASE*(MaxRaiseHeight - 0x0A)) //user specified maximum reached
        return;

    if (tempP->z >= TRIANGLE_INCREASE*(0x3C - 0x0A)) //maximum reached (0x3C is max)
        return;

    tempP->y -= TRIANGLE_INCREASE;
    tempP->z += TRIANGLE_INCREASE;
    tempP->h += 0x01;
    CSurface::update_shading(map, VertexX, VertexY);

    //after (5*TRIANGLE_INCREASE) pixel all vertices around will be raised too
    //update first vertex left upside
    X = VertexX - (even ? 1 : 0);   if (X < 0) X += map->width;
    Y = VertexY-1;                  if (Y < 0) Y += map->height;
    //only modify if the other point is lower than the middle point of the hexagon (-5 cause point was raised a few lines before)
    if (map->vertex[Y*map->width+X].z < tempP->z-(5*TRIANGLE_INCREASE))
        modifyHeightRaise(X, Y);
    //update second vertex right upside
    X = VertexX + (even ? 0 : 1);   if (X >= map->width) X -= map->width;
    Y = VertexY-1;                  if (Y < 0) Y += map->height;
    //only modify if the other point is lower than the middle point of the hexagon (-5 cause point was raised a few lines before)
    if (map->vertex[Y*map->width+X].z < tempP->z-(5*TRIANGLE_INCREASE))
        modifyHeightRaise(X, Y);
    //update third point bottom left
    X = VertexX-1;                  if (X < 0) X += map->width;
    Y = VertexY;
    //only modify if the other point is lower than the middle point of the hexagon (-5 cause point was raised a few lines before)
    if (map->vertex[Y*map->width+X].z < tempP->z-(5*TRIANGLE_INCREASE))
        modifyHeightRaise(X, Y);
    //update fourth point bottom right
    X = VertexX+1;                  if (X >= map->width) X -= map->width;
    Y = VertexY;
    //only modify if the other point is lower than the middle point of the hexagon (-5 cause point was raised a few lines before)
    if (map->vertex[Y*map->width+X].z < tempP->z-(5*TRIANGLE_INCREASE))
        modifyHeightRaise(X, Y);
    //update fifth point down left
    X = VertexX - (even ? 1 : 0);   if (X < 0) X += map->width;
    Y = VertexY+1;                  if (Y >= map->height) Y -= map->height;
    //only modify if the other point is lower than the middle point of the hexagon (-5 cause point was raised a few lines before)
    if (map->vertex[Y*map->width+X].z < tempP->z-(5*TRIANGLE_INCREASE))
        modifyHeightRaise(X, Y);
    //update sixth point down right
    X = VertexX + (even ? 0 : 1);   if (X >= map->width) X -= map->width;
    Y = VertexY+1;                  if (Y >= map->height) Y -= map->height;
    //only modify if the other point is lower than the middle point of the hexagon (-5 cause point was raised a few lines before)
    if (map->vertex[Y*map->width+X].z < tempP->z-(5*TRIANGLE_INCREASE))
        modifyHeightRaise(X, Y);

    //at least setup the possible building and shading at the vertex and 2 sections around
    for (int i = 0; i < 19; i++)
    {
        modifyBuild(tempVertices[i].x, tempVertices[i].y);
        modifyShading(tempVertices[i].x, tempVertices[i].y);
    }
}

void CMap::modifyHeightReduce(int VertexX, int VertexY)
{
    //vertex count for the points
    int X, Y;
    struct point *tempP = &map->vertex[VertexY*map->width+VertexX];
    //this is to setup the building depending on the vertices around
    struct cursorPoint tempVertices[19];
    calculateVerticesAround(tempVertices, VertexX, VertexY, 2);

    bool even = false;
    if (VertexY%2 == 0)
        even = true;

    //DO IT
    if (tempP->z <= TRIANGLE_INCREASE*(MinReduceHeight - 0x0A)) //user specified minimum reached
        return;

    if (tempP->z <= TRIANGLE_INCREASE*(0x00 - 0x0A)) //minimum reached (0x00 is min)
        return;

    tempP->y += TRIANGLE_INCREASE;
    tempP->z -= TRIANGLE_INCREASE;
    tempP->h -= 0x01;
    CSurface::update_shading(map, VertexX, VertexY);
    //after (5*TRIANGLE_INCREASE) pixel all vertices around will be reduced too
    //update first vertex left upside
    X = VertexX - (even ? 1 : 0);   if (X < 0) X += map->width;
    Y = VertexY-1;                  if (Y < 0) Y += map->height;
    //only modify if the other point is higher than the middle point of the hexagon (+5 cause point was reduced a few lines before)
    if (map->vertex[Y*map->width+X].z > tempP->z+(5*TRIANGLE_INCREASE))
        modifyHeightReduce(X, Y);
    //update second vertex right upside
    X = VertexX + (even ? 0 : 1);   if (X >= map->width) X -= map->width;
    Y = VertexY-1;                  if (Y < 0) Y += map->height;
    //only modify if the other point is higher than the middle point of the hexagon (+5 cause point was reduced a few lines before)
    if (map->vertex[Y*map->width+X].z > tempP->z+(5*TRIANGLE_INCREASE))
        modifyHeightReduce(X, Y);
    //update third point bottom left
    X = VertexX-1;                  if (X < 0) X += map->width;
    Y = VertexY;
    //only modify if the other point is higher than the middle point of the hexagon (+5 cause point was reduced a few lines before)
    if (map->vertex[Y*map->width+X].z > tempP->z+(5*TRIANGLE_INCREASE))
        modifyHeightReduce(X, Y);
    //update fourth point bottom right
    X = VertexX+1;                  if (X >= map->width) X -= map->width;
    Y = VertexY;
    //only modify if the other point is higher than the middle point of the hexagon (+5 cause point was reduced a few lines before)
    if (map->vertex[Y*map->width+X].z > tempP->z+(5*TRIANGLE_INCREASE))
        modifyHeightReduce(X, Y);
    //update fifth point down left
    X = VertexX - (even ? 1 : 0);   if (X < 0) X += map->width;
    Y = VertexY+1;                  if (Y >= map->height) Y -= map->height;
    //only modify if the other point is higher than the middle point of the hexagon (+5 cause point was reduced a few lines before)
    if (map->vertex[Y*map->width+X].z > tempP->z+(5*TRIANGLE_INCREASE))
        modifyHeightReduce(X, Y);
    //update sixth point down right
    X = VertexX + (even ? 0 : 1);   if (X >= map->width) X -= map->width;
    Y = VertexY+1;                  if (Y >= map->height) Y -= map->height;
    //only modify if the other point is higher than the middle point of the hexagon (+5 cause point was reduced a few lines before)
    if (map->vertex[Y*map->width+X].z > tempP->z+(5*TRIANGLE_INCREASE))
        modifyHeightReduce(X, Y);

    //at least setup the possible building and shading at the vertex and 2 sections around
    for (int i = 0; i < 19; i++)
    {
        modifyBuild(tempVertices[i].x, tempVertices[i].y);
        modifyShading(tempVertices[i].x, tempVertices[i].y);
    }
}

void CMap::modifyHeightPlane(int VertexX, int VertexY, Uint8 h)
{
    while (map->vertex[VertexY*map->width+VertexX].h < h)
        modifyHeightRaise(VertexX, VertexY);

    while (map->vertex[VertexY*map->width+VertexX].h > h)
        modifyHeightReduce(VertexX, VertexY);
}

void CMap::modifyHeightMakeBigHouse(int VertexX, int VertexY)
{
    //at first save all vertices we need to calculate the new building
    struct cursorPoint tempVertices[19];
    calculateVerticesAround(tempVertices, VertexX, VertexY, 2);


    Uint8 height = map->vertex[VertexY*map->width+VertexX].h;

    //calculate the building using the height of the vertices

    //test the whole section
    for (int i = 0; i < 6; i++)
    {
        while (height - map->vertex[tempVertices[i].y*map->width+tempVertices[i].x].h >= 0x04)
            modifyHeightRaise(tempVertices[i].x, tempVertices[i].y);

        while (map->vertex[tempVertices[i].y*map->width+tempVertices[i].x].h - height >= 0x04)
            modifyHeightReduce(tempVertices[i].x, tempVertices[i].y);
    }

    //test vertex lower right
    while (height - map->vertex[tempVertices[6].y*map->width+tempVertices[6].x].h >= 0x04)
        modifyHeightRaise(tempVertices[6].x, tempVertices[6].y);

    while (map->vertex[tempVertices[6].y*map->width+tempVertices[6].x].h - height >= 0x02)
        modifyHeightReduce(tempVertices[6].x, tempVertices[6].y);

    //now test the second section around the vertex

    //test the whole section
    for (int i = 7; i < 19; i++)
    {
        while (height - map->vertex[tempVertices[i].y*map->width+tempVertices[i].x].h >= 0x03)
            modifyHeightRaise(tempVertices[i].x, tempVertices[i].y);

        while (map->vertex[tempVertices[i].y*map->width+tempVertices[i].x].h - height >= 0x03)
            modifyHeightReduce(tempVertices[i].x, tempVertices[i].y);
    }

    //remove harbour if there is one
    if (    map->vertex[VertexY*map->width+VertexX].rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1_HARBOUR
        ||  map->vertex[VertexY*map->width+VertexX].rsuTexture == TRIANGLE_TEXTURE_MEADOW1_HARBOUR
        ||  map->vertex[VertexY*map->width+VertexX].rsuTexture == TRIANGLE_TEXTURE_MEADOW2_HARBOUR
        ||  map->vertex[VertexY*map->width+VertexX].rsuTexture == TRIANGLE_TEXTURE_MEADOW3_HARBOUR
        ||  map->vertex[VertexY*map->width+VertexX].rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2_HARBOUR
        ||  map->vertex[VertexY*map->width+VertexX].rsuTexture == TRIANGLE_TEXTURE_FLOWER_HARBOUR
        ||  map->vertex[VertexY*map->width+VertexX].rsuTexture == TRIANGLE_TEXTURE_MINING_MEADOW_HARBOUR
       )
    {
        map->vertex[VertexY*map->width+VertexX].rsuTexture -= 0x40;
    }
}

void CMap::modifyShading(int VertexX, int VertexY)
{
    //temporary to keep the lines short
    int X, Y;
    //this is to setup the shading depending on the vertices around (2 sections from the cursor)
    struct cursorPoint tempVertices[19];
    calculateVerticesAround(tempVertices, VertexX, VertexY, 2);

    //shading stakes
    int A, B, C, D, Result;

    //shading stake of point right upside (first section)
    X = tempVertices[2].x;
    Y = tempVertices[2].y;
    A = 9 * ( map->vertex[Y*map->width+X].h - map->vertex[VertexY*map->width+VertexX].h );
    //shading stake of point left (first section)
    X = tempVertices[3].x;
    Y = tempVertices[3].y;
    B = -6 * ( map->vertex[Y*map->width+X].h - map->vertex[VertexY*map->width+VertexX].h );
    //shading stake of point left (second section)
    X = tempVertices[12].x;
    Y = tempVertices[12].y;
    C = -3 * ( map->vertex[Y*map->width+X].h - map->vertex[VertexY*map->width+VertexX].h );
    //shading stake of point bottom/middle left (second section)
    X = tempVertices[14].x;
    Y = tempVertices[14].y;
    D = -9 * ( map->vertex[Y*map->width+X].h - map->vertex[VertexY*map->width+VertexX].h );

    Result = 0x40 + A + B + C + D;
    if (Result > 0x80)
        Result = 0x80;
    else if (Result < 0x00)
        Result = 0x00;

    map->vertex[VertexY*map->width+VertexX].shading = Result;
}

void CMap::modifyTexture(int VertexX, int VertexY, bool rsu, bool usd)
{
    if (modeContent == TRIANGLE_TEXTURE_MEADOW_MIXED || modeContent == TRIANGLE_TEXTURE_MEADOW_MIXED_HARBOUR)
    {
        int newContent = rand()%3;
        if (newContent == 0)
        {
            if (modeContent == TRIANGLE_TEXTURE_MEADOW_MIXED)
                newContent = TRIANGLE_TEXTURE_MEADOW1;
            else
                newContent = TRIANGLE_TEXTURE_MEADOW1_HARBOUR;
        }
        else if (newContent == 1)
        {
            if (modeContent == TRIANGLE_TEXTURE_MEADOW_MIXED)
                newContent = TRIANGLE_TEXTURE_MEADOW2;
            else
                newContent = TRIANGLE_TEXTURE_MEADOW2_HARBOUR;
        }
        else
        {
            if (modeContent == TRIANGLE_TEXTURE_MEADOW_MIXED)
                newContent = TRIANGLE_TEXTURE_MEADOW3;
            else
                newContent = TRIANGLE_TEXTURE_MEADOW3_HARBOUR;
        }
        if (rsu)
            map->vertex[VertexY*map->width+VertexX].rsuTexture = newContent;
        if (usd)
            map->vertex[VertexY*map->width+VertexX].usdTexture = newContent;
    }
    else
    {
        if (rsu)
            map->vertex[VertexY*map->width+VertexX].rsuTexture = modeContent;
        if (usd)
            map->vertex[VertexY*map->width+VertexX].usdTexture = modeContent;
    }

    //at least setup the possible building and the resources at the vertex and 1 section/2 sections around
    struct cursorPoint tempVertices[19];
    calculateVerticesAround(tempVertices, VertexX, VertexY, 2);
    for (int i = 0; i < 19; i++)
    {
        if (i < 7)
            modifyBuild(tempVertices[i].x, tempVertices[i].y);
        modifyResource(tempVertices[i].x, tempVertices[i].y);
    }
}

void CMap::modifyTextureMakeHarbour(int VertexX, int VertexY)
{
    if (    map->vertex[VertexY*map->width+VertexX].rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1
        ||  map->vertex[VertexY*map->width+VertexX].rsuTexture == TRIANGLE_TEXTURE_MEADOW1
        ||  map->vertex[VertexY*map->width+VertexX].rsuTexture == TRIANGLE_TEXTURE_MEADOW2
        ||  map->vertex[VertexY*map->width+VertexX].rsuTexture == TRIANGLE_TEXTURE_MEADOW3
        ||  map->vertex[VertexY*map->width+VertexX].rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2
        ||  map->vertex[VertexY*map->width+VertexX].rsuTexture == TRIANGLE_TEXTURE_FLOWER
        ||  map->vertex[VertexY*map->width+VertexX].rsuTexture == TRIANGLE_TEXTURE_MINING_MEADOW
       )
    {
        map->vertex[VertexY*map->width+VertexX].rsuTexture += 0x40;
    }
}

void CMap::modifyObject(int VertexX, int VertexY)
{
    if (mode == EDITOR_MODE_CUT)
    {
        //prevent cutting a player position
        if (map->vertex[VertexY*map->width+VertexX].objectInfo != 0x80)
        {
            map->vertex[VertexY*map->width+VertexX].objectType = 0x00;
            map->vertex[VertexY*map->width+VertexX].objectInfo = 0x00;
        }
    }
    else if (mode == EDITOR_MODE_TREE)
    {
        //if there is another object at the vertex, return
        if (map->vertex[VertexY*map->width+VertexX].objectInfo != 0x00)
                return;
        if (modeContent == 0xFF)
        {
            //mixed wood
            if (modeContent2 == 0xC4)
            {
                int newContent = rand()%3;
                if (newContent == 0)
                    newContent = 0x30;
                else if (newContent == 1)
                    newContent = 0x70;
                else
                    newContent = 0xB0;
                //we set different start pictures for the tree, cause the trees should move different, so we add a random value that walks from 0 to 7
                map->vertex[VertexY*map->width+VertexX].objectType = newContent + rand()%8;
                map->vertex[VertexY*map->width+VertexX].objectInfo = modeContent2;
            }
            //mixed palm
            else //if (modeContent2 == 0xC5)
            {
                int newContent = rand()%2;
                int newContent2;
                if (newContent == 0)
                {
                    newContent = 0x30;
                    newContent2 = 0xC5;
                }
                else
                {
                    newContent = 0xF0;
                    newContent2 = 0xC4;
                }
                //we set different start pictures for the tree, cause the trees should move different, so we add a random value that walks from 0 to 7
                map->vertex[VertexY*map->width+VertexX].objectType = newContent + rand()%8;
                map->vertex[VertexY*map->width+VertexX].objectInfo = newContent2;
            }
        }
        else
        {
            //we set different start pictures for the tree, cause the trees should move different, so we add a random value that walks from 0 to 7
            map->vertex[VertexY*map->width+VertexX].objectType = modeContent + rand()%8;
            map->vertex[VertexY*map->width+VertexX].objectInfo = modeContent2;
        }
    }
    else if (mode == EDITOR_MODE_LANDSCAPE)
    {
        //if there is another object at the vertex, return
        if (map->vertex[VertexY*map->width+VertexX].objectInfo != 0x00)
                return;

        if (modeContent == 0x01)
        {
            int newContent = modeContent + rand()%6;
            int newContent2 = 0xCC + rand()%2;

            map->vertex[VertexY*map->width+VertexX].objectType = newContent;
            map->vertex[VertexY*map->width+VertexX].objectInfo = newContent2;

            //now set up the buildings around the granite
            modifyBuild(VertexX, VertexY);
        }
        else if (modeContent == 0x05)
        {
            int newContent = modeContent + rand()%2;

            map->vertex[VertexY*map->width+VertexX].objectType = newContent;
            map->vertex[VertexY*map->width+VertexX].objectInfo = modeContent2;
        }
        else if (modeContent == 0x02)
        {
            int newContent = modeContent + rand()%3;

            map->vertex[VertexY*map->width+VertexX].objectType = newContent;
            map->vertex[VertexY*map->width+VertexX].objectInfo = modeContent2;
        }
        else if (modeContent == 0x0C)
        {
            int newContent = modeContent + rand()%2;

            map->vertex[VertexY*map->width+VertexX].objectType = newContent;
            map->vertex[VertexY*map->width+VertexX].objectInfo = modeContent2;
        }
        else if (modeContent == 0x25)
        {
            int newContent = modeContent + rand()%3;

            map->vertex[VertexY*map->width+VertexX].objectType = newContent;
            map->vertex[VertexY*map->width+VertexX].objectInfo = modeContent2;
        }
        else if (modeContent == 0x10)
        {
            int newContent = rand()%4;
            if (newContent == 0)
                newContent = 0x10;
            else if (newContent == 1)
                newContent = 0x11;
            else if (newContent == 2)
                newContent = 0x12;
            else
                newContent = 0x0A;

            map->vertex[VertexY*map->width+VertexX].objectType = newContent;
            map->vertex[VertexY*map->width+VertexX].objectInfo = modeContent2;
        }
        else if (modeContent == 0x0E)
        {
            int newContent = rand()%4;
            if (newContent == 0)
                newContent = 0x0E;
            else if (newContent == 1)
                newContent = 0x0F;
            else if (newContent == 2)
                newContent = 0x13;
            else
                newContent = 0x14;

            map->vertex[VertexY*map->width+VertexX].objectType = newContent;
            map->vertex[VertexY*map->width+VertexX].objectInfo = modeContent2;
        }
        else if (modeContent == 0x07)
        {
            int newContent = modeContent + rand()%2;

            map->vertex[VertexY*map->width+VertexX].objectType = newContent;
            map->vertex[VertexY*map->width+VertexX].objectInfo = modeContent2;
        }
        else if (modeContent == 0x00)
        {
            int newContent = rand()%3;
            if (newContent == 0)
                newContent = 0x00;
            else if (newContent == 1)
                newContent = 0x01;
            else
                newContent = 0x22;

            map->vertex[VertexY*map->width+VertexX].objectType = newContent;
            map->vertex[VertexY*map->width+VertexX].objectInfo = modeContent2;
        }
        else if (modeContent == 0x18)
        {
            int newContent = modeContent + rand()%7;

            map->vertex[VertexY*map->width+VertexX].objectType = newContent;
            map->vertex[VertexY*map->width+VertexX].objectInfo = modeContent2;
        }
        else if (modeContent == 0x09)
        {
            map->vertex[VertexY*map->width+VertexX].objectType = modeContent;
            map->vertex[VertexY*map->width+VertexX].objectInfo = modeContent2;
        }
    }
    //at least setup the possible building at the vertex and 1 section around
    struct cursorPoint tempVertices[7];
    calculateVerticesAround(tempVertices, VertexX, VertexY, 1);
    for (int i = 0; i < 7; i++)
        modifyBuild(tempVertices[i].x, tempVertices[i].y);
}

void CMap::modifyAnimal(int VertexX, int VertexY)
{
    if (mode == EDITOR_MODE_CUT)
    {
        map->vertex[VertexY*map->width+VertexX].animal = 0x00;
    }
    else if (mode == EDITOR_MODE_ANIMAL)
    {
        //if there is another object at the vertex, return
        if (map->vertex[VertexY*map->width+VertexX].animal != 0x00)
                return;

        if (modeContent > 0x00 && modeContent <= 0x06)
            map->vertex[VertexY*map->width+VertexX].animal = modeContent;
    }
}

void CMap::modifyBuild(int VertexX, int VertexY)
{
    //at first save all vertices we need to calculate the new building
    struct cursorPoint tempVertices[19];
    calculateVerticesAround(tempVertices, VertexX, VertexY, 2);

    ///evtl. keine festen werte sondern addition und subtraktion wegen originalkompatibilitaet (bei baeumen bspw. keine 0x00 sondern 0x68)


    Uint8 building;
    Uint8 height = map->vertex[VertexY*map->width+VertexX].h, temp;

    //calculate the building using the height of the vertices
    //this building is a mine
    if (    map->vertex[VertexY*map->width+VertexX].rsuTexture == TRIANGLE_TEXTURE_MINING1
        ||  map->vertex[VertexY*map->width+VertexX].rsuTexture == TRIANGLE_TEXTURE_MINING2
        ||  map->vertex[VertexY*map->width+VertexX].rsuTexture == TRIANGLE_TEXTURE_MINING3
        ||  map->vertex[VertexY*map->width+VertexX].rsuTexture == TRIANGLE_TEXTURE_MINING4
       )
    {
        building = 0x05;
        //test vertex lower right
        temp = map->vertex[tempVertices[6].y*map->width+tempVertices[6].x].h;
        if ( temp - height >= 0x04 )
            building = 0x01;
    }
    //not a mine
    else
    {
        building = 0x04;
        //test the whole section
        for (int i = 0; i < 6; i++)
        {
            temp = map->vertex[tempVertices[i].y*map->width+tempVertices[i].x].h;
            if (height - temp >= 0x04 || temp - height >= 0x04)
                building = 0x01;
        }

        //test vertex lower right
        temp = map->vertex[tempVertices[6].y*map->width+tempVertices[6].x].h;
        if (height - temp >= 0x04 || temp - height >= 0x02 )
            building = 0x01;

        //now test the second section around the vertex
        if (building > 0x02)
        {
            //test the whole section
            for (int i = 7; i < 19; i++)
            {
                temp = map->vertex[tempVertices[i].y*map->width+tempVertices[i].x].h;
                if (height - temp >= 0x03 || temp - height >= 0x03)
                    building = 0x02;
            }
        }
    }

    //test if there is an object AROUND the vertex (trees or granite)
    if (building > 0x01)
    {
        for (int i = 1; i < 7; i++)
        {
            if (    map->vertex[tempVertices[i].y*map->width+tempVertices[i].x].objectInfo == 0xC4  //tree
                ||  map->vertex[tempVertices[i].y*map->width+tempVertices[i].x].objectInfo == 0xC5  //tree
                ||  map->vertex[tempVertices[i].y*map->width+tempVertices[i].x].objectInfo == 0xC6  //tree
               )
            {
                //if lower right
                if (i == 6)
                {
                    building = 0x01;
                    break;
                }
                else
                    building = 0x02;
            }
            else if (   map->vertex[tempVertices[i].y*map->width+tempVertices[i].x].objectInfo == 0xCC  //granite
                    ||  map->vertex[tempVertices[i].y*map->width+tempVertices[i].x].objectInfo == 0xCD //granite
                    )
            {
                building = 0x01;
                break;
            }
        }
    }

    //test if there is an object AT the vertex (trees or granite)
    if (building > 0x00)
    {
        if (    map->vertex[VertexY*map->width+VertexX].objectInfo == 0xC4  //tree
            ||  map->vertex[VertexY*map->width+VertexX].objectInfo == 0xC5  //tree
            ||  map->vertex[VertexY*map->width+VertexX].objectInfo == 0xC6  //tree
            ||  map->vertex[VertexY*map->width+VertexX].objectInfo == 0xCC  //granite
            ||  map->vertex[VertexY*map->width+VertexX].objectInfo == 0xCD  //granite
           )
        {
            building = 0x00;
        }
    }

    //test if there is snow or lava at the vertex or around the vertex and touching the vertex (first section)
    if (building > 0x00)
    {
        if (    map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_SNOW
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_SNOW
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_LAVA
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_LAVA
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_SNOW
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_SNOW
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_LAVA
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_LAVA
            ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_SNOW
            ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_LAVA
            ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_SNOW
            ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_LAVA
           )
        {
            building = 0x00;
        }
    }

    //test if there is snow or lava on the right side (RSU), in lower left (USD) or in lower right (first section)
    if (building > 0x01)
    {
        if (    map->vertex[tempVertices[4].y*map->width+tempVertices[4].x].rsuTexture == TRIANGLE_TEXTURE_SNOW
            ||  map->vertex[tempVertices[4].y*map->width+tempVertices[4].x].rsuTexture == TRIANGLE_TEXTURE_LAVA
            ||  map->vertex[tempVertices[5].y*map->width+tempVertices[5].x].usdTexture == TRIANGLE_TEXTURE_SNOW
            ||  map->vertex[tempVertices[5].y*map->width+tempVertices[5].x].usdTexture == TRIANGLE_TEXTURE_LAVA
            ||  map->vertex[tempVertices[6].y*map->width+tempVertices[6].x].rsuTexture == TRIANGLE_TEXTURE_SNOW
            ||  map->vertex[tempVertices[6].y*map->width+tempVertices[6].x].usdTexture == TRIANGLE_TEXTURE_SNOW
            ||  map->vertex[tempVertices[6].y*map->width+tempVertices[6].x].rsuTexture == TRIANGLE_TEXTURE_LAVA
            ||  map->vertex[tempVertices[6].y*map->width+tempVertices[6].x].usdTexture == TRIANGLE_TEXTURE_LAVA
           )
        {
            building = 0x01;
        }
    }

    //test if vertex is surrounded by water or swamp
    if (building > 0x00)
    {
        if (    (   map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_SWAMP
                )
            &&  (   map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_SWAMP
                )
            &&  (   map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_SWAMP
                )
            &&  (   map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_SWAMP
                )
            &&  (   map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_SWAMP
                )
            &&  (   map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_SWAMP
                )
           )
        {
            building = 0x00;
        }
        else if (   (   map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_WATER
                    ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_SWAMP
                    )
                ||  (   map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_WATER
                    ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_SWAMP
                    )
                ||  (   map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_WATER
                    ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_SWAMP
                    )
                ||  (   map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_WATER
                    ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_SWAMP
                    )
                ||  (   map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_WATER
                    ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_SWAMP
                    )
                ||  (   map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_WATER
                    ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_SWAMP
                    )
                )
        {
            building = 0x01;
        }
    }

    //test if there is steppe at the vertex or touching the vertex
    if (building > 0x01)
    {
        if (    map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_STEPPE
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_STEPPE
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_STEPPE
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_STEPPE
            ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_STEPPE
            ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_STEPPE
           )
        {
            building = 0x01;
        }
    }

    //test if vertex is surrounded by mining-textures
    if (building > 0x01)
    {
        if (    (   map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_MINING1
                ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_MINING2
                ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_MINING3
                ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_MINING4
                )
            &&  (   map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_MINING1
                ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_MINING2
                ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_MINING3
                ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_MINING4
                )
            &&  (   map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_MINING1
                ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_MINING2
                ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_MINING3
                ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_MINING4
                )
            &&  (   map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_MINING1
                ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_MINING2
                ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_MINING3
                ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_MINING4
                )
            &&  (   map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_MINING1
                ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_MINING2
                ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_MINING3
                ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_MINING4
                )
            &&  (   map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_MINING1
                ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_MINING2
                ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_MINING3
                ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_MINING4
                )
           )
        {
            building = 0x05;
        }
        else if (   (   map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_MINING1
                    ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_MINING2
                    ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_MINING3
                    ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_MINING4
                    )
                ||  (   map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_MINING1
                    ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_MINING2
                    ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_MINING3
                    ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_MINING4
                    )
                ||  (   map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_MINING1
                    ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_MINING2
                    ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_MINING3
                    ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_MINING4
                    )
                ||  (   map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_MINING1
                    ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_MINING2
                    ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_MINING3
                    ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_MINING4
                    )
                ||  (   map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_MINING1
                    ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_MINING2
                    ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_MINING3
                    ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_MINING4
                    )
                ||  (   map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_MINING1
                    ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_MINING2
                    ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_MINING3
                    ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_MINING4
                    )
               )
        {
            building = 0x01;
        }
    }

    //test for headquarters around the point
    //NOTE: In EDITORMODE don't test AT the point, cause in Original game we need a big house AT the point, otherwise the game wouldn't set a player there
    if ( building > 0x00)
    {
        for (int i = 1; i < 7; i++)
        {
            if ( map->vertex[tempVertices[i].y*map->width+tempVertices[i].x].objectInfo == 0x80 )
                building = 0x00;
        }
    }
    #ifndef _EDITORMODE
    if ( building > 0x00)
    {
        if ( map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].objectInfo == 0x80 )
            building = 0x00;
    }
    #endif

    //test for headquarters around (second section)
    if ( building > 0x01)
    {
        for (int i = 7; i < 19; i++)
        {
            if ( map->vertex[tempVertices[i].y*map->width+tempVertices[i].x].objectInfo == 0x80 )
            {
                if (i == 15 || i == 17 || i == 18)
                    building = 0x01;
                else
                {
                    //make middle house, but only if it's not a mine
                    if (building > 0x03 && building < 0x05)
                        building = 0x03;
                }
            }
        }
    }

    //Some additional information for "ingame"-building-calculation:
	//There is no difference between small, middle and big houses. If you set a small house on a vertex, the
	//buildings around will change like this where a middle or a big house.
	//Only a flag has another algorithm.
	//--Flagge einfuegen!!!

    map->vertex[VertexY*map->width+VertexX].build = building;
}

void CMap::modifyResource(int VertexX, int VertexY)
{
    //at first save all vertices we need to check
    struct cursorPoint tempVertices[19];
    calculateVerticesAround(tempVertices, VertexX, VertexY, 2);

    //SPECIAL CASE: test if we should set water only
    //test if vertex is surrounded by meadow and meadow-like textures
    if (    (   map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1_HARBOUR
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_MEADOW1
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_MEADOW1_HARBOUR
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_MEADOW2
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_MEADOW2_HARBOUR
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_MEADOW3
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_MEADOW3_HARBOUR
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2_HARBOUR
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_FLOWER
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_FLOWER_HARBOUR
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_MINING_MEADOW
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_MINING_MEADOW_HARBOUR
            )
        &&  (   map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1_HARBOUR
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_MEADOW1
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_MEADOW1_HARBOUR
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_MEADOW2
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_MEADOW2_HARBOUR
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_MEADOW3
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_MEADOW3_HARBOUR
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2_HARBOUR
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_FLOWER
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_FLOWER_HARBOUR
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_MINING_MEADOW
            ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_MINING_MEADOW_HARBOUR
            )
        &&  (   map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1_HARBOUR
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_MEADOW1
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_MEADOW1_HARBOUR
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_MEADOW2
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_MEADOW2_HARBOUR
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_MEADOW3
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_MEADOW3_HARBOUR
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2_HARBOUR
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_FLOWER
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_FLOWER_HARBOUR
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_MINING_MEADOW
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_MINING_MEADOW_HARBOUR
            )
        &&  (   map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1_HARBOUR
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_MEADOW1
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_MEADOW1_HARBOUR
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_MEADOW2
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_MEADOW2_HARBOUR
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_MEADOW3
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_MEADOW3_HARBOUR
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2_HARBOUR
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_FLOWER
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_FLOWER_HARBOUR
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_MINING_MEADOW
            ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_MINING_MEADOW_HARBOUR
            )
        &&  (   map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1
            ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1_HARBOUR
            ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_MEADOW1
            ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_MEADOW1_HARBOUR
            ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_MEADOW2
            ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_MEADOW2_HARBOUR
            ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_MEADOW3
            ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_MEADOW3_HARBOUR
            ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2
            ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2_HARBOUR
            ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_FLOWER
            ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_FLOWER_HARBOUR
            ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_MINING_MEADOW
            ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_MINING_MEADOW_HARBOUR
            )
        &&  (   map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1
            ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1_HARBOUR
            ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_MEADOW1
            ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_MEADOW1_HARBOUR
            ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_MEADOW2
            ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_MEADOW2_HARBOUR
            ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_MEADOW3
            ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_MEADOW3_HARBOUR
            ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2
            ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2_HARBOUR
            ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_FLOWER
            ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_FLOWER_HARBOUR
            ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_MINING_MEADOW
            ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_MINING_MEADOW_HARBOUR
            )
        )
    {
        map->vertex[VertexY*map->width+VertexX].resource = 0x21;
    }
    //SPECIAL CASE: test if we should set fishes only
    //test if vertex is surrounded by water (first section) and at least one non-water texture in the second section
    else if (   (   map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_WATER
                )
            &&  (   map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_WATER
                )
            &&  (   map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_WATER
                )
            &&  (   map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_WATER
                )
            &&  (   map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_WATER
                )
            &&  (   map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_WATER
                )
            &&  (   map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].usdTexture != TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].rsuTexture != TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[4].y*map->width+tempVertices[4].x].rsuTexture != TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[4].y*map->width+tempVertices[4].x].usdTexture != TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[5].y*map->width+tempVertices[5].x].rsuTexture != TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[5].y*map->width+tempVertices[5].x].usdTexture != TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[6].y*map->width+tempVertices[6].x].rsuTexture != TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[6].y*map->width+tempVertices[6].x].usdTexture != TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[7].y*map->width+tempVertices[7].x].rsuTexture != TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[7].y*map->width+tempVertices[7].x].usdTexture != TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[8].y*map->width+tempVertices[8].x].rsuTexture != TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[8].y*map->width+tempVertices[8].x].usdTexture != TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[9].y*map->width+tempVertices[9].x].rsuTexture != TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[10].y*map->width+tempVertices[10].x].rsuTexture != TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[10].y*map->width+tempVertices[10].x].usdTexture != TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[11].y*map->width+tempVertices[11].x].rsuTexture != TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[12].y*map->width+tempVertices[12].x].usdTexture != TRIANGLE_TEXTURE_WATER
                ||  map->vertex[tempVertices[14].y*map->width+tempVertices[14].x].usdTexture != TRIANGLE_TEXTURE_WATER
                )
            )
    {
        map->vertex[VertexY*map->width+VertexX].resource = 0x87;
    }
    //test if vertex is surrounded by mining textures
    else if (   (   map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_MINING1
                ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_MINING2
                ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_MINING3
                ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].rsuTexture == TRIANGLE_TEXTURE_MINING4
                )
            &&  (   map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_MINING1
                ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_MINING2
                ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_MINING3
                ||  map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].usdTexture == TRIANGLE_TEXTURE_MINING4
                )
            &&  (   map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_MINING1
                ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_MINING2
                ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_MINING3
                ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].rsuTexture == TRIANGLE_TEXTURE_MINING4
                )
            &&  (   map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_MINING1
                ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_MINING2
                ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_MINING3
                ||  map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].usdTexture == TRIANGLE_TEXTURE_MINING4
                )
            &&  (   map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_MINING1
                ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_MINING2
                ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_MINING3
                ||  map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].rsuTexture == TRIANGLE_TEXTURE_MINING4
                )
            &&  (   map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_MINING1
                ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_MINING2
                ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_MINING3
                ||  map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].usdTexture == TRIANGLE_TEXTURE_MINING4
                )
            )
    {
        //check which resource to set
        if (mode == EDITOR_MODE_RESOURCE_RAISE)
        {
            //if there is no or another resource at the moment
            if (    map->vertex[VertexY*map->width+VertexX].resource == 0x40
                ||  map->vertex[VertexY*map->width+VertexX].resource < modeContent
                ||  map->vertex[VertexY*map->width+VertexX].resource > modeContent+6
               )
            {
                map->vertex[VertexY*map->width+VertexX].resource = modeContent;
            }
            else if (map->vertex[VertexY*map->width+VertexX].resource >= modeContent && map->vertex[VertexY*map->width+VertexX].resource <= modeContent+6)
            {
                //maximum not reached?
                if (map->vertex[VertexY*map->width+VertexX].resource != modeContent+6)
                    map->vertex[VertexY*map->width+VertexX].resource++;
            }
        }
        else if (mode == EDITOR_MODE_RESOURCE_REDUCE)
        {
            //minimum not reached?
            if (map->vertex[VertexY*map->width+VertexX].resource != 0x40)
            {
                map->vertex[VertexY*map->width+VertexX].resource--;
                //minimum now reached? if so, set it to 0x40
                if (    map->vertex[VertexY*map->width+VertexX].resource == 0x48
                    ||  map->vertex[VertexY*map->width+VertexX].resource == 0x50
                    ||  map->vertex[VertexY*map->width+VertexX].resource == 0x58
                    //in case of coal we already have a 0x40, so don't check this
                    //||  map->vertex[VertexY*map->width+VertexX].resource == 0x40
                   )
                    map->vertex[VertexY*map->width+VertexX].resource = 0x40;
            }
        }
        else if (map->vertex[VertexY*map->width+VertexX].resource == 0x00)
            map->vertex[VertexY*map->width+VertexX].resource = 0x40;
    }
    else
        map->vertex[VertexY*map->width+VertexX].resource = 0x00;
}

void CMap::modifyPlayer(int VertexX, int VertexY)
{
    //if we have repositioned a player, we need the old position to recalculate the buildings there
    bool PlayerRePositioned = false;
    int oldPositionX = 0;
    int oldPositionY = 0;

    //set player position
    if (mode == EDITOR_MODE_FLAG)
    {
        //only allowed on big houses (0x04) --> but in cheat mode within the game also small houses (0x02) are allowed
        if (   /*map->vertex[VertexY*map->width+VertexX].objectType == 0x00
            && map->vertex[VertexY*map->width+VertexX].objectInfo == 0x00
            &&*/ map->vertex[VertexY*map->width+VertexX].build%8 == 0x04
            && map->vertex[VertexY*map->width+VertexX].objectInfo != 0x80
           )
        {
            map->vertex[VertexY*map->width+VertexX].objectType = modeContent;
            map->vertex[VertexY*map->width+VertexX].objectInfo = 0x80;

            //save old position if exists
            if (PlayerHQx[modeContent] != 0xFFFF && PlayerHQy[modeContent] != 0xFFFF)
            {
                oldPositionX = PlayerHQx[modeContent];
                oldPositionY = PlayerHQy[modeContent];
                map->vertex[oldPositionY*map->width+oldPositionX].objectType = 0x00;
                map->vertex[oldPositionY*map->width+oldPositionX].objectInfo = 0x00;
                PlayerRePositioned = true;
            }
            PlayerHQx[modeContent] = VertexX;
            PlayerHQy[modeContent] = VertexY;

            //for compatibility with original settlers 2 we write the headquarters positions to the map header (for the first 7 players)
            if (modeContent >= 0 && modeContent < 7)
            {
                map->HQx[modeContent] = VertexX;
                map->HQy[modeContent] = VertexY;
            }

            //setup number of players in map header
            if (!PlayerRePositioned)
                map->player++;
        }
    }
    //delete player position
    else if (mode == EDITOR_MODE_FLAG_DELETE)
    {
        if (map->vertex[VertexY*map->width+VertexX].objectInfo == 0x80)
        {
            //at first delete the player position using the number of the player as saved in objectType
            if (map->vertex[VertexY*map->width+VertexX].objectType < MAXPLAYERS)
            {
                PlayerHQx[map->vertex[VertexY*map->width+VertexX].objectType] = 0xFFFF;
                PlayerHQy[map->vertex[VertexY*map->width+VertexX].objectType] = 0xFFFF;

                //for compatibility with original settlers 2 we write the headquarters positions to the map header (for the first 7 players)
                if (modeContent >= 0 && modeContent < 7)
                {
                    map->HQx[map->vertex[VertexY*map->width+VertexX].objectType] = 0xFFFF;
                    map->HQy[map->vertex[VertexY*map->width+VertexX].objectType] = 0xFFFF;
                }
            }

            map->vertex[VertexY*map->width+VertexX].objectType = 0x00;
            map->vertex[VertexY*map->width+VertexX].objectInfo = 0x00;

            //setup number of players in map header
            map->player--;
        }
    }

    //at least setup the possible building at the vertex and 2 sections around
    struct cursorPoint tempVertices[19];
    calculateVerticesAround(tempVertices, VertexX, VertexY, 2);
    for (int i = 0; i < 19; i++)
        modifyBuild(tempVertices[i].x, tempVertices[i].y);

    if (PlayerRePositioned)
    {
        calculateVerticesAround(tempVertices, oldPositionX, oldPositionY, 2);
        for (int i = 0; i < 19; i++)
            modifyBuild(tempVertices[i].x, tempVertices[i].y);
    }
}

int CMap::getActiveVertices(int tempChangeSection)
{
    int total = 0;
    for (int i = tempChangeSection; i > 0; i--)
        total += i;
    return ( 6*total + 1);
}

void CMap::calculateVertices()
{
    bool even = false;
    if (VertexY%2 == 0)
        even = true;

    int index = 0;
    for (int i = -MAX_CHANGE_SECTION; i <= MAX_CHANGE_SECTION; i++)
    {
        if (abs(i)%2 == 0)
        {
            for (int j = -MAX_CHANGE_SECTION; j <= MAX_CHANGE_SECTION; j++, index++)
            {
                Vertices[index].x = VertexX + j;
                if (Vertices[index].x < 0)
                    Vertices[index].x += map->width;
                else if (Vertices[index].x >= map->width)
                    Vertices[index].x -= map->width;
                Vertices[index].y = VertexY + i;
                if (Vertices[index].y < 0)
                    Vertices[index].y += map->height;
                else if (Vertices[index].y >= map->height)
                    Vertices[index].y -= map->height;
                Vertices[index].blit_x = correctMouseBlitX(Vertices[index].x, Vertices[index].y);
                Vertices[index].blit_y = correctMouseBlitY(Vertices[index].x, Vertices[index].y);
            }
        }
        else
        {
            for (int j = -MAX_CHANGE_SECTION; j <= MAX_CHANGE_SECTION-1; j++, index++)
            {
                Vertices[index].x = VertexX + (even ? j : j+1);
                if (Vertices[index].x < 0)
                    Vertices[index].x += map->width;
                else if (Vertices[index].x >= map->width)
                    Vertices[index].x -= map->width;
                Vertices[index].y = VertexY + i;
                if (Vertices[index].y < 0)
                    Vertices[index].y += map->height;
                else if (Vertices[index].y >= map->height)
                    Vertices[index].y -= map->height;
                Vertices[index].blit_x = correctMouseBlitX(Vertices[index].x, Vertices[index].y);
                Vertices[index].blit_y = correctMouseBlitY(Vertices[index].x, Vertices[index].y);
            }
        }
    }
    //check if cursor vertices should change randomly
    if (VertexActivityRandom || VertexFillRandom)
        setupVerticesActivity();
}

void CMap::calculateVerticesAround(struct cursorPoint Vertices[], int VertexX, int VertexY, int ChangeSection)
{
    bool even = false;
    if (VertexY%2 == 0)
        even = true;

    Vertices[0].x = VertexX;
    Vertices[0].y = VertexY;
    Vertices[0].blit_x = MouseBlitX;
    Vertices[0].blit_y = MouseBlitY;

    if (ChangeSection > 0)
    {
        Vertices[1].x = VertexX - (even ? 1 : 0);   if (Vertices[1].x < 0) Vertices[1].x += map->width;
        Vertices[1].y = VertexY-1;                  if (Vertices[1].y < 0) Vertices[1].y += map->height;
        Vertices[1].blit_x = correctMouseBlitX(Vertices[1].x, Vertices[1].y);
        Vertices[1].blit_y = correctMouseBlitY(Vertices[1].x, Vertices[1].y);
        Vertices[2].x = VertexX + (even ? 0 : 1);   if (Vertices[2].x >= map->width) Vertices[2].x -= map->width;
        Vertices[2].y = VertexY-1;                  if (Vertices[2].y < 0) Vertices[2].y += map->height;
        Vertices[2].blit_x = correctMouseBlitX(Vertices[2].x, Vertices[2].y);
        Vertices[2].blit_y = correctMouseBlitY(Vertices[2].x, Vertices[2].y);
        Vertices[3].x = VertexX-1;                  if (Vertices[3].x < 0) Vertices[3].x += map->width;
        Vertices[3].y = VertexY;
        Vertices[3].blit_x = correctMouseBlitX(Vertices[3].x, Vertices[3].y);
        Vertices[3].blit_y = correctMouseBlitY(Vertices[3].x, Vertices[3].y);
        Vertices[4].x = VertexX+1;                  if (Vertices[4].x >= map->width) Vertices[4].x -= map->width;
        Vertices[4].y = VertexY;
        Vertices[4].blit_x = correctMouseBlitX(Vertices[4].x, Vertices[4].y);
        Vertices[4].blit_y = correctMouseBlitY(Vertices[4].x, Vertices[4].y);
        Vertices[5].x = VertexX - (even ? 1 : 0);   if (Vertices[5].x < 0) Vertices[5].x += map->width;
        Vertices[5].y = VertexY+1;                  if (Vertices[5].y >= map->height) Vertices[5].y -= map->height;
        Vertices[5].blit_x = correctMouseBlitX(Vertices[5].x, Vertices[5].y);
        Vertices[5].blit_y = correctMouseBlitY(Vertices[5].x, Vertices[5].y);
        Vertices[6].x = VertexX + (even ? 0 : 1);   if (Vertices[6].x >= map->width) Vertices[6].x -= map->width;
        Vertices[6].y = VertexY+1;                  if (Vertices[6].y >= map->height) Vertices[6].y -= map->height;
        Vertices[6].blit_x = correctMouseBlitX(Vertices[6].x, Vertices[6].y);
        Vertices[6].blit_y = correctMouseBlitY(Vertices[6].x, Vertices[6].y);
    }
    if (ChangeSection > 1)
    {
        Vertices[7].x = VertexX - 1;                 if (Vertices[7].x < 0) Vertices[7].x += map->width;
        Vertices[7].y = VertexY-2;                   if (Vertices[7].y < 0) Vertices[7].y += map->height;
        Vertices[7].blit_x = correctMouseBlitX(Vertices[7].x, Vertices[7].y);
        Vertices[7].blit_y = correctMouseBlitY(Vertices[7].x, Vertices[7].y);
        Vertices[8].x = VertexX;
        Vertices[8].y = VertexY-2;                   if (Vertices[8].y < 0) Vertices[8].y += map->height;
        Vertices[8].blit_x = correctMouseBlitX(Vertices[8].x, Vertices[8].y);
        Vertices[8].blit_y = correctMouseBlitY(Vertices[8].x, Vertices[8].y);
        Vertices[9].x = VertexX + 1;                 if (Vertices[9].x >= map->width) Vertices[9].x -= map->width;
        Vertices[9].y = VertexY-2;                   if (Vertices[9].y < 0) Vertices[9].y += map->height;
        Vertices[9].blit_x = correctMouseBlitX(Vertices[9].x, Vertices[9].y);
        Vertices[9].blit_y = correctMouseBlitY(Vertices[9].x, Vertices[9].y);
        Vertices[10].x = VertexX - (even ? 2 : 1);   if (Vertices[10].x < 0) Vertices[10].x += map->width;
        Vertices[10].y = VertexY-1;                  if (Vertices[10].y < 0) Vertices[10].y += map->height;
        Vertices[10].blit_x = correctMouseBlitX(Vertices[10].x, Vertices[10].y);
        Vertices[10].blit_y = correctMouseBlitY(Vertices[10].x, Vertices[10].y);
        Vertices[11].x = VertexX + (even ? 1 : 2);   if (Vertices[11].x >= map->width) Vertices[11].x -= map->width;
        Vertices[11].y = VertexY-1;                  if (Vertices[11].y < 0) Vertices[11].y += map->height;
        Vertices[11].blit_x = correctMouseBlitX(Vertices[11].x, Vertices[11].y);
        Vertices[11].blit_y = correctMouseBlitY(Vertices[11].x, Vertices[11].y);
        Vertices[12].x = VertexX-2;                  if (Vertices[12].x < 0) Vertices[12].x += map->width;
        Vertices[12].y = VertexY;
        Vertices[12].blit_x = correctMouseBlitX(Vertices[12].x, Vertices[12].y);
        Vertices[12].blit_y = correctMouseBlitY(Vertices[12].x, Vertices[12].y);
        Vertices[13].x = VertexX+2;                  if (Vertices[13].x >= map->width) Vertices[13].x -= map->width;
        Vertices[13].y = VertexY;
        Vertices[13].blit_x = correctMouseBlitX(Vertices[13].x, Vertices[13].y);
        Vertices[13].blit_y = correctMouseBlitY(Vertices[13].x, Vertices[13].y);
        Vertices[14].x = VertexX - (even ? 2 : 1);   if (Vertices[14].x < 0) Vertices[14].x += map->width;
        Vertices[14].y = VertexY+1;                  if (Vertices[14].y >= map->height) Vertices[14].y -= map->height;
        Vertices[14].blit_x = correctMouseBlitX(Vertices[14].x, Vertices[14].y);
        Vertices[14].blit_y = correctMouseBlitY(Vertices[14].x, Vertices[14].y);
        Vertices[15].x = VertexX + (even ? 1 : 2);   if (Vertices[15].x >= map->width) Vertices[15].x -= map->width;
        Vertices[15].y = VertexY+1;                  if (Vertices[15].y >= map->height) Vertices[15].y -= map->height;
        Vertices[15].blit_x = correctMouseBlitX(Vertices[15].x, Vertices[15].y);
        Vertices[15].blit_y = correctMouseBlitY(Vertices[15].x, Vertices[15].y);
        Vertices[16].x = VertexX - 1;                if (Vertices[16].x < 0) Vertices[16].x += map->width;
        Vertices[16].y = VertexY+2;                  if (Vertices[16].y >= map->height) Vertices[16].y -= map->height;
        Vertices[16].blit_x = correctMouseBlitX(Vertices[16].x, Vertices[16].y);
        Vertices[16].blit_y = correctMouseBlitY(Vertices[16].x, Vertices[16].y);
        Vertices[17].x = VertexX;
        Vertices[17].y = VertexY+2;                  if (Vertices[17].y >= map->height) Vertices[17].y -= map->height;
        Vertices[17].blit_x = correctMouseBlitX(Vertices[17].x, Vertices[17].y);
        Vertices[17].blit_y = correctMouseBlitY(Vertices[17].x, Vertices[17].y);
        Vertices[18].x = VertexX + 1;                if (Vertices[18].x >= map->width) Vertices[18].x -= map->width;
        Vertices[18].y = VertexY+2;                  if (Vertices[18].y >= map->height) Vertices[18].y -= map->height;
        Vertices[18].blit_x = correctMouseBlitX(Vertices[18].x, Vertices[18].y);
        Vertices[18].blit_y = correctMouseBlitY(Vertices[18].x, Vertices[18].y);
    }
}

void CMap::setupVerticesActivity(void)
{
    int index = 0;
    for (int i = -MAX_CHANGE_SECTION; i <= MAX_CHANGE_SECTION; i++)
    {
        if (abs(i)%2 == 0)
        {
            for (int j = -MAX_CHANGE_SECTION; j <= MAX_CHANGE_SECTION; j++, index++)
            {
                if (abs(i) <= ChangeSection && abs(j) <= ChangeSection-(ChangeSectionHexagonMode ? abs(i/2) : 0))
                {
                    //check if cursor vertices should change randomly
                    if (VertexActivityRandom)
                        Vertices[index].active = (rand()%2==1 ? true : false);
                    else
                        Vertices[index].active = true;

                    //decide which triangle-textures will be filled at this vertex (necessary for border)
                    Vertices[index].fill_rsu = (VertexFillRSU ? true : (VertexFillRandom ? (rand()%2 == 1 ? true : false) : false));
                    Vertices[index].fill_usd = (VertexFillUSD ? true : (VertexFillRandom ? (rand()%2 == 1 ? true : false) : false));

                    //if we have a ChangeSection greater than zero
                    if (ChangeSection)
                    {
                        //if we are in hexagon mode
                        if (ChangeSectionHexagonMode)
                        {
                            //if we walk through the upper rows of the cursor field
                            if (i < 0)
                            {
                                //right vertex of the row
                                if (j == ChangeSection-abs(i/2))
                                    Vertices[index].fill_usd = false;
                            }
                            //if we are at the last lower row
                            else if (i == ChangeSection)
                            {
                                Vertices[index].fill_rsu = false;
                                Vertices[index].fill_usd = false;
                            }
                            //if we walk through the lower rows of the cursor field
                            else //if (i >= 0 && i != ChangeSection)
                            {
                                //left vertex of the row
                                if (j == -ChangeSection+abs(i/2))
                                    Vertices[index].fill_rsu = false;
                                //right vertex of the row
                                else if (j == ChangeSection-abs(i/2))
                                {
                                    Vertices[index].fill_rsu = false;
                                    Vertices[index].fill_usd = false;
                                }
                            }
                        }
                        //we are in square mode
                        else
                        {
                            //if we are at the last lower row
                            if (i == ChangeSection)
                            {
                                Vertices[index].fill_rsu = false;
                                Vertices[index].fill_usd = false;
                            }
                            //left vertex of the row
                            else if (j == -ChangeSection)
                                Vertices[index].fill_rsu = false;
                            //right vertex of the row
                            else if (j == ChangeSection)
                            {
                                Vertices[index].fill_rsu = false;
                                Vertices[index].fill_usd = false;
                            }
                        }
                    }
                }
                else
                {
                    Vertices[index].active = false;
                    Vertices[index].fill_rsu = false;
                    Vertices[index].fill_usd = false;
                }
            }
        }
        else
        {
            for (int j = -MAX_CHANGE_SECTION; j <= MAX_CHANGE_SECTION-1; j++, index++)
            {
                if (abs(i) <= ChangeSection && ( j<0 ? abs(j) <= ChangeSection-(ChangeSectionHexagonMode ? abs(i/2) : 0) : j <= ChangeSection-1-(ChangeSectionHexagonMode ? abs(i/2) : 0)))
                {
                    //check if cursor vertices should change randomly
                    if (VertexActivityRandom)
                        Vertices[index].active = (rand()%2==1 ? true : false);
                    else
                        Vertices[index].active = true;

                    //decide which triangle-textures will be filled at this vertex (necessary for border)
                    Vertices[index].fill_rsu = (VertexFillRSU ? true : (VertexFillRandom ? (rand()%2 == 1 ? true : false) : false));
                    Vertices[index].fill_usd = (VertexFillUSD ? true : (VertexFillRandom ? (rand()%2 == 1 ? true : false) : false));

                    //if we have a ChangeSection greater than zero
                    if (ChangeSection)
                    {
                        //if we are in hexagon mode
                        if (ChangeSectionHexagonMode)
                        {
                            //if we walk through the upper rows of the cursor field
                            if (i < 0)
                            {
                                //right vertex of the row
                                if (j == ChangeSection-1-abs(i/2))
                                    Vertices[index].fill_usd = false;
                            }
                            //if we are at the last lower row
                            else if (i == ChangeSection)
                            {
                                Vertices[index].fill_rsu = false;
                                Vertices[index].fill_usd = false;
                            }
                            //if we walk through the lower rows of the cursor field
                            else //if (i >= 0 && i != ChangeSection)
                            {
                                //left vertex of the row
                                if (j == -ChangeSection+abs(i/2))
                                    Vertices[index].fill_rsu = false;
                                //right vertex of the row
                                else if (j == ChangeSection-1-abs(i/2))
                                {
                                    Vertices[index].fill_rsu = false;
                                    Vertices[index].fill_usd = false;
                                }
                            }
                        }
                        //we are in square mode
                        else
                        {
                            //if we are at the last lower row
                            if (i == ChangeSection)
                            {
                                Vertices[index].fill_rsu = false;
                                Vertices[index].fill_usd = false;
                            }
                            //right vertex of the row
                            else if (j == ChangeSection-1)
                                Vertices[index].fill_usd = false;
                        }
                    }
                }
                else
                {
                    Vertices[index].active = false;
                    Vertices[index].fill_rsu = false;
                    Vertices[index].fill_usd = false;
                }
            }
        }
    }
    //NOTE: to understand this '-(ChangeSectionHexagonMode ? abs(i/2) : 0)'
    //if we don't change the cursor size in square-mode, but in hexagon mode,
    //at each row there have to be missing as much vertices as the row number is
    // i = row number --> so at the left side of the row there are missing i/2
    //and at the right side there are missing i/2. That makes it look like an hexagon.
}

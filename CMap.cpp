#include "CMap.h"

CMap::CMap(char *filename)
{
    Surf_Map = NULL;
    Surf_RightMenubar = NULL;
    displayRect.x = 0;
    displayRect.y = 0;
    displayRect.w = global::s2->GameResolutionX;
    displayRect.h = global::s2->GameResolutionY;
    map = (bobMAP*)CFile::open_file(filename, WLD); //TODO: open_file(filename, SWD); if really necessary

    //load the right MAP0x.LST for all pictures
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
    CFile::set_palActual(CFile::get_palArray());

    CSurface::get_nodeVectors(map);
    needSurface = true;
    active = true;
    VertexX = 10;
    VertexY = 10;
    BuildHelp = false;
    MouseBlitX = correctMouseBlitX(VertexX, VertexY);
    MouseBlitY = correctMouseBlitY(VertexX, VertexY);
    ChangeSection = 1;
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
    Vertices = (struct vertexPoint*)malloc(VertexCounter*sizeof(struct vertexPoint));
    calculateVertices();
    setupVerticesActivity();
    mode = EDITOR_MODE_RAISE;
    lastMode = EDITOR_MODE_RAISE;
    modeContent = 0x00;
    modeContent2 = 0x00;
    modify = false;
}

CMap::~CMap()
{
    //free all surfaces that MAP0x.LST needed
    for (int i = MAPPIC_ARROWCROSS_YELLOW; i <= MAPPIC_LAST_ENTRY; i++)
    {
        SDL_FreeSurface(global::bmpArray[i].surface);
        global::bmpArray[i].surface = NULL;
    }
    //set back bmpArray-pointer, cause MAP0x.LST is no longer needed
    CFile::set_bmpArray(global::bmpArray+MAPPIC_ARROWCROSS_YELLOW);
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

void CMap::setMouseData(SDL_MouseMotionEvent motion)
{
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

    saveVertex(motion.x, motion.y, motion.state);
}

void CMap::setMouseData(SDL_MouseButtonEvent button)
{
    if (button.state == SDL_PRESSED)
    {
        #ifdef _EDITORMODE
        //find out if user clicked on one of the game menu pictures
        //we start with lower menubar
        if (button.button == SDL_BUTTON_LEFT && button.x >= (displayRect.w/2+203) && button.x <= (displayRect.w/2+240)
                                             && button.y >= (displayRect.h-35) && button.y <= (displayRect.h-3)
           )
        {
            //the editor-main-menu picture was clicked
            callback::EditorQuitMenu(INITIALIZING_CALL); //"quit" menu is temporary, later this will be "main" menu
            return;
        }
        else if (button.button == SDL_BUTTON_LEFT && button.x >= (displayRect.w/2-236) && button.x <= (displayRect.w/2-199)
                                                  && button.y >= (displayRect.h-35) && button.y <= (displayRect.h-3)
           )
        {
            //the height-mode picture was clicked
            mode = EDITOR_MODE_RAISE;
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
        else if (button.button == SDL_BUTTON_LEFT && button.x >= (displayRect.w/2-88) && button.x <= (displayRect.w/2-51)
                                                  && button.y >= (displayRect.h-35) && button.y <= (displayRect.h-3)
           )
        {
            //the landscape-mode picture was clicked
            mode = EDITOR_MODE_LANDSCAPE;
            callback::EditorLandscapeMenu(INITIALIZING_CALL);
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
            callback::EditorMinimapMenu(INITIALIZING_CALL);
            return;
        }
        //now we check the right menubar
        else if (button.button == SDL_BUTTON_LEFT && button.x >= (displayRect.w-37) && button.x <= (displayRect.w)
                                                  && button.y >= (displayRect.h/2+200) && button.y <= (displayRect.h/2+237)
           )
        {
            //the temproray save-map picture was clicked
            CFile::save_file("./WORLDS/NEW_MAP.SWD", SWD, map);
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
                modify = true;
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
        if (key.keysym.sym == SDLK_LSHIFT && mode == EDITOR_MODE_RAISE)
            mode = EDITOR_MODE_REDUCE;
        else if (key.keysym.sym == SDLK_LCTRL)
        {
            lastMode = mode;
            mode = EDITOR_MODE_CUT;
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
        else if (key.keysym.sym == SDLK_SPACE)
        {
            BuildHelp = !BuildHelp;
        }
    }
    else if (key.type == SDL_KEYUP)
    {
        //user probably released EDITOR_MODE_REDUCE
        if (key.keysym.sym == SDLK_LSHIFT && mode == EDITOR_MODE_REDUCE)
            mode = EDITOR_MODE_RAISE;
        //user probably released EDITOR_MODE_CUT
        else if (key.keysym.sym == SDLK_LCTRL)
            mode = lastMode;
    }
}

void CMap::saveVertex(Uint16 MouseX, Uint16 MouseY, Uint8 MouseState)
{
    //if user raises or reduces the height of a vertex, don't let the cursor jump to another vertex
    //if ( (MouseState == SDL_PRESSED) && (mode == EDITOR_MODE_RAISE || mode == EDITOR_MODE_REDUCE) )
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
        if ( (Surf_Map = SDL_CreateRGBSurface(SDL_HWSURFACE, displayRect.w, displayRect.h, 32, 0, 0, 0, 0)) == NULL )
            return false;
        needSurface = false;
    }
    else
        //clear the surface before drawing new (in normal case not needed)
        //SDL_FillRect( Surf_Map, NULL, SDL_MapRGB(Surf_Map->format,0,0,0) );

    //touch vertex data if user modifies it
    if (modify)
        modifyVertex();

    if (map->vertex != NULL)
        CSurface::DrawTriangleField(Surf_Map, displayRect, map);

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

    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_LANDSCAPE].surface, displayRect.w/2-84, displayRect.h-37);

    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_BUILDHELP].surface, displayRect.w/2+96, displayRect.h-35);
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_MINIMAP].surface, displayRect.w/2+131, displayRect.h-37);

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

    //draw picture to cursor position
#ifdef _EDITORMODE
    int symbol_index;
    switch (mode)
    {
        case EDITOR_MODE_CUT:           symbol_index = CURSOR_SYMBOL_SCISSORS;
                                        break;
        case EDITOR_MODE_TREE:          symbol_index = CURSOR_SYMBOL_TREE;
                                        break;
        case EDITOR_MODE_RAISE:         symbol_index = CURSOR_SYMBOL_ARROW_UP;
                                        break;
        case EDITOR_MODE_REDUCE:        symbol_index = CURSOR_SYMBOL_ARROW_DOWN;
                                        break;
        case EDITOR_MODE_TEXTURE:       symbol_index = CURSOR_SYMBOL_TEXTURE;
                                        break;
        case EDITOR_MODE_LANDSCAPE:     symbol_index = CURSOR_SYMBOL_LANDSCAPE;
                                        break;
        case EDITOR_MODE_FLAG:          symbol_index = CURSOR_SYMBOL_FLAG;
                                        break;
        case EDITOR_MODE_PICKAXE_MINUS: symbol_index = CURSOR_SYMBOL_PICKAXE_MINUS;
                                        break;
        case EDITOR_MODE_PICKAXE_PLUS:  symbol_index = CURSOR_SYMBOL_PICKAXE_PLUS;
                                        break;
        case EDITOR_MODE_ANIMAL:        symbol_index = CURSOR_SYMBOL_ANIMAL;
                                        break;
        default:                        symbol_index = CURSOR_SYMBOL_ARROW_UP;
                                        break;
    }
    for (int i = 0; i < VertexCounter; i++)
        if (Vertices[i].active)
            CSurface::Draw(Surf_Map, global::bmpArray[symbol_index].surface, Vertices[i].blit_x-10, Vertices[i].blit_y-10);

    //text for x and y of vertex (shown in upper left corner)
    sprintf(textBuffer, "%d    %d", VertexX, VertexY);
    CFont::writeText(Surf_Map, textBuffer, 20, 20);
#else
    CSurface::Draw(Surf_Map, global::bmpArray[CIRCLE_FLAT_GREY].surface, MouseBlitX-10, MouseBlitY-10);
#endif

    return true;
}

void CMap::drawMinimap(SDL_Surface *Window)
{
    Uint32 *pixel;
    Uint32 *row;

    Uint8 r8,g8,b8;
    Sint16 r,g,b;

    if (Window->w < map->width || Window->h < map->height)
        return;

    for (int y = 0; y < map->height; y++)
    {
        row = (Uint32 *)Window->pixels + (y+20)*Window->pitch/4;
        for (int x = 0; x < map->width; x++)
        {
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

            pixel = row + x + 6;

            r = ( (r*map->vertex[y*map->width+x].i) >>16 );
            g = ( (g*map->vertex[y*map->width+x].i) >>16 );
            b = ( (b*map->vertex[y*map->width+x].i) >>16 );
            r8 = (Uint8)(r > 255 ? 255 : (r < 0 ? 0 : r));
            g8 = (Uint8)(g > 255 ? 255 : (g < 0 ? 0 : g));
            b8 = (Uint8)(b > 255 ? 255 : (b < 0 ? 0 : b));
            *pixel = ( (r8 << Window->format->Rshift) + (g8 << Window->format->Gshift) + (b8 << Window->format->Bshift) );
        }
    }

    //draw the arrow --> 6px is width of left window frame and 20px is the height of the upper window frame
    CSurface::Draw(Window, global::bmpArray[MAPPIC_ARROWCROSS_ORANGE].surface, 6+(displayRect.x+displayRect.w/2)/TRIANGLE_WIDTH-global::bmpArray[MAPPIC_ARROWCROSS_ORANGE].nx, 20+(displayRect.y+displayRect.h/2)/TRIANGLE_HEIGHT-global::bmpArray[MAPPIC_ARROWCROSS_ORANGE].ny);
}

void CMap::modifyVertex(void)
{
    static Uint32 TimeOfLastModification = SDL_GetTicks();

    if ( (SDL_GetTicks() - TimeOfLastModification) < 50 )
        return;
    else
        TimeOfLastModification = SDL_GetTicks();

    if (mode == EDITOR_MODE_RAISE || mode == EDITOR_MODE_REDUCE)
    {
        for (int i = 0; i < VertexCounter; i++)
            if (Vertices[i].active)
                modifyHeight(Vertices[i].x, Vertices[i].y);
    }
    //at this time we need a content to set
    else if (mode == EDITOR_MODE_CUT)
    {
        for (int i = 0; i < VertexCounter; i++)
            if (Vertices[i].active)
                modifyObject(Vertices[i].x, Vertices[i].y);
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
}

void CMap::modifyHeight(int VertexX, int VertexY)
{
    //vertex count for the points
    int X, Y;
    struct point *tempP = &map->vertex[VertexY*map->width+VertexX];
    //this is to setup the buldings around the vertex (2 sections from the cursor)
    struct vertexPoint tempVertices[19];
    calculateVerticesAround(tempVertices, VertexX, VertexY, 2);

    bool even = false;
    if (VertexY%2 == 0)
        even = true;

    if (mode == EDITOR_MODE_RAISE)
    {
        if (tempP->z >= 250) //maximum reached, 250 = 5*(0x3C - 0x0A), see open_wld for info
            return;
        tempP->y -= 5;
        tempP->z += 5;
        tempP->h += 0x01;
        CSurface::update_shading(map, VertexX, VertexY);

        //after 25 pixel all vertices around will be raised too
        //update first vertex left upside
        X = VertexX - (even ? 1 : 0);   if (X < 0) X += map->width;
        Y = VertexY-1;                  if (Y < 0) Y += map->height;
        //only modify if the other point is lower than the middle point of the hexagon (-5 cause point was raised a few lines before)
        if (map->vertex[Y*map->width+X].z < tempP->z-25)
            modifyHeight(X, Y);
        //update second vertex right upside
        X = VertexX + (even ? 0 : 1);   if (X >= map->width) X -= map->width;
        Y = VertexY-1;                  if (Y < 0) Y += map->height;
        //only modify if the other point is lower than the middle point of the hexagon (-5 cause point was raised a few lines before)
        if (map->vertex[Y*map->width+X].z < tempP->z-25)
            modifyHeight(X, Y);
        //update third point bottom left
        X = VertexX-1;                  if (X < 0) X += map->width;
        Y = VertexY;
        //only modify if the other point is lower than the middle point of the hexagon (-5 cause point was raised a few lines before)
        if (map->vertex[Y*map->width+X].z < tempP->z-25)
            modifyHeight(X, Y);
        //update fourth point bottom right
        X = VertexX+1;                  if (X >= map->width) X -= map->width;
        Y = VertexY;
        //only modify if the other point is lower than the middle point of the hexagon (-5 cause point was raised a few lines before)
        if (map->vertex[Y*map->width+X].z < tempP->z-25)
            modifyHeight(X, Y);
        //update fifth point down left
        X = VertexX - (even ? 1 : 0);   if (X < 0) X += map->width;
        Y = VertexY+1;                  if (Y >= map->height) Y -= map->height;
        //only modify if the other point is lower than the middle point of the hexagon (-5 cause point was raised a few lines before)
        if (map->vertex[Y*map->width+X].z < tempP->z-25)
            modifyHeight(X, Y);
        //update sixth point down right
        X = VertexX + (even ? 0 : 1);   if (X >= map->width) X -= map->width;
        Y = VertexY+1;                  if (Y >= map->height) Y -= map->height;
        //only modify if the other point is lower than the middle point of the hexagon (-5 cause point was raised a few lines before)
        if (map->vertex[Y*map->width+X].z < tempP->z-25)
            modifyHeight(X, Y);
    }
    else if (mode == EDITOR_MODE_REDUCE)
    {
        if (tempP->z <= -50) //minimum reached, -50 = 5*(0x00 - 0x0A), see open_wld for info
            return;
        tempP->y += 5;
        tempP->z -= 5;
        tempP->h -= 0x01;
        CSurface::update_shading(map, VertexX, VertexY);
        //after 25 pixel all vertices around will be reduced too
        //update first vertex left upside
        X = VertexX - (even ? 1 : 0);   if (X < 0) X += map->width;
        Y = VertexY-1;                  if (Y < 0) Y += map->height;
        //only modify if the other point is higher than the middle point of the hexagon (+5 cause point was reduced a few lines before)
        if (map->vertex[Y*map->width+X].z > tempP->z+25)
            modifyHeight(X, Y);
        //update second vertex right upside
        X = VertexX + (even ? 0 : 1);   if (X >= map->width) X -= map->width;
        Y = VertexY-1;                  if (Y < 0) Y += map->height;
        //only modify if the other point is higher than the middle point of the hexagon (+5 cause point was reduced a few lines before)
        if (map->vertex[Y*map->width+X].z > tempP->z+25)
            modifyHeight(X, Y);
        //update third point bottom left
        X = VertexX-1;                  if (X < 0) X += map->width;
        Y = VertexY;
        //only modify if the other point is higher than the middle point of the hexagon (+5 cause point was reduced a few lines before)
        if (map->vertex[Y*map->width+X].z > tempP->z+25)
            modifyHeight(X, Y);
        //update fourth point bottom right
        X = VertexX+1;                  if (X >= map->width) X -= map->width;
        Y = VertexY;
        //only modify if the other point is higher than the middle point of the hexagon (+5 cause point was reduced a few lines before)
        if (map->vertex[Y*map->width+X].z > tempP->z+25)
            modifyHeight(X, Y);
        //update fifth point down left
        X = VertexX - (even ? 1 : 0);   if (X < 0) X += map->width;
        Y = VertexY+1;                  if (Y >= map->height) Y -= map->height;
        //only modify if the other point is higher than the middle point of the hexagon (+5 cause point was reduced a few lines before)
        if (map->vertex[Y*map->width+X].z > tempP->z+25)
            modifyHeight(X, Y);
        //update sixth point down right
        X = VertexX + (even ? 0 : 1);   if (X >= map->width) X -= map->width;
        Y = VertexY+1;                  if (Y >= map->height) Y -= map->height;
        //only modify if the other point is higher than the middle point of the hexagon (+5 cause point was reduced a few lines before)
        if (map->vertex[Y*map->width+X].z > tempP->z+25)
            modifyHeight(X, Y);
    }
    //at least setup the possible building at the vertex and 2 sections around
    for (int i = 0; i < 19; i++)
        modifyBuild(tempVertices[i].x, tempVertices[i].y);
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

    //at least setup the possible building at the vertex and 1 section around
    struct vertexPoint tempVertices[7];
    calculateVerticesAround(tempVertices, VertexX, VertexY, 1);
    for (int i = 0; i < 7; i++)
        modifyBuild(tempVertices[i].x, tempVertices[i].y);
}

void CMap::modifyObject(int VertexX, int VertexY)
{
    if (mode == EDITOR_MODE_CUT)
    {
        map->vertex[VertexY*map->width+VertexX].objectType = 0x00;
        map->vertex[VertexY*map->width+VertexX].objectInfo = 0x00;
        map->vertex[VertexY*map->width+VertexX].animal = 0x00;
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
    }
    //at least setup the possible building at the vertex and 1 section around
    struct vertexPoint tempVertices[7];
    calculateVerticesAround(tempVertices, VertexX, VertexY, 1);
    for (int i = 0; i < 7; i++)
        modifyBuild(tempVertices[i].x, tempVertices[i].y);
}

void CMap::modifyBuild(int VertexX, int VertexY)
{
    //at first save all vertices we need to calculate the new building
    struct vertexPoint tempVertices[19];
    calculateVerticesAround(tempVertices, VertexX, VertexY, 2);

    //evtl. keine festen werte sondern addition und subtraktion wegen originalkompatibilitaet (bei baeumen bspw. keine 0x00 sondern 0x68)


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

    //test if there is snow or lava in lower right (first section)
    if (building > 0x01)
    {
        if (    map->vertex[tempVertices[6].y*map->width+tempVertices[6].x].rsuTexture == TRIANGLE_TEXTURE_SNOW
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


    map->vertex[VertexY*map->width+VertexX].build = building;
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

void CMap::calculateVerticesAround(struct vertexPoint Vertices[], int VertexX, int VertexY, int ChangeSection)
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

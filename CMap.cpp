#include "CMap.h"

CMap::CMap(char *filename)
{
    Surf_Map = NULL;
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
    VertexX = 0;
    VertexY = 0;
    BuildHelp = false;
    MouseBlitX = 0;
    MouseBlitY = 0;
    ChangeSection = 0;
    VertexCounter = 1;
    for (int i = 0; i < 37; i++)
    {
        Vertices[i].x = 0;
        Vertices[i].y = 0;
        Vertices[i].blit_x = 0;
        Vertices[i].blit_y = 0;
    }
    mode = EDITOR_MODE_RAISE;
    modeContent = -1;
    modeContent2 = -1;
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
    if ((motion.state&SDL_BUTTON(SDL_BUTTON_RIGHT))!=0)
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
            if (BuildHelp)
                BuildHelp = false;
            else
                BuildHelp = true;
            return;
        }
        else if (button.button == SDL_BUTTON_LEFT && button.x >= (displayRect.w/2+131) && button.x <= (displayRect.w/2+168)
                                                  && button.y >= (displayRect.h-35) && button.y <= (displayRect.h-3)
           )
        {
            //the temproray save-map picture was clicked
            CFile::save_file("./WORLDS/NEW_MAP.SWD", SWD, map);
            return;
        }

        //touch vertex data
        if (button.button == SDL_BUTTON_LEFT)
            modify = true;
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
        //touch vertex data
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
            mode = EDITOR_MODE_CUT;
            modeContent = 0;
        }
        else if (key.keysym.sym == SDLK_KP_PLUS)
        {
            if ( getActiveVertices(ChangeSection+1) <= 19) //temproary set to seven, cause more is not implemented yet
            {
                ChangeSection++;
                VertexCounter = getActiveVertices(ChangeSection);
                calculateVertices(Vertices, VertexX, VertexY, ChangeSection);
            }
        }
        else if (key.keysym.sym == SDLK_KP_MINUS)
        {
            if (ChangeSection > 0)
            {
                ChangeSection--;
                VertexCounter = getActiveVertices(ChangeSection);
                calculateVertices(Vertices, VertexX, VertexY, ChangeSection);
            }
        }
        else if (key.keysym.sym == SDLK_SPACE)
        {
            if (BuildHelp)
                BuildHelp = false;
            else
                BuildHelp = true;
        }
    }
    else if (key.type == SDL_KEYUP)
    {
        if (key.keysym.sym == SDLK_LSHIFT)
            mode = EDITOR_MODE_RAISE;
        else if (key.keysym.sym == SDLK_LCTRL)
            mode = EDITOR_MODE_RAISE;
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

    MouseBlitX = correctMouseBlitX(MouseBlitX, VertexX, VertexY);
    MouseBlitY = correctMouseBlitY(MouseBlitY, VertexX, VertexY);

    calculateVertices(Vertices, VertexX, VertexY, ChangeSection);
}

int CMap::correctMouseBlitX(int MouseBlitX, int VertexX, int VertexY)
{
    MouseBlitX = map->vertex[VertexY*map->width+VertexX].x;
    if (MouseBlitX < displayRect.x)
        MouseBlitX += map->width_pixel;
    else if (MouseBlitX > (displayRect.x+displayRect.w))
        MouseBlitX -= map->width_pixel;
    MouseBlitX -= displayRect.x;

    return MouseBlitX;
}
int CMap::correctMouseBlitY(int MouseBlitX, int VertexX, int VertexY)
{
    MouseBlitY = map->vertex[VertexY*map->width+VertexX].y;
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

    //draw menubar
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR].surface, displayRect.w/2-global::bmpArray[MENUBAR].w/2, displayRect.h-global::bmpArray[MENUBAR].h);

    //draw pictures to menubar
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
    //temprorary to save a map
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_BUGKILL].surface, displayRect.w/2+131, displayRect.h-37);

    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_COMPUTER].surface, displayRect.w/2+207, displayRect.h-35);
#else

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
        CSurface::Draw(Surf_Map, global::bmpArray[symbol_index].surface, Vertices[i].blit_x-10, Vertices[i].blit_y-10);
#else
    CSurface::Draw(Surf_Map, global::bmpArray[CIRCLE_FLAT_GREY].surface, MouseBlitX-10, MouseBlitY-10);
#endif

    return true;
}

void CMap::modifyVertex(void)
{
    static Uint32 TimeOfLastModification = SDL_GetTicks();

    if ( (SDL_GetTicks() - TimeOfLastModification) < 50 )
        return;
    else
        TimeOfLastModification = SDL_GetTicks();

    if (mode == EDITOR_MODE_RAISE || mode == EDITOR_MODE_REDUCE)
        for (int i = 0; i < VertexCounter; i++)
            modifyHeight(Vertices[i].x, Vertices[i].y);
    //if not raise or reduce, we need a content to set, so if there is no content, return
    else if (modeContent == -1)
        return;
    else if (mode == EDITOR_MODE_CUT)
    {
        for (int i = 0; i < VertexCounter; i++)
            modifyObject(Vertices[i].x, Vertices[i].y);
    }
    else if (mode == EDITOR_MODE_TEXTURE)
    {
        if (ChangeSection == 0)
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
                if (modeContent2 == 0xFD || modeContent2 == 0xFF)
                    map->vertex[VertexY*map->width+VertexX].rsuTexture = newContent;
                if (modeContent2 == 0xFE || modeContent2 == 0xFF)
                    map->vertex[VertexY*map->width+VertexX].usdTexture = newContent;
            }
            else
            {
                if (modeContent2 == 0xFD || modeContent2 == 0xFF)
                    map->vertex[VertexY*map->width+VertexX].rsuTexture = modeContent;
                if (modeContent2 == 0xFE || modeContent2 == 0xFF)
                    map->vertex[VertexY*map->width+VertexX].usdTexture = modeContent;
            }
        }
        else if (ChangeSection == 1)
            modifyTexture(Vertices[0].x, Vertices[0].y);
        else if (ChangeSection == 2)
        {
            for (int i = 0; i < 7; i++)
                modifyTexture(Vertices[i].x, Vertices[i].y);
        }
        else if (ChangeSection == 3)
        {
            for (int i = 0; i < 19; i++)
                modifyTexture(Vertices[i].x, Vertices[i].y);
        }
    }
    else if (mode == EDITOR_MODE_TREE)
    {
        for (int i = 0; i < VertexCounter; i++)
            modifyObject(Vertices[i].x, Vertices[i].y);
    }
    else if (mode == EDITOR_MODE_LANDSCAPE)
    {
        for (int i = 0; i < VertexCounter; i++)
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
    calculateVertices(tempVertices, VertexX, VertexY, 2);

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

void CMap::modifyTexture(int VertexX, int VertexY)
{
    //vertex count for the points
    int X, Y;

    bool even = false;
    if (VertexY%2 == 0)
        even = true;

    if (modeContent == TRIANGLE_TEXTURE_MEADOW_MIXED || modeContent == TRIANGLE_TEXTURE_MEADOW_MIXED_HARBOUR)
    {
        int newContent;

        newContent = rand()%3;
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
        if (modeContent2 == 0xFD || modeContent2 == 0xFF)
            map->vertex[VertexY*map->width+VertexX].rsuTexture = newContent;
        if (modeContent2 == 0xFE || modeContent2 == 0xFF)
            map->vertex[VertexY*map->width+VertexX].usdTexture = newContent;

        //update first vertex left upside
        X = VertexX - (even ? 1 : 0);   if (X < 0) X += map->width;
        Y = VertexY-1;                  if (Y < 0) Y += map->height;
        newContent = rand()%3;
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
        if (modeContent2 == 0xFD || modeContent2 == 0xFF)
            map->vertex[Y*map->width+X].rsuTexture = newContent;
        if (modeContent2 == 0xFE || modeContent2 == 0xFF)
            map->vertex[Y*map->width+X].usdTexture = newContent;
        //update second vertex right upside
        X = VertexX + (even ? 0 : 1);   if (X >= map->width) X -= map->width;
        Y = VertexY-1;                  if (Y < 0) Y += map->height;
        newContent = rand()%3;
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
        if (modeContent2 == 0xFD || modeContent2 == 0xFF)
            map->vertex[Y*map->width+X].rsuTexture = newContent;
        //update third point bottom left
        X = VertexX-1;                  if (X < 0) X += map->width;
        Y = VertexY;
        newContent = rand()%3;
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
        if (modeContent2 == 0xFE || modeContent2 == 0xFF)
            map->vertex[Y*map->width+X].usdTexture = newContent;
    }
    else
    {
        if (modeContent2 == 0xFD || modeContent2 == 0xFF)
            map->vertex[VertexY*map->width+VertexX].rsuTexture = modeContent;
        if (modeContent2 == 0xFE || modeContent2 == 0xFF)
            map->vertex[VertexY*map->width+VertexX].usdTexture = modeContent;

        //update first vertex left upside
        X = VertexX - (even ? 1 : 0);   if (X < 0) X += map->width;
        Y = VertexY-1;                  if (Y < 0) Y += map->height;
        if (modeContent2 == 0xFD || modeContent2 == 0xFF)
            map->vertex[Y*map->width+X].rsuTexture = modeContent;
        if (modeContent2 == 0xFE || modeContent2 == 0xFF)
            map->vertex[Y*map->width+X].usdTexture = modeContent;
        //update second vertex right upside
        X = VertexX + (even ? 0 : 1);   if (X >= map->width) X -= map->width;
        Y = VertexY-1;                  if (Y < 0) Y += map->height;
        if (modeContent2 == 0xFD || modeContent2 == 0xFF)
            map->vertex[Y*map->width+X].rsuTexture = modeContent;
        //update third point bottom left
        X = VertexX-1;                  if (X < 0) X += map->width;
        Y = VertexY;
        if (modeContent2 == 0xFE || modeContent2 == 0xFF)
            map->vertex[Y*map->width+X].usdTexture = modeContent;
    }
    //now we are finished, all six triangles have the new texture
}

void CMap::modifyObject(int VertexX, int VertexY)
{
    if (mode == EDITOR_MODE_CUT)
    {
        map->vertex[VertexY*map->width+VertexX].objectType = modeContent;
        map->vertex[VertexY*map->width+VertexX].objectInfo = modeContent;
        map->vertex[VertexY*map->width+VertexX].animal = modeContent;
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
        //now set up the buildings around the tree
        modifyBuild(VertexX, VertexY);
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
}

void CMap::modifyBuild(int VertexX, int VertexY)
{
    //at first save all vertices we need to modify
    struct vertexPoint tempVertices[19];
    calculateVertices(tempVertices, VertexX, VertexY, 2);

    if (mode == EDITOR_MODE_CUT)
    {
        ;
    }
    if (mode == EDITOR_MODE_RAISE || mode == EDITOR_MODE_REDUCE)
    {
        Uint8 building = 0x04;
        Uint8 height = map->vertex[VertexY*map->width+VertexX].h, temp;

        //at first let's take a look at the first section around the vertex
        //test the whole section
        //for (int i = 0; i < 7; i++)
        //{
        //    temp = map->vertex[tempVertices[i].y*map->width+tempVertices[i].x].z;
        //    if (height - temp >= 0x03)
        //        building = 0x04;
        //}
        //test the whole section
        for (int i = 0; i < 6; i++)
        {
            temp = map->vertex[tempVertices[i].y*map->width+tempVertices[i].x].h;
            //if ((height - temp <= 0x04 && height - temp >= 0x01) || temp - height >= 0x04)
            if (height - temp >= 0x04 || temp - height >= 0x04)
                building = 0x01;
        }

        //test vertex lower right
        temp = map->vertex[tempVertices[6].y*map->width+tempVertices[6].x].h;
        //if ( (height - temp <= 0x02 && height - temp >= 0x01) || temp - height >= 0x02 )
        if (height - temp >= 0x02 || temp - height >= 0x02 )
            building = 0x01;

        //now test the second section around the vertex
        if (building > 0x01)
        {
            //test the whole section
            for (int i = 7; i < 18; i++)
            {
                temp = map->vertex[tempVertices[i].y*map->width+tempVertices[i].x].h;
                //if ((height - temp <= 0x03 && height - temp >= 0x01) || temp - height >= 0x03)
                if (height - temp >= 0x03 || temp - height >= 0x03)
                    building = 0x02;
            }
        }

        map->vertex[VertexY*map->width+VertexX].build = building;
    }
    else if (mode == EDITOR_MODE_TREE)
    {
        //small houses and left up a flag
        Uint8 building;
        map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].build = 0x00;
        for (int i = 1; i <= 6; i++)
        {
            building = map->vertex[tempVertices[i].y*map->width+tempVertices[i].x].build;
            //Before setting tree was it possible to set a middle/great house AND NOT a mine?
            if (building%8 > 0x02 && building%8 != 0x05)
                map->vertex[tempVertices[i].y*map->width+tempVertices[i].x].build -= (building%8 - 0x02);
        }
        building = map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].build;
        if (building%8 > 0x01)
            map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].build -= (building%8 - 0x01);
    }
    else if (mode == EDITOR_MODE_LANDSCAPE)
    {
        //flags around the stone
        Uint8 building;
        map->vertex[tempVertices[0].y*map->width+tempVertices[0].x].build = 0x00;
        for (int i = 1; i <= 6; i++)
        {
            building = map->vertex[tempVertices[i].y*map->width+tempVertices[i].x].build;
            //Before setting granite was it possible to set a flag or anything bigger?
            if (building%8 > 0x01)
                map->vertex[tempVertices[i].y*map->width+tempVertices[i].x].build -= (building%8 - 0x01);
        }
    }
}

int CMap::getActiveVertices(int tempChangeSection)
{
    int total = 0;
    for (int i = tempChangeSection; i > 0; i--)
        total += i;
    return ( 6*total + 1);
}

void CMap::calculateVertices(struct vertexPoint tempVertices[], int VertexX, int VertexY, int tempChangeSection)
{
    bool even = false;
    if (VertexY%2 == 0)
        even = true;

    tempVertices[0].x = VertexX;
    tempVertices[0].y = VertexY;
    tempVertices[0].blit_x = MouseBlitX;
    tempVertices[0].blit_y = MouseBlitY;

    if (tempChangeSection > 0)
    {
        tempVertices[1].x = VertexX - (even ? 1 : 0);   if (tempVertices[1].x < 0) tempVertices[1].x += map->width;
        tempVertices[1].y = VertexY-1;                  if (tempVertices[1].y < 0) tempVertices[1].y += map->height;
        tempVertices[1].blit_x = correctMouseBlitX(map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].x, tempVertices[1].x, tempVertices[1].y);
        tempVertices[1].blit_y = correctMouseBlitY(map->vertex[tempVertices[1].y*map->width+tempVertices[1].x].y, tempVertices[1].x, tempVertices[1].y);
        tempVertices[2].x = VertexX + (even ? 0 : 1);   if (tempVertices[2].x >= map->width) tempVertices[2].x -= map->width;
        tempVertices[2].y = VertexY-1;                  if (tempVertices[2].y < 0) tempVertices[2].y += map->height;
        tempVertices[2].blit_x = correctMouseBlitX(map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].x, tempVertices[2].x, tempVertices[2].y);
        tempVertices[2].blit_y = correctMouseBlitY(map->vertex[tempVertices[2].y*map->width+tempVertices[2].x].y, tempVertices[2].x, tempVertices[2].y);
        tempVertices[3].x = VertexX-1;                  if (tempVertices[3].x < 0) tempVertices[3].x += map->width;
        tempVertices[3].y = VertexY;
        tempVertices[3].blit_x = correctMouseBlitX(map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].x, tempVertices[3].x, tempVertices[3].y);
        tempVertices[3].blit_y = correctMouseBlitY(map->vertex[tempVertices[3].y*map->width+tempVertices[3].x].y, tempVertices[3].x, tempVertices[3].y);
        tempVertices[4].x = VertexX+1;                  if (tempVertices[4].x >= map->width) tempVertices[4].x -= map->width;
        tempVertices[4].y = VertexY;
        tempVertices[4].blit_x = correctMouseBlitX(map->vertex[tempVertices[4].y*map->width+tempVertices[4].x].x, tempVertices[4].x, tempVertices[4].y);
        tempVertices[4].blit_y = correctMouseBlitY(map->vertex[tempVertices[4].y*map->width+tempVertices[4].x].y, tempVertices[4].x, tempVertices[4].y);
        tempVertices[5].x = VertexX - (even ? 1 : 0);   if (tempVertices[5].x < 0) tempVertices[5].x += map->width;
        tempVertices[5].y = VertexY+1;                  if (tempVertices[5].y >= map->height) tempVertices[5].y -= map->height;
        tempVertices[5].blit_x = correctMouseBlitX(map->vertex[tempVertices[5].y*map->width+tempVertices[5].x].x, tempVertices[5].x, tempVertices[5].y);
        tempVertices[5].blit_y = correctMouseBlitY(map->vertex[tempVertices[5].y*map->width+tempVertices[5].x].y, tempVertices[5].x, tempVertices[5].y);
        tempVertices[6].x = VertexX + (even ? 0 : 1);   if (tempVertices[6].x >= map->width) tempVertices[6].x -= map->width;
        tempVertices[6].y = VertexY+1;                  if (tempVertices[6].y >= map->height) tempVertices[6].y -= map->height;
        tempVertices[6].blit_x = correctMouseBlitX(map->vertex[tempVertices[6].y*map->width+tempVertices[6].x].x, tempVertices[6].x, tempVertices[6].y);
        tempVertices[6].blit_y = correctMouseBlitY(map->vertex[tempVertices[6].y*map->width+tempVertices[6].x].y, tempVertices[6].x, tempVertices[6].y);
    }
    if (tempChangeSection > 1)
    {
        tempVertices[7].x = VertexX - 1;                 if (tempVertices[7].x < 0) tempVertices[7].x += map->width;
        tempVertices[7].y = VertexY-2;                   if (tempVertices[7].y < 0) tempVertices[7].y += map->height;
        tempVertices[7].blit_x = correctMouseBlitX(map->vertex[tempVertices[7].y*map->width+tempVertices[7].x].x, tempVertices[7].x, tempVertices[7].y);
        tempVertices[7].blit_y = correctMouseBlitY(map->vertex[tempVertices[7].y*map->width+tempVertices[7].x].y, tempVertices[7].x, tempVertices[7].y);
        tempVertices[8].x = VertexX;
        tempVertices[8].y = VertexY-2;                   if (tempVertices[8].y < 0) tempVertices[8].y += map->height;
        tempVertices[8].blit_x = correctMouseBlitX(map->vertex[tempVertices[8].y*map->width+tempVertices[8].x].x, tempVertices[8].x, tempVertices[8].y);
        tempVertices[8].blit_y = correctMouseBlitY(map->vertex[tempVertices[8].y*map->width+tempVertices[8].x].y, tempVertices[8].x, tempVertices[8].y);
        tempVertices[9].x = VertexX + 1;                 if (tempVertices[9].x < 0) tempVertices[9].x += map->width;
        tempVertices[9].y = VertexY-2;                   if (tempVertices[9].y < 0) tempVertices[9].y += map->height;
        tempVertices[9].blit_x = correctMouseBlitX(map->vertex[tempVertices[9].y*map->width+tempVertices[9].x].x, tempVertices[9].x, tempVertices[9].y);
        tempVertices[9].blit_y = correctMouseBlitY(map->vertex[tempVertices[9].y*map->width+tempVertices[9].x].y, tempVertices[9].x, tempVertices[9].y);
        tempVertices[10].x = VertexX - (even ? 2 : 1);   if (tempVertices[10].x < 0) tempVertices[10].x += map->width;
        tempVertices[10].y = VertexY-1;                  if (tempVertices[10].y < 0) tempVertices[10].y += map->height;
        tempVertices[10].blit_x = correctMouseBlitX(map->vertex[tempVertices[10].y*map->width+tempVertices[10].x].x, tempVertices[10].x, tempVertices[10].y);
        tempVertices[10].blit_y = correctMouseBlitY(map->vertex[tempVertices[10].y*map->width+tempVertices[10].x].y, tempVertices[10].x, tempVertices[10].y);
        tempVertices[11].x = VertexX + (even ? 1 : 2);   if (tempVertices[11].x >= map->width) tempVertices[11].x -= map->width;
        tempVertices[11].y = VertexY-1;                  if (tempVertices[11].y < 0) tempVertices[11].y += map->height;
        tempVertices[11].blit_x = correctMouseBlitX(map->vertex[tempVertices[11].y*map->width+tempVertices[11].x].x, tempVertices[11].x, tempVertices[11].y);
        tempVertices[11].blit_y = correctMouseBlitY(map->vertex[tempVertices[11].y*map->width+tempVertices[11].x].y, tempVertices[11].x, tempVertices[11].y);
        tempVertices[12].x = VertexX-2;                  if (tempVertices[12].x < 0) tempVertices[12].x += map->width;
        tempVertices[12].y = VertexY;
        tempVertices[12].blit_x = correctMouseBlitX(map->vertex[tempVertices[12].y*map->width+tempVertices[12].x].x, tempVertices[12].x, tempVertices[12].y);
        tempVertices[12].blit_y = correctMouseBlitY(map->vertex[tempVertices[12].y*map->width+tempVertices[12].x].y, tempVertices[12].x, tempVertices[12].y);
        tempVertices[13].x = VertexX+2;                  if (tempVertices[13].x >= map->width) tempVertices[13].x -= map->width;
        tempVertices[13].y = VertexY;
        tempVertices[13].blit_x = correctMouseBlitX(map->vertex[tempVertices[13].y*map->width+tempVertices[13].x].x, tempVertices[13].x, tempVertices[13].y);
        tempVertices[13].blit_y = correctMouseBlitY(map->vertex[tempVertices[13].y*map->width+tempVertices[13].x].y, tempVertices[13].x, tempVertices[13].y);
        tempVertices[14].x = VertexX - (even ? 2 : 1);   if (tempVertices[14].x < 0) tempVertices[14].x += map->width;
        tempVertices[14].y = VertexY+1;                  if (tempVertices[14].y < 0) tempVertices[14].y += map->height;
        tempVertices[14].blit_x = correctMouseBlitX(map->vertex[tempVertices[14].y*map->width+tempVertices[14].x].x, tempVertices[14].x, tempVertices[14].y);
        tempVertices[14].blit_y = correctMouseBlitY(map->vertex[tempVertices[14].y*map->width+tempVertices[14].x].y, tempVertices[14].x, tempVertices[14].y);
        tempVertices[15].x = VertexX + (even ? 1 : 2);   if (tempVertices[15].x >= map->width) tempVertices[15].x -= map->width;
        tempVertices[15].y = VertexY+1;                  if (tempVertices[15].y < 0) tempVertices[15].y += map->height;
        tempVertices[15].blit_x = correctMouseBlitX(map->vertex[tempVertices[15].y*map->width+tempVertices[15].x].x, tempVertices[15].x, tempVertices[15].y);
        tempVertices[15].blit_y = correctMouseBlitY(map->vertex[tempVertices[15].y*map->width+tempVertices[15].x].y, tempVertices[15].x, tempVertices[15].y);
        tempVertices[16].x = VertexX - 1;                if (tempVertices[16].x < 0) tempVertices[16].x += map->width;
        tempVertices[16].y = VertexY+2;                  if (tempVertices[16].y < 0) tempVertices[16].y += map->height;
        tempVertices[16].blit_x = correctMouseBlitX(map->vertex[tempVertices[16].y*map->width+tempVertices[16].x].x, tempVertices[16].x, tempVertices[16].y);
        tempVertices[16].blit_y = correctMouseBlitY(map->vertex[tempVertices[16].y*map->width+tempVertices[16].x].y, tempVertices[16].x, tempVertices[16].y);
        tempVertices[17].x = VertexX;
        tempVertices[17].y = VertexY+2;                  if (tempVertices[17].y < 0) tempVertices[17].y += map->height;
        tempVertices[17].blit_x = correctMouseBlitX(map->vertex[tempVertices[17].y*map->width+tempVertices[17].x].x, tempVertices[17].x, tempVertices[17].y);
        tempVertices[17].blit_y = correctMouseBlitY(map->vertex[tempVertices[17].y*map->width+tempVertices[17].x].y, tempVertices[17].x, tempVertices[17].y);
        tempVertices[18].x = VertexX + 1;                if (tempVertices[18].x < 0) tempVertices[18].x += map->width;
        tempVertices[18].y = VertexY+2;                  if (tempVertices[18].y < 0) tempVertices[18].y += map->height;
        tempVertices[18].blit_x = correctMouseBlitX(map->vertex[tempVertices[18].y*map->width+tempVertices[18].x].x, tempVertices[18].x, tempVertices[18].y);
        tempVertices[18].blit_y = correctMouseBlitY(map->vertex[tempVertices[18].y*map->width+tempVertices[18].x].y, tempVertices[18].x, tempVertices[18].y);
    }
    if (tempChangeSection > 2)
    {
        ;//remember setting up setKeyboardData
    }
}

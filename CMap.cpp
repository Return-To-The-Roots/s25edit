#include "CMap.h"

CMap::CMap(char *filename)
{
    //following can be used to not blit map, but let map draw directly to display instead (see also render loop)
    //Surf_Map = global::s2->Surf_Display;
    Surf_Map = NULL;
    displayRect.x = 0;
    displayRect.y = 0;
    displayRect.w = global::s2->GameResolutionX;
    displayRect.h = global::s2->GameResolutionY;
    map = (bobMAP*)CFile::open_file(filename, WLD);
    CSurface::get_nodeVectors(map);
    needSurface = true;
    active = true;
    VertexX = 0;
    VertexY = 0;
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
    modify = false;
}

CMap::~CMap()
{
    SDL_FreeSurface(Surf_Map);
}

void CMap::setMouseData(SDL_MouseMotionEvent motion)
{
    //important for blitting the right field of the map
    //is right mouse button pressed?
    if ((motion.state&SDL_BUTTON(SDL_BUTTON_RIGHT))!=0)
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

    saveVertex(motion.x, motion.y, motion.state);
}

void CMap::setMouseData(SDL_MouseButtonEvent button)
{
    if (button.state == SDL_PRESSED)
    {
        #ifdef _EDITORMODE
        //find out if user clicked on one of the game menu pictures
        //TODO

        //touch vertex data
        if (button.button == SDL_BUTTON_LEFT)
            modify = true;
        #else
        //find out if user clicked on one of the game menu pictures
        if (button.button == SDL_BUTTON_LEFT && button.x >= (global::s2->GameResolutionX/2-74) && button.x <= (global::s2->GameResolutionX/2-37)
                                             && button.y >= (global::s2->GameResolutionY-37) && button.y <= (global::s2->GameResolutionY-4)
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
            mode = EDITOR_MODE_CUT;
        else if (key.keysym.sym == SDLK_KP_PLUS)
        {
            if ( getActiveVertices(ChangeSection+1) <= 7) //temproary set to seven, cause more is not implemented yet
            {
                ChangeSection++;
                VertexCounter = getActiveVertices(ChangeSection);
                actualizeVertices();
            }
        }
        else if (key.keysym.sym == SDLK_KP_MINUS)
        {
            if (ChangeSection > 0)
            {
                ChangeSection--;
                VertexCounter = getActiveVertices(ChangeSection);
                actualizeVertices();
            }
        }
    }
    else if (key.type == SDL_KEYUP)
    {
        if (key.keysym.sym == SDLK_LSHIFT || key.keysym.sym == SDLK_LCTRL)
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

    actualizeVertices();
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
        //commenting the following 4 lines out ("SDL_FreeSurface" to "return") can be used
        //to not blit map to temp surface, but let map draw directly to display instead (see also constructor and render loop)
        SDL_FreeSurface(Surf_Map);
        Surf_Map = NULL;
        if ( (Surf_Map = SDL_CreateRGBSurface(SDL_SWSURFACE, displayRect.w, displayRect.h, 32, 0, 0, 0, 0)) == NULL )
            return false;
        needSurface = false;
    }
    else
        //clear the surface before drawing new (in normal case not needed)
        SDL_FillRect( Surf_Map, NULL, SDL_MapRGB(Surf_Map->format,0,0,0) );

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
        case EDITOR_MODE_STONE:         symbol_index = CURSOR_SYMBOL_STONE;
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
}

void CMap::modifyHeight(int VertexX, int VertexY)
{
    //vertex count for the points
    int X, Y;
    struct point *tempP = &map->vertex[VertexY*map->width+VertexX];

    bool even = false;
    if (VertexY%2 == 0)
        even = true;

    if (mode == EDITOR_MODE_RAISE)
    {
        if (tempP->z >= 250) //maximum reached, 250 = 5*(0x3C - 0x0A), see open_wld for info
            return;
        tempP->y -= 5;
        tempP->z += 5;
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
}

int CMap::getActiveVertices(int ChangeSection)
{
    int total = 0;
    for (int i = ChangeSection; i > 0; i--)
        total += i;
    return ( 6*total + 1);
}

void CMap::actualizeVertices(void)
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
        Vertices[1].blit_x = correctMouseBlitX(map->vertex[Vertices[1].y*map->width+Vertices[1].x].x, Vertices[1].x, Vertices[1].y);
        Vertices[1].blit_y = correctMouseBlitY(map->vertex[Vertices[1].y*map->width+Vertices[1].x].y, Vertices[1].x, Vertices[1].y);
        Vertices[2].x = VertexX + (even ? 0 : 1);   if (Vertices[2].x >= map->width) Vertices[2].x -= map->width;
        Vertices[2].y = VertexY-1;                  if (Vertices[2].y < 0) Vertices[2].y += map->height;
        Vertices[2].blit_x = correctMouseBlitX(map->vertex[Vertices[2].y*map->width+Vertices[2].x].x, Vertices[2].x, Vertices[2].y);
        Vertices[2].blit_y = correctMouseBlitY(map->vertex[Vertices[2].y*map->width+Vertices[2].x].y, Vertices[2].x, Vertices[2].y);
        Vertices[3].x = VertexX-1;                  if (Vertices[3].x < 0) Vertices[3].x += map->width;
        Vertices[3].y = VertexY;
        Vertices[3].blit_x = correctMouseBlitX(map->vertex[Vertices[3].y*map->width+Vertices[3].x].x, Vertices[3].x, Vertices[3].y);
        Vertices[3].blit_y = correctMouseBlitY(map->vertex[Vertices[3].y*map->width+Vertices[3].x].y, Vertices[3].x, Vertices[3].y);
        Vertices[4].x = VertexX+1;                  if (Vertices[4].x >= map->width) Vertices[4].x -= map->width;
        Vertices[4].y = VertexY;
        Vertices[4].blit_x = correctMouseBlitX(map->vertex[Vertices[4].y*map->width+Vertices[4].x].x, Vertices[4].x, Vertices[4].y);
        Vertices[4].blit_y = correctMouseBlitY(map->vertex[Vertices[4].y*map->width+Vertices[4].x].y, Vertices[4].x, Vertices[4].y);
        Vertices[5].x = VertexX - (even ? 1 : 0);   if (Vertices[5].x < 0) Vertices[5].x += map->width;
        Vertices[5].y = VertexY+1;                  if (Vertices[5].y >= map->height) Vertices[5].y -= map->height;
        Vertices[5].blit_x = correctMouseBlitX(map->vertex[Vertices[5].y*map->width+Vertices[5].x].x, Vertices[5].x, Vertices[5].y);
        Vertices[5].blit_y = correctMouseBlitY(map->vertex[Vertices[5].y*map->width+Vertices[5].x].y, Vertices[5].x, Vertices[5].y);
        Vertices[6].x = VertexX + (even ? 0 : 1);   if (Vertices[6].x >= map->width) Vertices[6].x -= map->width;
        Vertices[6].y = VertexY+1;                  if (Vertices[6].y >= map->height) Vertices[6].y -= map->height;
        Vertices[6].blit_x = correctMouseBlitX(map->vertex[Vertices[6].y*map->width+Vertices[6].x].x, Vertices[6].x, Vertices[6].y);
        Vertices[6].blit_y = correctMouseBlitY(map->vertex[Vertices[6].y*map->width+Vertices[6].x].y, Vertices[6].x, Vertices[6].y);
    }
    if (ChangeSection > 1)
    {
        ;//remember setting up setKeyboardData
    }
    if (ChangeSection > 2)
    {
        ;//remember setting up setKeyboardData
    }
}

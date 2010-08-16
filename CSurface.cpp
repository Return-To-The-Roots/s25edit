#include "CSurface.h"

char CSurface::roundCount = 0;
Uint32 CSurface::roundTime = 0;

CSurface::CSurface()
{
}

bool CSurface::Draw(SDL_Surface *Surf_Dest, SDL_Surface *Surf_Src, int X, int Y)
{
    if (Surf_Dest == NULL || Surf_Src == NULL)
        return false;

    SDL_Rect DestR;

    DestR.x = X;
    DestR.y = Y;

    SDL_BlitSurface(Surf_Src, NULL, Surf_Dest, &DestR);

    return true;
}

bool CSurface::Draw(SDL_Surface *Surf_Dest, SDL_Surface *Surf_Src, int X, int Y, int angle)
{
    if (Surf_Dest == NULL || Surf_Src == NULL)
        return false;

    Uint16 px, py;

    switch (angle)
    {
        case    90: px = 0;
                    py = Surf_Src->h-1;
                    break;
        case   180: px = Surf_Src->w-1;
                    py = Surf_Src->h-1;
                    break;
        case   270: px = Surf_Src->w-1;
                    py = 0;
                    break;
        default:    return false;
    }

    sge_transform(Surf_Src, Surf_Dest, (float)angle, 1.0, 1.0, px, py, X, Y, SGE_TSAFE);

    return true;
}

bool CSurface::Draw(SDL_Surface *Surf_Dest, SDL_Surface *Surf_Src, int X, int Y, int X2, int Y2, int W, int H)
{
    if (Surf_Dest == NULL || Surf_Src == NULL)
        return false;

    SDL_Rect DestR;

    DestR.x = X;
    DestR.y = Y;

    SDL_Rect SrcR;

    SrcR.x = X2;
    SrcR.y = Y2;
    SrcR.w = W;
    SrcR.h = H;

    SDL_BlitSurface(Surf_Src, &SrcR, Surf_Dest, &DestR);

    return true;
}

//this is the example function from the sdl-documentation to draw pixels
void CSurface::DrawPixel_Color(SDL_Surface *screen, int x, int y, Uint32 color)
{
    if (SDL_MUSTLOCK(screen))
        SDL_LockSurface(screen);

    switch (screen->format->BytesPerPixel) {
        case 1: {
            Uint8 *bufp;

            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
            *bufp = color;
        }
        break;

        case 2: {
            Uint16 *bufp;

            bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
            *bufp = color;
        }
        break;

        case 3: {
            Uint8 *bufp;

            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x * 3;
            if(SDL_BYTEORDER == SDL_LIL_ENDIAN) {
                bufp[0] = color;
                bufp[1] = color >> 8;
                bufp[2] = color >> 16;
            } else {
                bufp[2] = color;
                bufp[1] = color >> 8;
                bufp[0] = color >> 16;
            }
        }
        break;

        case 4: {
            Uint32 *bufp;

            bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
            *bufp = color;
        }
        break;
    }

    if (SDL_MUSTLOCK(screen))
        SDL_UnlockSurface(screen);
}

//this is the example function from the sdl-documentation to draw pixels
void CSurface::DrawPixel_RGB(SDL_Surface *screen, int x, int y, Uint8 R, Uint8 G, Uint8 B)
{
    if (SDL_MUSTLOCK(screen))
        SDL_LockSurface(screen);

    Uint32 color = SDL_MapRGB(screen->format, R, G, B);

    switch (screen->format->BytesPerPixel) {
        case 1: {
            Uint8 *bufp;

            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
            *bufp = color;
        }
        break;

        case 2: {
            Uint16 *bufp;

            bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
            *bufp = color;
        }
        break;

        case 3: {
            Uint8 *bufp;

            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x * 3;
            if(SDL_BYTEORDER == SDL_LIL_ENDIAN) {
                bufp[0] = color;
                bufp[1] = color >> 8;
                bufp[2] = color >> 16;
            } else {
                bufp[2] = color;
                bufp[1] = color >> 8;
                bufp[0] = color >> 16;
            }
        }
        break;

        case 4: {
            Uint32 *bufp;

            bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
            *bufp = color;
        }
        break;
    }

    if (SDL_MUSTLOCK(screen))
        SDL_UnlockSurface(screen);
}

void CSurface::DrawPixel_RGBA(SDL_Surface *screen, int x, int y, Uint8 R, Uint8 G, Uint8 B, Uint8 A)
{
    if (SDL_MUSTLOCK(screen))
        SDL_LockSurface(screen);

    Uint32 color = SDL_MapRGBA(screen->format, R, G, B, A);

    switch (screen->format->BytesPerPixel) {
        case 1: {
            Uint8 *bufp;

            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
            *bufp = color;
        }
        break;

        case 2: {
            Uint16 *bufp;

            bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
            *bufp = color;
        }
        break;

        case 3: {
            Uint8 *bufp;

            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x * 3;
            if(SDL_BYTEORDER == SDL_LIL_ENDIAN) {
                bufp[0] = color;
                bufp[1] = color >> 8;
                bufp[2] = color >> 16;
            } else {
                bufp[2] = color;
                bufp[1] = color >> 8;
                bufp[0] = color >> 16;
            }
        }
        break;

        case 4: {
            Uint32 *bufp;

            bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
            *bufp = color;
        }
        break;
    }

    if (SDL_MUSTLOCK(screen))
        SDL_UnlockSurface(screen);
}

Uint32 CSurface::GetPixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    switch(bpp)
    {
        case 1:     return *p;

        case 2:     return *(Uint16 *)p;

        case 3:     if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
                        return p[0] << 16 | p[1] << 8 | p[2];
                    else
                        return p[0] | p[1] << 8 | p[2] << 16;

        case 4:     return *(Uint32 *)p;

        default:    return 0; /* shouldn’t happen, but avoids warnings */
    }
}

void CSurface::DrawTriangleField(SDL_Surface *display, struct DisplayRectangle displayRect, bobMAP *myMap)
{
    Uint16 width = myMap->width;
    Uint16 height = myMap->height;
    Uint8 type = myMap->type;
    struct point *vertex = myMap->vertex;
    struct point tempP1, tempP2, tempP3;

    Uint16 row_start;
    Uint16 row_end;
    Uint16 col_start;
    Uint16 col_end;
    bool view_outside_edges;

    //draw triangle field
    for (int k = 0; k < 4; k++)
    {
        //beware calling DrawTriangle for each triangle

        //IMPORTANT: integer values like +8 or -1 are for tolerance to beware of high triangles are not shown

        //at first call DrawTriangle for all triangles inside the map edges
        row_start = (displayRect.y > 2*TRIANGLE_HEIGHT ? displayRect.y : 2*TRIANGLE_HEIGHT)/TRIANGLE_HEIGHT - 2;
        row_end = (displayRect.y+displayRect.h)/TRIANGLE_HEIGHT + 8;
        col_start = (displayRect.x > TRIANGLE_WIDTH ? displayRect.x : TRIANGLE_WIDTH)/TRIANGLE_WIDTH - 1;
        col_end = (displayRect.x+displayRect.w)/TRIANGLE_WIDTH + 1;

        if (k > 0)
        {
            //now call DrawTriangle for all triangles outside the map edges
            view_outside_edges = false;

            if (k == 1 || k == 3)
            {
                //at first call DrawTriangle for all triangles up or down outside
                if (displayRect.y < 0)
                {
                    row_start = height-1 - (-displayRect.y/TRIANGLE_HEIGHT) - 1;
                    row_end = height-1;
                    view_outside_edges = true;
                }
                else if ( (displayRect.y + displayRect.h) > myMap->height_pixel )
                {
                    row_start = 0;
                    row_end = ((displayRect.y + displayRect.h) - myMap->height_pixel)/TRIANGLE_HEIGHT + 8;
                    view_outside_edges = true;
                }
            }

            if (k == 2 || k == 3)
            {
                //now call DrawTriangle for all triangles left or right outside
                if (displayRect.x < 0)
                {
                    col_start = width-1 - (-displayRect.x/TRIANGLE_WIDTH) - 1;
                    col_end = width-1;
                    view_outside_edges = true;
                }
                else if ( (displayRect.x + displayRect.w) > myMap->width_pixel )
                {
                    col_start = 0;
                    col_end = ((displayRect.x + displayRect.w) - myMap->width_pixel)/TRIANGLE_WIDTH + 1;
                    view_outside_edges = true;
                }
            }

            //if displayRect is not outside the map edges, there is nothing to do
            if (!view_outside_edges)
                continue;
        }

        for (Uint16 j = /*0*/ row_start; j < height-1 && j <= row_end; j++)
        {
            if (j%2 == 0)
            {
                //first RightSideUp
                tempP2.x = 0;
                tempP2.y = vertex[(j+1)*width+width-1].y;
                tempP2.z = vertex[(j+1)*width+width-1].z;
                tempP2.i = vertex[(j+1)*width+width-1].i;
                DrawTriangle(display, displayRect, myMap, type, vertex[j*width+0], tempP2, vertex[(j+1)*width+0]);
                for (Uint16 i = /*1*/ (col_start>0?col_start:1); i < width && i <= col_end; i++)
                {
                    //RightSideUp
                    DrawTriangle(display, displayRect, myMap, type, vertex[j*width+i], vertex[(j+1)*width+i-1], vertex[(j+1)*width+i]);
                    //UpSideDown
                    if (i < width)
                        DrawTriangle(display, displayRect, myMap, type, vertex[(j+1)*width+i-1], vertex[j*width+i-1], vertex[j*width+i]);
                }
                //last UpSideDown
                tempP3.x = vertex[j*width+width-1].x+TRIANGLE_WIDTH;
                tempP3.y = vertex[j*width+0].y;
                tempP3.z = vertex[j*width+0].z;
                tempP3.i = vertex[j*width+0].i;
                DrawTriangle(display, displayRect, myMap, type, vertex[(j+1)*width+width-1], vertex[j*width+width-1], tempP3);
            }
            else
            {
                for (Uint16 i = /*0*/ col_start; i < width-1 && i <= col_end; i++)
                {
                    //RightSideUp
                    DrawTriangle(display, displayRect, myMap, type, vertex[j*width+i], vertex[(j+1)*width+i], vertex[(j+1)*width+i+1]);
                    //UpSideDown
                    DrawTriangle(display, displayRect, myMap, type, vertex[(j+1)*width+i+1], vertex[j*width+i], vertex[j*width+i+1]);
                }
                //last RightSideUp
                tempP3.x = vertex[(j+1)*width+width-1].x+TRIANGLE_WIDTH;
                tempP3.y = vertex[(j+1)*width+0].y;
                tempP3.z = vertex[(j+1)*width+0].z;
                tempP3.i = vertex[(j+1)*width+0].i;
                DrawTriangle(display, displayRect, myMap, type, vertex[j*width+width-1], vertex[(j+1)*width+width-1], tempP3);
                //last UpSideDown
                tempP1.x = vertex[(j+1)*width+width-1].x+TRIANGLE_WIDTH;
                tempP1.y = vertex[(j+1)*width+0].y;
                tempP1.z = vertex[(j+1)*width+0].z;
                tempP1.i = vertex[(j+1)*width+0].i;
                tempP3.x = vertex[j*width+width-1].x+TRIANGLE_WIDTH;
                tempP3.y = vertex[j*width+0].y;
                tempP3.z = vertex[j*width+0].z;
                tempP3.i = vertex[j*width+0].i;
                DrawTriangle(display, displayRect, myMap, type, tempP1, vertex[j*width+width-1], tempP3);
            }
        }

        //draw last line
        for (Uint16 i = /*0*/ col_start; i < width-1 && i <= col_end; i++)
        {
            //RightSideUp
            tempP2.x = vertex[0*width+i].x;
            tempP2.y = height*TRIANGLE_HEIGHT + vertex[0*width+i].y;
            tempP2.z = vertex[0*width+i].z;
            tempP2.i = vertex[0*width+i].i;
            tempP3.x = vertex[0*width+i+1].x;
            tempP3.y = height*TRIANGLE_HEIGHT + vertex[0*width+i+1].y;
            tempP3.z = vertex[0*width+i+1].z;
            tempP3.i = vertex[0*width+i+1].i;
            DrawTriangle(display, displayRect, myMap, type, vertex[(height-1)*width+i], tempP2, tempP3);
            //UpSideDown
            tempP1.x = vertex[0*width+i+1].x;
            tempP1.y = height*TRIANGLE_HEIGHT + vertex[0*width+i+1].y;
            tempP1.z = vertex[0*width+i+1].z;
            tempP1.i = vertex[0*width+i+1].i;
            DrawTriangle(display, displayRect, myMap, type, tempP1, vertex[(height-1)*width+i], vertex[(height-1)*width+i+1]);
        }
    }

    //last RightSideUp
    tempP2.x = vertex[0*width+width-1].x;
    tempP2.y = height*TRIANGLE_HEIGHT + vertex[0*width+width-1].y;
    tempP2.z = vertex[0*width+width-1].z;
    tempP2.i = vertex[0*width+width-1].i;
    tempP3.x = vertex[0*width+width-1].x+TRIANGLE_WIDTH;
    tempP3.y = height*TRIANGLE_HEIGHT + vertex[0*width+0].y;
    tempP3.z = vertex[0*width+0].z;
    tempP3.i = vertex[0*width+0].i;
    DrawTriangle(display, displayRect, myMap, type, vertex[(height-1)*width+width-1], tempP2, tempP3);
    //last UpSideDown
    tempP1.x = vertex[0*width+width-1].x+TRIANGLE_WIDTH;
    tempP1.y = height*TRIANGLE_HEIGHT + vertex[0*width+0].y;
    tempP1.z = vertex[0*width+0].z;
    tempP1.i = vertex[0*width+0].i;
    tempP3.x = vertex[(height-1)*width+width-1].x+TRIANGLE_WIDTH;
    tempP3.y = vertex[(height-1)*width+0].y;
    tempP3.z = vertex[(height-1)*width+0].z;
    tempP3.i = vertex[(height-1)*width+0].i;
    DrawTriangle(display, displayRect, myMap, type, tempP1, vertex[(height-1)*width+width-1], tempP3);

    //at least increase the round counter
    if (SDL_GetTicks() - roundTime > 30)
    {
        roundTime = SDL_GetTicks();
        if (roundCount >= 7)
            roundCount = 0;
        else
            roundCount++;
    }
}

void CSurface::DrawTriangle(SDL_Surface *display, struct DisplayRectangle displayRect, bobMAP *myMap, Uint8 type, struct point P1, struct point P2, struct point P3)
{
    //prevent drawing triangles that are not shown
    if  ( ( (P1.x < displayRect.x && P2.x < displayRect.x && P3.x < displayRect.x ) || (P1.x > (displayRect.x+displayRect.w) && P2.x > (displayRect.x+displayRect.w) && P3.x > (displayRect.x+displayRect.w)) )
        || ( (P1.y < displayRect.y && P2.y < displayRect.y && P3.y < displayRect.y ) || (P1.y > (displayRect.y+displayRect.h) && P2.y > (displayRect.y+displayRect.h) && P3.y > (displayRect.y+displayRect.h)) )
        )
    {
        bool triangle_shown = false;

        if ( displayRect.x < 0 )
        {
            int outside_x = displayRect.x;
            int outside_w = -displayRect.x;
            if ( (((P1.x-myMap->width_pixel) >= (outside_x)) && ((P1.x-myMap->width_pixel) <= (outside_x+outside_w)))
              || (((P2.x-myMap->width_pixel) >= (outside_x)) && ((P2.x-myMap->width_pixel) <= (outside_x+outside_w)))
              || (((P3.x-myMap->width_pixel) >= (outside_x)) && ((P3.x-myMap->width_pixel) <= (outside_x+outside_w)))
               )
            {
                P1.x -= myMap->width_pixel;
                P2.x -= myMap->width_pixel;
                P3.x -= myMap->width_pixel;
                triangle_shown = true;
            }
        }
        else if ( (displayRect.x + displayRect.w) > (myMap->width_pixel) )
        {
            int outside_x = myMap->width_pixel;
            int outside_w = displayRect.x + displayRect.w - myMap->width_pixel;
            if ( (((P1.x+myMap->width_pixel) >= (outside_x)) && ((P1.x+myMap->width_pixel) <= (outside_x+outside_w)))
              || (((P2.x+myMap->width_pixel) >= (outside_x)) && ((P2.x+myMap->width_pixel) <= (outside_x+outside_w)))
              || (((P3.x+myMap->width_pixel) >= (outside_x)) && ((P3.x+myMap->width_pixel) <= (outside_x+outside_w)))
               )
            {
                P1.x += myMap->width_pixel;
                P2.x += myMap->width_pixel;
                P3.x += myMap->width_pixel;
                triangle_shown = true;
            }
        }

        if ( displayRect.y < 0 )
        {
            int outside_y = displayRect.y;
            int outside_h = -displayRect.y;
            if ( (((P1.y-myMap->height_pixel) >= (outside_y)) && ((P1.y-myMap->height_pixel) <= (outside_y+outside_h)))
              || (((P2.y-myMap->height_pixel) >= (outside_y)) && ((P2.y-myMap->height_pixel) <= (outside_y+outside_h)))
              || (((P3.y-myMap->height_pixel) >= (outside_y)) && ((P3.y-myMap->height_pixel) <= (outside_y+outside_h)))
               )
            {
                P1.y -= myMap->height_pixel;
                P2.y -= myMap->height_pixel;
                P3.y -= myMap->height_pixel;
                triangle_shown = true;
            }
        }
        else if ( (displayRect.y + displayRect.h) > (myMap->height_pixel) )
        {
            int outside_y = myMap->height_pixel;
            int outside_h = displayRect.y + displayRect.h - myMap->height_pixel;
            if ( (((P1.y+myMap->height_pixel) >= (outside_y)) && ((P1.y+myMap->height_pixel) <= (outside_y+outside_h)))
              || (((P2.y+myMap->height_pixel) >= (outside_y)) && ((P2.y+myMap->height_pixel) <= (outside_y+outside_h)))
              || (((P3.y+myMap->height_pixel) >= (outside_y)) && ((P3.y+myMap->height_pixel) <= (outside_y+outside_h)))
               )
            {
                P1.y += myMap->height_pixel;
                P2.y += myMap->height_pixel;
                P3.y += myMap->height_pixel;
                triangle_shown = true;
            }
        }

        if (!triangle_shown)
            return;
    }

    //find out the texture for the triangle
    unsigned char upperX, upperY, leftX, leftY, rightX, rightY;
    Uint8 texture, texture_raw;
    SDL_Surface *Surf_Tileset;

    switch (type)
    {
        case MAP_GREENLAND:     Surf_Tileset = global::bmpArray[TILESET_GREENLAND].surface;
                                break;
        case MAP_WASTELAND:     Surf_Tileset = global::bmpArray[TILESET_WASTELAND].surface;
                                break;
        case MAP_WINTERLAND:    Surf_Tileset = global::bmpArray[TILESET_WINTERLAND].surface;
                                break;
        default:                Surf_Tileset = global::bmpArray[TILESET_GREENLAND].surface;
                                break;
    }

    if (P1.y < P2.y)
        texture = P1.rsuTexture;
    else
        texture = P2.usdTexture;
    texture_raw = texture;
    if (texture_raw >= 0x40)
        //it's a harbour
        texture_raw -= 0x40;

    switch (texture_raw)
    {
        case TRIANGLE_TEXTURE_STEPPE_MEADOW1:   upperX = 17;
                                                upperY = 96;
                                                leftX = 0;
                                                leftY = 126;
                                                rightX = 35;
                                                rightY = 126;
                                                break;
        case TRIANGLE_TEXTURE_MINING1:          upperX = 17;
                                                upperY = 48;
                                                leftX = 0;
                                                leftY = 78;
                                                rightX = 35;
                                                rightY = 78;
                                                break;
        case TRIANGLE_TEXTURE_SNOW:             upperX = 17;
                                                upperY = 0;
                                                leftX = 0;
                                                leftY = 30;
                                                rightX = 35;
                                                rightY = 30;
                                                break;
        case TRIANGLE_TEXTURE_SWAMP:            upperX = 113;
                                                upperY = 0;
                                                leftX = 96;
                                                leftY = 30;
                                                rightX = 131;
                                                rightY = 30;
                                                break;
        case TRIANGLE_TEXTURE_STEPPE:           upperX = 65;
                                                upperY = 0;
                                                leftX = 48;
                                                leftY = 30;
                                                rightX = 83;
                                                rightY = 30;
                                                break;
        case TRIANGLE_TEXTURE_WATER:            upperX = 219;
                                                upperY = 51;//48;
                                                leftX = 195;//192;
                                                leftY = 75;
                                                rightX = 243;//246;
                                                rightY = 75;
                                                break;
        case TRIANGLE_TEXTURE_MEADOW1:          upperX = 65;
                                                upperY = 96;
                                                leftX = 48;
                                                leftY = 126;
                                                rightX = 83;
                                                rightY = 126;
                                                break;
        case TRIANGLE_TEXTURE_MEADOW2:          upperX = 113;
                                                upperY = 96;
                                                leftX = 96;
                                                leftY = 126;
                                                rightX = 131;
                                                rightY = 126;
                                                break;
        case TRIANGLE_TEXTURE_MEADOW3:          upperX = 161;
                                                upperY = 96;
                                                leftX = 144;
                                                leftY = 126;
                                                rightX = 179;
                                                rightY = 126;
                                                break;
        case TRIANGLE_TEXTURE_MINING2:          upperX = 65;
                                                upperY = 48;
                                                leftX = 48;
                                                leftY = 78;
                                                rightX = 83;
                                                rightY = 78;
                                                break;
        case TRIANGLE_TEXTURE_MINING3:          upperX = 113;
                                                upperY = 48;
                                                leftX = 96;
                                                leftY = 78;
                                                rightX = 131;
                                                rightY = 78;
                                                break;
        case TRIANGLE_TEXTURE_MINING4:          upperX = 161;
                                                upperY = 48;
                                                leftX = 144;
                                                leftY = 78;
                                                rightX = 179;
                                                rightY = 78;
                                                break;
        case TRIANGLE_TEXTURE_STEPPE_MEADOW2:   upperX = 17;
                                                upperY = 144;
                                                leftX = 0;
                                                leftY = 174;
                                                rightX = 35;
                                                rightY = 174;
                                                break;
        case TRIANGLE_TEXTURE_FLOWER:           upperX = 161;
                                                upperY = 0;
                                                leftX = 144;
                                                leftY = 30;
                                                rightX = 179;
                                                rightY = 30;
                                                break;
        case TRIANGLE_TEXTURE_LAVA:             upperX = 219;
                                                upperY = 105;//104;
                                                leftX = 193;//192;
                                                leftY = 131;
                                                rightX = 245;//246;
                                                rightY = 131;
                                                break;
        case TRIANGLE_TEXTURE_MINING_MEADOW:    upperX = 65;
                                                upperY = 144;
                                                leftX = 48;
                                                leftY = 174;
                                                rightX = 83;
                                                rightY = 174;
                                                break;
        default:                                //TRIANGLE_TEXTURE_FLOWER
                                                upperX = 161;
                                                upperY = 0;
                                                leftX = 144;
                                                leftY = 30;
                                                rightX = 179;
                                                rightY = 30;
                                                break;
    }


    //draw the triangle
    //do not shade water and lava
    if (texture == TRIANGLE_TEXTURE_WATER || texture == TRIANGLE_TEXTURE_LAVA)
        sge_TexturedTrigon(display, (Sint16)(P1.x-displayRect.x), (Sint16)(P1.y-displayRect.y), (Sint16)(P2.x-displayRect.x), (Sint16)(P2.y-displayRect.y), (Sint16)(P3.x-displayRect.x), (Sint16)(P3.y-displayRect.y), Surf_Tileset, upperX, upperY, leftX, leftY, rightX, rightY);
    else
        sge_FadedTexturedTrigon(display, (Sint16)(P1.x-displayRect.x), (Sint16)(P1.y-displayRect.y), (Sint16)(P2.x-displayRect.x), (Sint16)(P2.y-displayRect.y), (Sint16)(P3.x-displayRect.x), (Sint16)(P3.y-displayRect.y), Surf_Tileset, upperX, upperY, leftX, leftY, rightX, rightY, P1.i,P2.i,P3.i);


    //blit picture to vertex (trees, animals, buildings and so on) --> BUT ONLY AT P1 ON RIGHTSIDEUP-TRIANGLES

    //blit objects
    if (P1.y < P2.y)
    {
        int objIdx = 0;
        switch (P1.objectInfo)
        {
            //tree
            case    0xC4:   if (P1.objectType >= 0x30 && P1.objectType <= 0x37)
                            {
                                if (P1.objectType + roundCount > 0x37)
                                    objIdx = MAPPIC_TREE_PINE + (P1.objectType - 0x30) + (roundCount - 7);
                                else
                                    objIdx = MAPPIC_TREE_PINE + (P1.objectType - 0x30) + roundCount;

                            }
                            else if (P1.objectType >= 0x70 && P1.objectType <= 0x77)
                            {
                                if (P1.objectType + roundCount > 0x77)
                                    objIdx = MAPPIC_TREE_BIRCH + (P1.objectType - 0x70) + (roundCount - 7);
                                else
                                    objIdx = MAPPIC_TREE_BIRCH + (P1.objectType - 0x70) + roundCount;
                            }
                            else if (P1.objectType >= 0xB0 && P1.objectType <= 0xB7)
                            {
                                if (P1.objectType + roundCount > 0xB7)
                                    objIdx = MAPPIC_TREE_OAK + (P1.objectType - 0xB0) + (roundCount - 7);
                                else
                                    objIdx = MAPPIC_TREE_OAK + (P1.objectType - 0xB0) + roundCount;
                            }
                            else if (P1.objectType >= 0xF0 && P1.objectType <= 0xF7)
                            {
                                if (P1.objectType + roundCount > 0xF7)
                                    objIdx = MAPPIC_TREE_PALM1 + (P1.objectType - 0xF0) + (roundCount - 7);
                                else
                                    objIdx = MAPPIC_TREE_PALM1 + (P1.objectType - 0xF0) + roundCount;
                            }
                            break;
            //tree
            case    0xC5:   if (P1.objectType >= 0x30 && P1.objectType <= 0x37)
                            {
                                if (P1.objectType + roundCount > 0x37)
                                    objIdx = MAPPIC_TREE_PALM2 + (P1.objectType - 0x30) + (roundCount - 7);
                                else
                                    objIdx = MAPPIC_TREE_PALM2 + (P1.objectType - 0x30) + roundCount;

                            }
                            else if (P1.objectType >= 0x70 && P1.objectType <= 0x77)
                            {
                                if (P1.objectType + roundCount > 0x77)
                                    objIdx = MAPPIC_TREE_PINEAPPLE + (P1.objectType - 0x70) + (roundCount - 7);
                                else
                                    objIdx = MAPPIC_TREE_PINEAPPLE + (P1.objectType - 0x70) + roundCount;
                            }
                            else if (P1.objectType >= 0xB0 && P1.objectType <= 0xB7)
                            {
                                if (P1.objectType + roundCount > 0xB7)
                                    objIdx = MAPPIC_TREE_CYPRESS + (P1.objectType - 0xB0) + (roundCount - 7);
                                else
                                    objIdx = MAPPIC_TREE_CYPRESS + (P1.objectType - 0xB0) + roundCount;
                            }
                            else if (P1.objectType >= 0xF0 && P1.objectType <= 0xF7)
                            {
                                if (P1.objectType + roundCount > 0xF7)
                                    objIdx = MAPPIC_TREE_CHERRY + (P1.objectType - 0xF0) + (roundCount - 7);
                                else
                                    objIdx = MAPPIC_TREE_CHERRY + (P1.objectType - 0xF0) + roundCount;
                            }
                            break;
            //tree
            case    0xC6:   if (P1.objectType >= 0x30 && P1.objectType <= 0x37)
                            {
                                if (P1.objectType + roundCount > 0x37)
                                    objIdx = MAPPIC_TREE_FIR + (P1.objectType - 0x30) + (roundCount - 7);
                                else
                                    objIdx = MAPPIC_TREE_FIR + (P1.objectType - 0x30) + roundCount;

                            }
                            break;
            //landscape
            case    0xC8:   switch (P1.objectType)
                            {
                                case 0x00:  objIdx = MAPPIC_MUSHROOM1;
                                            break;
                                case 0x01:  objIdx = MAPPIC_MUSHROOM2;
                                            break;
                                case 0x02:  objIdx = MAPPIC_STONE1;
                                            break;
                                case 0x03:  objIdx = MAPPIC_STONE2;
                                            break;
                                case 0x04:  objIdx = MAPPIC_STONE3;
                                            break;
                                case 0x05:  objIdx = MAPPIC_TREE_TRUNK_DEAD;
                                            break;
                                case 0x06:  objIdx = MAPPIC_TREE_DEAD;
                                            break;
                                case 0x07:  objIdx = MAPPIC_BONE1;
                                            break;
                                case 0x08:  objIdx = MAPPIC_BONE2;
                                            break;

                                case 0x10:  objIdx = MAPPIC_BUSH2;
                                            break;
                                case 0x11:  objIdx = MAPPIC_BUSH3;
                                            break;
                                case 0x12:  objIdx = MAPPIC_BUSH4;
                                            break;

                                case 0x0A:  objIdx = MAPPIC_BUSH1;
                                            break;

                                case 0x0C:  objIdx = MAPPIC_CACTUS1;
                                            break;
                                case 0x0D:  objIdx = MAPPIC_CACTUS2;
                                            break;
                                case 0x0E:  objIdx = MAPPIC_SHRUB1;
                                            break;
                                case 0x0F:  objIdx = MAPPIC_SHRUB2;
                                            break;

                                case 0x13:  objIdx = MAPPIC_SHRUB3;
                                            break;
                                case 0x14:  objIdx = MAPPIC_SHRUB4;
                                            break;

                                case 0x18:  objIdx = MIS1BOBS_STONE1;
                                            break;
                                case 0x19:  objIdx = MIS1BOBS_STONE2;
                                            break;
                                case 0x1A:  objIdx = MIS1BOBS_STONE3;
                                            break;
                                case 0x1B:  objIdx = MIS1BOBS_STONE4;
                                            break;
                                case 0x1C:  objIdx = MIS1BOBS_STONE5;
                                            break;
                                case 0x1D:  objIdx = MIS1BOBS_STONE6;
                                            break;
                                case 0x1E:  objIdx = MIS1BOBS_STONE7;
                                            break;

                                case 0x22:  objIdx = MAPPIC_MUSHROOM3;
                                            break;

                                case 0x25:  objIdx = MAPPIC_PEBBLE1;
                                            break;
                                case 0x26:  objIdx = MAPPIC_PEBBLE2;
                                            break;
                                case 0x27:  objIdx = MAPPIC_PEBBLE3;
                                            break;
                                default:    break;
                            }
                            break;
            //stone
            case    0xCC:   objIdx = MAPPIC_GRANITE_1_1 + (P1.objectType - 0x01);
                            break;
            //stone
            case    0xCD:   objIdx = MAPPIC_GRANITE_2_1 + (P1.objectType - 0x01);
                            break;
            default:        break;
        }
        if (objIdx != 0)
            Draw(display, global::bmpArray[objIdx].surface, (int)(P1.x-displayRect.x-global::bmpArray[objIdx].nx), (int)(P1.y-displayRect.y-global::bmpArray[objIdx].ny));
    }

    //blit buildings
    if (global::s2->getMapObj()->getBuildHelp())
    {
        if (P1.y < P2.y)
        {
            switch (P1.build%8)
            {
                case 0x01:  Draw(display, global::bmpArray[MAPPIC_FLAG].surface, (int)(P1.x-displayRect.x-global::bmpArray[MAPPIC_FLAG].nx), (int)(P1.y-displayRect.y-global::bmpArray[MAPPIC_FLAG].ny));
                            break;
                case 0x02:  Draw(display, global::bmpArray[MAPPIC_HOUSE_SMALL].surface, (int)(P1.x-displayRect.x-global::bmpArray[MAPPIC_HOUSE_SMALL].nx), (int)(P1.y-displayRect.y-global::bmpArray[MAPPIC_HOUSE_SMALL].ny));
                            break;
                case 0x03:  Draw(display, global::bmpArray[MAPPIC_HOUSE_MIDDLE].surface, (int)(P1.x-displayRect.x-global::bmpArray[MAPPIC_HOUSE_MIDDLE].nx), (int)(P1.y-displayRect.y-global::bmpArray[MAPPIC_HOUSE_MIDDLE].ny));
                            break;
                case 0x04:  if (   texture == TRIANGLE_TEXTURE_STEPPE_MEADOW1_HARBOUR || texture == TRIANGLE_TEXTURE_MEADOW1_HARBOUR || texture == TRIANGLE_TEXTURE_MEADOW2_HARBOUR
                                || texture == TRIANGLE_TEXTURE_MEADOW3_HARBOUR || texture == TRIANGLE_TEXTURE_STEPPE_MEADOW2_HARBOUR || texture == TRIANGLE_TEXTURE_FLOWER_HARBOUR
                                || texture == TRIANGLE_TEXTURE_MINING_MEADOW_HARBOUR)
                                Draw(display, global::bmpArray[MAPPIC_HOUSE_HARBOUR].surface, (int)(P1.x-displayRect.x-global::bmpArray[MAPPIC_HOUSE_HARBOUR].nx), (int)(P1.y-displayRect.y-global::bmpArray[MAPPIC_HOUSE_HARBOUR].ny));
                            else
                                Draw(display, global::bmpArray[MAPPIC_HOUSE_BIG].surface, (int)(P1.x-displayRect.x-global::bmpArray[MAPPIC_HOUSE_BIG].nx), (int)(P1.y-displayRect.y-global::bmpArray[MAPPIC_HOUSE_BIG].ny));
                            break;
                case 0x05:  Draw(display, global::bmpArray[MAPPIC_MINE].surface, (int)(P1.x-displayRect.x-global::bmpArray[MAPPIC_MINE].nx), (int)(P1.y-displayRect.y-global::bmpArray[MAPPIC_MINE].ny));
                            break;
                default:    break;
            }
        }
    }
}

void CSurface::get_nodeVectors(bobMAP *myMap)
{
    //prepare triangle field
    int height = myMap->height;
    int width = myMap->width;
    struct point *vertex = myMap->vertex;
    struct point /*tempP1,*/ tempP2, tempP3;

    //get flat vectors
    for (int j = 0; j < height-1; j++)
    {
        if (j%2 == 0)
        {
            //vector of first triangle
            tempP2.x = 0;
            tempP2.y = vertex[(j+1)*width+width-1].y;
            tempP2.z = vertex[(j+1)*width+width-1].z;
            vertex[j*width+0].flatVector = get_flatVector(&vertex[j*width+0], &tempP2, &vertex[(j+1)*width+0]);

            for (int i = 1; i < width; i++)
                vertex[j*width+i].flatVector = get_flatVector(&vertex[j*width+i], &vertex[(j+1)*width+i-1], &vertex[(j+1)*width+i]);
        }
        else
        {
            for (int i = 0; i < width-1; i++)
                vertex[j*width+i].flatVector = get_flatVector(&vertex[j*width+i], &vertex[(j+1)*width+i], &vertex[(j+1)*width+i+1]);

            //vector of last triangle
            tempP3.x = vertex[(j+1)*width+width-1].x+TRIANGLE_WIDTH;
            tempP3.y = vertex[(j+1)*width+0].y;
            tempP3.z = vertex[(j+1)*width+0].z;
            vertex[j*width+width-1].flatVector = get_flatVector(&vertex[j*width+width-1], &vertex[(j+1)*width+width-1], &tempP3);
        }
    }
    //flat vectors of last line
    for (int i = 0; i < width-1; i++)
    {
        tempP2.x = vertex[0*width+i].x;
        tempP2.y = height*TRIANGLE_HEIGHT + vertex[0*width+i].y;
        tempP2.z = vertex[0*width+i].z;
        tempP3.x = vertex[0*width+i+1].x;
        tempP3.y = height*TRIANGLE_HEIGHT + vertex[0*width+i+1].y;
        tempP3.z = vertex[0*width+i+1].z;
        vertex[(height-1)*width+i].flatVector = get_flatVector(&vertex[(height-1)*width+i], &tempP2, &tempP3);
    }
    //vector of last Triangle
    tempP2.x = vertex[0*width+width-1].x;
    tempP2.y = height*TRIANGLE_HEIGHT + vertex[0*width+width-1].y;
    tempP2.z = vertex[0*width+width-1].z;
    tempP3.x = vertex[0*width+width-1].x+TRIANGLE_WIDTH;
    tempP3.y = height*TRIANGLE_HEIGHT + vertex[0*width+0].y;
    tempP3.z = vertex[0*width+0].z;
    vertex[(height-1)*width+width-1].flatVector = get_flatVector(&vertex[(height-1)*width+width-1], &tempP2, &tempP3);

    //now get the vector at each node and save it to vertex[j*width+i].normVector
    //temporary index
    int index, index2;
    for (int j = 0; j < height; j++)
    {
        if (j%2 == 0)
        {
            for (int i = 0; i < width; i++)
            {
                index = (i-1 < 0 ? width-1 : i-1);
                if (j == 0) //first line
                    vertex[j*width+i].normVector = get_nodeVector(vertex[(height-1)*width+index].flatVector, vertex[(height-1)*width+i].flatVector, vertex[j*width+i].flatVector);
                else
                    vertex[j*width+i].normVector = get_nodeVector(vertex[(j-1)*width+index].flatVector, vertex[(j-1)*width+i].flatVector, vertex[j*width+i].flatVector);
                vertex[j*width+i].i = get_LightIntensity(vertex[j*width+i].normVector);
            }
        }
        else
        {
            for (int i = 0; i < width; i++)
            {
                if (i-1 < 0)
                    index = width-1;
                else
                    index = i-1;

                if (i+1 >= width)
                    index2 = 0;
                else
                    index2 = i+1;

                vertex[j*width+i].normVector = get_nodeVector(vertex[(j-1)*width+i].flatVector, vertex[(j-1)*width+index2].flatVector, vertex[j*width+i].flatVector);
                vertex[j*width+i].i = get_LightIntensity(vertex[j*width+i].normVector);
            }
        }
    }
}

Sint32 CSurface::get_LightIntensity(struct vector node)
{
    //we calculate the light intensity right now
    float I, Ip = 1.1, kd = 1, light_const = 1.0;
    struct vector L = { -10, 5, 0.5 };
    L = get_normVector(L);
    I = Ip*kd*(node.x*L.x+node.y*L.y+node.z*L.z)+light_const;
    return (Sint32)(I*pow(2, 16));
}

struct vector CSurface::get_nodeVector(struct vector v1, struct vector v2, struct vector v3)
{
    struct vector node;
    //deviding through 6 is not necessary cause normal vector would be the same
    node.x = v1.x + v2.x + v3.x;
    node.y = v1.y + v2.y + v3.y;
    node.z = v1.z + v2.z + v3.z;
    node = get_normVector(node);
    return node;
}

struct vector CSurface::get_normVector(struct vector v)
{
    struct vector normal;
    float length = sqrt( pow(v.x, 2) + pow(v.y, 2) + pow(v.z, 2) );
    //in case vector length equals 0 (should not happen)
    if ( length == 0)
    {
        normal.x = 0;
        normal.y = 0;
        normal.z = 1;
    }
    else
    {
        normal = v;
        normal.x /= length;
        normal.y /= length;
        normal.z /= length;
    }

    return normal;
}

struct vector CSurface::get_flatVector(struct point *P1, struct point *P2, struct point *P3)
{
    //vector components
    float vax, vay, vaz, vbx, vby, vbz;
    //cross product
    struct vector cross;

    vax = P1->x-P2->x;
    vay = P1->y-P2->y;
    vaz = P1->z-P2->z;
    vbx = P3->x-P2->x;
    vby = P3->y-P2->y;
    vbz = P3->z-P2->z;

    cross.x = (vay*vbz-vaz*vby);
    cross.y = (vaz*vbx-vax*vbz);
    cross.z = (vax*vby-vay*vbx);
    //normalize
    cross = get_normVector(cross);

    return cross;
}

void CSurface::update_shading(bobMAP *myMap, int VertexX, int VertexY)
{
    //vertex count for the points
    int X, Y;

    bool even = false;
    if (VertexY%2 == 0)
        even = true;

    update_flatVectors(myMap, VertexX, VertexY);
    update_nodeVector(myMap, VertexX, VertexY);

    //now update all nodeVectors around VertexX and VertexY
    //update first vertex left upside
    X = VertexX - (even ? 1 : 0);   if (X < 0) X += myMap->width;
    Y = VertexY-1;                  if (Y < 0) Y += myMap->height;
    update_nodeVector(myMap, X, Y);
    //update second vertex right upside
    X = VertexX + (even ? 0 : 1);   if (X >= myMap->width) X -= myMap->width;
    Y = VertexY-1;                  if (Y < 0) Y += myMap->height;
    update_nodeVector(myMap, X, Y);
    //update third point bottom left
    X = VertexX-1;                  if (X < 0) X += myMap->width;
    Y = VertexY;
    update_nodeVector(myMap, X, Y);
    //update fourth point bottom right
    X = VertexX+1;                  if (X >= myMap->width) X -= myMap->width;
    Y = VertexY;
    update_nodeVector(myMap, X, Y);
    //update fifth point down left
    X = VertexX - (even ? 1 : 0);   if (X < 0) X += myMap->width;
    Y = VertexY+1;                  if (Y >= myMap->height) Y -= myMap->height;
    update_nodeVector(myMap, X, Y);
    //update sixth point down right
    X = VertexX + (even ? 0 : 1);   if (X >= myMap->width) X -= myMap->width;
    Y = VertexY+1;                  if (Y >= myMap->height) Y -= myMap->height;
    update_nodeVector(myMap, X, Y);
}

void CSurface::update_flatVectors(bobMAP *myMap, int VertexX, int VertexY)
{
    //point structures for the triangles, Pmiddle is the point in the middle of the hexagon we will update
    struct point *P1, *P2, *P3, *Pmiddle;
    //vertex count for the points
    int P1x, P1y, P2x, P2y, P3x, P3y;

    bool even = false;
    if (VertexY%2 == 0)
        even = true;

    Pmiddle = &myMap->vertex[VertexY*myMap->width+VertexX];

    //update first triangle left upside
    P1x = VertexX - (even ? 1 : 0);   if (P1x < 0) P1x += myMap->width;
    P1y = VertexY-1;                  if (P1y < 0) P1y += myMap->height;
    P1 = &myMap->vertex[P1y*myMap->width+P1x];
    P2x = VertexX-1;                  if (P2x < 0) P2x += myMap->width;
    P2y = VertexY;
    P2 = &myMap->vertex[P2y*myMap->width+P2x];
    P3 = Pmiddle;
    P1->flatVector = get_flatVector(P1, P2, P3);

    //update second triangle right upside
    P1x = VertexX + (even ? 0 : 1);   if (P1x >= myMap->width) P1x -= myMap->width;
    P1y = VertexY-1;                  if (P1y < 0) P1y += myMap->height;
    P1 = &myMap->vertex[P1y*myMap->width+P1x];
    P2 = Pmiddle;
    P3x = VertexX+1;                  if (P3x >= myMap->width) P3x -= myMap->width;
    P3y = VertexY;
    P3 = &myMap->vertex[P3y*myMap->width+P3x];
    P1->flatVector = get_flatVector(P1, P2, P3);

    //update third triangle down middle
    P1 = Pmiddle;
    P2x = VertexX - (even ? 1 : 0);   if (P2x < 0) P2x += myMap->width;
    P2y = VertexY+1;                  if (P2y >= myMap->height) P2y -= myMap->height;
    P2 = &myMap->vertex[P2y*myMap->width+P2x];
    P3x = VertexX + (even ? 0 : 1);   if (P3x >= myMap->width) P3x -= myMap->width;
    P3y = VertexY+1;                  if (P3y >= myMap->height) P3y -= myMap->height;
    P3 = &myMap->vertex[P3y*myMap->width+P3x];
    P1->flatVector = get_flatVector(P1, P2, P3);
}

void CSurface::update_nodeVector(bobMAP *myMap, int VertexX, int VertexY)
{
    int j = VertexY;
    int i = VertexX;
    int width = myMap->width;
    int height = myMap->height;
    struct point *vertex = myMap->vertex;

    //temporary index
    int index, index2;

    if (j%2 == 0)
    {
        index = (i-1 < 0 ? width-1 : i-1);
        if (j == 0) //first line
            vertex[j*width+i].normVector = get_nodeVector(vertex[(height-1)*width+index].flatVector, vertex[(height-1)*width+i].flatVector, vertex[j*width+i].flatVector);
        else
            vertex[j*width+i].normVector = get_nodeVector(vertex[(j-1)*width+index].flatVector, vertex[(j-1)*width+i].flatVector, vertex[j*width+i].flatVector);
        vertex[j*width+i].i = get_LightIntensity(vertex[j*width+i].normVector);
    }
    else
    {
        if (i-1 < 0)
            index = width-1;
        else
            index = i-1;

        if (i+1 >= width)
            index2 = 0;
        else
            index2 = i+1;

        vertex[j*width+i].normVector = get_nodeVector(vertex[(j-1)*width+i].flatVector, vertex[(j-1)*width+index2].flatVector, vertex[j*width+i].flatVector);
        vertex[j*width+i].i = get_LightIntensity(vertex[j*width+i].normVector);
    }
}

float CSurface::absf(float a)
{
    if ( a >= 0 )
        return a;
    else
        return a*(-1);
}


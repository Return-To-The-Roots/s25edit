// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CSurface.h"
#include "Rect.h"
#include "globals.h"
#include <algorithm>
#include <cmath>

// drawTextures removed — terrain is pre-computed in GL

bool CSurface::Draw(SDL_Surface* Surf_Dest, SDL_Surface* Surf_Src, int X, int Y)
{
    if(!Surf_Dest || !Surf_Src)
        return false;

    SDL_Rect DestR;

    DestR.x = X;
    DestR.y = Y;

    // Ensure source pixels get alpha=255 when blitting to an RGBA surface,
    // so only colorkey-skipped pixels remain transparent.
    bool destHasAlpha = (Surf_Dest->format->Amask != 0);
    SDL_BlendMode oldBlend = SDL_BLENDMODE_NONE;
    Uint8 oldMod = 255;
    if(destHasAlpha)
    {
        SDL_GetSurfaceBlendMode(Surf_Src, &oldBlend);
        SDL_GetSurfaceAlphaMod(Surf_Src, &oldMod);
        SDL_SetSurfaceBlendMode(Surf_Src, SDL_BLENDMODE_BLEND);
        SDL_SetSurfaceAlphaMod(Surf_Src, 255);
    }

    SDL_BlitSurface(Surf_Src, nullptr, Surf_Dest, &DestR);

    if(destHasAlpha)
    {
        SDL_SetSurfaceBlendMode(Surf_Src, oldBlend);
        SDL_SetSurfaceAlphaMod(Surf_Src, oldMod);
    }

    return true;
}

bool CSurface::Draw(SDL_Surface* Surf_Dest, SdlSurface& Surf_Src, int X, int Y)
{
    return Draw(Surf_Dest, Surf_Src.get(), X, Y);
}

bool CSurface::Draw(SdlSurface& Surf_Dest, SdlSurface& Surf_Src, Position pos)
{
    return Draw(Surf_Dest.get(), Surf_Src.get(), pos.x, pos.y);
}

bool CSurface::Draw(SdlSurface& Surf_Dest, SDL_Surface* Surf_Src, int X, int Y)
{
    return Draw(Surf_Dest.get(), Surf_Src, X, Y);
}

SdlSurface CSurface::ConvertToRgba(SDL_Surface* src)
{
    if(!src)
        return nullptr;
    if(src->format->BitsPerPixel == 32)
        return SdlSurface(SDL_ConvertSurface(src, src->format, 0));
    if(src->format->BitsPerPixel != 8 || !src->format->palette)
        return nullptr;

    SDL_Surface* dst = SDL_CreateRGBSurface(0, src->w, src->h, 32, 0xFF0000, 0xFF00, 0xFF, 0xFF000000);
    if(!dst)
        return nullptr;

    Uint32 ck;
    const bool hasCK = (SDL_GetColorKey(src, &ck) == 0);
    const Uint8 ckIdx = (hasCK ? static_cast<Uint8>(ck & 0xFF) : 0xFF);

    SDL_LockSurface(src);
    SDL_LockSurface(dst);
    for(int y = 0; y < src->h; y++)
    {
        const Uint8* srcRow = (const Uint8*)src->pixels + y * src->pitch;
        Uint32* dstRow = (Uint32*)((Uint8*)dst->pixels + y * dst->pitch);
        for(int x = 0; x < src->w; x++)
        {
            const Uint8 idx = srcRow[x];
            const SDL_Color& c = src->format->palette->colors[idx];
            const Uint8 a = (hasCK && idx == ckIdx) ? 0 : 0xFF;
            dstRow[x] = (Uint32(a) << 24) | (Uint32(c.r) << 16) | (Uint32(c.g) << 8) | Uint32(c.b);
        }
    }
    SDL_UnlockSurface(dst);
    SDL_UnlockSurface(src);

    return SdlSurface(dst);
}

bool CSurface::Draw(SDL_Surface* Surf_Dest, SDL_Surface* Surf_Src, int X, int Y, int angle)
{
    if(!Surf_Dest || !Surf_Src)
        return false;

    if(angle != 90 && angle != 180 && angle != 270)
        return false;

    // Simple software rotation for 90/180/270 degrees
    SDL_Surface* rotated =
      SDL_CreateRGBSurface(0, Surf_Src->h, Surf_Src->w, Surf_Src->format->BitsPerPixel, Surf_Src->format->Rmask,
                           Surf_Src->format->Gmask, Surf_Src->format->Bmask, Surf_Src->format->Amask);
    if(!rotated)
        return false;
    // Copy palette for 8-bit surfaces
    if(Surf_Src->format->palette && rotated->format->palette)
        SDL_SetPaletteColors(rotated->format->palette, Surf_Src->format->palette->colors, 0, 256);
    SDL_LockSurface(Surf_Src);
    SDL_LockSurface(rotated);
    int srcW = Surf_Src->w, srcH = Surf_Src->h, bpp = Surf_Src->format->BytesPerPixel;
    for(int sy = 0; sy < srcH; sy++)
    {
        for(int sx = 0; sx < srcW; sx++)
        {
            int dx, dy;
            switch(angle)
            {
                case 90:
                    dx = srcH - 1 - sy;
                    dy = sx;
                    break;
                case 180:
                    dx = srcW - 1 - sx;
                    dy = srcH - 1 - sy;
                    break;
                case 270:
                    dx = sy;
                    dy = srcW - 1 - sx;
                    break;
                default:
                    dx = sx;
                    dy = sy;
                    break;
            }
            memcpy((Uint8*)rotated->pixels + dy * rotated->pitch + dx * bpp,
                   (Uint8*)Surf_Src->pixels + sy * Surf_Src->pitch + sx * bpp, bpp);
        }
    }
    SDL_UnlockSurface(rotated);
    SDL_UnlockSurface(Surf_Src);
    SDL_Rect dst = {(Sint16)X, (Sint16)Y, 0, 0};
    SDL_BlitSurface(rotated, nullptr, Surf_Dest, &dst);
    SDL_FreeSurface(rotated);

    return true;
}

bool CSurface::Draw(SdlSurface& Surf_Dest, SdlSurface& Surf_Src, int X, int Y, int angle)
{
    return Draw(Surf_Dest.get(), Surf_Src.get(), X, Y, angle);
}

bool CSurface::Draw(SDL_Surface* Surf_Dest, SDL_Surface* Surf_Src, int X, int Y, int X2, int Y2, int W, int H)
{
    if(!Surf_Dest || !Surf_Src)
        return false;

    SDL_Rect DestR;

    DestR.x = X;
    DestR.y = Y;

    SDL_Rect SrcR;

    SrcR.x = X2;
    SrcR.y = Y2;
    SrcR.w = W;
    SrcR.h = H;

    bool destHasAlpha = (Surf_Dest->format->Amask != 0);
    SDL_BlendMode oldBlend = SDL_BLENDMODE_NONE;
    Uint8 oldMod = 255;
    if(destHasAlpha)
    {
        SDL_GetSurfaceBlendMode(Surf_Src, &oldBlend);
        SDL_GetSurfaceAlphaMod(Surf_Src, &oldMod);
        SDL_SetSurfaceBlendMode(Surf_Src, SDL_BLENDMODE_BLEND);
        SDL_SetSurfaceAlphaMod(Surf_Src, 255);
    }

    SDL_BlitSurface(Surf_Src, &SrcR, Surf_Dest, &DestR);

    if(destHasAlpha)
    {
        SDL_SetSurfaceBlendMode(Surf_Src, oldBlend);
        SDL_SetSurfaceAlphaMod(Surf_Src, oldMod);
    }

    return true;
}

bool CSurface::Draw(SDL_Surface* Surf_Dest, SdlSurface& Surf_Src, int X, int Y, int X2, int Y2, int W, int H)
{
    return Draw(Surf_Dest, Surf_Src.get(), X, Y, X2, Y2, W, H);
}

// this is the example function from the SDL-documentation to draw pixels
void CSurface::DrawPixel_Color(SDL_Surface* screen, int x, int y, Uint32 color)
{
    int bpp = screen->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8* p = (Uint8*)screen->pixels + y * screen->pitch + x * bpp;

    if(SDL_MUSTLOCK(screen))
        SDL_LockSurface(screen);

    switch(bpp)
    {
        case 1: *p = color; break;

        case 2: *(Uint16*)p = color; break;

        case 3:
            if((SDL_BYTEORDER) == SDL_LIL_ENDIAN)
            {
                p[0] = color;
                p[1] = color >> 8;
                p[2] = color >> 16;
            } else
            {
                p[2] = color;
                p[1] = color >> 8;
                p[0] = color >> 16;
            }
            break;

        case 4: *(Uint32*)p = color; break;
    }

    if(SDL_MUSTLOCK(screen))
        SDL_UnlockSurface(screen);
}

// this is the example function from the sdl-documentation to draw pixels
void CSurface::DrawPixel_RGB(SDL_Surface* screen, int x, int y, Uint8 R, Uint8 G, Uint8 B)
{
    DrawPixel_Color(screen, x, y, SDL_MapRGB(screen->format, R, G, B));
}

void CSurface::DrawPixel_RGBA(SDL_Surface* screen, int x, int y, Uint8 R, Uint8 G, Uint8 B, Uint8 A)
{
    DrawPixel_Color(screen, x, y, SDL_MapRGBA(screen->format, R, G, B, A));
}

Uint32 CSurface::GetPixel(SDL_Surface* surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
    switch(bpp)
    {
        case 1: return *p;

        case 2: return *(Uint16*)p;

        case 3:
            if((SDL_BYTEORDER) == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;

        case 4: return *(Uint32*)p; //-V206

        default: return 0; /* shouldn't happen, but avoids warnings */
    }
}

void CSurface::get_nodeVectors(bobMAP& myMap)
{
    // prepare triangle field
    int height = myMap.height;
    int width = myMap.width;
    IntVector tempP2, tempP3;

    // get flat vectors
    for(int j = 0; j < height - 1; j++)
    {
        if(j % 2 == 0)
        {
            // vector of first triangle
            tempP2.x = 0;
            tempP2.y = myMap.getVertex(width - 1, j + 1).y;
            tempP2.z = myMap.getVertex(width - 1, j + 1).z;
            myMap.getVertex(0, j).flatVector = get_flatVector(myMap.getVertex(0, j), tempP2, myMap.getVertex(0, j + 1));

            for(int i = 1; i < width; i++)
                myMap.getVertex(i, j).flatVector =
                  get_flatVector(myMap.getVertex(i, j), myMap.getVertex(i - 1, j + 1), myMap.getVertex(i, j + 1));
        } else
        {
            for(int i = 0; i < width - 1; i++)
                myMap.getVertex(i, j).flatVector =
                  get_flatVector(myMap.getVertex(i, j), myMap.getVertex(i, j + 1), myMap.getVertex(i + 1, j + 1));

            // vector of last triangle
            tempP3.x = myMap.getVertex(width - 1, j + 1).x + triangleWidth;
            tempP3.y = myMap.getVertex(0, j + 1).y;
            tempP3.z = myMap.getVertex(0, j + 1).z;
            myMap.getVertex(width - 1, j).flatVector =
              get_flatVector(myMap.getVertex(width - 1, j), myMap.getVertex(width - 1, j + 1), tempP3);
        }
    }
    // flat vectors of last line
    for(int i = 0; i < width - 1; i++)
    {
        tempP2 = myMap.getVertex(i, 0);
        tempP2.y += height * triangleHeight;
        tempP3 = myMap.getVertex(i + 1, 0);
        tempP3.y += height * triangleHeight;
        myMap.getVertex(i, height - 1).flatVector = get_flatVector(myMap.getVertex(i, height - 1), tempP2, tempP3);
    }
    // vector of last Triangle
    tempP2 = myMap.getVertex(width - 1, 0);
    tempP2.y += height * triangleHeight;
    tempP3.x = myMap.getVertex(width - 1, 0).x + triangleWidth;
    tempP3.y = height * triangleHeight + myMap.getVertex(0, 0).y;
    tempP3.z = myMap.getVertex(0, 0).z;
    myMap.getVertex(width - 1, height - 1).flatVector =
      get_flatVector(myMap.getVertex(width - 1, height - 1), tempP2, tempP3);

    // now get the vector at each node and save it to myMap.getVertex(j*width+i, 0).normVector
    for(int j = 0; j < height; j++)
    {
        if(j % 2 == 0)
        {
            for(int i = 0; i < width; i++)
            {
                MapNode& curVertex = myMap.getVertex(i, j);
                int iM1 = (i == 0 ? width - 1 : i - 1);
                if(j == 0) // first line
                    curVertex.normVector =
                      get_nodeVector(myMap.getVertex(iM1, height - 1).flatVector,
                                     myMap.getVertex(i, height - 1).flatVector, curVertex.flatVector);
                else
                    curVertex.normVector = get_nodeVector(myMap.getVertex(iM1, j - 1).flatVector,
                                                          myMap.getVertex(i, j - 1).flatVector, curVertex.flatVector);
                curVertex.i = get_LightIntensity(curVertex.normVector);
            }
        } else
        {
            for(int i = 0; i < width; i++)
            {
                MapNode& curVertex = myMap.getVertex(i, j);
                int iP1 = (i + 1 == width ? 0 : i + 1);

                curVertex.normVector = get_nodeVector(myMap.getVertex(i, j - 1).flatVector,
                                                      myMap.getVertex(iP1, j - 1).flatVector, curVertex.flatVector);
                curVertex.i = get_LightIntensity(curVertex.normVector);
            }
        }
    }
}

Sint32 CSurface::get_LightIntensity(const vector& node)
{
    // we calculate the light intensity right now
    float I, Ip = 1.1f, kd = 1, light_const = 1.0f;
    vector L = {-10, 5, 0.5f};
    L = get_normVector(L);
    I = Ip * kd * (node.x * L.x + node.y * L.y + node.z * L.z) + light_const;
    return (Sint32)(I * pow(2, 16));
}

vector CSurface::get_nodeVector(const vector& v1, const vector& v2, const vector& v3)
{
    vector node;
    // dividing through 3 is not necessary cause normal vector would be the same
    node.x = v1.x + v2.x + v3.x;
    node.y = v1.y + v2.y + v3.y;
    node.z = v1.z + v2.z + v3.z;
    node = get_normVector(node);
    return node;
}

vector CSurface::get_normVector(const vector& v)
{
    vector normal;
    auto length = static_cast<float>(sqrt(pow(v.x, 2) + pow(v.y, 2) + pow(v.z, 2)));
    // in case vector length equals 0 (should not happen)
    if(std::abs(length) < 1e-20f)
    {
        normal.x = 0;
        normal.y = 0;
        normal.z = 1;
    } else
    {
        normal = v;
        normal.x /= length;
        normal.y /= length;
        normal.z /= length;
    }

    return normal;
}

vector CSurface::get_flatVector(const IntVector& P1, const IntVector& P2, const IntVector& P3)
{
    // vector components
    float vax, vay, vaz, vbx, vby, vbz;
    // cross product
    vector cross;

    vax = static_cast<float>(P1.x - P2.x);
    vay = static_cast<float>(P1.y - P2.y);
    vaz = static_cast<float>(P1.z - P2.z);
    vbx = static_cast<float>(P3.x - P2.x);
    vby = static_cast<float>(P3.y - P2.y);
    vbz = static_cast<float>(P3.z - P2.z);

    cross.x = (vay * vbz - vaz * vby);
    cross.y = (vaz * vbx - vax * vbz);
    cross.z = (vax * vby - vay * vbx);
    // normalize
    cross = get_normVector(cross);

    return cross;
}

void CSurface::update_shading(bobMAP& myMap, int VertexX, int VertexY)
{
    // vertex count for the points
    int X, Y;

    bool even = false;
    if(VertexY % 2 == 0)
        even = true;

    update_flatVectors(myMap, VertexX, VertexY);
    update_nodeVector(myMap, VertexX, VertexY);

    // now update all nodeVectors around VertexX and VertexY
    // update first vertex left upside
    X = VertexX - (even ? 1 : 0);
    if(X < 0)
        X += myMap.width;
    Y = VertexY - 1;
    if(Y < 0)
        Y += myMap.height;
    update_nodeVector(myMap, X, Y);
    // update second vertex right upside
    X = VertexX + (even ? 0 : 1);
    if(X >= myMap.width)
        X -= myMap.width;
    Y = VertexY - 1;
    if(Y < 0)
        Y += myMap.height;
    update_nodeVector(myMap, X, Y);
    // update third point bottom left
    X = VertexX - 1;
    if(X < 0)
        X += myMap.width;
    Y = VertexY;
    update_nodeVector(myMap, X, Y);
    // update fourth point bottom right
    X = VertexX + 1;
    if(X >= myMap.width)
        X -= myMap.width;
    Y = VertexY;
    update_nodeVector(myMap, X, Y);
    // update fifth point down left
    X = VertexX - (even ? 1 : 0);
    if(X < 0)
        X += myMap.width;
    Y = VertexY + 1;
    if(Y >= myMap.height)
        Y -= myMap.height;
    update_nodeVector(myMap, X, Y);
    // update sixth point down right
    X = VertexX + (even ? 0 : 1);
    if(X >= myMap.width)
        X -= myMap.width;
    Y = VertexY + 1;
    if(Y >= myMap.height)
        Y -= myMap.height;
    update_nodeVector(myMap, X, Y);
}

void CSurface::update_flatVectors(bobMAP& myMap, int VertexX, int VertexY)
{
    // point structures for the triangles, Pmiddle is the point in the middle of the hexagon we will update
    MapNode *P1, *P2, *P3, *Pmiddle;
    // vertex count for the points
    int P1x, P1y, P2x, P2y, P3x, P3y;

    bool even = false;
    if(VertexY % 2 == 0)
        even = true;

    Pmiddle = &myMap.getVertex(VertexX, VertexY);

    // update first triangle left upside
    P1x = VertexX - (even ? 1 : 0);
    if(P1x < 0)
        P1x += myMap.width;
    P1y = VertexY - 1;
    if(P1y < 0)
        P1y += myMap.height;
    P1 = &myMap.getVertex(P1x, P1y);
    P2x = VertexX - 1;
    if(P2x < 0)
        P2x += myMap.width;
    P2y = VertexY;
    P2 = &myMap.getVertex(P2x, P2y);
    P3 = Pmiddle;
    P1->flatVector = get_flatVector(*P1, *P2, *P3);

    // update second triangle right upside
    P1x = VertexX + (even ? 0 : 1);
    if(P1x >= myMap.width)
        P1x -= myMap.width;
    P1y = VertexY - 1;
    if(P1y < 0)
        P1y += myMap.height;
    P1 = &myMap.getVertex(P1x, P1y);
    P2 = Pmiddle;
    P3x = VertexX + 1;
    if(P3x >= myMap.width)
        P3x -= myMap.width;
    P3y = VertexY;
    P3 = &myMap.getVertex(P3x, P3y);
    P1->flatVector = get_flatVector(*P1, *P2, *P3);

    // update third triangle down middle
    P1 = Pmiddle;
    P2x = VertexX - (even ? 1 : 0);
    if(P2x < 0)
        P2x += myMap.width;
    P2y = VertexY + 1;
    if(P2y >= myMap.height)
        P2y -= myMap.height;
    P2 = &myMap.getVertex(P2x, P2y);
    P3x = VertexX + (even ? 0 : 1);
    if(P3x >= myMap.width)
        P3x -= myMap.width;
    P3y = VertexY + 1;
    if(P3y >= myMap.height)
        P3y -= myMap.height;
    P3 = &myMap.getVertex(P3x, P3y);
    P1->flatVector = get_flatVector(*P1, *P2, *P3);
}

void CSurface::update_nodeVector(bobMAP& myMap, int VertexX, int VertexY)
{
    int j = VertexY;
    int i = VertexX;
    int width = myMap.width;
    int height = myMap.height;

    if(j % 2 == 0)
    {
        MapNode& curVertex = myMap.getVertex(i, j);
        int iM1 = (i == 0 ? width - 1 : i - 1);
        if(j == 0) // first line
            curVertex.normVector = get_nodeVector(myMap.getVertex(iM1, height - 1).flatVector,
                                                  myMap.getVertex(i, height - 1).flatVector, curVertex.flatVector);
        else
            curVertex.normVector = get_nodeVector(myMap.getVertex(iM1, j - 1).flatVector,
                                                  myMap.getVertex(i, j - 1).flatVector, curVertex.flatVector);
        curVertex.i = get_LightIntensity(curVertex.normVector);
    } else
    {
        MapNode& curVertex = myMap.getVertex(i, j);
        int iP1 = (i + 1 == width ? 0 : i + 1);

        curVertex.normVector = get_nodeVector(myMap.getVertex(i, j - 1).flatVector,
                                              myMap.getVertex(iP1, j - 1).flatVector, curVertex.flatVector);
        curVertex.i = get_LightIntensity(curVertex.normVector);
    }
}

float CSurface::absf(float a)
{
    if(a >= 0)
        return a;
    else
        return a * (-1);
}

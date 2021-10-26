// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CSurface.h"
#include "CGame.h"
#include "CMap.h"
#include "Rect.h"
#include "SGE/sge_blib.h"
#include "SGE/sge_rotation.h"
#include "globals.h"
#include "gameData/EdgeDesc.h"
#include "gameData/TerrainDesc.h"
#include <algorithm>
#include <cassert>
#include <cmath>

namespace {
SDL_Rect rect2SDL_Rect(const Rect& rect)
{
    Point<Sint16> origin(rect.getOrigin());
    Point<Uint16> size(rect.getSize());
    SDL_Rect result;
    result.x = origin.x;
    result.y = origin.y;
    result.w = size.x;
    result.h = size.y;
    return result;
}

void DrawPreCalcFadedTexturedTrigon(SDL_Surface* dest, const Point16& p1, const Point16& p2, const Point16& p3,
                                    SDL_Surface* source, const SDL_Rect& rect, Uint16 I1, Uint16 I2,
                                    Uint8 PreCalcPalettes[][256])
{
    Sint16 right = rect.x + rect.w - 1;
    Sint16 middle = rect.x + rect.w / Sint16(2);
    Sint16 bottom = rect.y + rect.h - 1;
    Uint32 colorKey;
    const int keycount = (SDL_GetColorKey(source, &colorKey) < 0) ? 0 : 1;
    sge_PreCalcFadedTexturedTrigonColorKeys(dest, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, source, rect.x, rect.y, right,
                                            rect.y, middle, bottom, I1, I2, I2, PreCalcPalettes, &colorKey, keycount);
}

void DrawFadedTexturedTrigon(SDL_Surface* dest, const Point16& p1, const Point16& p2, const Point16& p3,
                             SDL_Surface* source, const SDL_Rect& rect, Sint32 I1, Sint32 I2)
{
    Sint16 right = rect.x + rect.w - 1;
    Sint16 middle = rect.x + rect.w / Sint16(2);
    Sint16 bottom = rect.y + rect.h - 1;
    Uint32 colorKey;
    const int keycount = (SDL_GetColorKey(source, &colorKey) < 0) ? 0 : 1;
    sge_FadedTexturedTrigonColorKeys(dest, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, source, rect.x, rect.y, right, rect.y,
                                     middle, bottom, I1, I2, I2, &colorKey, keycount);
}
} // namespace

bool CSurface::drawTextures = false;

bool CSurface::Draw(SDL_Surface* Surf_Dest, SDL_Surface* Surf_Src, int X, int Y)
{
    if(!Surf_Dest || !Surf_Src)
        return false;

    SDL_Rect DestR;

    DestR.x = X;
    DestR.y = Y;

    SDL_BlitSurface(Surf_Src, nullptr, Surf_Dest, &DestR);

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

bool CSurface::Draw(SDL_Surface* Surf_Dest, SDL_Surface* Surf_Src, int X, int Y, int angle)
{
    if(!Surf_Dest || !Surf_Src)
        return false;

    Uint16 px, py;

    switch(angle)
    {
        case 90:
            px = 0;
            py = Surf_Src->h - 1;
            break;
        case 180:
            px = Surf_Src->w - 1;
            py = Surf_Src->h - 1;
            break;
        case 270:
            px = Surf_Src->w - 1;
            py = 0;
            break;
        default: return false;
    }

    sge_transform(Surf_Src, Surf_Dest, (float)angle, 1.0, 1.0, px, py, X, Y, SGE_TSAFE);

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

    SDL_BlitSurface(Surf_Src, &SrcR, Surf_Dest, &DestR);

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

void CSurface::DrawTriangleField(SDL_Surface* display, const DisplayRectangle& displayRect, const bobMAP& myMap)
{
    Uint16 width = myMap.width;
    Uint16 height = myMap.height;
    auto type = myMap.type;
    MapNode tempP1, tempP2, tempP3;

    // min size to avoid underflows
    if(width < 8 || height < 8)
        return;

    assert(displayRect.left < myMap.width_pixel);
    assert(displayRect.right > 0);
    assert(displayRect.top < myMap.height_pixel);
    assert(displayRect.bottom > 0);

    Uint8 maxH = 0;
    for(int y = 0; y < height; ++y)
    {
        for(int x = 0; x < width; ++x)
        {
            maxH = std::max(myMap.getVertex(x, y).h, maxH);
        }
    }
    const int additionalRows = TRIANGLE_INCREASE * std::max(0, maxH - 0x0A) / TRIANGLE_HEIGHT;

    // draw triangle field
    // NOTE: WE DO THIS TWICE, AT FIRST ONLY TRIANGLE-TEXTURES, AT SECOND THE TEXTURE-BORDERS AND OBJECTS
    for(int i = 0; i < 2; i++)
    {
        drawTextures = (i == 0);

        for(int k = 0; k < 4; k++)
        {
            // beware calling DrawTriangle for each triangle

            // IMPORTANT: integer values like +8 or -1 are for tolerance to beware of high triangles are not shown

            // at first call DrawTriangle for all triangles inside the map edges
            int row_start = std::max(displayRect.top, 2 * TRIANGLE_HEIGHT) / TRIANGLE_HEIGHT - 2;
            int row_end = (displayRect.bottom) / TRIANGLE_HEIGHT + additionalRows;
            int col_start = std::max<int>(displayRect.left, TRIANGLE_WIDTH) / TRIANGLE_WIDTH - 1;
            int col_end = (displayRect.right) / TRIANGLE_WIDTH + 1;
            bool view_outside_edges;

            if(k > 0)
            {
                // now call DrawTriangle for all triangles outside the map edges
                view_outside_edges = false;

                if(k == 1 || k == 3)
                {
                    // at first call DrawTriangle for all triangles up or down outside
                    if(displayRect.top < 0)
                    {
                        row_start = std::max(0, height - 1 - (-displayRect.top / TRIANGLE_HEIGHT) - 1);
                        row_end = height - 1;
                        view_outside_edges = true;
                    } else if(displayRect.bottom > myMap.height_pixel)
                    {
                        row_start = 0;
                        row_end = (displayRect.bottom - myMap.height_pixel) / TRIANGLE_HEIGHT + 8;
                        view_outside_edges = true;
                    } else if(displayRect.top <= 2 * TRIANGLE_HEIGHT)
                    {
                        // this is for draw triangles that are reduced under the lower map edge (have bigger y-coords as
                        // myMap.height_pixel)
                        row_start = height - 3;
                        row_end = height - 1;
                        view_outside_edges = true;
                    } else if(displayRect.bottom >= (myMap.height_pixel - 8 * TRIANGLE_HEIGHT))
                    {
                        // this is for draw triangles that are raised over the upper map edge (have negative y-coords)
                        row_start = 0;
                        row_end = 8;
                        view_outside_edges = true;
                    }
                }

                if(k == 2 || k == 3)
                {
                    // now call DrawTriangle for all triangles left or right outside
                    if(displayRect.left <= 0)
                    {
                        col_start = std::max(0, width - 1 - (-displayRect.left / TRIANGLE_WIDTH) - 1);
                        col_end = width - 1;
                        view_outside_edges = true;
                    } else if(displayRect.left < TRIANGLE_WIDTH)
                    {
                        col_start = width - 2;
                        col_end = width - 1;
                        view_outside_edges = true;
                    } else if(displayRect.right > myMap.width_pixel)
                    {
                        col_start = 0;
                        col_end = (displayRect.right - myMap.width_pixel) / TRIANGLE_WIDTH + 1;
                        view_outside_edges = true;
                    }
                }

                // if displayRect is not outside the map edges, there is nothing to do
                if(!view_outside_edges)
                    continue;
            }

            assert(col_start >= 0);
            assert(row_start >= 0);
            assert(col_start <= col_end);
            assert(row_start <= row_end);

            for(unsigned y = row_start; y < height - 1u && y <= static_cast<unsigned>(row_end); y++)
            {
                if(y % 2 == 0)
                {
                    // first RightSideUp
                    tempP2 = myMap.getVertex(width - 1, y + 1);
                    tempP2.x = 0;
                    DrawTriangle(display, displayRect, myMap, type, myMap.getVertex(0, y), tempP2,
                                 myMap.getVertex(0, y + 1));
                    for(unsigned x = std::max(col_start, 1); x < width && x <= static_cast<unsigned>(col_end); x++)
                    {
                        // RightSideUp
                        DrawTriangle(display, displayRect, myMap, type, myMap.getVertex(x, y),
                                     myMap.getVertex(x - 1, y + 1), myMap.getVertex(x, y + 1));
                        // UpSideDown
                        DrawTriangle(display, displayRect, myMap, type, myMap.getVertex(x - 1, y + 1),
                                     myMap.getVertex(x - 1, y), myMap.getVertex(x, y));
                    }
                    // last UpSideDown
                    tempP3 = myMap.getVertex(0, y);
                    tempP3.x = myMap.getVertex(width - 1, y).x + TRIANGLE_WIDTH;
                    DrawTriangle(display, displayRect, myMap, type, myMap.getVertex(width - 1, y + 1),
                                 myMap.getVertex(width - 1, y), tempP3);
                } else
                {
                    for(unsigned x = col_start; x < width - 1u && x <= static_cast<unsigned>(col_end); x++)
                    {
                        // RightSideUp
                        DrawTriangle(display, displayRect, myMap, type, myMap.getVertex(x, y),
                                     myMap.getVertex(x, y + 1), myMap.getVertex(x + 1, y + 1));
                        // UpSideDown
                        DrawTriangle(display, displayRect, myMap, type, myMap.getVertex(x + 1, y + 1),
                                     myMap.getVertex(x, y), myMap.getVertex(x + 1, y));
                    }
                    // last RightSideUp
                    tempP3 = myMap.getVertex(0, y + 1);
                    tempP3.x = myMap.getVertex(width - 1, y + 1).x + TRIANGLE_WIDTH;
                    DrawTriangle(display, displayRect, myMap, type, myMap.getVertex(width - 1, y),
                                 myMap.getVertex(width - 1, y + 1), tempP3);
                    // last UpSideDown
                    tempP1 = myMap.getVertex(0, y + 1);
                    tempP1.x = myMap.getVertex(width - 1, y + 1).x + TRIANGLE_WIDTH;
                    tempP3 = myMap.getVertex(0, y);
                    tempP3.x = myMap.getVertex(width - 1, y).x + TRIANGLE_WIDTH;
                    DrawTriangle(display, displayRect, myMap, type, tempP1, myMap.getVertex(width - 1, y), tempP3);
                }
            }

            // draw last line
            for(unsigned x = col_start; x < width - 1u && x <= static_cast<unsigned>(col_end); x++)
            {
                // RightSideUp
                tempP2 = myMap.getVertex(x, 0);
                tempP2.y = height * TRIANGLE_HEIGHT + myMap.getVertex(x, 0).y;
                tempP3 = myMap.getVertex(x + 1, 0);
                tempP3.y = height * TRIANGLE_HEIGHT + myMap.getVertex(x + 1, 0).y;
                DrawTriangle(display, displayRect, myMap, type, myMap.getVertex(x, height - 1), tempP2, tempP3);
                // UpSideDown
                tempP1 = myMap.getVertex(x + 1, 0);
                tempP1.y = height * TRIANGLE_HEIGHT + myMap.getVertex(x + 1, 0).y;
                DrawTriangle(display, displayRect, myMap, type, tempP1, myMap.getVertex(x, height - 1),
                             myMap.getVertex(x + 1, height - 1));
            }
        }

        // last RightSideUp
        tempP2 = myMap.getVertex(width - 1, 0);
        tempP2.y += height * TRIANGLE_HEIGHT;
        tempP3 = myMap.getVertex(0, 0);
        tempP3.x = myMap.getVertex(width - 1, 0).x + TRIANGLE_WIDTH;
        tempP3.y += height * TRIANGLE_HEIGHT;
        DrawTriangle(display, displayRect, myMap, type, myMap.getVertex(width - 1, height - 1), tempP2, tempP3);
        // last UpSideDown
        tempP1 = myMap.getVertex(0, 0);
        tempP1.x = myMap.getVertex(width - 1, 0).x + TRIANGLE_WIDTH;
        tempP1.y += height * TRIANGLE_HEIGHT;
        tempP3 = myMap.getVertex(0, height - 1);
        tempP3.x = myMap.getVertex(width - 1, height - 1).x + TRIANGLE_WIDTH;
        DrawTriangle(display, displayRect, myMap, type, tempP1, myMap.getVertex(width - 1, height - 1), tempP3);
    }
}

namespace {
enum class BorderPreference
{
    None,
    LeftTop,
    RightBottom
};
BorderPreference CalcBorders(const bobMAP& map, Uint8 s2Id1, Uint8 s2Id2, SDL_Rect& borderRect)
{
    // we have to decide which border to blit, "left or right" or "top or bottom"
    s2Id1 &= ~(0x40 | 0x80);
    s2Id2 &= ~(0x40 | 0x80);

    assert(s2Id1 < map.s2IdToTerrain.size());
    assert(s2Id2 < map.s2IdToTerrain.size());

    DescIdx<TerrainDesc> idxTop = map.s2IdToTerrain[s2Id1];
    DescIdx<TerrainDesc> idxBottom = map.s2IdToTerrain[s2Id2];
    if(idxTop == idxBottom)
        return BorderPreference::None;
    const TerrainDesc& t1 = global::worldDesc.get(idxTop);
    const TerrainDesc& t2 = global::worldDesc.get(idxBottom);
    if(t1.edgePriority > t2.edgePriority)
    {
        if(!t1.edgeType)
            return BorderPreference::None;
        borderRect = rect2SDL_Rect(global::worldDesc.get(t1.edgeType).posInTexture);
        return BorderPreference::LeftTop;
    } else if(t1.edgePriority < t2.edgePriority)
    {
        if(!t2.edgeType)
            return BorderPreference::None;
        borderRect = rect2SDL_Rect(global::worldDesc.get(t2.edgeType).posInTexture);
        return BorderPreference::RightBottom;
    }
    return BorderPreference::None;
}

template<typename T>
constexpr bool isInRange(T val, T min, T max)
{
    return val >= min && val <= max;
}

/// Return true if triangle is drawn
bool GetAdjustedPoints(const DisplayRectangle& displayRect, const bobMAP& myMap, Point32& p1, Point32& p2, Point32& p3)
{
    if((!isInRange(p1.x, displayRect.left, displayRect.right) && !isInRange(p2.x, displayRect.left, displayRect.right)
        && !isInRange(p3.x, displayRect.left, displayRect.right))
       || (!isInRange(p1.y, displayRect.top, displayRect.bottom)
           && !isInRange(p2.y, displayRect.top, displayRect.bottom)
           && !isInRange(p3.y, displayRect.top, displayRect.bottom)))
    {
        bool triangle_shown = false;

        if(displayRect.left <= 0)
        {
            int outside_left = displayRect.left;
            int outside_right = 0;
            if(isInRange(p1.x - myMap.width_pixel, outside_left, outside_right)
               || isInRange(p2.x - myMap.width_pixel, outside_left, outside_right)
               || isInRange(p3.x - myMap.width_pixel, outside_left, outside_right))
            {
                p1.x -= myMap.width_pixel;
                p2.x -= myMap.width_pixel;
                p3.x -= myMap.width_pixel;
                triangle_shown = true;
            }
        } else if(displayRect.left < TRIANGLE_WIDTH)
        {
            int outside_left = displayRect.left;
            int outside_right = displayRect.left + TRIANGLE_WIDTH;
            if(isInRange(p1.x - myMap.width_pixel, outside_left, outside_right)
               || isInRange(p2.x - myMap.width_pixel, outside_left, outside_right)
               || isInRange(p3.x - myMap.width_pixel, outside_left, outside_right))
            {
                p1.x -= myMap.width_pixel;
                p2.x -= myMap.width_pixel;
                p3.x -= myMap.width_pixel;
                triangle_shown = true;
            }
        } else if(displayRect.right > myMap.width_pixel)
        {
            int outside_left = myMap.width_pixel;
            int outside_right = displayRect.right;
            if(isInRange(p1.x + myMap.width_pixel, outside_left, outside_right)
               || isInRange(p2.x + myMap.width_pixel, outside_left, outside_right)
               || isInRange(p3.x + myMap.width_pixel, outside_left, outside_right))
            {
                p1.x += myMap.width_pixel;
                p2.x += myMap.width_pixel;
                p3.x += myMap.width_pixel;
                triangle_shown = true;
            }
        }

        if(displayRect.top < 0)
        {
            int outside_top = displayRect.top;
            int outside_bottom = 0;
            if(isInRange(p1.y - myMap.height_pixel, outside_top, outside_bottom)
               || isInRange(p2.y - myMap.height_pixel, outside_top, outside_bottom)
               || isInRange(p3.y - myMap.height_pixel, outside_top, outside_bottom))
            {
                p1.y -= myMap.height_pixel;
                p2.y -= myMap.height_pixel;
                p3.y -= myMap.height_pixel;
                triangle_shown = true;
            }
        } else if(displayRect.bottom > myMap.height_pixel)
        {
            int outside_top = myMap.height_pixel;
            int outside_bottom = displayRect.bottom;
            if(isInRange(p1.y + myMap.height_pixel, outside_top, outside_bottom)
               || isInRange(p2.y + myMap.height_pixel, outside_top, outside_bottom)
               || isInRange(p3.y + myMap.height_pixel, outside_top, outside_bottom))
            {
                p1.y += myMap.height_pixel;
                p2.y += myMap.height_pixel;
                p3.y += myMap.height_pixel;
                triangle_shown = true;
            }
        }

        // now test if triangle has negative y-coords cause it's raised over the upper map edge
        if(p1.y < 0 || p2.y < 0 || p3.y < 0)
        {
            if(isInRange(p1.y + myMap.height_pixel, displayRect.top, displayRect.bottom)
               || isInRange(p2.y + myMap.height_pixel, displayRect.top, displayRect.bottom)
               || isInRange(p3.y + myMap.height_pixel, displayRect.top, displayRect.bottom))
            {
                p1.y += myMap.height_pixel;
                p2.y += myMap.height_pixel;
                p3.y += myMap.height_pixel;
                triangle_shown = true;
            }
        }

        // now test if triangle has bigger y-coords as myMap.height_pixel cause it's reduced under the lower map edge
        if(p1.y > myMap.height_pixel || p2.y > myMap.height_pixel || p3.y > myMap.height_pixel)
        {
            if(isInRange(p1.y - myMap.height_pixel, displayRect.top, displayRect.bottom)
               || isInRange(p2.y - myMap.height_pixel, displayRect.top, displayRect.bottom)
               || isInRange(p3.y - myMap.height_pixel, displayRect.top, displayRect.bottom))
            {
                p1.y -= myMap.height_pixel;
                p2.y -= myMap.height_pixel;
                p3.y -= myMap.height_pixel;
                triangle_shown = true;
            }
        }

        if(!triangle_shown)
            return false;
    }
    p1 -= displayRect.getOrigin();
    p2 -= displayRect.getOrigin();
    p3 -= displayRect.getOrigin();
    return true;
}
} // namespace

void CSurface::GetTerrainTextureCoords(MapType mapType, TriangleTerrainType texture, bool isRSU, int texture_move,
                                       Point16& upper, Point16& left, Point16& right, Point16& upper2, Point16& left2,
                                       Point16& right2)
{
    const auto animOffset = Point16(-texture_move, texture_move);
    switch(texture)
    {
            // in case of USD-Triangle "upper.x" and "upper.y" means "lowerX" and "lowerY"
        case TRIANGLE_TEXTURE_STEPPE_MEADOW1:
            upper = Point16(17, 96);
            left = Point16(0, 126);
            right = Point16(35, 126);
            break;
        case TRIANGLE_TEXTURE_MINING1:
            upper = Point16(17, 48);
            left = Point16(0, 78);
            right = Point16(35, 78);
            break;
        case TRIANGLE_TEXTURE_SNOW:
            if(isRSU)
            {
                upper = Point16(17, 0);
                left = Point16(0, 30);
                right = Point16(35, 30);
            } else
            {
                upper = Point16(17, 28);
                left = Point16(0, 0);
                right = Point16(37, 0);
            }
            if(mapType == MAP_WINTERLAND)
            {
                if(isRSU)
                {
                    upper2 = Point16(231, 61) + animOffset;
                    left2 = Point16(207, 62) + animOffset;
                    right2 = Point16(223, 78) + animOffset;
                } else
                {
                    upper2 = Point16(224, 79) + animOffset;
                    left2 = Point16(232, 62) + animOffset;
                    right2 = Point16(245, 76) + animOffset;
                }
            }
            break;
        case TRIANGLE_TEXTURE_SWAMP:
            upper = Point16(113, 0);
            left = Point16(96, 30);
            right = Point16(131, 30);
            if(mapType == MAP_WINTERLAND)
            {
                if(isRSU)
                {
                    upper2 = Point16(231, 61) + animOffset;
                    left2 = Point16(207, 62) + animOffset;
                    right2 = Point16(223, 78) + animOffset;
                } else
                {
                    upper2 = Point16(224, 79) + animOffset;
                    left2 = Point16(232, 62) + animOffset;
                    right2 = Point16(245, 76) + animOffset;
                }
            }
            break;
        case TRIANGLE_TEXTURE_STEPPE:
        case TRIANGLE_TEXTURE_STEPPE_:
        case TRIANGLE_TEXTURE_STEPPE__:
        case TRIANGLE_TEXTURE_STEPPE___:
            upper = Point16(65, 0);
            left = Point16(48, 30);
            right = Point16(83, 30);
            break;
        case TRIANGLE_TEXTURE_WATER:
        case TRIANGLE_TEXTURE_WATER_:
        case TRIANGLE_TEXTURE_WATER__:
            if(isRSU)
            {
                upper = Point16(231, 61) + animOffset;
                left = Point16(207, 62) + animOffset;
                right = Point16(223, 78) + animOffset;
            } else
            {
                upper = Point16(224, 79) + animOffset;
                left = Point16(232, 62) + animOffset;
                right = Point16(245, 76) + animOffset;
            }
            break;
        case TRIANGLE_TEXTURE_MEADOW1:
            upper = Point16(65, 96);
            left = Point16(48, 126);
            right = Point16(83, 126);
            break;
        case TRIANGLE_TEXTURE_MEADOW2:
            upper = Point16(113, 96);
            left = Point16(96, 126);
            right = Point16(131, 126);
            break;
        case TRIANGLE_TEXTURE_MEADOW3:
            upper = Point16(161, 96);
            left = Point16(144, 126);
            right = Point16(179, 126);
            break;
        case TRIANGLE_TEXTURE_MINING2:
            upper = Point16(65, 48);
            left = Point16(48, 78);
            right = Point16(83, 78);
            break;
        case TRIANGLE_TEXTURE_MINING3:
            upper = Point16(113, 48);
            left = Point16(96, 78);
            right = Point16(131, 78);
            break;
        case TRIANGLE_TEXTURE_MINING4:
            upper = Point16(161, 48);
            left = Point16(144, 78);
            right = Point16(179, 78);
            break;
        case TRIANGLE_TEXTURE_STEPPE_MEADOW2:
            upper = Point16(17, 144);
            left = Point16(0, 174);
            right = Point16(35, 174);
            break;
        case TRIANGLE_TEXTURE_FLOWER:
            upper = Point16(161, 0);
            left = Point16(144, 30);
            right = Point16(179, 30);
            break;
        case TRIANGLE_TEXTURE_LAVA:
            if(isRSU)
            {
                upper = Point16(231, 117) + animOffset;
                left = Point16(207, 118) + animOffset;
                right = Point16(223, 134) + animOffset;
            } else
            {
                upper = Point16(224, 135) + animOffset;
                left = Point16(232, 118) + animOffset;
                right = Point16(245, 132) + animOffset;
            }
            break;
        case TRIANGLE_TEXTURE_MINING_MEADOW:
            upper = Point16(65, 144);
            left = Point16(48, 174);
            right = Point16(83, 174);
            break;
        default: // TRIANGLE_TEXTURE_FLOWER
            upper = Point16(161, 0);
            left = Point16(144, 30);
            right = Point16(179, 30);
            break;
    }
}

void CSurface::DrawTriangle(SDL_Surface* display, const DisplayRectangle& displayRect, const bobMAP& myMap,
                            MapType type, const MapNode& P1, const MapNode& P2, const MapNode& P3)
{
    Point32 p1(P1.x, P1.y);
    Point32 p2(P2.x, P2.y);
    Point32 p3(P3.x, P3.y);
    // prevent drawing triangles that are not shown
    if(!GetAdjustedPoints(displayRect, myMap, p1, p2, p3))
        return;

    // for moving water, lava, objects and so on
    // This is very tricky: there are ice floes in the winterland and the water under this floes is moving.
    // I don't know how this works in original settlers 2 but i solved it this way:
    // i texture the triangle with normal water and then draw the floe over it. To Extract the floe
    // from it's surrounded water, i use this color keys below. These are the color values for the water texture.
    // I wrote a special SGE-Function that uses these color keys and ignores them in the Surf_Tileset.
    static std::array<Uint32, 5> colorkeys = {14191, 14195, 13167, 13159, 11119};
    static int texture_move = 0;
    static int roundCount = 0;
    static Uint32 roundTimeObjects = SDL_GetTicks();
    static Uint32 roundTimeTextures = SDL_GetTicks();
    if(SDL_GetTicks() - roundTimeObjects > 30)
    {
        roundTimeObjects = SDL_GetTicks();
        if(roundCount >= 7)
            roundCount = 0;
        else
            roundCount++;
    }
    if(SDL_GetTicks() - roundTimeTextures > 170)
    {
        roundTimeTextures = SDL_GetTicks();
        texture_move++;
        if(texture_move > 14)
            texture_move = 0;
    }

    SDL_Surface* Surf_Tileset;
    switch(type)
    {
        case MAP_GREENLAND:
        default:
            Surf_Tileset = global::bmpArray[global::s2->getMapObj()->getBitsPerPixel() == 8 ? TILESET_GREENLAND_8BPP :
                                                                                              TILESET_GREENLAND_32BPP]
                             .surface.get();
            break;
        case MAP_WASTELAND:
            Surf_Tileset = global::bmpArray[global::s2->getMapObj()->getBitsPerPixel() == 8 ? TILESET_WASTELAND_8BPP :
                                                                                              TILESET_WASTELAND_32BPP]
                             .surface.get();
            break;
        case MAP_WINTERLAND:
            Surf_Tileset = global::bmpArray[global::s2->getMapObj()->getBitsPerPixel() == 8 ? TILESET_WINTERLAND_8BPP :
                                                                                              TILESET_WINTERLAND_32BPP]
                             .surface.get();
            break;
    }

    bool const isRSU = p1.y < p2.y;

    if(drawTextures)
    {
        // upper2, ..... are for special use in winterland.
        Point16 upper, left, right, upper2, left2, right2;
        auto const texture =
          TriangleTerrainType((isRSU ? P1.rsuTexture : P2.usdTexture) & ~0x40); // Mask out harbor bit
        GetTerrainTextureCoords(type, texture, isRSU, texture_move, upper, left, right, upper2, left2, right2);

        // draw the triangle
        // do not shade water and lava
        if(texture == TRIANGLE_TEXTURE_WATER || texture == TRIANGLE_TEXTURE_LAVA)
            sge_TexturedTrigon(display, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, Surf_Tileset, upper.x, upper.y, left.x,
                               left.y, right.x, right.y);
        else
        {
            // draw special winterland textures with moving water (ice floe textures)
            if(type == MAP_WINTERLAND && (texture == TRIANGLE_TEXTURE_SNOW || texture == TRIANGLE_TEXTURE_SWAMP))
            {
                sge_TexturedTrigon(display, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, Surf_Tileset, upper2.x, upper2.y,
                                   left2.x, left2.y, right2.x, right2.y);
                if(global::s2->getMapObj()->getBitsPerPixel() == 8)
                    sge_PreCalcFadedTexturedTrigonColorKeys(display, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, Surf_Tileset,
                                                            upper.x, upper.y, left.x, left.y, right.x, right.y,
                                                            P1.shading << 8, P2.shading << 8, P3.shading << 8,
                                                            gouData[type], colorkeys.data(), colorkeys.size());
                else
                    sge_FadedTexturedTrigonColorKeys(display, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, Surf_Tileset, upper.x,
                                                     upper.y, left.x, left.y, right.x, right.y, P1.i, P2.i, P3.i,
                                                     colorkeys.data(), colorkeys.size());
            } else
            {
                if(global::s2->getMapObj()->getBitsPerPixel() == 8)
                    sge_PreCalcFadedTexturedTrigon(display, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, Surf_Tileset, upper.x,
                                                   upper.y, left.x, left.y, right.x, right.y, P1.shading << 8,
                                                   P2.shading << 8, P3.shading << 8, gouData[type]);
                else
                    sge_FadedTexturedTrigon(display, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, Surf_Tileset, upper.x, upper.y,
                                            left.x, left.y, right.x, right.y, P1.i, P2.i, P3.i);
            }
        }
        return;
    }

    // blit borders
    /// PRIORITY FROM HIGH TO LOW: SNOW, MINING_MEADOW, STEPPE, STEPPE_MEADOW2, MINING, MEADOW, FLOWER, STEPPE_MEADOW1,
    /// SWAMP, WATER, LAVA
    if(global::s2->getMapObj()->getRenderBorders())
    {
        // RSU-Triangle
        if(isRSU)
        {
            // left upper / right lower edge - therefore get the usd-texture from left to compare
            Uint16 col = (P1.VertexX - 1 < 0 ? myMap.width - 1 : P1.VertexX - 1);
            MapNode tempP = myMap.getVertex(col, P1.VertexY);

            SDL_Rect BorderRect;
            auto borderSide = CalcBorders(myMap, tempP.usdTexture, P1.rsuTexture, BorderRect);
            if(borderSide != BorderPreference::None)
            {
                Point16 tmpP1{p1}, tmpP2{p2};
                Point32 thirdPt;
                if(borderSide == BorderPreference::LeftTop)
                    thirdPt = p3;
                else
                {
                    tmpP1 += Point16(1, 0);
                    tmpP2 += Point16(1, 0);
                    thirdPt = Point32(tempP.x, tempP.y) - displayRect.getOrigin();
                    // Shift it close to p1
                    auto diff = thirdPt - p1;
                    if(diff.x < -myMap.width_pixel / 2)
                        thirdPt.x += myMap.width_pixel;
                    else if(diff.x > myMap.width_pixel / 2)
                        thirdPt.x -= myMap.width_pixel;
                    if(diff.y < -myMap.height_pixel / 2)
                        thirdPt.y += myMap.height_pixel;
                    else if(diff.y > myMap.height_pixel / 2)
                        thirdPt.y -= myMap.height_pixel;
                }
                Point16 tipPt{(p1 + p2 + thirdPt) / 3};

                if(global::s2->getMapObj()->getBitsPerPixel() == 8)
                    DrawPreCalcFadedTexturedTrigon(display, tmpP1, tmpP2, tipPt, Surf_Tileset, BorderRect,
                                                   P1.shading << 8, P2.shading << 8, gouData[type]);
                else
                    DrawFadedTexturedTrigon(display, tmpP1, tmpP2, tipPt, Surf_Tileset, BorderRect, P1.i, P2.i);
            }
        }
        // USD-Triangle
        else
        {
            SDL_Rect BorderRect;
            // left lower / right upper
            auto borderSide = CalcBorders(myMap, P2.rsuTexture, P2.usdTexture, BorderRect);

            if(borderSide != BorderPreference::None)
            {
                Uint16 col = (P1.VertexX - 1 < 0 ? myMap.width - 1 : P1.VertexX - 1);
                MapNode tempP = myMap.getVertex(col, P1.VertexY);

                Point16 tmpP1{p1}, tmpP2{p2};
                Point32 thirdPt;
                if(borderSide == BorderPreference::LeftTop)
                {
                    thirdPt = p3;
                    tmpP1 -= Point16(1, 0);
                    tmpP2 -= Point16(1, 0);
                } else
                {
                    thirdPt = Point32(tempP.x, tempP.y) - displayRect.getOrigin();
                    // Shift it close to p1
                    auto diff = thirdPt - p1;
                    if(diff.x < -myMap.width_pixel / 2)
                        thirdPt.x += myMap.width_pixel;
                    else if(diff.x > myMap.width_pixel / 2)
                        thirdPt.x -= myMap.width_pixel;
                    if(diff.y < -myMap.height_pixel / 2)
                        thirdPt.y += myMap.height_pixel;
                    else if(diff.y > myMap.height_pixel / 2)
                        thirdPt.y -= myMap.height_pixel;
                }

                Point16 tipPt{(p1 + p2 + thirdPt) / 3};

                if(global::s2->getMapObj()->getBitsPerPixel() == 8)
                    DrawPreCalcFadedTexturedTrigon(display, tmpP1, tmpP2, tipPt, Surf_Tileset, BorderRect,
                                                   P1.shading << 8, P2.shading << 8, gouData[type]);
                else
                    DrawFadedTexturedTrigon(display, tmpP1, tmpP2, tipPt, Surf_Tileset, BorderRect, P1.i, P2.i);
            }

            // top / bottom - therefore get the rsu-texture one line above to compare
            Uint16 row = (P2.VertexY - 1 < 0 ? myMap.height - 1 : P2.VertexY - 1);
            Uint16 col = (P2.VertexY % 2 == 0 ? P2.VertexX : (P2.VertexX + 1 > myMap.width - 1 ? 0 : P2.VertexX + 1));
            MapNode tempP = myMap.getVertex(col, row);

            borderSide = CalcBorders(myMap, tempP.rsuTexture, P2.usdTexture, BorderRect);
            if(borderSide != BorderPreference::None)
            {
                Point32 thirdPt;
                if(borderSide == BorderPreference::LeftTop)
                    thirdPt = p1;
                else
                {
                    thirdPt = Point32(tempP.x, tempP.y) - displayRect.getOrigin();
                    // Shift it close to p2
                    auto diff = thirdPt - p2;
                    if(diff.x < -myMap.width_pixel / 2)
                        thirdPt.x += myMap.width_pixel;
                    else if(diff.x > myMap.width_pixel / 2)
                        thirdPt.x -= myMap.width_pixel;
                    if(diff.y < -myMap.height_pixel / 2)
                        thirdPt.y += myMap.height_pixel;
                    else if(diff.y > myMap.height_pixel / 2)
                        thirdPt.y -= myMap.height_pixel;
                }
                Point16 tipPt{(p2 + p3 + thirdPt) / 3};

                if(global::s2->getMapObj()->getBitsPerPixel() == 8)
                    DrawPreCalcFadedTexturedTrigon(display, Point16(p2), Point16(p3), tipPt, Surf_Tileset, BorderRect,
                                                   P2.shading << 8, P3.shading << 8, gouData[type]);
                else
                    DrawFadedTexturedTrigon(display, Point16(p2), Point16(p3), tipPt, Surf_Tileset, BorderRect, P2.i,
                                            P3.i);
            }
        }
    }

    // blit picture to vertex (trees, animals, buildings and so on) --> BUT ONLY AT node1 ON RIGHTSIDEUP-TRIANGLES

    // blit objects
    if(!isRSU)
    {
        int objIdx = 0;
        switch(P2.objectInfo)
        {
            // tree
            case 0xC4:
                if(P2.objectType >= 0x30 && P2.objectType <= 0x37)
                {
                    if(P2.objectType + roundCount > 0x37)
                        objIdx = MAPPIC_TREE_PINE + (P2.objectType - 0x30) + (roundCount - 7);
                    else
                        objIdx = MAPPIC_TREE_PINE + (P2.objectType - 0x30) + roundCount;

                } else if(P2.objectType >= 0x70 && P2.objectType <= 0x77)
                {
                    if(P2.objectType + roundCount > 0x77)
                        objIdx = MAPPIC_TREE_BIRCH + (P2.objectType - 0x70) + (roundCount - 7);
                    else
                        objIdx = MAPPIC_TREE_BIRCH + (P2.objectType - 0x70) + roundCount;
                } else if(P2.objectType >= 0xB0 && P2.objectType <= 0xB7)
                {
                    if(P2.objectType + roundCount > 0xB7)
                        objIdx = MAPPIC_TREE_OAK + (P2.objectType - 0xB0) + (roundCount - 7);
                    else
                        objIdx = MAPPIC_TREE_OAK + (P2.objectType - 0xB0) + roundCount;
                } else if(P2.objectType >= 0xF0 && P2.objectType <= 0xF7)
                {
                    if(P2.objectType + roundCount > 0xF7)
                        objIdx = MAPPIC_TREE_PALM1 + (P2.objectType - 0xF0) + (roundCount - 7);
                    else
                        objIdx = MAPPIC_TREE_PALM1 + (P2.objectType - 0xF0) + roundCount;
                }
                break;
            // tree
            case 0xC5:
                if(P2.objectType >= 0x30 && P2.objectType <= 0x37)
                {
                    if(P2.objectType + roundCount > 0x37)
                        objIdx = MAPPIC_TREE_PALM2 + (P2.objectType - 0x30) + (roundCount - 7);
                    else
                        objIdx = MAPPIC_TREE_PALM2 + (P2.objectType - 0x30) + roundCount;

                } else if(P2.objectType >= 0x70 && P2.objectType <= 0x77)
                {
                    if(P2.objectType + roundCount > 0x77)
                        objIdx = MAPPIC_TREE_PINEAPPLE + (P2.objectType - 0x70) + (roundCount - 7);
                    else
                        objIdx = MAPPIC_TREE_PINEAPPLE + (P2.objectType - 0x70) + roundCount;
                } else if(P2.objectType >= 0xB0 && P2.objectType <= 0xB7)
                {
                    if(P2.objectType + roundCount > 0xB7)
                        objIdx = MAPPIC_TREE_CYPRESS + (P2.objectType - 0xB0) + (roundCount - 7);
                    else
                        objIdx = MAPPIC_TREE_CYPRESS + (P2.objectType - 0xB0) + roundCount;
                } else if(P2.objectType >= 0xF0 && P2.objectType <= 0xF7)
                {
                    if(P2.objectType + roundCount > 0xF7)
                        objIdx = MAPPIC_TREE_CHERRY + (P2.objectType - 0xF0) + (roundCount - 7);
                    else
                        objIdx = MAPPIC_TREE_CHERRY + (P2.objectType - 0xF0) + roundCount;
                }
                break;
            // tree
            case 0xC6:
                if(P2.objectType >= 0x30 && P2.objectType <= 0x37)
                {
                    if(P2.objectType + roundCount > 0x37)
                        objIdx = MAPPIC_TREE_FIR + (P2.objectType - 0x30) + (roundCount - 7);
                    else
                        objIdx = MAPPIC_TREE_FIR + (P2.objectType - 0x30) + roundCount;
                }
                break;
            // landscape
            case 0xC8:
                switch(P2.objectType)
                {
                    case 0x00: objIdx = MAPPIC_MUSHROOM1; break;
                    case 0x01: objIdx = MAPPIC_MUSHROOM2; break;
                    case 0x02: objIdx = MAPPIC_STONE1; break;
                    case 0x03: objIdx = MAPPIC_STONE2; break;
                    case 0x04: objIdx = MAPPIC_STONE3; break;
                    case 0x05: objIdx = MAPPIC_TREE_TRUNK_DEAD; break;
                    case 0x06: objIdx = MAPPIC_TREE_DEAD; break;
                    case 0x07: objIdx = MAPPIC_BONE1; break;
                    case 0x08: objIdx = MAPPIC_BONE2; break;
                    case 0x09: objIdx = MAPPIC_FLOWERS; break;
                    case 0x10: objIdx = MAPPIC_BUSH2; break;
                    case 0x11: objIdx = MAPPIC_BUSH3; break;
                    case 0x12: objIdx = MAPPIC_BUSH4; break;

                    case 0x0A: objIdx = MAPPIC_BUSH1; break;

                    case 0x0C: objIdx = MAPPIC_CACTUS1; break;
                    case 0x0D: objIdx = MAPPIC_CACTUS2; break;
                    case 0x0E: objIdx = MAPPIC_SHRUB1; break;
                    case 0x0F: objIdx = MAPPIC_SHRUB2; break;

                    case 0x13: objIdx = MAPPIC_SHRUB3; break;
                    case 0x14: objIdx = MAPPIC_SHRUB4; break;

                    case 0x16: objIdx = MAPPIC_DOOR; break;

                    case 0x18: objIdx = MIS1BOBS_STONE1; break;
                    case 0x19: objIdx = MIS1BOBS_STONE2; break;
                    case 0x1A: objIdx = MIS1BOBS_STONE3; break;
                    case 0x1B: objIdx = MIS1BOBS_STONE4; break;
                    case 0x1C: objIdx = MIS1BOBS_STONE5; break;
                    case 0x1D: objIdx = MIS1BOBS_STONE6; break;
                    case 0x1E: objIdx = MIS1BOBS_STONE7; break;

                    case 0x22: objIdx = MAPPIC_MUSHROOM3; break;

                    case 0x25: objIdx = MAPPIC_PEBBLE1; break;
                    case 0x26: objIdx = MAPPIC_PEBBLE2; break;
                    case 0x27: objIdx = MAPPIC_PEBBLE3; break;
                    default: break;
                }
                break;
            // stone
            case 0xCC: objIdx = MAPPIC_GRANITE_1_1 + (P2.objectType - 0x01); break;
            // stone
            case 0xCD: objIdx = MAPPIC_GRANITE_2_1 + (P2.objectType - 0x01); break;
            // headquarter
            case 0x80: // node2.objectType is the number of the player beginning with 0x00
                       //%7 cause in the original game there are only 7 players and 7 different flags
                objIdx = FLAG_BLUE_DARK + P2.objectType % 7;
                break;

            default: break;
        }
        if(objIdx != 0)
            Draw(display, global::bmpArray[objIdx].surface, (int)(p2.x - global::bmpArray[objIdx].nx),
                 (int)(p2.y - global::bmpArray[objIdx].ny));
    }

    // blit resources
    if(!isRSU)
    {
        if(P2.resource >= 0x41 && P2.resource <= 0x47)
        {
            for(char i = 0x41; i <= P2.resource; i++)
                Draw(display, global::bmpArray[PICTURE_RESOURCE_COAL].surface,
                     (int)(p2.x - global::bmpArray[PICTURE_RESOURCE_COAL].nx),
                     (int)(p2.y - global::bmpArray[PICTURE_RESOURCE_COAL].ny - (4 * (i - 0x40))));
        } else if(P2.resource >= 0x49 && P2.resource <= 0x4F)
        {
            for(char i = 0x49; i <= P2.resource; i++)
                Draw(display, global::bmpArray[PICTURE_RESOURCE_ORE].surface,
                     (int)(p2.x - global::bmpArray[PICTURE_RESOURCE_ORE].nx),
                     (int)(p2.y - global::bmpArray[PICTURE_RESOURCE_ORE].ny - (4 * (i - 0x48))));
        }
        if(P2.resource >= 0x51 && P2.resource <= 0x57)
        {
            for(char i = 0x51; i <= P2.resource; i++)
                Draw(display, global::bmpArray[PICTURE_RESOURCE_GOLD].surface,
                     (int)(p2.x - global::bmpArray[PICTURE_RESOURCE_GOLD].nx),
                     (int)(p2.y - global::bmpArray[PICTURE_RESOURCE_GOLD].ny - (4 * (i - 0x50))));
        }
        if(P2.resource >= 0x59 && P2.resource <= 0x5F)
        {
            for(char i = 0x59; i <= P2.resource; i++)
                Draw(display, global::bmpArray[PICTURE_RESOURCE_GRANITE].surface,
                     (int)(p2.x - global::bmpArray[PICTURE_RESOURCE_GRANITE].nx),
                     (int)(p2.y - global::bmpArray[PICTURE_RESOURCE_GRANITE].ny - (4 * (i - 0x58))));
        }
        // blit animals
        if(P2.animal > 0x00 && P2.animal <= 0x06)
        {
            Draw(display, global::bmpArray[PICTURE_SMALL_BEAR + P2.animal].surface,
                 (int)(p2.x - global::bmpArray[PICTURE_SMALL_BEAR + P2.animal].nx),
                 (int)(p2.y - global::bmpArray[PICTURE_SMALL_BEAR + P2.animal].ny));
        }
    }

    // blit buildings
    if(global::s2->getMapObj()->getRenderBuildHelp())
    {
        if(!isRSU)
        {
            switch(P2.build % 8)
            {
                case 0x01:
                    Draw(display, global::bmpArray[MAPPIC_FLAG].surface, (int)(p2.x - global::bmpArray[MAPPIC_FLAG].nx),
                         (int)(p2.y - global::bmpArray[MAPPIC_FLAG].ny));
                    break;
                case 0x02:
                    Draw(display, global::bmpArray[MAPPIC_HOUSE_SMALL].surface,
                         (int)(p2.x - global::bmpArray[MAPPIC_HOUSE_SMALL].nx),
                         (int)(p2.y - global::bmpArray[MAPPIC_HOUSE_SMALL].ny));
                    break;
                case 0x03:
                    Draw(display, global::bmpArray[MAPPIC_HOUSE_MIDDLE].surface,
                         (int)(p2.x - global::bmpArray[MAPPIC_HOUSE_MIDDLE].nx),
                         (int)(p2.y - global::bmpArray[MAPPIC_HOUSE_MIDDLE].ny));
                    break;
                case 0x04:
                    if(P2.rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1_HARBOUR
                       || P2.rsuTexture == TRIANGLE_TEXTURE_MEADOW1_HARBOUR
                       || P2.rsuTexture == TRIANGLE_TEXTURE_MEADOW2_HARBOUR
                       || P2.rsuTexture == TRIANGLE_TEXTURE_MEADOW3_HARBOUR
                       || P2.rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2_HARBOUR
                       || P2.rsuTexture == TRIANGLE_TEXTURE_FLOWER_HARBOUR
                       || P2.rsuTexture == TRIANGLE_TEXTURE_MINING_MEADOW_HARBOUR)
                        Draw(display, global::bmpArray[MAPPIC_HOUSE_HARBOUR].surface,
                             (int)(p2.x - global::bmpArray[MAPPIC_HOUSE_HARBOUR].nx),
                             (int)(p2.y - global::bmpArray[MAPPIC_HOUSE_HARBOUR].ny));
                    else
                        Draw(display, global::bmpArray[MAPPIC_HOUSE_BIG].surface,
                             (int)(p2.x - global::bmpArray[MAPPIC_HOUSE_BIG].nx),
                             (int)(p2.y - global::bmpArray[MAPPIC_HOUSE_BIG].ny));
                    break;
                case 0x05:
                    Draw(display, global::bmpArray[MAPPIC_MINE].surface, (int)(p2.x - global::bmpArray[MAPPIC_MINE].nx),
                         (int)(p2.y - global::bmpArray[MAPPIC_MINE].ny));
                    break;
                default: break;
            }
        }
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
            tempP3.x = myMap.getVertex(width - 1, j + 1).x + TRIANGLE_WIDTH;
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
        tempP2.y += height * TRIANGLE_HEIGHT;
        tempP3 = myMap.getVertex(i + 1, 0);
        tempP3.y += height * TRIANGLE_HEIGHT;
        myMap.getVertex(i, height - 1).flatVector = get_flatVector(myMap.getVertex(i, height - 1), tempP2, tempP3);
    }
    // vector of last Triangle
    tempP2 = myMap.getVertex(width - 1, 0);
    tempP2.y += height * TRIANGLE_HEIGHT;
    tempP3.x = myMap.getVertex(width - 1, 0).x + TRIANGLE_WIDTH;
    tempP3.y = height * TRIANGLE_HEIGHT + myMap.getVertex(0, 0).y;
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

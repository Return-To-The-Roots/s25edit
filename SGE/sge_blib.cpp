/*
 *	SDL Graphics Extension
 *	Triangles of every sort
 *
 *	Started 000428
 *
 *	License: LGPL v2+ (see the file LICENSE)
 *	(c)2000-2003 Anders Lindström & Johan E. Thelin
 */

/*********************************************************************
 *  This library is free software; you can redistribute it and/or    *
 *  modify it under the terms of the GNU Library General Public      *
 *  License as published by the Free Software Foundation; either     *
 *  version 2 of the License, or (at your option) any later version. *
 *********************************************************************/

/*
 *  Written with some help from Johan E. Thelin.
 */

#include "sge_blib.h"
#include "FixedPoint.h"
#include "sge_primitives.h"
#include "sge_primitives_int.h"
#include "sge_surface.h"
#include <boost/numeric/conversion/cast.hpp>
#include <array>

using boost::numeric_cast;
using FixedPoint = s25edit::FixedPoint<Sint32, 16>;
using UFixedPoint = s25edit::FixedPoint<Uint32, 16>;

#define SWAP(x, y, temp) \
    (temp) = x;          \
    (x) = y;             \
    (y) = temp

/* Globals used for sge_Update/sge_Lock (defined in sge_surface) */
extern Uint8 _sge_update;
extern Uint8 _sge_lock;
extern Uint8 _sge_alpha_hack;

namespace {

constexpr Uint32 MapRGB(const SDL_PixelFormat& format, Uint8 r, Uint8 g, Uint8 b)
{
    return ((Uint32)r >> format.Rloss) << format.Rshift | ((Uint32)g >> format.Gloss) << format.Gshift
           | ((Uint32)b >> format.Bloss) << format.Bshift | format.Amask;
}

constexpr Uint32 MapRGB(const SDL_PixelFormat& format, FixedPoint R, FixedPoint G, FixedPoint B)
{
    return MapRGB(format, static_cast<Uint8>(R.toUnsigned()), static_cast<Uint8>(G.toUnsigned()), static_cast<Uint8>(B.toUnsigned()));
}

constexpr Uint32 MapRGB(Uint8 r, Uint8 g, Uint8 b)
{
    return (Uint32)r << 16 | (Uint32)g << 8 | (Uint32)b;
}

Uint32 ScaleRGB(Uint32 value, Sint32 factor)
{
    const auto r = (Sint32((value & 0xFF0000) >> 16) * factor) >> 16;
    const auto g = (Sint32((value & 0x00FF00) >> 8) * factor) >> 16;
    const auto b = (Sint32((value & 0x0000FF)) * factor) >> 16;
    const auto r8 = (Uint8)(r > 255 ? 255 : (r < 0 ? 0 : r));
    const auto g8 = (Uint8)(g > 255 ? 255 : (g < 0 ? 0 : g));
    const auto b8 = (Uint8)(b > 255 ? 255 : (b < 0 ? 0 : b));
    return MapRGB(r8, g8, b8);
}
} // namespace
//==================================================================================
// Draws a horisontal line, fading the colors
//==================================================================================
static void _FadedLine(SDL_Surface* dest, Sint16 x1, Sint16 x2, Sint16 y, FixedPoint r1, FixedPoint g1, FixedPoint b1, FixedPoint r2,
                       FixedPoint g2, FixedPoint b2)
{
    Sint16 x;
    FixedPoint t;

    /* Fix coords */
    if(x1 > x2)
    {
        SWAP(x1, x2, x);
        SWAP(r1, r2, t);
        SWAP(g1, g2, t);
        SWAP(b1, b2, t);
    }

    /* We use fixedpoint math */
    auto R = r1;
    auto G = g1;
    auto B = b1;

    /* Color step value */
    auto rstep = (r2 - r1) / Sint32(x2 - x1 + 1);
    auto gstep = (g2 - g1) / Sint32(x2 - x1 + 1);
    auto bstep = (b2 - b1) / Sint32(x2 - x1 + 1);

    /* Clipping */
    if(x2 < sge_clip_xmin(dest) || x1 > sge_clip_xmax(dest) || y < sge_clip_ymin(dest) || y > sge_clip_ymax(dest))
        return;
    if(x1 < sge_clip_xmin(dest))
    {
        /* Update start colors */
        R += (sge_clip_xmin(dest) - x1) * rstep;
        G += (sge_clip_xmin(dest) - x1) * gstep;
        B += (sge_clip_xmin(dest) - x1) * bstep;
        x1 = sge_clip_xmin(dest);
    }
    if(x2 > sge_clip_xmax(dest))
        x2 = sge_clip_xmax(dest);

    const auto dstFormat = *dest->format;
    switch(dstFormat.BytesPerPixel)
    {
        case 1:
        { /* Assuming 8-bpp */
            Uint8* pixel;
            Uint8* row = (Uint8*)dest->pixels + y * dest->pitch;

            for(x = x1; x <= x2; x++)
            {
                pixel = row + x;

                *pixel = MapRGB(*dest->format, R, G, B);

                R += rstep;
                G += gstep;
                B += bstep;
            }
        }
        break;

        case 2:
        { /* Probably 15-bpp or 16-bpp */
            Uint16* pixel;
            Uint16* row = (Uint16*)dest->pixels + y * dest->pitch / 2;

            for(x = x1; x <= x2; x++)
            {
                pixel = row + x;

                *pixel = MapRGB(*dest->format, R, G, B);

                R += rstep;
                G += gstep;
                B += bstep;
            }
        }
        break;

        case 3:
        { /* Slow 24-bpp mode, usually not used */
            Uint8* pixel;
            Uint8* row = (Uint8*)dest->pixels + y * dest->pitch;

            Uint8 rshift8 = dstFormat.Rshift / 8;
            Uint8 gshift8 = dstFormat.Gshift / 8;
            Uint8 bshift8 = dstFormat.Bshift / 8;

            for(x = x1; x <= x2; x++)
            {
                pixel = row + x * 3;

                *(pixel + rshift8) = R.toUnsigned();
                *(pixel + gshift8) = G.toUnsigned();
                *(pixel + bshift8) = B.toUnsigned();

                R += rstep;
                G += gstep;
                B += bstep;
            }
        }
        break;

        case 4:
        { /* Probably 32-bpp */
            Uint32* pixel;
            Uint32* row = (Uint32*)dest->pixels + y * dest->pitch / 4;

            for(x = x1; x <= x2; x++)
            {
                pixel = row + x;

                *pixel = MapRGB(*dest->format, R, G, B);

                R += rstep;
                G += gstep;
                B += bstep;
            }
        }
        break;
    }
}

void sge_FadedLine(SDL_Surface* dest, Sint16 x1, Sint16 x2, Sint16 y, Uint8 r1, Uint8 g1, Uint8 b1, Uint8 r2, Uint8 g2, Uint8 b2)
{
    if(_sge_lock && SDL_MUSTLOCK(dest))
        if(SDL_LockSurface(dest) < 0)
            return;

    _FadedLine(dest, x1, x2, y, FixedPoint(r1), FixedPoint(g1), FixedPoint(b1), FixedPoint(r2), FixedPoint(g2), FixedPoint(b2));

    if(_sge_lock && SDL_MUSTLOCK(dest))
        SDL_UnlockSurface(dest);

    if(!_sge_update)
    {
        return;
    }
    sge_UpdateRect(dest, x1, y, absDiff(x1, x2) + 1, 1);
}

template<int dstBytesPerPixel>
static void _CopyPixelsWithDifferentFormat(SDL_Surface* dest, Sint16 y, Sint16 x1, Sint16 x2, SDL_Surface* source, FixedPoint srcx,
                                           FixedPoint srcy, const SDL_PixelFormat* srcFormat, FixedPoint xstep, FixedPoint ystep)
{
    switch(dstBytesPerPixel)
    {
        case 1:
        { /* Assuming 8-bpp */
            Uint8* pixel;
            Uint8* row = (Uint8*)dest->pixels + y * dest->pitch;

            for(int x = x1; x <= x2; x++)
            {
                pixel = row + x;

                Uint8 r, g, b;
                SDL_GetRGB(sge_GetPixel(source, srcx.toInt(), srcy.toInt()), srcFormat, &r, &g, &b);
                *pixel = SDL_MapRGB(dest->format, r, g, b);

                srcx += xstep;
                srcy += ystep;
            }
        }
        break;

        case 2:
        { /* Probably 15-bpp or 16-bpp */
            Uint16* pixel;
            Uint16* row = (Uint16*)dest->pixels + y * dest->pitch / sizeof(Uint16);

            for(int x = x1; x <= x2; x++)
            {
                pixel = row + x;

                Uint8 r, g, b;
                SDL_GetRGB(sge_GetPixel(source, srcx.toInt(), srcy.toInt()), srcFormat, &r, &g, &b);
                *pixel = MapRGB(*dest->format, r, g, b);

                srcx += xstep;
                srcy += ystep;
            }
        }
        break;

        case 3:
        { /* Slow 24-bpp mode, usually not used */
            Uint8* pixel;
            Uint8* row = (Uint8*)dest->pixels + y * dest->pitch;

            Uint8 rshift8 = dest->format->Rshift / 8;
            Uint8 gshift8 = dest->format->Gshift / 8;
            Uint8 bshift8 = dest->format->Bshift / 8;

            for(int x = x1; x <= x2; x++)
            {
                pixel = row + x * 3;

                Uint8 r, g, b;
                SDL_GetRGB(sge_GetPixel(source, srcx.toInt(), srcy.toInt()), srcFormat, &r, &g, &b);

                *(pixel + rshift8) = r;
                *(pixel + gshift8) = g;
                *(pixel + bshift8) = b;

                srcx += xstep;
                srcy += ystep;
            }
        }
        break;

        case 4:
        { /* Probably 32-bpp */
            Uint32* pixel;
            Uint32* row = (Uint32*)dest->pixels + y * dest->pitch / sizeof(Uint32);

            for(int x = x1; x <= x2; x++)
            {
                pixel = row + x;

                Uint8 r, g, b;
                SDL_GetRGB(sge_GetPixel(source, srcx.toInt(), srcy.toInt()), srcFormat, &r, &g, &b);
                *pixel = MapRGB(r, g, b);

                srcx += xstep;
                srcy += ystep;
            }
        }
        break;
    }
}

//==================================================================================
// Draws a horisontal, textured line
//==================================================================================
template<int srcBytesPerPixel, int dstBytesPerPixel>
static void _TexturedLine(SDL_Surface* dest, Sint16 x1, Sint16 x2, Sint16 y, SDL_Surface* source, FixedPoint sx1, FixedPoint sy1,
                          FixedPoint sx2, FixedPoint sy2)
{
    Sint16 _tmp1;
    FixedPoint _tmp2;

    /* Fix coords */
    if(x1 > x2)
    {
        SWAP(x1, x2, _tmp1);
        SWAP(sx1, sx2, _tmp2);
        SWAP(sy1, sy2, _tmp2);
    }
    if(x2 < sge_clip_xmin(dest))
        return;

    /* Fixed point texture starting coords */
    auto srcx = sx1;
    auto srcy = sy1;

    /* Texture coords stepping value */
    FixedPoint xstep = (sx2 - sx1) / Sint32(x2 - x1 + 1);
    FixedPoint ystep = (sy2 - sy1) / Sint32(x2 - x1 + 1);

    /* Clipping */
    assert(y >= sge_clip_ymin(dest) && y <= sge_clip_ymax(dest));
    assert(x1 <= sge_clip_xmax(dest) && x2 <= sge_clip_xmax(dest));

    if(x1 < sge_clip_xmin(dest))
    {
        /* Fix texture starting coord */
        srcx += (sge_clip_xmin(dest) - x1) * xstep;
        srcy += (sge_clip_xmin(dest) - x1) * ystep;
        x1 = sge_clip_xmin(dest);
    }

    if(dstBytesPerPixel == srcBytesPerPixel)
    {
        /* Fast mode. Just copy the pixel */

        switch(dstBytesPerPixel)
        {
            case 1:
            { /* Assuming 8-bpp */
                Uint8* row = (Uint8*)dest->pixels + y * dest->pitch;

                for(int x = x1; x <= x2; x++)
                {
                    auto* pixel = row + x;

                    const auto pixel_value = *((Uint8*)source->pixels + srcy.toInt() * source->pitch + srcx.toInt());

                    if(pixel_value != source->format->colorkey)
                        *pixel = pixel_value;

                    srcx += xstep;
                    srcy += ystep;
                }
            }
            break;

            case 2:
            { /* Probably 15-bpp or 16-bpp */
                Uint16* row = (Uint16*)dest->pixels + y * dest->pitch / 2;

                Uint16 pitch = source->pitch / 2;

                for(int x = x1; x <= x2; x++)
                {
                    auto* pixel = row + x;

                    *pixel = *((Uint16*)source->pixels + srcy.toInt() * pitch + srcx.toInt());

                    srcx += xstep;
                    srcy += ystep;
                }
            }
            break;

            case 3:
            { /* Slow 24-bpp mode, usually not used */
                const auto dstFormat = *dest->format;
                Uint8* row = (Uint8*)dest->pixels + y * dest->pitch;

                Uint8 rshift8 = dstFormat.Rshift / 8;
                Uint8 gshift8 = dstFormat.Gshift / 8;
                Uint8 bshift8 = dstFormat.Bshift / 8;

                for(int x = x1; x <= x2; x++)
                {
                    auto* pixel = row + x * 3;
                    auto* srcpixel = (Uint8*)source->pixels + srcy.toInt() * source->pitch + srcx.toInt() * 3;

                    *(pixel + rshift8) = *(srcpixel + rshift8);
                    *(pixel + gshift8) = *(srcpixel + gshift8);
                    *(pixel + bshift8) = *(srcpixel + bshift8);

                    srcx += xstep;
                    srcy += ystep;
                }
            }
            break;

            case 4:
            { /* Probably 32-bpp */
                Uint32* pixel = (Uint32*)dest->pixels + y * dest->pitch / sizeof(Uint32) + x1;

                const Uint16 pitch = source->pitch / sizeof(Uint32);
                const Uint32 colorkey = source->format->colorkey;

                for(int x = x1; x <= x2; x++, ++pixel)
                {
                    const auto pixel_value = *((Uint32*)source->pixels + srcy.toInt() * pitch + srcx.toInt());
                    if(pixel_value != colorkey)
                        *pixel = pixel_value;

                    srcx += xstep;
                    srcy += ystep;
                }
            }
            break;
        }
    } else
    {
        /* Slow mode. We must translate every pixel color! */
        _CopyPixelsWithDifferentFormat<dstBytesPerPixel>(dest, y, x1, x2, source, srcx, srcy, source->format, xstep, ystep);
    }
}

static auto makeIsColorKey()
{
    return [](const Uint32) { return false; };
}

static auto makeIsColorKey(Uint32 colorKey)
{
    return [colorKey](const Uint32 color) { return color == colorKey; };
}

static auto makeIsColorKey(const Uint32 keys[], int keycount)
{
    return [keys, keycount](const Uint32 color) {
        for(int i = 0; i < keycount; i++)
        {
            if(keys[i] == color)
                return true;
        }
        return false;
    };
}

//==================================================================================
// Draws a horisontal, gouraud shaded and textured line
//==================================================================================
template<class T_IsColorKey = decltype(makeIsColorKey())>
static void _FadedTexturedLine(SDL_Surface* dest, Sint16 x1, Sint16 x2, Sint16 y, SDL_Surface* source, FixedPoint sx1, FixedPoint sy1,
                               FixedPoint sx2, FixedPoint sy2, Sint32 i1, Sint32 i2, T_IsColorKey isColorKey)
{
    /* Fix coords */
    if(x1 > x2)
    {
        Sint16 _tmp1;
        FixedPoint _tmp2;
        Sint32 _tmp3;
        SWAP(x1, x2, _tmp1);
        SWAP(sx1, sx2, _tmp2);
        SWAP(sy1, sy2, _tmp2);
        SWAP(i1, i2, _tmp3);
    }
    if(x2 < sge_clip_xmin(dest))
        return;

    /* We use fixedpoint math */
    Sint32 I = i1;

    /* Color step value */
    Sint32 istep = (i2 - i1) / Sint32(x2 - x1 + 1);

    /* Texture coords stepping value */
    auto xstep = (sx2 - sx1) / Sint32(x2 - x1 + 1);
    auto ystep = (sy2 - sy1) / Sint32(x2 - x1 + 1);

    /* Clipping */
    assert(y >= sge_clip_ymin(dest) && y <= sge_clip_ymax(dest));
    assert(x1 <= sge_clip_xmax(dest) && x2 <= sge_clip_xmax(dest));

    if(x1 < sge_clip_xmin(dest))
    {
        /* Update start colors */
        I += (sge_clip_xmin(dest) - x1) * istep;
        /* Fix texture starting coord */
        sx1 += (sge_clip_xmin(dest) - x1) * xstep;
        sy1 += (sge_clip_xmin(dest) - x1) * ystep;
        x1 = sge_clip_xmin(dest);
    }

    assert(dest->format->BytesPerPixel == source->format->BytesPerPixel);
    assert(dest->format->BytesPerPixel == 4);

    Uint32* pixel = (Uint32*)dest->pixels + y * dest->pitch / sizeof(Uint32) + x1;
    const Uint16 pitch = source->pitch / sizeof(Uint32);

    for(int x = x1; x <= x2; x++, ++pixel)
    {
        const Uint32 pixel_value = *((Uint32*)source->pixels + sy1.toInt() * pitch + sx1.toInt());
        if(!isColorKey(pixel_value))
        {
            *pixel = ScaleRGB(pixel_value, I);
        }

        I += istep;

        sx1 += xstep;
        sy1 += ystep;
    }
}

//==================================================================================
// Draws a horisontal, textured line with precalculated gouraud shading
//==================================================================================
template<class T_IsColorKey>
static void _FadedTexturedLine(SDL_Surface* dest, Sint16 x1, Sint16 x2, Sint16 y, SDL_Surface* source, FixedPoint sx1, FixedPoint sy1,
                               FixedPoint sx2, FixedPoint sy2, Uint16 i1, Uint16 i2, T_IsColorKey isColorKey, Uint8 PreCalcPalettes[][256])
{
    /* Fix coords */
    if(x1 > x2)
    {
        Sint16 _tmp1;
        FixedPoint _tmp2;
        Uint16 _tmp3;
        SWAP(x1, x2, _tmp1);
        SWAP(sx1, sx2, _tmp2);
        SWAP(sy1, sy2, _tmp2);
        SWAP(i1, i2, _tmp3);
    }
    if(x2 < sge_clip_xmin(dest))
        return;

    /* We use fixedpoint math */
    Uint16 I = i1;

    /* Color step value */
    Sint16 istep = (i2 - i1) / (x2 - x1 + 1);

    /* Texture coords stepping value */
    auto xstep = (sx2 - sx1) / Sint32(x2 - x1 + 1);
    auto ystep = (sy2 - sy1) / Sint32(x2 - x1 + 1);

    /* Clipping */
    assert(y >= sge_clip_ymin(dest) && y <= sge_clip_ymax(dest));
    assert(x1 <= sge_clip_xmax(dest) && x2 <= sge_clip_xmax(dest));

    if(x1 < sge_clip_xmin(dest))
    {
        /* Update start colors */
        I += (sge_clip_xmin(dest) - x1) * istep;
        /* Fix texture starting coord */
        sx1 += (sge_clip_xmin(dest) - x1) * xstep;
        sy1 += (sge_clip_xmin(dest) - x1) * ystep;
        x1 = sge_clip_xmin(dest);
    }

    assert(dest->format->BytesPerPixel == source->format->BytesPerPixel);
    assert(dest->format->BytesPerPixel == 1);

    Uint8* pixel = (Uint8*)dest->pixels + y * dest->pitch + x1;
    const Uint16 pitch = source->pitch;

    for(int x = x1; x <= x2; x++, ++pixel)
    {
        const auto pixel_value = *((Uint8*)source->pixels + sy1.toInt() * pitch + sx1.toInt());

        if(!isColorKey(pixel_value))
        {
            *pixel = PreCalcPalettes[(Uint8)(I >> 8)][pixel_value];
        }

        sx1 += xstep;
        sy1 += ystep;

        I += istep;
    }
}

void sge_FadedTexturedLine(SDL_Surface* dest, Sint16 x1, Sint16 x2, Sint16 y, SDL_Surface* source, Sint16 sx1, Sint16 sy1, Sint16 sx2,
                           Sint16 sy2, Sint32 i1, Sint32 i2)
{
    if(_sge_lock && SDL_MUSTLOCK(dest))
        if(SDL_LockSurface(dest) < 0)
            return;
    if(_sge_lock && SDL_MUSTLOCK(source))
        if(SDL_LockSurface(source) < 0)
            return;

    {
        const auto maxX = sge_clip_xmax(dest);
        x1 = std::min<int>(x1, maxX);
        x2 = std::min<int>(x2, maxX);
    }

    _FadedTexturedLine(dest, x1, x2, y, source, FixedPoint(sx1), FixedPoint(sy1), FixedPoint(sx2), FixedPoint(sy2), i1, i2,
                       makeIsColorKey());

    if(_sge_lock && SDL_MUSTLOCK(dest))
        SDL_UnlockSurface(dest);
    if(_sge_lock && SDL_MUSTLOCK(source))
        SDL_UnlockSurface(source);

    if(!_sge_update)
    {
        return;
    }
    sge_UpdateRect(dest, x1, y, absDiff(x1, x2) + 1, 1);
}

//==================================================================================
// Draws a trigon
//==================================================================================
void sge_Trigon(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint32 color)
{
    if(_sge_lock && SDL_MUSTLOCK(dest))
        if(SDL_LockSurface(dest) < 0)
            return;

    _Line(dest, x1, y1, x2, y2, color);
    _Line(dest, x1, y1, x3, y3, color);
    _Line(dest, x3, y3, x2, y2, color);

    if(_sge_lock && SDL_MUSTLOCK(dest))
        SDL_UnlockSurface(dest);

    if(!_sge_update)
    {
        return;
    }

    Sint16 xmax = x1, ymax = y1, xmin = x1, ymin = y1;
    xmax = (xmax > x2) ? xmax : x2;
    ymax = (ymax > y2) ? ymax : y2;
    xmin = (xmin < x2) ? xmin : x2;
    ymin = (ymin < y2) ? ymin : y2;
    xmax = (xmax > x3) ? xmax : x3;
    ymax = (ymax > y3) ? ymax : y3;
    xmin = (xmin < x3) ? xmin : x3;
    ymin = (ymin < y3) ? ymin : y3;

    sge_UpdateRect(dest, xmin, ymin, numeric_cast<Uint16>(xmax - xmin + 1), numeric_cast<Uint16>(ymax - ymin + 1));
}

void sge_Trigon(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint8 R, Uint8 G, Uint8 B)
{
    sge_Trigon(dest, x1, y1, x2, y2, x3, y3, SDL_MapRGB(dest->format, R, G, B));
}

//==================================================================================
// Draws a trigon (alpha)
//==================================================================================
void sge_TrigonAlpha(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint32 color, Uint8 alpha)
{
    if(_sge_lock && SDL_MUSTLOCK(dest))
        if(SDL_LockSurface(dest) < 0)
            return;

    _LineAlpha(dest, x1, y1, x2, y2, color, alpha);
    _LineAlpha(dest, x1, y1, x3, y3, color, alpha);
    _LineAlpha(dest, x3, y3, x2, y2, color, alpha);

    if(_sge_lock && SDL_MUSTLOCK(dest))
        SDL_UnlockSurface(dest);

    if(!_sge_update)
    {
        return;
    }

    Sint16 xmax = x1, ymax = y1, xmin = x1, ymin = y1;
    xmax = (xmax > x2) ? xmax : x2;
    ymax = (ymax > y2) ? ymax : y2;
    xmin = (xmin < x2) ? xmin : x2;
    ymin = (ymin < y2) ? ymin : y2;
    xmax = (xmax > x3) ? xmax : x3;
    ymax = (ymax > y3) ? ymax : y3;
    xmin = (xmin < x3) ? xmin : x3;
    ymin = (ymin < y3) ? ymin : y3;

    sge_UpdateRect(dest, xmin, ymin, numeric_cast<Uint16>(xmax - xmin + 1), numeric_cast<Uint16>(ymax - ymin + 1));
}

void sge_TrigonAlpha(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint8 R, Uint8 G, Uint8 B,
                     Uint8 alpha)
{
    sge_TrigonAlpha(dest, x1, y1, x2, y2, x3, y3, SDL_MapRGB(dest->format, R, G, B), alpha);
}

//==================================================================================
// Draws an AA trigon (alpha)
//==================================================================================
void sge_AATrigonAlpha(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint32 color, Uint8 alpha)
{
    if(_sge_lock && SDL_MUSTLOCK(dest))
        if(SDL_LockSurface(dest) < 0)
            return;

    _AALineAlpha(dest, x1, y1, x2, y2, color, alpha);
    _AALineAlpha(dest, x1, y1, x3, y3, color, alpha);
    _AALineAlpha(dest, x3, y3, x2, y2, color, alpha);

    if(_sge_lock && SDL_MUSTLOCK(dest))
        SDL_UnlockSurface(dest);

    if(!_sge_update)
    {
        return;
    }

    Sint16 xmax = x1, ymax = y1, xmin = x1, ymin = y1;
    xmax = (xmax > x2) ? xmax : x2;
    ymax = (ymax > y2) ? ymax : y2;
    xmin = (xmin < x2) ? xmin : x2;
    ymin = (ymin < y2) ? ymin : y2;
    xmax = (xmax > x3) ? xmax : x3;
    ymax = (ymax > y3) ? ymax : y3;
    xmin = (xmin < x3) ? xmin : x3;
    ymin = (ymin < y3) ? ymin : y3;

    sge_UpdateRect(dest, xmin, ymin, xmax - xmin + 1, ymax - ymin + 1);
}

void sge_AATrigonAlpha(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint8 R, Uint8 G, Uint8 B,
                       Uint8 alpha)
{
    sge_AATrigonAlpha(dest, x1, y1, x2, y2, x3, y3, SDL_MapRGB(dest->format, R, G, B), alpha);
}

void sge_AATrigon(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint32 color)
{
    sge_AATrigonAlpha(dest, x1, y1, x2, y2, x3, y3, color, 255);
}

void sge_AATrigon(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint8 R, Uint8 G, Uint8 B)
{
    sge_AATrigonAlpha(dest, x1, y1, x2, y2, x3, y3, SDL_MapRGB(dest->format, R, G, B), 255);
}

//==================================================================================
// Draws a filled trigon
//==================================================================================
void sge_FilledTrigon(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint32 color)
{
    Sint16 y;

    if(y1 == y3)
        return;

    /* Sort coords */
    if(y1 > y2)
    {
        SWAP(y1, y2, y);
        SWAP(x1, x2, y);
    }
    if(y2 > y3)
    {
        SWAP(y2, y3, y);
        SWAP(x2, x3, y);
    }
    if(y1 > y2)
    {
        SWAP(y1, y2, y);
        SWAP(x1, x2, y);
    }

    /*
     * How do we calculate the starting and ending x coordinate of the horizontal line
     * on each y coordinate?  We can do this by using a standard line algorithm but
     * instead of plotting pixels, use the x coordinates as start and stop
     * coordinates for the horizontal line.
     * So we will simply trace the outlining of the triangle; this will require 3 lines.
     * Line 1 is the line between (x1,y1) and (x2,y2)
     * Line 2 is the line between (x1,y1) and (x3,y3)
     * Line 3 is the line between (x2,y2) and (x3,y3)
     *
     * We can divide the triangle into 2 halfs. The upper half will be outlined by line
     * 1 and 2. The lower half will be outlined by line line 2 and 3.
     */

    /* Starting coords for the three lines */
    auto xa = FixedPoint(x1);
    auto xb = xa;
    auto xc = FixedPoint(x2);

    /* Lines step values */
    auto m2 = FixedPoint(x3 - x1) / Sint32(y3 - y1);

    /* Upper half of the triangle */
    if(y1 == y2)
        _HLine(dest, x1, x2, y1, color);
    else
    {
        auto m1 = (xc - xa) / Sint32(y2 - y1);

        for(y = y1; y <= y2; y++)
        {
            _HLine(dest, xa.toInt(), xb.toInt(), y, color);

            xa += m1;
            xb += m2;
        }
    }

    /* Lower half of the triangle */
    if(y2 == y3)
        _HLine(dest, x2, x3, y2, color);
    else
    {
        auto m3 = FixedPoint(x3 - x2) / Sint32(y3 - y2);

        for(y = y2 + 1; y <= y3; y++)
        {
            _HLine(dest, xb.toInt(), xc.toInt(), y, color);

            xb += m2;
            xc += m3;
        }
    }

    if(!_sge_update)
    {
        return;
    }

    Sint16 xmax = x1, xmin = x1;
    xmax = (xmax > x2) ? xmax : x2;
    xmin = (xmin < x2) ? xmin : x2;
    xmax = (xmax > x3) ? xmax : x3;
    xmin = (xmin < x3) ? xmin : x3;

    sge_UpdateRect(dest, xmin, y1, numeric_cast<Uint16>(xmax - xmin + 1), numeric_cast<Uint16>(y3 - y1 + 1));
}

void sge_FilledTrigon(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint8 R, Uint8 G, Uint8 B)
{
    sge_FilledTrigon(dest, x1, y1, x2, y2, x3, y3, SDL_MapRGB(dest->format, R, G, B));
}

//==================================================================================
// Draws a filled trigon (alpha)
//==================================================================================
void sge_FilledTrigonAlpha(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint32 color, Uint8 alpha)
{
    Sint16 y;

    if(y1 == y3)
        return;

    /* Sort coords */
    if(y1 > y2)
    {
        SWAP(y1, y2, y);
        SWAP(x1, x2, y);
    }
    if(y2 > y3)
    {
        SWAP(y2, y3, y);
        SWAP(x2, x3, y);
    }
    if(y1 > y2)
    {
        SWAP(y1, y2, y);
        SWAP(x1, x2, y);
    }

    auto xa = FixedPoint(x1);
    auto xb = xa;
    auto xc = FixedPoint(x2);

    auto m2 = FixedPoint(x3 - x1) / Sint32(y3 - y1);

    if(_sge_lock && SDL_MUSTLOCK(dest))
        if(SDL_LockSurface(dest) < 0)
            return;

    /* Upper half of the triangle */
    if(y1 == y2)
        _HLineAlpha(dest, x1, x2, y1, color, alpha);
    else
    {
        auto m1 = (xc - xa) / Sint32(y2 - y1);

        for(y = y1; y <= y2; y++)
        {
            _HLineAlpha(dest, xa.toInt(), xb.toInt(), y, color, alpha);

            xa += m1;
            xb += m2;
        }
    }

    /* Lower half of the triangle */
    if(y2 == y3)
        _HLineAlpha(dest, x2, x3, y2, color, alpha);
    else
    {
        auto m3 = FixedPoint(x3 - x2) / Sint32(y3 - y2);

        for(y = y2 + 1; y <= y3; y++)
        {
            _HLineAlpha(dest, xb.toInt(), xc.toInt(), y, color, alpha);

            xb += m2;
            xc += m3;
        }
    }

    if(_sge_lock && SDL_MUSTLOCK(dest))
        SDL_UnlockSurface(dest);

    if(!_sge_update)
    {
        return;
    }

    Sint16 xmax = x1, xmin = x1;
    xmax = (xmax > x2) ? xmax : x2;
    xmin = (xmin < x2) ? xmin : x2;
    xmax = (xmax > x3) ? xmax : x3;
    xmin = (xmin < x3) ? xmin : x3;

    sge_UpdateRect(dest, xmin, y1, numeric_cast<Uint16>(xmax - xmin + 1), numeric_cast<Uint16>(y3 - y1 + 1));
}

void sge_FilledTrigonAlpha(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint8 R, Uint8 G, Uint8 B,
                           Uint8 alpha)
{
    sge_FilledTrigonAlpha(dest, x1, y1, x2, y2, x3, y3, SDL_MapRGB(dest->format, R, G, B), alpha);
}

//==================================================================================
// Draws a gourand shaded trigon
//==================================================================================
void sge_FadedTrigon(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint32 c1, Uint32 c2, Uint32 c3)
{
    Sint16 y;

    if(y1 == y3)
        return;

    Uint8 c = 0;
    SDL_Color col1;
    SDL_Color col2;
    SDL_Color col3;

    col1 = sge_GetRGB(dest, c1);
    col2 = sge_GetRGB(dest, c2);
    col3 = sge_GetRGB(dest, c3);

    /* Sort coords */
    if(y1 > y2)
    {
        SWAP(y1, y2, y);
        SWAP(x1, x2, y);
        SWAP(col1.r, col2.r, c);
        SWAP(col1.g, col2.g, c);
        SWAP(col1.b, col2.b, c);
    }
    if(y2 > y3)
    {
        SWAP(y2, y3, y);
        SWAP(x2, x3, y);
        SWAP(col2.r, col3.r, c);
        SWAP(col2.g, col3.g, c);
        SWAP(col2.b, col3.b, c);
    }
    if(y1 > y2)
    {
        SWAP(y1, y2, y);
        SWAP(x1, x2, y);
        SWAP(col1.r, col2.r, c);
        SWAP(col1.g, col2.g, c);
        SWAP(col1.b, col2.b, c);
    }

    /*
     * We trace three lines exactly like in sge_FilledTrigon(), but here we
     * must also keep track of the colors. We simply calculate how the color
     * will change along the three lines.
     */

    /* Starting coords for the three lines */
    auto xa = FixedPoint(x1);
    auto xb = xa;
    auto xc = FixedPoint(x2);

    /* Starting colors (rgb) for the three lines */
    auto r1 = FixedPoint(col1.r);
    auto r2 = r1;
    auto r3 = FixedPoint(col2.r);

    auto g1 = FixedPoint(col1.g);
    auto g2 = g1;
    auto g3 = FixedPoint(col2.g);

    auto b1 = FixedPoint(col1.b);
    auto b2 = b1;
    auto b3 = FixedPoint(col2.b);

    /* Lines step values */
    auto m2 = FixedPoint(x3 - x1) / Sint32(y3 - y1);

    /* Colors step values */
    auto rstep2 = FixedPoint(col3.r - col1.r) / Sint32(y3 - y1);

    auto gstep2 = FixedPoint(col3.g - col1.g) / Sint32(y3 - y1);

    auto bstep2 = FixedPoint(col3.b - col1.b) / Sint32(y3 - y1);

    if(_sge_lock && SDL_MUSTLOCK(dest))
        if(SDL_LockSurface(dest) < 0)
            return;

    /* Upper half of the triangle */
    if(y1 == y2)
        _FadedLine(dest, x1, x2, y1, r1, g1, b1, r3, g3, b3);
    else
    {
        auto m1 = (xc - xa) / Sint32(y2 - y1);

        auto rstep1 = (r3 - r1) / Sint32(y2 - y1);
        auto gstep1 = (g3 - g1) / Sint32(y2 - y1);
        auto bstep1 = (b3 - b1) / Sint32(y2 - y1);

        for(y = y1; y <= y2; y++)
        {
            _FadedLine(dest, xa.toInt(), xb.toInt(), y, r1, g1, b1, r2, g2, b2);

            xa += m1;
            xb += m2;

            r1 += rstep1;
            g1 += gstep1;
            b1 += bstep1;

            r2 += rstep2;
            g2 += gstep2;
            b2 += bstep2;
        }
    }

    /* Lower half of the triangle */
    if(y2 == y3)
        _FadedLine(dest, x2, x3, y2, r3, g3, b3, FixedPoint(col3.r), FixedPoint(col3.g), FixedPoint(col3.b));
    else
    {
        auto m3 = FixedPoint(x3 - x2) / Sint32(y3 - y2);

        auto rstep3 = FixedPoint(col3.r - col2.r) / Sint32(y3 - y2);
        auto gstep3 = FixedPoint(col3.g - col2.g) / Sint32(y3 - y2);
        auto bstep3 = FixedPoint(col3.b - col2.b) / Sint32(y3 - y2);

        for(y = y2 + 1; y <= y3; y++)
        {
            _FadedLine(dest, xb.toInt(), xc.toInt(), y, r2, g2, b2, r3, g3, b3);

            xb += m2;
            xc += m3;

            r2 += rstep2;
            g2 += gstep2;
            b2 += bstep2;

            r3 += rstep3;
            g3 += gstep3;
            b3 += bstep3;
        }
    }

    if(_sge_lock && SDL_MUSTLOCK(dest))
        SDL_UnlockSurface(dest);

    if(!_sge_update)
    {
        return;
    }

    Sint16 xmax = x1, xmin = x1;
    xmax = (xmax > x2) ? xmax : x2;
    xmin = (xmin < x2) ? xmin : x2;
    xmax = (xmax > x3) ? xmax : x3;
    xmin = (xmin < x3) ? xmin : x3;

    sge_UpdateRect(dest, xmin, y1, xmax - xmin + 1, y3 - y1 + 1);
}

//==================================================================================
// Draws a texured trigon (fast)
//==================================================================================
template<int srcBytesPerPixel, int dstBytesPerPixel>
static void _TexturedTrigon(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, SDL_Surface* source,
                            Sint16 sx1, Sint16 sy1, Sint16 sx2, Sint16 sy2, Sint16 sx3, Sint16 sy3)
{
    Sint16 y;

    if(y1 == y3)
        return;

    /* Sort coords */
    if(y1 > y2)
    {
        SWAP(y1, y2, y);
        SWAP(x1, x2, y);
        SWAP(sx1, sx2, y);
        SWAP(sy1, sy2, y);
    }
    if(y2 > y3)
    {
        SWAP(y2, y3, y);
        SWAP(x2, x3, y);
        SWAP(sx2, sx3, y);
        SWAP(sy2, sy3, y);
    }
    if(y1 > y2)
    {
        SWAP(y1, y2, y);
        SWAP(x1, x2, y);
        SWAP(sx1, sx2, y);
        SWAP(sy1, sy2, y);
    }
    {
        const auto maxX = sge_clip_xmax(dest);
        x1 = std::min<int>(x1, maxX);
        x2 = std::min<int>(x2, maxX);
        x3 = std::min<int>(x3, maxX);
    }
    const auto minY = sge_clip_ymin(dest);
    const auto maxY = sge_clip_ymax(dest);

    /*
     * Again we do the same thing as in sge_FilledTrigon(). But here we must keep track of how the
     * texture coords change along the lines.
     */

    /* Starting coords for the three lines */
    auto xa = FixedPoint(x1);
    auto xb = xa;
    auto xc = FixedPoint(x2);

    /* Lines step values */
    auto m2 = FixedPoint(x3 - x1) / Sint32(y3 - y1);

    /* Starting texture coords for the three lines */
    auto srcx1 = FixedPoint(sx1);
    auto srcx1_2 = srcx1;
    auto srcx2 = FixedPoint(sx2);

    auto srcy1 = FixedPoint(sy1);
    auto srcy1_2 = srcy1;
    auto srcy2 = FixedPoint(sy2);

    /* Texture coords stepping value */
    auto xstep2 = FixedPoint(sx3 - sx1) / Sint32(y3 - y1);
    auto ystep2 = FixedPoint(sy3 - sy1) / Sint32(y3 - y1);

    if(_sge_lock && SDL_MUSTLOCK(dest))
        if(SDL_LockSurface(dest) < 0)
            return;
    if(_sge_lock && SDL_MUSTLOCK(source))
        if(SDL_LockSurface(source) < 0)
            return;

    /* Upper half of the triangle */
    if(y1 == y2)
    {
        if(y1 >= minY && y1 <= maxY)
            _TexturedLine<srcBytesPerPixel, dstBytesPerPixel>(dest, x1, x2, y1, source, srcx1, srcy1, srcx2, srcy2);
    } else
    {
        auto m1 = (xc - xa) / Sint32(y2 - y1);

        auto xstep1 = (srcx2 - srcx1) / Sint32(y2 - y1);
        auto ystep1 = (srcy2 - srcy1) / Sint32(y2 - y1);

        for(y = y1; y <= std::min<int>(y2, maxY); y++)
        {
            if(y >= minY)
                _TexturedLine<srcBytesPerPixel, dstBytesPerPixel>(dest, xa.toInt(), xb.toInt(), y, source, srcx1, srcy1, srcx1_2, srcy1_2);

            xa += m1;
            xb += m2;

            srcx1 += xstep1;
            srcx1_2 += xstep2;
            srcy1 += ystep1;
            srcy1_2 += ystep2;
        }
    }

    /* Lower half of the triangle */
    if(y2 == y3)
    {
        if(y2 >= minY && y2 <= maxY)
            _TexturedLine<srcBytesPerPixel, dstBytesPerPixel>(dest, x2, x3, y2, source, srcx2, srcy2, FixedPoint(sx3), FixedPoint(sy3));
    } else
    {
        auto m3 = FixedPoint(x3 - x2) / Sint32(y3 - y2);

        auto xstep3 = FixedPoint(sx3 - sx2) / Sint32(y3 - y2);
        auto ystep3 = FixedPoint(sy3 - sy2) / Sint32(y3 - y2);

        for(y = y2 + 1; y <= std::min<int>(y3, maxY); y++)
        {
            if(y >= minY)
                _TexturedLine<srcBytesPerPixel, dstBytesPerPixel>(dest, xb.toInt(), xc.toInt(), y, source, srcx1_2, srcy1_2, srcx2, srcy2);

            xb += m2;
            xc += m3;

            srcx1_2 += xstep2;
            srcx2 += xstep3;
            srcy1_2 += ystep2;
            srcy2 += ystep3;
        }
    }

    if(_sge_lock && SDL_MUSTLOCK(dest))
        SDL_UnlockSurface(dest);
    if(_sge_lock && SDL_MUSTLOCK(source))
        SDL_UnlockSurface(source);

    if(!_sge_update)
    {
        return;
    }

    Sint16 xmax = x1, xmin = x1;
    xmax = (xmax > x2) ? xmax : x2;
    xmin = (xmin < x2) ? xmin : x2;
    xmax = (xmax > x3) ? xmax : x3;
    xmin = (xmin < x3) ? xmin : x3;

    sge_UpdateRect(dest, xmin, y1, numeric_cast<Uint16>(xmax - xmin + 1), numeric_cast<Uint16>(y3 - y1 + 1));
}

void sge_TexturedTrigon(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, SDL_Surface* source,
                        Sint16 sx1, Sint16 sy1, Sint16 sx2, Sint16 sy2, Sint16 sx3, Sint16 sy3)
{
    switch(dest->format->BytesPerPixel)
    {
        case 1:
            if(source->format->BytesPerPixel == 1)
                return _TexturedTrigon<1, 1>(dest, x1, y1, x2, y2, x3, y3, source, sx1, sy1, sx2, sy2, sx3, sy3);
            if(source->format->BytesPerPixel == 4)
                return _TexturedTrigon<4, 1>(dest, x1, y1, x2, y2, x3, y3, source, sx1, sy1, sx2, sy2, sx3, sy3);
            break;
        case 4:
            if(source->format->BytesPerPixel == 1)
                return _TexturedTrigon<1, 4>(dest, x1, y1, x2, y2, x3, y3, source, sx1, sy1, sx2, sy2, sx3, sy3);
            if(source->format->BytesPerPixel == 4)
                return _TexturedTrigon<4, 4>(dest, x1, y1, x2, y2, x3, y3, source, sx1, sy1, sx2, sy2, sx3, sy3);
            break;
    }
    assert(false);
}

//==================================================================================
// Draws a gouraud shaded and texured trigon (fast) (respecting colorkeys)
//==================================================================================
// Aditional args: isColorKey, (opt) Uint8 PreCalcPalettes[][256]
template<class... T_Args>
static void _FadedTexturedTrigonColorKeys(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3,
                                          SDL_Surface* source, Sint16 sx1, Sint16 sy1, Sint16 sx2, Sint16 sy2, Sint16 sx3, Sint16 sy3,
                                          Sint32 I1, Sint32 I2, Sint32 I3, T_Args... args)
{
    if(y1 == y3)
        return;

    /* Sort coords */
    if(y1 > y2)
    {
        Sint32 i = 0;
        Sint16 _tmp;
        SWAP(y1, y2, _tmp);
        SWAP(x1, x2, _tmp);
        SWAP(sx1, sx2, _tmp);
        SWAP(sy1, sy2, _tmp);
        SWAP(I1, I2, i);
    }
    if(y2 > y3)
    {
        Sint32 i = 0;
        Sint16 _tmp;
        SWAP(y2, y3, _tmp);
        SWAP(x2, x3, _tmp);
        SWAP(sx2, sx3, _tmp);
        SWAP(sy2, sy3, _tmp);
        SWAP(I2, I3, i);
    }
    if(y1 > y2)
    {
        Sint32 i = 0;
        Sint16 _tmp;
        SWAP(y1, y2, _tmp);
        SWAP(x1, x2, _tmp);
        SWAP(sx1, sx2, _tmp);
        SWAP(sy1, sy2, _tmp);
        SWAP(I1, I2, i);
    }
    {
        const auto maxX = sge_clip_xmax(dest);
        x1 = std::min<int>(x1, maxX);
        x2 = std::min<int>(x2, maxX);
        x3 = std::min<int>(x3, maxX);
    }
    const auto minY = sge_clip_ymin(dest);
    const auto maxY = sge_clip_ymax(dest);

    /*
     * Again we do the same thing as in sge_FilledTrigon(). But here we must keep track of how the
     * texture coords change along the lines.
     */

    /* Starting coords for the three lines */
    auto xa = FixedPoint(x1);
    auto xb = xa;
    auto xc = FixedPoint(x2);

    /* Starting colors (rgb) for the three lines */
    Sint32 i1 = I1;
    Sint32 i2 = i1;
    Sint32 i3 = I2;

    /* Lines step values */
    auto m2 = FixedPoint(x3 - x1) / Sint32(y3 - y1);

    /* Colors step values */
    Sint32 istep2 = (I3 - i1) / Sint32(y3 - y1);

    /* Starting texture coords for the three lines */
    auto srcx1 = FixedPoint(sx1);
    auto srcx1_2 = srcx1;
    auto srcx2 = FixedPoint(sx2);
    auto srcx3 = FixedPoint(sx3);

    auto srcy1 = FixedPoint(sy1);
    auto srcy1_2 = srcy1;
    auto srcy2 = FixedPoint(sy2);
    auto srcy3 = FixedPoint(sy3);

    /* Texture coords stepping value */
    auto xstep2 = (srcx3 - srcx1) / Sint32(y3 - y1);

    auto ystep2 = (srcy3 - srcy1) / Sint32(y3 - y1);

    if(_sge_lock && SDL_MUSTLOCK(dest))
        if(SDL_LockSurface(dest) < 0)
            return;
    if(_sge_lock && SDL_MUSTLOCK(source))
        if(SDL_LockSurface(source) < 0)
            return;

    /* Upper half of the triangle */
    if(y1 == y2)
    {
        if(y1 >= minY && y1 <= maxY)
            _FadedTexturedLine(dest, x1, x2, y1, source, srcx1, srcy1, srcx2, srcy2, I1, I2, args...);
    } else
    {
        auto m1 = (xc - xa) / Sint32(y2 - y1);

        auto istep1 = (I2 - I1) / Sint32(y2 - y1);

        auto xstep1 = (srcx2 - srcx1) / Sint32(y2 - y1);
        auto ystep1 = (srcy2 - srcy1) / Sint32(y2 - y1);

        for(int y = y1; y <= std::min<int>(y2, maxY); y++)
        {
            if(y >= minY)
                _FadedTexturedLine(dest, xa.toInt(), xb.toInt(), y, source, srcx1, srcy1, srcx1_2, srcy1_2, i1, i2, args...);

            xa += m1;
            xb += m2;

            i1 += istep1;

            i2 += istep2;

            srcx1 += xstep1;
            srcx1_2 += xstep2;
            srcy1 += ystep1;
            srcy1_2 += ystep2;
        }
    }

    /* Lower half of the triangle */
    if(y2 == y3)
    {
        if(y2 >= minY && y2 <= maxY)
            _FadedTexturedLine(dest, x2, x3, y2, source, srcx2, srcy2, srcx3, srcy3, I2, I3, args...);
    } else
    {
        auto m3 = FixedPoint(x3 - x2) / Sint32(y3 - y2);

        Sint32 istep3 = (I3 - I2) / Sint32(y3 - y2);

        auto xstep3 = (srcx3 - srcx2) / Sint32(y3 - y2);
        auto ystep3 = (srcy3 - srcy2) / Sint32(y3 - y2);

        for(int y = y2 + 1; y <= std::min<int>(y3, maxY); y++)
        {
            if(y >= minY)
                _FadedTexturedLine(dest, xb.toInt(), xc.toInt(), y, source, srcx1_2, srcy1_2, srcx2, srcy2, i2, i3, args...);

            xb += m2;
            xc += m3;

            i2 += istep2;

            i3 += istep3;

            srcx1_2 += xstep2;
            srcx2 += xstep3;
            srcy1_2 += ystep2;
            srcy2 += ystep3;
        }
    }

    if(_sge_lock && SDL_MUSTLOCK(dest))
        SDL_UnlockSurface(dest);
    if(_sge_lock && SDL_MUSTLOCK(source))
        SDL_UnlockSurface(source);

    if(!_sge_update)
    {
        return;
    }

    Sint16 xmax = x1, xmin = x1;
    xmax = (xmax > x2) ? xmax : x2;
    xmin = (xmin < x2) ? xmin : x2;
    xmax = (xmax > x3) ? xmax : x3;
    xmin = (xmin < x3) ? xmin : x3;

    sge_UpdateRect(dest, xmin, y1, numeric_cast<Uint16>(xmax - xmin + 1), numeric_cast<Uint16>(y3 - y1 + 1));
}

void sge_FadedTexturedTrigonColorKeys(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3,
                                      SDL_Surface* source, Sint16 sx1, Sint16 sy1, Sint16 sx2, Sint16 sy2, Sint16 sx3, Sint16 sy3,
                                      Sint32 I1, Sint32 I2, Sint32 I3, Uint32 keys[], int keycount)
{
    if(keycount == 0)
        _FadedTexturedTrigonColorKeys(dest, x1, y1, x2, y2, x3, y3, source, sx1, sy1, sx2, sy2, sx3, sy3, I1, I2, I3, makeIsColorKey());
    else if(keycount == 1)
        _FadedTexturedTrigonColorKeys(dest, x1, y1, x2, y2, x3, y3, source, sx1, sy1, sx2, sy2, sx3, sy3, I1, I2, I3,
                                      makeIsColorKey(keys[0]));
    else
        _FadedTexturedTrigonColorKeys(dest, x1, y1, x2, y2, x3, y3, source, sx1, sy1, sx2, sy2, sx3, sy3, I1, I2, I3,
                                      makeIsColorKey(keys, keycount));
}

void sge_FadedTexturedTrigon(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, SDL_Surface* source,
                             Sint16 sx1, Sint16 sy1, Sint16 sx2, Sint16 sy2, Sint16 sx3, Sint16 sy3, Sint32 I1, Sint32 I2, Sint32 I3)
{
    _FadedTexturedTrigonColorKeys(dest, x1, y1, x2, y2, x3, y3, source, sx1, sy1, sx2, sy2, sx3, sy3, I1, I2, I3, makeIsColorKey());
}

void sge_PreCalcFadedTexturedTrigonColorKeys(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3,
                                             SDL_Surface* source, Sint16 sx1, Sint16 sy1, Sint16 sx2, Sint16 sy2, Sint16 sx3, Sint16 sy3,
                                             Uint16 I1, Uint16 I2, Uint16 I3, Uint8 PreCalcPalettes[][256], Uint32 keys[], int keycount)
{
    if(keycount == 0)
        _FadedTexturedTrigonColorKeys(dest, x1, y1, x2, y2, x3, y3, source, sx1, sy1, sx2, sy2, sx3, sy3, I1, I2, I3, makeIsColorKey(),
                                      PreCalcPalettes);
    else if(keycount == 1)
        _FadedTexturedTrigonColorKeys(dest, x1, y1, x2, y2, x3, y3, source, sx1, sy1, sx2, sy2, sx3, sy3, I1, I2, I3,
                                      makeIsColorKey(keys[0]), PreCalcPalettes);
    else
        _FadedTexturedTrigonColorKeys(dest, x1, y1, x2, y2, x3, y3, source, sx1, sy1, sx2, sy2, sx3, sy3, I1, I2, I3,
                                      makeIsColorKey(keys, keycount), PreCalcPalettes);
}

void sge_PreCalcFadedTexturedTrigon(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3,
                                    SDL_Surface* source, Sint16 sx1, Sint16 sy1, Sint16 sx2, Sint16 sy2, Sint16 sx3, Sint16 sy3, Uint16 I1,
                                    Uint16 I2, Uint16 I3, Uint8 PreCalcPalettes[][256])
{
    _FadedTexturedTrigonColorKeys(dest, x1, y1, x2, y2, x3, y3, source, sx1, sy1, sx2, sy2, sx3, sy3, I1, I2, I3, makeIsColorKey(),
                                  PreCalcPalettes);
}

//==================================================================================
// Draws a texured *RECTANGLE*
//==================================================================================
template<int srcBytesPerPixel, int dstBytesPerPixel>
static void _TexturedRect(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Sint16 x4, Sint16 y4,
                          SDL_Surface* source, Sint16 sx1, Sint16 sy1, Sint16 sx2, Sint16 sy2, Sint16 sx3, Sint16 sy3, Sint16 sx4,
                          Sint16 sy4)
{
    Sint16 y;

    if(y1 == y3 || y1 == y4 || y4 == y2)
        return;

    /* Sort the coords */
    if(y1 > y2)
    {
        SWAP(x1, x2, y);
        SWAP(y1, y2, y);
        SWAP(sx1, sx2, y);
        SWAP(sy1, sy2, y);
    }
    if(y2 > y3)
    {
        SWAP(x3, x2, y);
        SWAP(y3, y2, y);
        SWAP(sx3, sx2, y);
        SWAP(sy3, sy2, y);
    }
    if(y1 > y2)
    {
        SWAP(x1, x2, y);
        SWAP(y1, y2, y);
        SWAP(sx1, sx2, y);
        SWAP(sy1, sy2, y);
    }
    if(y3 > y4)
    {
        SWAP(x3, x4, y);
        SWAP(y3, y4, y);
        SWAP(sx3, sx4, y);
        SWAP(sy3, sy4, y);
    }
    if(y2 > y3)
    {
        SWAP(x3, x2, y);
        SWAP(y3, y2, y);
        SWAP(sx3, sx2, y);
        SWAP(sy3, sy2, y);
    }
    if(y1 > y2)
    {
        SWAP(x1, x2, y);
        SWAP(y1, y2, y);
        SWAP(sx1, sx2, y);
        SWAP(sy1, sy2, y);
    }
    {
        const auto maxX = sge_clip_xmax(dest);
        x1 = std::min<int>(x1, maxX);
        x2 = std::min<int>(x2, maxX);
        x3 = std::min<int>(x3, maxX);
        x4 = std::min<int>(x4, maxX);
    }
    const auto minY = sge_clip_ymin(dest);
    const auto maxY = sge_clip_ymax(dest);

    /*
     * We do this exactly like sge_TexturedTrigon(), but here we must trace four lines.
     */

    auto xa = FixedPoint(x1);
    auto xb = xa;
    auto xc = FixedPoint(x2);
    auto xd = FixedPoint(x3);

    auto m2 = FixedPoint(x3 - x1) / Sint32(y3 - y1);
    auto m3 = FixedPoint(x4 - x2) / Sint32(y4 - y2);

    auto srcx1 = FixedPoint(sx1);
    auto srcx1_2 = srcx1;
    auto srcx2 = FixedPoint(sx2);
    auto srcx3 = FixedPoint(sx3);

    auto srcy1 = FixedPoint(sy1);
    auto srcy1_2 = srcy1;
    auto srcy2 = FixedPoint(sy2);
    auto srcy3 = FixedPoint(sy3);

    auto xstep2 = (srcx3 - srcx1) / Sint32(y3 - y1);
    auto xstep3 = FixedPoint(sx4 - sx2) / Sint32(y4 - y2);

    auto ystep2 = (srcy3 - srcy1) / Sint32(y3 - y1);
    auto ystep3 = FixedPoint(sy4 - sy2) / Sint32(y4 - y2);

    if(_sge_lock && SDL_MUSTLOCK(dest))
        if(SDL_LockSurface(dest) < 0)
            return;

    /* Upper bit of the rectangle */
    if(y1 == y2)
    {
        if(y1 >= minY && y1 <= maxY)
            _TexturedLine<srcBytesPerPixel, dstBytesPerPixel>(dest, x1, x2, y1, source, srcx1, srcy1, srcx2, srcy2);
    } else
    {
        auto m1 = (xc - xa) / Sint32(y2 - y1);

        auto xstep1 = (srcx2 - srcx1) / Sint32(y2 - y1);
        auto ystep1 = (srcy2 - srcy1) / Sint32(y2 - y1);

        for(y = y1; y <= std::min<int>(y2, maxY); y++)
        {
            if(y >= minY)
                _TexturedLine<srcBytesPerPixel, dstBytesPerPixel>(dest, xa.toInt(), xb.toInt(), y, source, srcx1, srcy1, srcx1_2, srcy1_2);

            xa += m1;
            xb += m2;

            srcx1 += xstep1;
            srcx1_2 += xstep2;
            srcy1 += ystep1;
            srcy1_2 += ystep2;
        }
    }

    /* Middle bit of the rectangle */
    for(y = y2 + 1; y <= std::min<int>(y3, maxY); y++)
    {
        if(y >= minY)
            _TexturedLine<srcBytesPerPixel, dstBytesPerPixel>(dest, xb.toInt(), xc.toInt(), y, source, srcx1_2, srcy1_2, srcx2, srcy2);

        xb += m2;
        xc += m3;

        srcx1_2 += xstep2;
        srcx2 += xstep3;
        srcy1_2 += ystep2;
        srcy2 += ystep3;
    }

    /* Lower bit of the rectangle */
    if(y3 == y4)
    {
        if(y3 >= minY && y3 <= maxY)
            _TexturedLine<srcBytesPerPixel, dstBytesPerPixel>(dest, x3, x4, y3, source, srcx3, srcy3, FixedPoint(sx4), FixedPoint(sy4));
    } else
    {
        auto m4 = FixedPoint(x4 - x3) / Sint32(y4 - y3);

        auto xstep4 = FixedPoint(sx4 - sx3) / Sint32(y4 - y3);
        auto ystep4 = FixedPoint(sy4 - sy3) / Sint32(y4 - y3);

        for(y = y3 + 1; y <= std::min<int>(y4, maxY); y++)
        {
            if(y >= minY)
                _TexturedLine<srcBytesPerPixel, dstBytesPerPixel>(dest, xc.toInt(), xd.toInt(), y, source, srcx2, srcy2, srcx3, srcy3);

            xc += m3;
            xd += m4;

            srcx2 += xstep3;
            srcx3 += xstep4;
            srcy2 += ystep3;
            srcy3 += ystep4;
        }
    }

    if(_sge_lock && SDL_MUSTLOCK(dest))
        SDL_UnlockSurface(dest);

    if(!_sge_update)
    {
        return;
    }

    Sint16 xmax = x1, xmin = x1;
    xmax = (xmax > x2) ? xmax : x2;
    xmin = (xmin < x2) ? xmin : x2;
    xmax = (xmax > x3) ? xmax : x3;
    xmin = (xmin < x3) ? xmin : x3;
    xmax = (xmax > x4) ? xmax : x4;
    xmin = (xmin < x4) ? xmin : x4;

    sge_UpdateRect(dest, xmin, y1, xmax - xmin + 1, y4 - y1 + 1);
}

void sge_TexturedRect(SDL_Surface* dest, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Sint16 x4, Sint16 y4,
                      SDL_Surface* source, Sint16 sx1, Sint16 sy1, Sint16 sx2, Sint16 sy2, Sint16 sx3, Sint16 sy3, Sint16 sx4, Sint16 sy4)
{
    switch(dest->format->BytesPerPixel)
    {
        case 1:
            if(source->format->BytesPerPixel == 4)
                return _TexturedRect<4, 1>(dest, x1, y1, x2, y2, x3, y3, x4, y4, source, sx1, sy1, sx2, sy2, sx3, sy3, sx4, sy4);
            if(source->format->BytesPerPixel == 1)
                return _TexturedRect<1, 1>(dest, x1, y1, x2, y2, x3, y3, x4, y4, source, sx1, sy1, sx2, sy2, sx3, sy3, sx4, sy4);
            break;
        case 4:
            if(source->format->BytesPerPixel == 1)
                return _TexturedRect<1, 4>(dest, x1, y1, x2, y2, x3, y3, x4, y4, source, sx1, sy1, sx2, sy2, sx3, sy3, sx4, sy4);
            if(source->format->BytesPerPixel == 4)
                return _TexturedRect<4, 4>(dest, x1, y1, x2, y2, x3, y3, x4, y4, source, sx1, sy1, sx2, sy2, sx3, sy3, sx4, sy4);
            break;
    }
    assert(false);
}

//==================================================================================
// And now to something completly different: Polygons!
//==================================================================================

/* Base polygon structure */
class pline
{
public:
    virtual ~pline() = default;
    pline* next;

    Sint16 x1, x2, y1, y2;

    FixedPoint fx, fm;

    Sint16 x;

    virtual void update()
    {
        x = fx.toInt();
        fx += fm;
    }
};

/* Pointer storage (to preserve polymorphism) */
struct pline_p
{
    pline* p;
};

/* Radix sort */
static pline* rsort(pline* inlist)
{
    if(!inlist)
        return nullptr;

    // 16 radix-buckets
    std::array<pline*, 16> bucket = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                     nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
    std::array<pline*, 16> bi; // bucket itterator (points to last element in bucket)

    pline* plist = inlist;

    int i, k;
    pline* j;
    Uint8 nr;

    // Radix sort in 4 steps (16-bit numbers)
    for(i = 0; i < 4; i++)
    {
        for(j = plist; j; j = j->next)
        {
            nr = Uint8((j->x >> (4 * i)) & 0x000F); // Get bucket number

            if(!bucket[nr])
                bucket[nr] = j; // First in bucket
            else
                bi[nr]->next = j; // Put last in bucket

            bi[nr] = j; // Update bucket itterator
        }

        // Empty buckets (recombine list)
        j = nullptr;
        for(k = 0; k < 16; k++)
        {
            if(bucket[k])
            {
                if(j)
                    j->next = bucket[k]; // Connect elements in buckets
                else
                    plist = bucket[k]; // First element

                j = bi[k];
            }
            bucket[k] = nullptr; // Empty
        }
        j->next = nullptr; // Terminate list
    }

    return plist;
}

/* Calculate the scanline for y */
static pline* get_scanline(pline_p* plist, Uint16 n, Sint32 y)
{
    pline* p = nullptr;
    pline* list = nullptr;
    pline* li = nullptr;

    for(int i = 0; i < n; i++)
    {
        // Is polyline on this scanline?
        p = plist[i].p;
        if(p->y1 <= y && p->y2 >= y && (p->y1 != p->y2))
        {
            if(list)
                li->next = p; // Add last in list
            else
                list = p; // Add first in list

            li = p; // Update itterator

            // Calculate x
            p->update();
        }
    }

    if(li)
        li->next = nullptr; // terminate

    // Sort list
    return rsort(list);
}

/* Removes duplicates if needed */
inline void remove_dup(pline* li, Sint16 y)
{
    if(li->next)
        if((y == li->y1 || y == li->y2) && (y == li->next->y1 || y == li->next->y2))
            if(((y == li->y1) ? -1 : 1) != ((y == li->next->y1) ? -1 : 1))
                li->next = li->next->next;
}

//==================================================================================
// Draws a n-points filled polygon
//==================================================================================

int sge_FilledPolygonAlpha(SDL_Surface* dest, Uint16 n, const Sint16* x, const Sint16* y, Uint32 color, Uint8 alpha)
{
    if(n < 3)
        return -1;

    if(_sge_lock && SDL_MUSTLOCK(dest))
        if(SDL_LockSurface(dest) < 0)
            return -2;

    auto* line = new pline[n];
    auto* plist = new pline_p[n];

    Sint16 y1, y2, x1, x2, tmp, sy;
    Sint16 ymin = y[1], ymax = y[1];
    Sint16 xmin = x[1], xmax = x[1];
    Uint16 i;

    /* Decompose polygon into straight lines */
    for(i = 0; i < n; i++)
    {
        y1 = y[i];
        x1 = x[i];

        if(i == n - 1)
        {
            // Last point == First point
            y2 = y[0];
            x2 = x[0];
        } else
        {
            y2 = y[i + 1];
            x2 = x[i + 1];
        }

        // Make sure y1 <= y2
        if(y1 > y2)
        {
            SWAP(y1, y2, tmp);
            SWAP(x1, x2, tmp);
        }

        // Reject polygons with negative coords
        if(y1 < 0 || x1 < 0 || x2 < 0)
        {
            if(_sge_lock && SDL_MUSTLOCK(dest))
                SDL_UnlockSurface(dest);

            delete[] line;
            delete[] plist;
            return -1;
        }

        if(y1 < ymin)
            ymin = y1;
        if(y2 > ymax)
            ymax = y2;
        if(x1 < xmin)
            xmin = x1;
        else if(x1 > xmax)
            xmax = x1;
        if(x2 < xmin)
            xmin = x2;
        else if(x2 > xmax)
            xmax = x2;

        // Fill structure
        line[i].y1 = y1;
        line[i].y2 = y2;
        line[i].x1 = x1;
        line[i].x2 = x2;

        // Start x-value (fixed point)
        line[i].fx = FixedPoint(x1);

        // Lines step value (fixed point)
        if(y1 != y2)
            line[i].fm = FixedPoint(x2 - x1) / Sint32(y2 - y1);
        else
            line[i].fm = FixedPoint(0);

        line[i].next = nullptr;

        // Add to list
        plist[i].p = &line[i];

        // Draw the polygon outline (looks nicer)
        if(alpha == SDL_ALPHA_OPAQUE)
            _Line(dest, x1, y1, x2, y2, color); // Can't do this with alpha, might overlap with the filling
    }

    /* Remove surface lock if _HLine() is to be used */
    if(_sge_lock && SDL_MUSTLOCK(dest) && alpha == SDL_ALPHA_OPAQUE)
        SDL_UnlockSurface(dest);

    pline* list = nullptr;
    pline* li = nullptr; // list itterator

    // Scan y-lines
    for(sy = ymin; sy <= ymax; sy++)
    {
        list = get_scanline(plist, n, sy);

        if(!list)
            continue; // nothing in list... hmmmm

        x1 = x2 = -1;

        // Draw horizontal lines between pairs
        for(li = list; li; li = li->next)
        {
            remove_dup(li, sy);

            if(x1 < 0)
                x1 = li->x + 1;
            else if(x2 < 0)
                x2 = li->x;

            if(x1 >= 0 && x2 >= 0)
            {
                if(x2 - x1 < 0 && alpha == SDL_ALPHA_OPAQUE)
                {
                    // Already drawn by the outline
                    x1 = x2 = -1;
                    continue;
                }

                if(alpha == SDL_ALPHA_OPAQUE)
                    _HLine(dest, x1, x2, sy, color);
                else
                    _HLineAlpha(dest, x1 - 1, x2, sy, color, alpha);

                x1 = x2 = -1;
            }
        }
    }

    if(_sge_lock && SDL_MUSTLOCK(dest) && alpha != SDL_ALPHA_OPAQUE)
        SDL_UnlockSurface(dest);

    delete[] line;
    delete[] plist;

    if(!_sge_update)
    {
        return 0;
    }

    sge_UpdateRect(dest, xmin, ymin, xmax - xmin + 1, ymax - ymin + 1);

    return 0;
}

int sge_FilledPolygonAlpha(SDL_Surface* dest, Uint16 n, Sint16* x, Sint16* y, Uint8 r, Uint8 g, Uint8 b, Uint8 alpha)
{
    return sge_FilledPolygonAlpha(dest, n, x, y, SDL_MapRGB(dest->format, r, g, b), alpha);
}

int sge_FilledPolygon(SDL_Surface* dest, Uint16 n, Sint16* x, Sint16* y, Uint32 color)
{
    return sge_FilledPolygonAlpha(dest, n, x, y, color, SDL_ALPHA_OPAQUE);
}

int sge_FilledPolygon(SDL_Surface* dest, Uint16 n, Sint16* x, Sint16* y, Uint8 r, Uint8 g, Uint8 b)
{
    return sge_FilledPolygonAlpha(dest, n, x, y, SDL_MapRGB(dest->format, r, g, b), SDL_ALPHA_OPAQUE);
}

//==================================================================================
// Draws a n-points (AA) filled polygon
//==================================================================================

int sge_AAFilledPolygon(SDL_Surface* dest, Uint16 n, const Sint16* x, const Sint16* y, Uint32 color)
{
    if(n < 3)
        return -1;

    if(_sge_lock && SDL_MUSTLOCK(dest))
        if(SDL_LockSurface(dest) < 0)
            return -2;

    auto* line = new pline[n];
    auto* plist = new pline_p[n];

    Sint16 y1, y2, x1, x2, tmp, sy;
    Sint16 ymin = y[1], ymax = y[1];
    Sint16 xmin = x[1], xmax = x[1];
    Uint16 i;

    /* Decompose polygon into straight lines */
    for(i = 0; i < n; i++)
    {
        y1 = y[i];
        x1 = x[i];

        if(i == n - 1)
        {
            // Last point == First point
            y2 = y[0];
            x2 = x[0];
        } else
        {
            y2 = y[i + 1];
            x2 = x[i + 1];
        }

        // Make sure y1 <= y2
        if(y1 > y2)
        {
            SWAP(y1, y2, tmp);
            SWAP(x1, x2, tmp);
        }

        // Reject polygons with negative coords
        if(y1 < 0 || x1 < 0 || x2 < 0)
        {
            if(_sge_lock && SDL_MUSTLOCK(dest))
                SDL_UnlockSurface(dest);

            delete[] line;
            delete[] plist;
            return -1;
        }

        if(y1 < ymin)
            ymin = y1;
        if(y2 > ymax)
            ymax = y2;
        if(x1 < xmin)
            xmin = x1;
        else if(x1 > xmax)
            xmax = x1;
        if(x2 < xmin)
            xmin = x2;
        else if(x2 > xmax)
            xmax = x2;

        // Fill structure
        line[i].y1 = y1;
        line[i].y2 = y2;
        line[i].x1 = x1;
        line[i].x2 = x2;

        // Start x-value (fixed point)
        line[i].fx = FixedPoint(x1);

        // Lines step value (fixed point)
        if(y1 != y2)
            line[i].fm = FixedPoint(x2 - x1) / Sint32(y2 - y1);
        else
            line[i].fm = FixedPoint(0);

        line[i].next = nullptr;

        // Add to list
        plist[i].p = &line[i];

        // Draw AA Line
        _AALineAlpha(dest, x1, y1, x2, y2, color, SDL_ALPHA_OPAQUE);
    }

    if(_sge_lock && SDL_MUSTLOCK(dest))
        SDL_UnlockSurface(dest);

    pline* list = nullptr;
    pline* li = nullptr; // list itterator

    // Scan y-lines
    for(sy = ymin; sy <= ymax; sy++)
    {
        list = get_scanline(plist, n, sy);

        if(!list)
            continue; // nothing in list... hmmmm

        x1 = x2 = -1;

        // Draw horizontal lines between pairs
        for(li = list; li; li = li->next)
        {
            remove_dup(li, sy);

            if(x1 < 0)
                x1 = li->x + 1;
            else if(x2 < 0)
                x2 = li->x;

            if(x1 >= 0 && x2 >= 0)
            {
                if(x2 - x1 < 0)
                {
                    x1 = x2 = -1;
                    continue;
                }

                _HLine(dest, x1, x2, sy, color);

                x1 = x2 = -1;
            }
        }
    }

    delete[] line;
    delete[] plist;

    if(!_sge_update)
    {
        return 0;
    }

    sge_UpdateRect(dest, xmin, ymin, xmax - xmin + 1, ymax - ymin + 1);

    return 0;
}

int sge_AAFilledPolygon(SDL_Surface* dest, Uint16 n, Sint16* x, Sint16* y, Uint8 r, Uint8 g, Uint8 b)
{
    return sge_AAFilledPolygon(dest, n, x, y, SDL_MapRGB(dest->format, r, g, b));
}

//==================================================================================
// Draws a n-points gourand shaded polygon
//==================================================================================

/* faded polygon structure */
class fpline final : public pline
{
public:
    Uint8 r1, r2;
    Uint8 g1, g2;
    Uint8 b1, b2;

    UFixedPoint fr, fg, fb;
    UFixedPoint fmr, fmg, fmb;

    Uint8 r, g, b;

    void update() override
    {
        x = fx.toInt();
        fx += fm;

        r = static_cast<Uint8>(fr.toUnsigned());
        g = static_cast<Uint8>(fg.toUnsigned());
        b = static_cast<Uint8>(fb.toUnsigned());

        fr += fmr;
        fg += fmg;
        fb += fmb;
    }
};

int sge_FadedPolygonAlpha(SDL_Surface* dest, Uint16 n, const Sint16* x, const Sint16* y, const Uint8* R, const Uint8* G, const Uint8* B,
                          Uint8 alpha)
{
    if(n < 3)
        return -1;

    if(_sge_lock && SDL_MUSTLOCK(dest))
        if(SDL_LockSurface(dest) < 0)
            return -2;

    auto* line = new fpline[n];
    auto* plist = new pline_p[n];

    Sint16 y1, y2, x1, x2, tmp, sy;
    Sint16 ymin = y[1], ymax = y[1];
    Sint16 xmin = x[1], xmax = x[1];
    Uint16 i;
    Uint8 r1 = 0, g1 = 0, b1 = 0, r2 = 0, g2 = 0, b2 = 0, t;

    // Decompose polygon into straight lines
    for(i = 0; i < n; i++)
    {
        y1 = y[i];
        x1 = x[i];
        r1 = R[i];
        g1 = G[i];
        b1 = B[i];

        if(i == n - 1)
        {
            // Last point == First point
            y2 = y[0];
            x2 = x[0];
            r2 = R[0];
            g2 = G[0];
            b2 = B[0];
        } else
        {
            y2 = y[i + 1];
            x2 = x[i + 1];
            r2 = R[i + 1];
            g2 = G[i + 1];
            b2 = B[i + 1];
        }

        // Make sure y1 <= y2
        if(y1 > y2)
        {
            SWAP(y1, y2, tmp);
            SWAP(x1, x2, tmp);
            SWAP(r1, r2, t);
            SWAP(g1, g2, t);
            SWAP(b1, b2, t);
        }

        // Reject polygons with negative coords
        if(y1 < 0 || x1 < 0 || x2 < 0)
        {
            if(_sge_lock && SDL_MUSTLOCK(dest))
                SDL_UnlockSurface(dest);

            delete[] line;
            delete[] plist;
            return -1;
        }

        if(y1 < ymin)
            ymin = y1;
        if(y2 > ymax)
            ymax = y2;
        if(x1 < xmin)
            xmin = x1;
        else if(x1 > xmax)
            xmax = x1;
        if(x2 < xmin)
            xmin = x2;
        else if(x2 > xmax)
            xmax = x2;

        // Fill structure
        line[i].y1 = y1;
        line[i].y2 = y2;
        line[i].x1 = x1;
        line[i].x2 = x2;
        line[i].r1 = r1;
        line[i].g1 = g1;
        line[i].b1 = b1;
        line[i].r2 = r2;
        line[i].g2 = g2;
        line[i].b2 = b2;

        // Start x-value (fixed point)
        line[i].fx = FixedPoint(x1);

        line[i].fr = UFixedPoint(r1);
        line[i].fg = UFixedPoint(g1);
        line[i].fb = UFixedPoint(b1);

        // Lines step value (fixed point)
        if(y1 != y2)
        {
            line[i].fm = FixedPoint(x2 - x1) / Sint32(y2 - y1);

            line[i].fmr = UFixedPoint(r2 - r1) / Uint32(y2 - y1);
            line[i].fmg = UFixedPoint(g2 - g1) / Uint32(y2 - y1);
            line[i].fmb = UFixedPoint(b2 - b1) / Uint32(y2 - y1);
        } else
        {
            line[i].fm = FixedPoint(0);
            line[i].fmr = UFixedPoint(0);
            line[i].fmg = UFixedPoint(0);
            line[i].fmb = UFixedPoint(0);
        }

        line[i].next = nullptr;

        // Add to list
        plist[i].p = &line[i];

        // Draw the polygon outline (looks nicer)
        if(alpha == SDL_ALPHA_OPAQUE)
            sge_DomcLine(dest, x1, y1, x2, y2, r1, g1, b1, r2, g2, b2,
                         _PutPixel); // Can't do this with alpha, might overlap with the filling
    }

    fpline* list = nullptr;
    fpline* li = nullptr; // list itterator

    // Scan y-lines
    for(sy = ymin; sy <= ymax; sy++)
    {
        list = (fpline*)get_scanline(plist, n, sy);

        if(!list)
            continue; // nothing in list... hmmmm

        x1 = x2 = -1;

        // Draw horizontal lines between pairs
        for(li = list; li; li = (fpline*)li->next)
        {
            remove_dup(li, sy);

            if(x1 < 0)
            {
                x1 = li->x + 1;
                r1 = li->r;
                g1 = li->g;
                b1 = li->b;
            } else if(x2 < 0)
            {
                x2 = li->x;
                r2 = li->r;
                g2 = li->g;
                b2 = li->b;
            }

            if(x1 >= 0 && x2 >= 0)
            {
                if(x2 - x1 < 0 && alpha == SDL_ALPHA_OPAQUE)
                {
                    x1 = x2 = -1;
                    continue;
                }

                if(alpha == SDL_ALPHA_OPAQUE)
                    _FadedLine(dest, x1, x2, sy, FixedPoint(r1), FixedPoint(g1), FixedPoint(b1), FixedPoint(r2), FixedPoint(g2),
                               FixedPoint(b2));
                else
                {
                    _sge_alpha_hack = alpha;
                    sge_DomcLine(dest, x1 - 1, sy, x2, sy, r1, g1, b1, r2, g2, b2, callback_alpha_hack);
                }

                x1 = x2 = -1;
            }
        }
    }

    if(_sge_lock && SDL_MUSTLOCK(dest))
        SDL_UnlockSurface(dest);

    delete[] line;
    delete[] plist;

    if(!_sge_update)
    {
        return 0;
    }

    sge_UpdateRect(dest, xmin, ymin, xmax - xmin + 1, ymax - ymin + 1);

    return 0;
}

int sge_FadedPolygon(SDL_Surface* dest, Uint16 n, Sint16* x, Sint16* y, Uint8* R, Uint8* G, Uint8* B)
{
    return sge_FadedPolygonAlpha(dest, n, x, y, R, G, B, SDL_ALPHA_OPAQUE);
}

//==================================================================================
// Draws a n-points (AA) gourand shaded polygon
//==================================================================================
int sge_AAFadedPolygon(SDL_Surface* dest, Uint16 n, const Sint16* x, const Sint16* y, const Uint8* R, const Uint8* G, const Uint8* B)
{
    if(n < 3)
        return -1;

    if(_sge_lock && SDL_MUSTLOCK(dest))
        if(SDL_LockSurface(dest) < 0)
            return -2;

    auto* line = new fpline[n];
    auto* plist = new pline_p[n];

    Sint16 y1, y2, x1, x2, tmp, sy;
    Sint16 ymin = y[1], ymax = y[1];
    Sint16 xmin = x[1], xmax = x[1];
    Uint16 i;
    Uint8 r1 = 0, g1 = 0, b1 = 0, r2 = 0, g2 = 0, b2 = 0, t;

    // Decompose polygon into straight lines
    for(i = 0; i < n; i++)
    {
        y1 = y[i];
        x1 = x[i];
        r1 = R[i];
        g1 = G[i];
        b1 = B[i];

        if(i == n - 1)
        {
            // Last point == First point
            y2 = y[0];
            x2 = x[0];
            r2 = R[0];
            g2 = G[0];
            b2 = B[0];
        } else
        {
            y2 = y[i + 1];
            x2 = x[i + 1];
            r2 = R[i + 1];
            g2 = G[i + 1];
            b2 = B[i + 1];
        }

        // Make sure y1 <= y2
        if(y1 > y2)
        {
            SWAP(y1, y2, tmp);
            SWAP(x1, x2, tmp);
            SWAP(r1, r2, t);
            SWAP(g1, g2, t);
            SWAP(b1, b2, t);
        }

        // Reject polygons with negative coords
        if(y1 < 0 || x1 < 0 || x2 < 0)
        {
            if(_sge_lock && SDL_MUSTLOCK(dest))
                SDL_UnlockSurface(dest);

            delete[] line;
            delete[] plist;
            return -1;
        }

        if(y1 < ymin)
            ymin = y1;
        if(y2 > ymax)
            ymax = y2;
        if(x1 < xmin)
            xmin = x1;
        else if(x1 > xmax)
            xmax = x1;
        if(x2 < xmin)
            xmin = x2;
        else if(x2 > xmax)
            xmax = x2;

        // Fill structure
        line[i].y1 = y1;
        line[i].y2 = y2;
        line[i].x1 = x1;
        line[i].x2 = x2;
        line[i].r1 = r1;
        line[i].g1 = g1;
        line[i].b1 = b1;
        line[i].r2 = r2;
        line[i].g2 = g2;
        line[i].b2 = b2;

        // Start x-value (fixed point)
        line[i].fx = FixedPoint(x1);

        line[i].fr = UFixedPoint(r1);
        line[i].fg = UFixedPoint(g1);
        line[i].fb = UFixedPoint(b1);

        // Lines step value (fixed point)
        if(y1 != y2)
        {
            line[i].fm = FixedPoint(x2 - x1) / Sint32(y2 - y1);

            line[i].fmr = UFixedPoint(r2 - r1) / Uint32(y2 - y1);
            line[i].fmg = UFixedPoint(g2 - g1) / Uint32(y2 - y1);
            line[i].fmb = UFixedPoint(b2 - b1) / Uint32(y2 - y1);
        } else
        {
            line[i].fm = FixedPoint(0);
            line[i].fmr = UFixedPoint(0);
            line[i].fmg = UFixedPoint(0);
            line[i].fmb = UFixedPoint(0);
        }

        line[i].next = nullptr;

        // Add to list
        plist[i].p = &line[i];

        // Draw the polygon outline (AA)
        _AAmcLineAlpha(dest, x1, y1, x2, y2, r1, g1, b1, r2, g2, b2, SDL_ALPHA_OPAQUE);
    }

    fpline* list = nullptr;
    fpline* li = nullptr; // list itterator

    // Scan y-lines
    for(sy = ymin; sy <= ymax; sy++)
    {
        list = (fpline*)get_scanline(plist, n, sy);

        if(!list)
            continue; // nothing in list... hmmmm

        x1 = x2 = -1;

        // Draw horizontal lines between pairs
        for(li = list; li; li = (fpline*)li->next)
        {
            remove_dup(li, sy);

            if(x1 < 0)
            {
                x1 = li->x + 1;
                r1 = li->r;
                g1 = li->g;
                b1 = li->b;
            } else if(x2 < 0)
            {
                x2 = li->x;
                r2 = li->r;
                g2 = li->g;
                b2 = li->b;
            }

            if(x1 >= 0 && x2 >= 0)
            {
                if(x2 - x1 < 0)
                {
                    x1 = x2 = -1;
                    continue;
                }

                _FadedLine(dest, x1, x2, sy, FixedPoint(r1), FixedPoint(g1), FixedPoint(b1), FixedPoint(r2), FixedPoint(g2),
                           FixedPoint(b2));

                x1 = x2 = -1;
            }
        }
    }

    if(_sge_lock && SDL_MUSTLOCK(dest))
        SDL_UnlockSurface(dest);

    delete[] line;
    delete[] plist;

    if(!_sge_update)
    {
        return 0;
    }

    sge_UpdateRect(dest, xmin, ymin, xmax - xmin + 1, ymax - ymin + 1);

    return 0;
}

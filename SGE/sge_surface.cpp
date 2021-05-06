// Copyright (C) 1999 - 2003 Anders Lindström
// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 *	SDL Graphics Extension
 *	Pixel, surface and color functions
 *
 *	Started 990815 (split from sge_draw 010611)
 */

/*
 *  Some of this code is taken from the "Introduction to SDL" and
 *  John Garrison's PowerPak
 */

#include "sge_surface.h"
#include <array>
#include <cstdarg>
#include <cstring>

/* Globals used for sge_Lock */
Uint8 _sge_lock = 1;

/**********************************************************************************/
/**                            Misc. functions                                   **/
/**********************************************************************************/

//==================================================================================
// Turns off automatic locking of surfaces
//==================================================================================
void sge_Lock_OFF()
{
    _sge_lock = 0;
}

//==================================================================================
// Turns on automatic locking (default)
//==================================================================================
void sge_Lock_ON()
{
    _sge_lock = 1;
}

//==================================================================================
// Returns locking mode (1-on and 0-off)
//==================================================================================
Uint8 sge_getLock()
{
    return _sge_lock;
}

//==================================================================================
// Creates a 32bit (8/8/8/8) alpha surface
// Map colors with sge_MapAlpha() and then use the Uint32 color versions of
// SGEs routines
//==================================================================================
SDL_Surface* sge_CreateAlphaSurface(Uint32 flags, int width, int height)
{
    return SDL_CreateRGBSurface(flags, width, height, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
}

//==================================================================================
// Returns the Uint32 color value for a 32bit (8/8/8/8) alpha surface
//==================================================================================
Uint32 sge_MapAlpha(Uint8 R, Uint8 G, Uint8 B, Uint8 A)
{
    Uint32 color = 0;

    color |= R << 24;
    color |= G << 16;
    color |= B << 8;
    color |= A;

    return color;
}

/**********************************************************************************/
/**                            Pixel functions                                   **/
/**********************************************************************************/

//==================================================================================
// Fast put pixel
//==================================================================================
void _PutPixel(SDL_Surface* surface, Sint16 x, Sint16 y, Uint32 color)
{
    if(x >= sge_clip_xmin(surface) && x <= sge_clip_xmax(surface) && y >= sge_clip_ymin(surface)
       && y <= sge_clip_ymax(surface))
    {
        switch(surface->format->BytesPerPixel)
        {
            case 1:
            { /* Assuming 8-bpp */
                *((Uint8*)surface->pixels + y * surface->pitch + x) = color;
            }
            break;

            case 2:
            { /* Probably 15-bpp or 16-bpp */
                *((Uint16*)surface->pixels + y * surface->pitch / 2 + x) = color;
            }
            break;

            case 3:
            { /* Slow 24-bpp mode, usually not used */
                Uint8* pix = (Uint8*)surface->pixels + y * surface->pitch + x * 3;

                /* Gack - slow, but endian correct */
                *(pix + surface->format->Rshift / 8) = color >> surface->format->Rshift;
                *(pix + surface->format->Gshift / 8) = color >> surface->format->Gshift;
                *(pix + surface->format->Bshift / 8) = color >> surface->format->Bshift;
                *(pix + surface->format->Ashift / 8) = color >> surface->format->Ashift;
            }
            break;

            case 4:
            { /* Probably 32-bpp */
                *((Uint32*)surface->pixels + y * surface->pitch / 4 + x) = color;
            }
            break;
        }
    }
}

//==================================================================================
// Fast put pixel (RGB)
//==================================================================================
void _PutPixel(SDL_Surface* surface, Sint16 x, Sint16 y, Uint8 R, Uint8 G, Uint8 B)
{
    _PutPixel(surface, x, y, SDL_MapRGB(surface->format, R, G, B));
}

//==================================================================================
// Fastest put pixel functions (don't mess up indata, thank you)
//==================================================================================
void _PutPixel8(SDL_Surface* surface, Sint16 x, Sint16 y, Uint32 color)
{
    *((Uint8*)surface->pixels + y * surface->pitch + x) = color;
}
void _PutPixel16(SDL_Surface* surface, Sint16 x, Sint16 y, Uint32 color)
{
    *((Uint16*)surface->pixels + y * surface->pitch / 2 + x) = color;
}
void _PutPixel24(SDL_Surface* surface, Sint16 x, Sint16 y, Uint32 color)
{
    Uint8* pix = (Uint8*)surface->pixels + y * surface->pitch + x * 3;

    /* Gack - slow, but endian correct */
    *(pix + surface->format->Rshift / 8) = color >> surface->format->Rshift;
    *(pix + surface->format->Gshift / 8) = color >> surface->format->Gshift;
    *(pix + surface->format->Bshift / 8) = color >> surface->format->Bshift;
    *(pix + surface->format->Ashift / 8) = color >> surface->format->Ashift;
}
void _PutPixel32(SDL_Surface* surface, Sint16 x, Sint16 y, Uint32 color)
{
    *((Uint32*)surface->pixels + y * surface->pitch / 4 + x) = color;
}
void _PutPixelX(SDL_Surface* dest, Sint16 x, Sint16 y, Uint32 color)
{
    switch(dest->format->BytesPerPixel)
    {
        case 1: *((Uint8*)dest->pixels + y * dest->pitch + x) = color; break;
        case 2: *((Uint16*)dest->pixels + y * dest->pitch / 2 + x) = color; break;
        case 3: _PutPixel24(dest, x, y, color); break;
        case 4: *((Uint32*)dest->pixels + y * dest->pitch / 4 + x) = color; break;
    }
}

//==================================================================================
// Safe put pixel
//==================================================================================
void sge_PutPixel(SDL_Surface* surface, Sint16 x, Sint16 y, Uint32 color)
{
    if(_sge_lock && SDL_MUSTLOCK(surface))
    {
        if(SDL_LockSurface(surface) < 0)
        {
            return;
        }
    }

    _PutPixel(surface, x, y, color);

    if(_sge_lock && SDL_MUSTLOCK(surface))
    {
        SDL_UnlockSurface(surface);
    }
}

//==================================================================================
// Safe put pixel (RGB)
//==================================================================================
void sge_PutPixel(SDL_Surface* surface, Sint16 x, Sint16 y, Uint8 R, Uint8 G, Uint8 B)
{
    sge_PutPixel(surface, x, y, SDL_MapRGB(surface->format, R, G, B));
}

//==================================================================================
// Calculate y pitch offset
// (the y pitch offset is constant for the same y coord and surface)
//==================================================================================
Sint32 sge_CalcYPitch(SDL_Surface* dest, Sint16 y)
{
    if(y >= sge_clip_ymin(dest) && y <= sge_clip_ymax(dest))
    {
        switch(dest->format->BytesPerPixel)
        {
            case 1: return y * dest->pitch; break;
            case 2: return y * dest->pitch / 2; break;
            case 3: return y * dest->pitch; break;
            case 4: return y * dest->pitch / 4; break;
        }
    }

    return -1;
}

//==================================================================================
// Put pixel with precalculated y pitch offset
//==================================================================================
void sge_pPutPixel(SDL_Surface* surface, Sint16 x, Sint32 ypitch, Uint32 color)
{
    if(x >= sge_clip_xmin(surface) && x <= sge_clip_xmax(surface) && ypitch >= 0)
    {
        switch(surface->format->BytesPerPixel)
        {
            case 1:
            { /* Assuming 8-bpp */
                *((Uint8*)surface->pixels + ypitch + x) = color;
            }
            break;

            case 2:
            { /* Probably 15-bpp or 16-bpp */
                *((Uint16*)surface->pixels + ypitch + x) = color;
            }
            break;

            case 3:
            { /* Slow 24-bpp mode, usually not used */
                /* Gack - slow, but endian correct */
                Uint8* pix = (Uint8*)surface->pixels + ypitch + x * 3;

                *(pix + surface->format->Rshift / 8) = color >> surface->format->Rshift;
                *(pix + surface->format->Gshift / 8) = color >> surface->format->Gshift;
                *(pix + surface->format->Bshift / 8) = color >> surface->format->Bshift;
                *(pix + surface->format->Ashift / 8) = color >> surface->format->Ashift;
            }
            break;

            case 4:
            { /* Probably 32-bpp */
                *((Uint32*)surface->pixels + ypitch + x) = color;
            }
            break;
        }
    }
}

//==================================================================================
// Get pixel
//==================================================================================
Uint32 sge_GetPixel(SDL_Surface* surface, Sint16 x, Sint16 y)
{
    if(x < 0 || x >= surface->w || y < 0 || y >= surface->h)
        return 0;

    switch(surface->format->BytesPerPixel)
    {
        case 1:
        { /* Assuming 8-bpp */
            return *((Uint8*)surface->pixels + y * surface->pitch + x);
        }
        break;

        case 2:
        { /* Probably 15-bpp or 16-bpp */
            return *((Uint16*)surface->pixels + y * surface->pitch / 2 + x);
        }
        break;

        case 3:
        { /* Slow 24-bpp mode, usually not used */
            Uint8* pix;
            int shift;
            Uint32 color = 0;

            pix = (Uint8*)surface->pixels + y * surface->pitch + x * 3;
            shift = surface->format->Rshift;
            color = *(pix + shift / 8) << shift;
            shift = surface->format->Gshift;
            color |= *(pix + shift / 8) << shift;
            shift = surface->format->Bshift;
            color |= *(pix + shift / 8) << shift;
            shift = surface->format->Ashift;
            color |= *(pix + shift / 8) << shift;
            return color;
        }
        break;

        case 4:
        { /* Probably 32-bpp */
            return *((Uint32*)surface->pixels + y * surface->pitch / 4 + x);
        }
        break;
    }
    return 0;
}

//==================================================================================
// Put pixel with alpha blending
//==================================================================================
void _PutPixelAlpha(SDL_Surface* surface, Sint16 x, Sint16 y, Uint32 color, Uint8 alpha)
{
    if(x >= sge_clip_xmin(surface) && x <= sge_clip_xmax(surface) && y >= sge_clip_ymin(surface)
       && y <= sge_clip_ymax(surface))
    {
        Uint32 Rmask = surface->format->Rmask, Gmask = surface->format->Gmask, Bmask = surface->format->Bmask,
               Amask = surface->format->Amask;
        Uint32 R, G, B, A = 0;

        switch(surface->format->BytesPerPixel)
        {
            case 1:
            { /* Assuming 8-bpp */
                if(alpha == 255)
                {
                    *((Uint8*)surface->pixels + y * surface->pitch + x) = color;
                } else
                {
                    Uint8* pixel = (Uint8*)surface->pixels + y * surface->pitch + x;

                    Uint8 dR = surface->format->palette->colors[*pixel].r;
                    Uint8 dG = surface->format->palette->colors[*pixel].g;
                    Uint8 dB = surface->format->palette->colors[*pixel].b;
                    Uint8 sR = surface->format->palette->colors[color].r;
                    Uint8 sG = surface->format->palette->colors[color].g;
                    Uint8 sB = surface->format->palette->colors[color].b;

                    dR = dR + (((sR - dR) * alpha) >> 8);
                    dG = dG + (((sG - dG) * alpha) >> 8);
                    dB = dB + (((sB - dB) * alpha) >> 8);

                    *pixel = SDL_MapRGB(surface->format, dR, dG, dB);
                }
            }
            break;

            case 2:
            { /* Probably 15-bpp or 16-bpp */
                if(alpha == 255)
                {
                    *((Uint16*)surface->pixels + y * surface->pitch / 2 + x) = color;
                } else
                {
                    Uint16* pixel = (Uint16*)surface->pixels + y * surface->pitch / 2 + x;
                    Uint32 dc = *pixel;

                    R = ((dc & Rmask) + ((((color & Rmask) - (dc & Rmask)) * alpha) >> 8)) & Rmask;
                    G = ((dc & Gmask) + ((((color & Gmask) - (dc & Gmask)) * alpha) >> 8)) & Gmask;
                    B = ((dc & Bmask) + ((((color & Bmask) - (dc & Bmask)) * alpha) >> 8)) & Bmask;
                    if(Amask)
                        A = ((dc & Amask) + ((((color & Amask) - (dc & Amask)) * alpha) >> 8)) & Amask;

                    *pixel = R | G | B | A;
                }
            }
            break;

            case 3:
            { /* Slow 24-bpp mode, usually not used */
                Uint8* pix = (Uint8*)surface->pixels + y * surface->pitch + x * 3;
                Uint8 rshift8 = surface->format->Rshift / 8;
                Uint8 gshift8 = surface->format->Gshift / 8;
                Uint8 bshift8 = surface->format->Bshift / 8;
                Uint8 ashift8 = surface->format->Ashift / 8;

                if(alpha == 255)
                {
                    *(pix + rshift8) = color >> surface->format->Rshift;
                    *(pix + gshift8) = color >> surface->format->Gshift;
                    *(pix + bshift8) = color >> surface->format->Bshift;
                    *(pix + ashift8) = color >> surface->format->Ashift;
                } else
                {
                    Uint8 dR, dG, dB, dA = 0;
                    Uint8 sR, sG, sB, sA = 0;

                    pix = (Uint8*)surface->pixels + y * surface->pitch + x * 3;

                    dR = *((pix) + rshift8);
                    dG = *((pix) + gshift8);
                    dB = *((pix) + bshift8);
                    dA = *((pix) + ashift8);

                    sR = (color >> surface->format->Rshift) & 0xff;
                    sG = (color >> surface->format->Gshift) & 0xff;
                    sB = (color >> surface->format->Bshift) & 0xff;
                    sA = (color >> surface->format->Ashift) & 0xff;

                    dR = dR + (((sR - dR) * alpha) >> 8);
                    dG = dG + (((sG - dG) * alpha) >> 8);
                    dB = dB + (((sB - dB) * alpha) >> 8);
                    dA = dA + (((sA - dA) * alpha) >> 8);

                    *((pix) + rshift8) = dR;
                    *((pix) + gshift8) = dG;
                    *((pix) + bshift8) = dB;
                    *((pix) + ashift8) = dA;
                }
            }
            break;

            case 4:
            { /* Probably 32-bpp */
                if(alpha == 255)
                {
                    *((Uint32*)surface->pixels + y * surface->pitch / 4 + x) = color;
                } else
                {
                    Uint32* pixel = (Uint32*)surface->pixels + y * surface->pitch / 4 + x;
                    Uint32 dc = *pixel;

                    R = ((dc & Rmask) + ((((color & Rmask) - (dc & Rmask)) * alpha) >> 8)) & Rmask;
                    G = ((dc & Gmask) + ((((color & Gmask) - (dc & Gmask)) * alpha) >> 8)) & Gmask;
                    B = ((dc & Bmask) + ((((color & Bmask) - (dc & Bmask)) * alpha) >> 8)) & Bmask;
                    if(Amask)
                        A = ((dc & Amask) + ((((color & Amask) - (dc & Amask)) * alpha) >> 8)) & Amask;

                    *pixel = R | G | B | A;
                }
            }
            break;
        }
    }
}

void sge_PutPixelAlpha(SDL_Surface* surface, Sint16 x, Sint16 y, Uint32 color, Uint8 alpha)
{
    if(_sge_lock && SDL_MUSTLOCK(surface))
        if(SDL_LockSurface(surface) < 0)
            return;

    _PutPixelAlpha(surface, x, y, color, alpha);

    /* unlock the display */
    if(_sge_lock && SDL_MUSTLOCK(surface))
    {
        SDL_UnlockSurface(surface);
    }
}

void _PutPixelAlpha(SDL_Surface* surface, Sint16 x, Sint16 y, Uint8 R, Uint8 G, Uint8 B, Uint8 alpha)
{
    _PutPixelAlpha(surface, x, y, SDL_MapRGB(surface->format, R, G, B), alpha);
}
void sge_PutPixelAlpha(SDL_Surface* surface, Sint16 x, Sint16 y, Uint8 R, Uint8 G, Uint8 B, Uint8 alpha)
{
    sge_PutPixelAlpha(surface, x, y, SDL_MapRGB(surface->format, R, G, B), alpha);
}

/**********************************************************************************/
/**                            Block functions                                   **/
/**********************************************************************************/

//==================================================================================
// The sge_write_block* functions copies the given block (a surface line) directly
// to the surface. This is *much* faster then using the put pixel functions to
// update a line. The block consist of Surface->w (the width of the surface) numbers
// of color values. Note the difference in byte size for the block elements for
// different color dephts. 24 bpp is slow and not included!
//==================================================================================
void sge_write_block8(SDL_Surface* Surface, Uint8* block, Sint16 y)
{
    memcpy((Uint8*)Surface->pixels + y * Surface->pitch, block, sizeof(Uint8) * Surface->w);
}
void sge_write_block16(SDL_Surface* Surface, Uint16* block, Sint16 y)
{
    memcpy((Uint16*)Surface->pixels + y * Surface->pitch / 2, block, sizeof(Uint16) * Surface->w);
}
void sge_write_block32(SDL_Surface* Surface, Uint32* block, Sint16 y)
{
    memcpy((Uint32*)Surface->pixels + y * Surface->pitch / 4, block, sizeof(Uint32) * Surface->w);
}

//==================================================================================
// ...and get
//==================================================================================
void sge_read_block8(SDL_Surface* Surface, Uint8* block, Sint16 y)
{
    memcpy(block, (Uint8*)Surface->pixels + y * Surface->pitch, sizeof(Uint8) * Surface->w);
}
void sge_read_block16(SDL_Surface* Surface, Uint16* block, Sint16 y)
{
    memcpy(block, (Uint16*)Surface->pixels + y * Surface->pitch / 2, sizeof(Uint16) * Surface->w);
}
void sge_read_block32(SDL_Surface* Surface, Uint32* block, Sint16 y)
{
    memcpy(block, (Uint32*)Surface->pixels + y * Surface->pitch / 4, sizeof(Uint32) * Surface->w);
}

/**********************************************************************************/
/**                       Blitting/surface functions                             **/
/**********************************************************************************/

//==================================================================================
// Clear surface to color
//==================================================================================
void sge_ClearSurface(SDL_Surface* Surface, Uint32 color)
{
    SDL_FillRect(Surface, nullptr, color);
}

//==================================================================================
// Clear surface to color (RGB)
//==================================================================================
void sge_ClearSurface(SDL_Surface* Surface, Uint8 R, Uint8 G, Uint8 B)
{
    sge_ClearSurface(Surface, SDL_MapRGB(Surface->format, R, G, B));
}

//==================================================================================
// Blit from one surface to another
// Warning! Alpha and color key is lost (=0) on Src surface
//==================================================================================
int sge_BlitTransparent(SDL_Surface* Src, SDL_Surface* Dest, Sint16 SrcX, Sint16 SrcY, Sint16 DestX, Sint16 DestY,
                        Sint16 W, Sint16 H, Uint32 Clear, Uint8 Alpha)
{
    SDL_Rect src, dest;
    int ret;

    /* Initialize our rectangles */
    src.x = SrcX;
    src.y = SrcY;
    src.w = W;
    src.h = H;

    dest.x = DestX;
    dest.y = DestY;
    dest.w = W;
    dest.h = H;

    /* Set the color to be transparent */
    SDL_SetColorKey(Src, SDL_TRUE, Clear);

    /* Set the alpha value */
    Uint8 oldAlpha;
    SDL_GetSurfaceAlphaMod(Src, &oldAlpha);
    SDL_SetSurfaceAlphaMod(Src, Alpha);

    /* Blit */
    ret = SDL_BlitSurface(Src, &src, Dest, &dest);

    /* Set normal levels */
    SDL_SetSurfaceAlphaMod(Src, oldAlpha);
    SDL_SetColorKey(Src, SDL_FALSE, 0);

    return ret;
}

//==================================================================================
// Blit from one surface to another (not touching alpha or color key -
// use SDL_SetColorKey and SDL_SetAlpha)
//==================================================================================
int sge_Blit(SDL_Surface* Src, SDL_Surface* Dest, Sint16 SrcX, Sint16 SrcY, Sint16 DestX, Sint16 DestY, Sint16 W,
             Sint16 H)
{
    SDL_Rect src, dest;
    int ret;

    /* Initialize our rectangles */
    src.x = SrcX;
    src.y = SrcY;
    src.w = W;
    src.h = H;

    dest.x = DestX;
    dest.y = DestY;
    dest.w = W;
    dest.h = H;

    /* Blit */
    ret = SDL_BlitSurface(Src, &src, Dest, &dest);

    return ret;
}

//==================================================================================
// Copies a surface to a new...
//==================================================================================
SDL_Surface* sge_copy_surface(SDL_Surface* src)
{
    return SDL_ConvertSurface(src, src->format, SDL_SWSURFACE);
}

/**********************************************************************************/
/**                            Palette functions                                 **/
/**********************************************************************************/
//==================================================================================
// Fill in a palette entry with R, G, B componenets
//==================================================================================
SDL_Color sge_FillPaletteEntry(Uint8 R, Uint8 G, Uint8 B)
{
    SDL_Color color;

    color.r = R;
    color.g = G;
    color.b = B;
    color.a = 0;

    return color;
}

//==================================================================================
// Get the RGB of a color value
// Needed in those dark days before SDL 1.0
//==================================================================================
SDL_Color sge_GetRGB(SDL_Surface* Surface, Uint32 Color)
{
    SDL_Color rgb;
    SDL_GetRGB(Color, Surface->format, &(rgb.r), &(rgb.g), &(rgb.b));

    return (rgb);
}

//==================================================================================
// Fades from (sR,sG,sB) to (dR,dG,dB), puts result in ctab[start] to ctab[stop]
//==================================================================================
void sge_Fader(SDL_Surface* Surface, Uint8 sR, Uint8 sG, Uint8 sB, Uint8 dR, Uint8 dG, Uint8 dB, Uint32* ctab,
               int start, int stop)
{
    // (sR,sG,sB) and (dR,dG,dB) are two points in space (the RGB cube).

    /* The vector for the straight line */
    std::array<int, 3> v;
    v[0] = dR - sR;
    v[1] = dG - sG;
    v[2] = dB - sB;

    /* Ref. point */
    int x0 = sR, y0 = sG, z0 = sB;

    // The line's equation is:
    // x= x0 + v[0] * t
    // y= y0 + v[1] * t
    // z= z0 + v[2] * t
    //
    // (x,y,z) will travel between the two points when t goes from 0 to 1.

    int i = start;
    double step = 1.0 / ((stop + 1) - start);

    for(double t = 0.0; t <= 1.0 && i <= stop; t += step)
    {
        ctab[i++] = SDL_MapRGB(Surface->format, (Uint8)(x0 + v[0] * t), (Uint8)(y0 + v[1] * t), (Uint8)(z0 + v[2] * t));
    }
}

//==================================================================================
// Fades from (sR,sG,sB,sA) to (dR,dG,dB,dA), puts result in ctab[start] to ctab[stop]
//==================================================================================
void sge_AlphaFader(Uint8 sR, Uint8 sG, Uint8 sB, Uint8 sA, Uint8 dR, Uint8 dG, Uint8 dB, Uint8 dA, Uint32* ctab,
                    int start, int stop)
{
    // (sR,sG,sB,sA) and (dR,dG,dB,dA) are two points in hyperspace (the RGBA hypercube).

    /* The vector for the straight line */
    std::array<int, 4> v;
    v[0] = dR - sR;
    v[1] = dG - sG;
    v[2] = dB - sB;
    v[3] = dA - sA;

    /* Ref. point */
    int x0 = sR, y0 = sG, z0 = sB, w0 = sA;

    // The line's equation is:
    // x= x0 + v[0] * t
    // y= y0 + v[1] * t
    // z= z0 + v[2] * t
    // w= w0 + v[3] * t
    //
    // (x,y,z,w) will travel between the two points when t goes from 0 to 1.

    int i = start;
    double step = 1.0 / ((stop + 1) - start);

    for(double t = 0.0; t <= 1.0 && i <= stop; t += step)
        ctab[i++] =
          sge_MapAlpha((Uint8)(x0 + v[0] * t), (Uint8)(y0 + v[1] * t), (Uint8)(z0 + v[2] * t), (Uint8)(w0 + v[3] * t));
}

//==================================================================================
// Copies a nice rainbow palette to the color table (ctab[start] to ctab[stop]).
// You must also set the intensity of the palette (0-bright 255-dark)
//==================================================================================
void sge_SetupRainbowPalette(SDL_Surface* Surface, Uint32* ctab, int intensity, int start, int stop)
{
    auto slice = (int)((stop - start) / 6);

    /* Red-Yellow */
    sge_Fader(Surface, 255, intensity, intensity, 255, 255, intensity, ctab, start, slice);
    /* Yellow-Green */
    sge_Fader(Surface, 255, 255, intensity, intensity, 255, intensity, ctab, slice + 1, 2 * slice);
    /* Green-Turquoise blue */
    sge_Fader(Surface, intensity, 255, intensity, intensity, 255, 255, ctab, 2 * slice + 1, 3 * slice);
    /* Turquoise blue-Blue */
    sge_Fader(Surface, intensity, 255, 255, intensity, intensity, 255, ctab, 3 * slice + 1, 4 * slice);
    /* Blue-Purple */
    sge_Fader(Surface, intensity, intensity, 255, 255, intensity, 255, ctab, 4 * slice + 1, 5 * slice);
    /* Purple-Red */
    sge_Fader(Surface, 255, intensity, 255, 255, intensity, intensity, ctab, 5 * slice + 1, stop);
}

//==================================================================================
// Copies a B&W palette to the color table (ctab[start] to ctab[stop]).
//==================================================================================
void sge_SetupBWPalette(SDL_Surface* Surface, Uint32* ctab, int start, int stop)
{
    sge_Fader(Surface, 0, 0, 0, 255, 255, 255, ctab, start, stop);
}

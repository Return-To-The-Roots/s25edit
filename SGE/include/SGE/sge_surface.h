// Copyright (C) 1999 - 2003 Anders Lindström
// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *	SDL Graphics Extension
 *	Pixel, surface and color functions (header)
 *
 *	Started 990815 (split from sge_draw 010611)
 */

#pragma once

#include "sge_internal.h"

/*
 *  Obsolete function names
 */
#define sge_copy_sblock8 sge_write_block8
#define sge_copy_sblock16 sge_write_block16
#define sge_copy_sblock32 sge_write_block32
#define sge_get_sblock8 sge_read_block8
#define sge_get_sblock16 sge_read_block16
#define sge_get_sblock32 sge_read_block32

#ifdef _SGE_C
extern "C"
{
#endif
    DECLSPEC void sge_Lock_OFF();
    DECLSPEC void sge_Lock_ON();
    DECLSPEC Uint8 sge_getLock();
    DECLSPEC SDL_Surface* sge_CreateAlphaSurface(Uint32 flags, int width, int height);
    DECLSPEC Uint32 sge_MapAlpha(Uint8 R, Uint8 G, Uint8 B, Uint8 A);

    DECLSPEC void _PutPixel(SDL_Surface* surface, Sint16 x, Sint16 y, Uint32 color);
    DECLSPEC void _PutPixel8(SDL_Surface* surface, Sint16 x, Sint16 y, Uint32 color);
    DECLSPEC void _PutPixel16(SDL_Surface* surface, Sint16 x, Sint16 y, Uint32 color);
    DECLSPEC void _PutPixel24(SDL_Surface* surface, Sint16 x, Sint16 y, Uint32 color);
    DECLSPEC void _PutPixel32(SDL_Surface* surface, Sint16 x, Sint16 y, Uint32 color);
    DECLSPEC void _PutPixelX(SDL_Surface* dest, Sint16 x, Sint16 y, Uint32 color);

    DECLSPEC Sint32 sge_CalcYPitch(SDL_Surface* dest, Sint16 y);
    DECLSPEC void sge_pPutPixel(SDL_Surface* surface, Sint16 x, Sint32 ypitch, Uint32 color);

    DECLSPEC void sge_PutPixel(SDL_Surface* surface, Sint16 x, Sint16 y, Uint32 color);
    DECLSPEC Uint32 sge_GetPixel(SDL_Surface* surface, Sint16 x, Sint16 y);

    DECLSPEC void _PutPixelAlpha(SDL_Surface* surface, Sint16 x, Sint16 y, Uint32 color, Uint8 alpha);
    DECLSPEC void sge_PutPixelAlpha(SDL_Surface* surface, Sint16 x, Sint16 y, Uint32 color, Uint8 alpha);

    DECLSPEC void sge_write_block8(SDL_Surface* Surface, Uint8* block, Sint16 y);
    DECLSPEC void sge_write_block16(SDL_Surface* Surface, Uint16* block, Sint16 y);
    DECLSPEC void sge_write_block32(SDL_Surface* Surface, Uint32* block, Sint16 y);
    DECLSPEC void sge_read_block8(SDL_Surface* Surface, Uint8* block, Sint16 y);
    DECLSPEC void sge_read_block16(SDL_Surface* Surface, Uint16* block, Sint16 y);
    DECLSPEC void sge_read_block32(SDL_Surface* Surface, Uint32* block, Sint16 y);

    DECLSPEC void sge_ClearSurface(SDL_Surface* Surface, Uint32 color);
    DECLSPEC int sge_BlitTransparent(SDL_Surface* Src, SDL_Surface* Dest, Sint16 SrcX, Sint16 SrcY, Sint16 DestX,
                                     Sint16 DestY, Sint16 W, Sint16 H, Uint32 Clear, Uint8 Alpha);
    DECLSPEC int sge_Blit(SDL_Surface* Src, SDL_Surface* Dest, Sint16 SrcX, Sint16 SrcY, Sint16 DestX, Sint16 DestY,
                          Sint16 W, Sint16 H);
    DECLSPEC SDL_Surface* sge_copy_surface(SDL_Surface* src);

    DECLSPEC SDL_Color sge_GetRGB(SDL_Surface* Surface, Uint32 Color);
    DECLSPEC SDL_Color sge_FillPaletteEntry(Uint8 R, Uint8 G, Uint8 B);
    DECLSPEC void sge_Fader(SDL_Surface* Surface, Uint8 sR, Uint8 sG, Uint8 sB, Uint8 dR, Uint8 dG, Uint8 dB,
                            Uint32* ctab, int start, int stop);
    DECLSPEC void sge_AlphaFader(Uint8 sR, Uint8 sG, Uint8 sB, Uint8 sA, Uint8 dR, Uint8 dG, Uint8 dB, Uint8 dA,
                                 Uint32* ctab, int start, int stop);
    DECLSPEC void sge_SetupRainbowPalette(SDL_Surface* Surface, Uint32* ctab, int intensity, int start, int stop);
    DECLSPEC void sge_SetupBWPalette(SDL_Surface* Surface, Uint32* ctab, int start, int stop);
#ifdef _SGE_C
}
#endif

#ifndef sge_C_ONLY
DECLSPEC void _PutPixel(SDL_Surface* surface, Sint16 x, Sint16 y, Uint8 R, Uint8 G, Uint8 B);
DECLSPEC void sge_PutPixel(SDL_Surface* surface, Sint16 x, Sint16 y, Uint8 R, Uint8 G, Uint8 B);
DECLSPEC void _PutPixelAlpha(SDL_Surface* surface, Sint16 x, Sint16 y, Uint8 R, Uint8 G, Uint8 B, Uint8 alpha);
DECLSPEC void sge_PutPixelAlpha(SDL_Surface* surface, Sint16 x, Sint16 y, Uint8 R, Uint8 G, Uint8 B, Uint8 alpha);
DECLSPEC void sge_ClearSurface(SDL_Surface* Surface, Uint8 R, Uint8 G, Uint8 B);
#endif /* sge_C_ONLY */

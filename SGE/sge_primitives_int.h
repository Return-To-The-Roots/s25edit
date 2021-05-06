// Copyright (C) 1999 - 2003 Anders Lindström
// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 *	SDL Graphics Extension
 *	Drawing primitives (header)
 *
 *	Started 990815 (split from sge_draw 010611)
 */

#pragma once

#include "sge_internal.h"
#include <SDL.h>

void _Line(SDL_Surface* surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
void _LineAlpha(SDL_Surface* Surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 Color, Uint8 alpha);
void _HLine(SDL_Surface* Surface, Sint16 x1, Sint16 x2, Sint16 y, Uint32 Color);
void _HLineAlpha(SDL_Surface* Surface, Sint16 x1, Sint16 x2, Sint16 y, Uint32 Color, Uint8 alpha);
void callback_alpha_hack(SDL_Surface* surf, Sint16 x, Sint16 y, Uint32 color);
void _AALineAlpha(SDL_Surface* dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color, Uint8 alpha);
void _AAmcLineAlpha(SDL_Surface* dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r1, Uint8 g1, Uint8 b1,
                    Uint8 r2, Uint8 g2, Uint8 b2, Uint8 alpha);

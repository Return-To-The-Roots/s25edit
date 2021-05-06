// Copyright (C) 1999 - 2003 Anders Lindström
// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 *	SDL Graphics Extension
 *	Rotation routines (header)
 *
 *	Started 000625
 */

#pragma once

#include "sge_internal.h"

/* Transformation flags */
#define SGE_TAA SGE_FLAG1
#define SGE_TSAFE SGE_FLAG2
#define SGE_TTMAP SGE_FLAG3

#ifdef _SGE_C
extern "C"
{
#endif
    DECLSPEC SDL_Rect sge_transform(SDL_Surface* src, SDL_Surface* dst, float angle, float xscale, float yscale,
                                    Uint16 px, Uint16 py, Uint16 qx, Uint16 qy, Uint8 flags);
    DECLSPEC SDL_Surface* sge_transform_surface(SDL_Surface* src, Uint32 bcol, float angle, float xscale, float yscale,
                                                Uint8 flags);

#ifdef _SGE_C
}
#endif

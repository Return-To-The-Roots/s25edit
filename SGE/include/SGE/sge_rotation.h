/*
 *	SDL Graphics Extension
 *	Rotation routines (header)
 *
 *	Started 000625
 *
 *	License: LGPL v2+ (see the file LICENSE)
 *	(c)1999-2003 Anders Lindström
 */

/*********************************************************************
 *  This library is free software; you can redistribute it and/or    *
 *  modify it under the terms of the GNU Library General Public      *
 *  License as published by the Free Software Foundation; either     *
 *  version 2 of the License, or (at your option) any later version. *
 *********************************************************************/

#ifndef sge_rotation_H
#define sge_rotation_H

#include "sge_internal.h"

/* Transformation flags */
#define SGE_TAA SGE_FLAG1
#define SGE_TSAFE SGE_FLAG2
#define SGE_TTMAP SGE_FLAG3

#ifdef _SGE_C
extern "C" {
#endif
DECLSPEC SDL_Rect sge_transform(SDL_Surface* src, SDL_Surface* dst, float angle, float xscale, float yscale, Uint16 px, Uint16 py,
                                Uint16 qx, Uint16 qy, Uint8 flags);
DECLSPEC SDL_Surface* sge_transform_surface(SDL_Surface* src, Uint32 bcol, float angle, float xscale, float yscale, Uint8 flags);

#ifdef _SGE_C
}
#endif

#endif /* sge_rotation_H */

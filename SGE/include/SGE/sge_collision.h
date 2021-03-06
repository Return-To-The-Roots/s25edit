// Copyright (C) 1999 - 2003 Anders Lindström
// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 *	SDL Graphics Extension
 *	Collision routines (header)
 *
 *	Started 000625
 */

#pragma once

#include "sge_internal.h"

/* The collision struct */
struct sge_cdata
{
    Uint8* map;
    Uint16 w, h;
};

#ifdef _SGE_C
extern "C"
{
#endif
    DECLSPEC sge_cdata* sge_make_cmap(SDL_Surface* img);
    DECLSPEC int sge_bbcheck(sge_cdata* cd1, Sint16 x1, Sint16 y1, sge_cdata* cd2, Sint16 x2, Sint16 y2);
    DECLSPEC int _sge_bbcheck(Sint16 x1, Sint16 y1, Sint16 w1, Sint16 h1, Sint16 x2, Sint16 y2, Sint16 w2, Sint16 h2);
    DECLSPEC int _sge_cmcheck(sge_cdata* cd1, Sint16 x1, Sint16 y1, sge_cdata* cd2, Sint16 x2, Sint16 y2);
    DECLSPEC int sge_cmcheck(sge_cdata* cd1, Sint16 x1, Sint16 y1, sge_cdata* cd2, Sint16 x2, Sint16 y2);
    DECLSPEC Sint16 sge_get_cx();
    DECLSPEC Sint16 sge_get_cy();
    DECLSPEC void sge_destroy_cmap(sge_cdata* cd);
    DECLSPEC void sge_unset_cdata(sge_cdata* cd, Sint16 x, Sint16 y, Sint16 w, Sint16 h);
    DECLSPEC void sge_set_cdata(sge_cdata* cd, Sint16 x, Sint16 y, Sint16 w, Sint16 h);
#ifdef _SGE_C
}
#endif

#ifndef _SGE_NO_CLASSES
class DECLSPEC sge_shape;
DECLSPEC int sge_bbcheck_shape(sge_shape* shape1, sge_shape* shape2);
#endif /* _SGE_NO_CLASSES */

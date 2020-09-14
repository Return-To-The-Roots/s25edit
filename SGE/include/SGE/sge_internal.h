/*
 *	SDL Graphics Extension
 *	SGE internal header
 *
 *	Started 000627
 *
 *	License: LGPL v2+ (see the file LICENSE)
 *	(c)2000-2003 Anders Lindström
 */

/*********************************************************************
 *  This library is free software; you can redistribute it and/or    *
 *  modify it under the terms of the GNU Library General Public      *
 *  License as published by the Free Software Foundation; either     *
 *  version 2 of the License, or (at your option) any later version. *
 *********************************************************************/

#pragma once

/* This header is included in all sge_*.h files */

#include "sge_config.h"
#include <SDL.h>

/*
 *  C compatibility
 *  Thanks to Ohbayashi Ippei (ohai@kmc.gr.jp) for this clever hack!
 */
#ifdef _SGE_C_AND_CPP
#ifdef __cplusplus
#define _SGE_C /* use extern "C" on base functions */
#include <type_traits>
template<typename T>
constexpr auto absDiff(T a, T b)
{
    using U = std::make_unsigned_t<T>;
    return static_cast<U>(a > b ? a - b : b - a);
}
#else
#define sge_C_ONLY      /* remove overloaded functions */
#define _SGE_NO_CLASSES /* no C++ classes */
#endif
#endif

/*
 *  Bit flags
 */
#define SGE_FLAG0 0x00
#define SGE_FLAG1 0x01
#define SGE_FLAG2 0x02
#define SGE_FLAG3 0x04
#define SGE_FLAG4 0x08
#define SGE_FLAG5 0x10
#define SGE_FLAG6 0x20
#define SGE_FLAG7 0x40
#define SGE_FLAG8 0x80

/*
 *  Macro to get clipping
 */
inline auto sge_clip_xmin(const SDL_Surface* pnt)
{
    return pnt->clip_rect.x;
}
inline auto sge_clip_xmax(const SDL_Surface* pnt)
{
    return (pnt->clip_rect.x + pnt->clip_rect.w - 1);
}
inline auto sge_clip_ymin(const SDL_Surface* pnt)
{
    return pnt->clip_rect.y;
}
inline auto sge_clip_ymax(const SDL_Surface* pnt)
{
    return (pnt->clip_rect.y + pnt->clip_rect.h - 1);
}

/*
 *  Some compilers use a special export keyword
 *  Thanks to Seung Chan Lim (limsc@maya.com or slim@djslim.com) to pointing this out
 *  (From SDL)
 */
#ifndef DECLSPEC
#ifdef __BEOS__
#if defined(__GNUC__)
#define DECLSPEC __declspec(dllexport)
#else
#define DECLSPEC __declspec(export)
#endif
#elif defined(WIN32)
#define DECLSPEC __declspec(dllexport)
#else
#define DECLSPEC
#endif
#endif

#ifdef __GNUC__
#define SGE_ATTRIBUTE_FORMAT(fmtStringIdx, firstArgIdx) __attribute__((format(printf, fmtStringIdx, firstArgIdx)))
#else
#define SGE_ATTRIBUTE_FORMAT(fmtStringIdx, firstArgIdx)
#endif

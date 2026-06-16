// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <SDL.h>
#include <array>
#include <memory>

struct SdlSurfaceDeleter
{
    void operator()(SDL_Surface* p) { SDL_FreeSurface(p); }
};
struct SDLWindowDestroyer
{
    void operator()(SDL_Window* p) const { SDL_DestroyWindow(p); }
};

using SdlWindow = std::unique_ptr<SDL_Window, SDLWindowDestroyer>;
using SdlSurface = std::unique_ptr<SDL_Surface, SdlSurfaceDeleter>;

inline SdlSurface makeRGBSurface(int width, int height, bool withAlpha = false)
{
    return SdlSurface(SDL_CreateRGBSurface(0, width, height, 32, 0xFF0000, 0xFF00, 0xFF, withAlpha ? 0xFF000000 : 0));
}
inline SdlSurface makePalSurface(int width, int height, const std::array<SDL_Color, 256>& palette)
{
    auto surf = SdlSurface(SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0));
    if(surf)
        SDL_SetPaletteColors(surf->format->palette, palette.data(), 0, palette.size());
    return surf;
}

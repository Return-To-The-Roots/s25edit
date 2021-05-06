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
struct SdlTextureDeleter
{
    void operator()(SDL_Texture* p) { SDL_DestroyTexture(p); }
};
struct SDLWindowDestroyer
{
    void operator()(SDL_Window* p) const { SDL_DestroyWindow(p); }
};
struct SDLRendererDestroyer
{
    void operator()(SDL_Renderer* p) const { SDL_DestroyRenderer(p); }
};

using SdlRenderer = std::unique_ptr<SDL_Renderer, SDLRendererDestroyer>;
using SdlWindow = std::unique_ptr<SDL_Window, SDLWindowDestroyer>;
using SdlSurface = std::unique_ptr<SDL_Surface, SdlSurfaceDeleter>;
using SdlTexture = std::unique_ptr<SDL_Texture, SdlTextureDeleter>;

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

inline SdlTexture makeSdlTexture(const SdlRenderer& renderer, Uint32 format, int access, int w, int h)
{
    return SdlTexture(SDL_CreateTexture(renderer.get(), format, access, w, h));
}

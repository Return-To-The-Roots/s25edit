// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Rect.h"
#include <SDL.h>

/// Wraps a GL texture with RAII and provides draw methods.
class GlTexture
{
public:
    GlTexture() = default;
    ~GlTexture();

    GlTexture(GlTexture&&) noexcept;
    GlTexture& operator=(GlTexture&&) noexcept;

    // No copy
    GlTexture(const GlTexture&) = delete;
    GlTexture& operator=(const GlTexture&) = delete;

    /// Load from a 32-bit or 8-bit paletted SDL surface.
    /// For 8-bit surfaces, optional colorkey is respected (keyed pixels become transparent).
    bool load(SDL_Surface* surface, bool linear = false);

    /// Draw the texture stretched to fill the given rect.
    void DrawFull(const Rect& destRect) const;

    /// Draw the texture at native size at the given position.
    void Draw(int x, int y) const;

private:
    unsigned int texture_ = 0;
    int width_ = 0, height_ = 0;

    void createTexture(const void* bgraPixels, int w, int h, bool linear);
};

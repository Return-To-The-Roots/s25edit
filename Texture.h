// Copyright (C) 2026 - 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Rect.h"
#include <Point.h>
#include <SDL.h>

/// Wraps a GL texture with RAII and provides draw methods.
class Texture
{
public:
    Texture() = default;
    ~Texture();

    Texture(Texture&&) noexcept;
    Texture& operator=(Texture&&) noexcept;

    // No copy
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    /// Load from a 32-bit or 8-bit paletted SDL surface.
    /// For 8-bit surfaces, optional colorkey is respected (keyed pixels become transparent).
    bool load(SDL_Surface* surface, bool filterLinear = false);

    /// Create an empty texture of the given size (for use as a render-target).
    void createEmpty(Extent size, bool filterLinear = false);

    /// Upload new pixel data to an existing texture (glTexSubImage2D).
    void upload(const void* bgraPixels);

    /// Draw the texture stretched to fill the given rect.
    void Draw(const Rect& destRect) const;

    /// Draw the texture at native size at the given position.
    void Draw(Position pos) const;

    /// Convenience overload for callers using separate x/y.
    void Draw(int x, int y) const { Draw(Position(x, y)); }

    /// Returns the raw GL texture name (for use with glBindTexture).
    unsigned getHandle() const { return texture_; }

    /// Width in pixels.
    int getWidth() const { return size_.x; }

    /// Height in pixels.
    int getHeight() const { return size_.y; }

    /// Returns true if the texture has been created.
    bool isValid() const { return texture_ != 0; }

private:
    unsigned int texture_ = 0;
    Extent size_;

    /// Internal: create or recreate texture from raw BGRA pixel data.
    void load(const void* bgraPixels, Extent size, bool filterLinear);
};

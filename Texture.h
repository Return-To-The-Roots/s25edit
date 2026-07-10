// Copyright (C) 2026 - 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ArchiveID.h"
#include "Rect.h"
#include <glad/glad.h>
#include <Point.h>
#include <SDL.h>
#include <memory>
#include <vector>

namespace libsiedler2 {
class baseArchivItem_Bitmap;
} // namespace libsiedler2

/// Wraps a texture with RAII and provides draw methods.
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

    /// Load from a libsiedler2 bitmap (paletted or BGRA).
    bool load(const libsiedler2::baseArchivItem_Bitmap& bitmap);

    /// Load raw BGRA pixel data directly.
    void load(const uint8_t* bgraPixels, Extent size);

    /// Create an empty texture of the given size (for use as a render-target).
    void createEmpty(Extent size);

    /// Upload new pixel data to an existing texture (glTexSubImage2D).
    void upload(const void* bgraPixels);

    /// Draw the texture stretched to fill the given rect.
    void draw(const Rect& destRect) const;

    /// Draw the texture at native size at the given position.
    void draw(Position pos) const;

    /// Tile the texture to fill the given rectangle.
    void drawTiled(const Rect& destRect) const;

    /// Size in pixels.
    Extent getSize() const { return size_; }

    /// Draw a sub-rect of the texture stretched to fill the given dest rect.
    void draw(const Rect& destRect, const Rect& srcRect) const;

    /// Returns true if the texture has been created.
    bool isValid() const { return texture_ != 0; }

    /// Draw at native size at the given position adjusted by the sprite anchor.
    void drawSprite(Position pos) const;

    /// Sprite anchor offset (set by getTexture).
    Position anchor() const { return anchor_; }

    /// Direct handle access for manual GL ops.
    GLuint getHandle() const { return texture_; }

    // Static bitmap-texture cache

    /// Return (or create on first use) a cached GL texture from a typed archive.
    static Texture& getTexture(ArchiveID archive, int index);

private:
    GLuint texture_ = 0;
    Extent size_;
    Position anchor_ = {0, 0}; // sprite anchor offset, set by getTexture
};

/// Draw a filled rectangle with a 32-bit ARGB colour.
void drawRect(const Rect& rect, unsigned color);

/// Draw a 3D-style button box: tiled background, 2px black frame (sunken if pressed, raised otherwise),
/// and tiled foreground inset by 2px. All button textures from EDITIO.IDX.
void drawButtonBox(const Rect& area, bool pressed, unsigned baseTex, unsigned faceTex);

/// Get or create the cached OpenGL texture from a typed archive.
Texture& getTexture(ArchiveID archive, int index);

// Copyright (C) 2025 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Point.h"
#include "Rect.h"
#include "defines.h"
#include <glad/glad.h>
#include <SDL.h>
#include <array>
#include <memory>
#include <unordered_map>
#include <vector>

class EditorGLTexture
{
public:
    EditorGLTexture() : texture_(0), width_(0), height_(0), texWidth_(0), texHeight_(0) {}
    ~EditorGLTexture() { destroy(); }

    EditorGLTexture(const EditorGLTexture&) = delete;
    EditorGLTexture& operator=(const EditorGLTexture&) = delete;

    EditorGLTexture(EditorGLTexture&& other) noexcept
        : texture_(other.texture_), width_(other.width_), height_(other.height_), texWidth_(other.texWidth_),
          texHeight_(other.texHeight_)
    {
        other.texture_ = 0;
    }

    EditorGLTexture& operator=(EditorGLTexture&& other) noexcept
    {
        if(this != &other)
        {
            destroy();
            texture_ = other.texture_;
            width_ = other.width_;
            height_ = other.height_;
            texWidth_ = other.texWidth_;
            texHeight_ = other.texHeight_;
            other.texture_ = 0;
        }
        return *this;
    }

    bool createFromSurface(SDL_Surface* surface);
    bool updateFromSurface(SDL_Surface* surface);
    void destroy();

    void draw(int dstX, int dstY) const;

    bool valid() const { return texture_ != 0; }
    GLuint texture() const { return texture_; }
    int width() const { return width_; }
    int height() const { return height_; }
    int texWidth() const { return texWidth_; }
    int texHeight() const { return texHeight_; }

private:
    bool convertSurface(SDL_Surface* surface);

    GLuint texture_;
    int width_;
    int height_;
    int texWidth_;
    int texHeight_;
    std::vector<uint32_t> pixelBuffer_;
};

namespace CSurfaceGL {

/// Initialise OpenGL state (viewport, blending, etc.) with the given
/// screen resolution for the default orthographic projection.
/// Call once after GL context creation.
void init(int screenWidth, int screenHeight);

/// Get-or-create a cached GL texture for an SDL surface.
const EditorGLTexture& getOrCreateTexture(SDL_Surface* surface);

} // namespace CSurfaceGL

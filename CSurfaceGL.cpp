// Copyright (C) 2025 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CSurfaceGL.h"
#include "Rect.h"
#include "TerrainRenderer.h"
#include "ogl/VBO.h"
#include "ogl/constants.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <emmintrin.h>

namespace {

// ---------------------------------------------------------------------------
bool isArgb32Format(const SDL_PixelFormat* fmt)
{
    return fmt->BitsPerPixel == 32 && fmt->Rmask == 0xFF0000 && fmt->Gmask == 0xFF00 && fmt->Bmask == 0xFF
           && fmt->BytesPerPixel == 4;
}

// ===========================================================================
// Texture cache  (SDL_Surface -> GL texture)
// ===========================================================================

static std::unordered_map<SDL_Surface*, EditorGLTexture> s_texCache;

} // anonymous namespace

// ===========================================================================
// EditorGLTexture
// ===========================================================================

bool EditorGLTexture::convertSurface(SDL_Surface* surface)
{
    int w = surface->w, h = surface->h, needed = w * h;
    if(int(pixelBuffer_.size()) != needed)
        pixelBuffer_.assign(needed, 0);
    Uint32 ck;
    bool hasCK = (SDL_GetColorKey(surface, &ck) == 0);

    if(isArgb32Format(surface->format) && !hasCK)
    {
        auto* src = (const uint32_t*)surface->pixels;
        auto* dst = pixelBuffer_.data();
        for(int i = 0; i < needed; i++)
        {
            uint32_t p = src[i];
            dst[i] = p ? (p | 0xFF000000) : 0;
        }
        return true;
    }

    if(surface->format->BitsPerPixel == 8)
    {
        SDL_Palette* pal = surface->format->palette;
        if(!pal)
            return false;
        for(int y = 0; y < h; y++)
        {
            auto* src = (const uint8_t*)((const uint8_t*)surface->pixels + y * surface->pitch);
            for(int x = 0; x < w; x++)
            {
                uint8_t idx = src[x];
                if(hasCK && idx == (ck & 0xFF))
                {
                    pixelBuffer_[y * w + x] = 0;
                    continue;
                }
                SDL_Color c = pal->colors[idx];
                pixelBuffer_[y * w + x] = (0xFF << 24) | (c.r << 16) | (c.g << 8) | c.b;
            }
        }
    } else
    {
        SDL_PixelFormat* fmt = surface->format;
        for(int y = 0; y < h; y++)
        {
            auto* src = (const uint32_t*)((const uint8_t*)surface->pixels + y * surface->pitch);
            for(int x = 0; x < w; x++)
            {
                uint32_t p = src[x];
                if(hasCK && p == ck)
                {
                    pixelBuffer_[y * w + x] = 0;
                    continue;
                }
                Uint8 r, g, b, a;
                SDL_GetRGBA(p, fmt, &r, &g, &b, &a);
                pixelBuffer_[y * w + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
            }
        }
    }
    return true;
}

bool EditorGLTexture::createFromSurface(SDL_Surface* s)
{
    destroy();
    if(!s)
        return false;
    width_ = s->w;
    height_ = s->h;
    if(!convertSurface(s))
        return false;
    texWidth_ = width_;
    texHeight_ = height_;
    glGenTextures(1, &texture_);
    if(!texture_)
        return false;
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth_, texHeight_, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixelBuffer_.data());
    return true;
}

bool EditorGLTexture::updateFromSurface(SDL_Surface* s)
{
    if(!texture_ || !s || s->w != width_ || s->h != height_)
        return false;
    Uint32 ck;
    bool hasCK = (SDL_GetColorKey(s, &ck) == 0);

    if(isArgb32Format(s->format) && !hasCK)
    {
        glBindTexture(GL_TEXTURE_2D, texture_);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, GL_BGRA, GL_UNSIGNED_BYTE, s->pixels);
        return true;
    }
    if(!convertSurface(s))
        return false;
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, GL_BGRA, GL_UNSIGNED_BYTE, pixelBuffer_.data());
    return true;
}

void EditorGLTexture::destroy()
{
    if(texture_)
    {
        glDeleteTextures(1, &texture_);
        texture_ = 0;
    }
    pixelBuffer_.clear();
    pixelBuffer_.shrink_to_fit();
}

void EditorGLTexture::draw(int x, int y) const
{
    if(!texture_)
        return;
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, texture_);
    GLfloat v[8] = {GLfloat(x),          GLfloat(y),           GLfloat(x),          GLfloat(y + height_),
                    GLfloat(x + width_), GLfloat(y + height_), GLfloat(x + width_), GLfloat(y)};
    float u2 = float(width_) / texWidth_, v2 = float(height_) / texHeight_;
    GLfloat tc[8] = {0, 0, 0, v2, u2, v2, u2, 0};
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, v);
    glTexCoordPointer(2, GL_FLOAT, 0, tc);
    glDrawArrays(GL_QUADS, 0, 4);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

// ===========================================================================
// CSurfaceGL namespace
// ===========================================================================

namespace CSurfaceGL {

void init(int screenWidth, int screenHeight)
{
    // Permanent GL state (s25client does this in VideoDriverWrapper.cpp).
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glClearColor(0, 0, 0, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, screenHeight, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

const EditorGLTexture& getOrCreateTexture(SDL_Surface* s)
{
    auto it = s_texCache.find(s);
    if(it == s_texCache.end())
    {
        EditorGLTexture t;
        if(t.createFromSurface(s))
            it = s_texCache.emplace(s, std::move(t)).first;
        else
        {
            static EditorGLTexture e;
            return e;
        }
    }
    return it->second;
}

} // namespace CSurfaceGL

// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "GlTexture.h"
#include <glad/glad.h>
#include <vector>

GlTexture::~GlTexture()
{
    if(texture_)
        glDeleteTextures(1, &texture_);
}

GlTexture::GlTexture(GlTexture&& other) noexcept
    : texture_(other.texture_), width_(other.width_), height_(other.height_)
{
    other.texture_ = 0;
    other.width_ = 0;
    other.height_ = 0;
}

GlTexture& GlTexture::operator=(GlTexture&& other) noexcept
{
    if(this != &other)
    {
        if(texture_)
            glDeleteTextures(1, &texture_);
        texture_ = other.texture_;
        width_ = other.width_;
        height_ = other.height_;
        other.texture_ = 0;
        other.width_ = 0;
        other.height_ = 0;
    }
    return *this;
}

void GlTexture::createTexture(const void* bgraPixels, int w, int h, bool linear)
{
    if(texture_)
        glDeleteTextures(1, &texture_);
    texture_ = 0;
    width_ = height_ = 0;

    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, bgraPixels);
    width_ = w;
    height_ = h;
}

bool GlTexture::load(SDL_Surface* surface, bool linear)
{
    if(!surface)
        return false;

    // For 8-bit paletted surfaces, honour colorkey if set.
    if(surface->format->palette)
    {
        const int w = surface->w, h = surface->h;
        std::vector<Uint32> pixels(static_cast<size_t>(w) * h);

        SDL_Palette* pal = surface->format->palette;
        Uint32 ck;
        const bool hasCK = (SDL_GetColorKey(surface, &ck) == 0);
        const Uint8 ckIdx = hasCK ? static_cast<Uint8>(ck & 0xFF) : 0;

        SDL_LockSurface(surface);
        for(int row = 0; row < h; row++)
        {
            const auto* src = (const Uint8*)surface->pixels + row * surface->pitch;
            for(int col = 0; col < w; col++)
            {
                const Uint8 idx = src[col];
                if(hasCK && idx == ckIdx)
                    pixels[row * w + col] = 0; // transparent
                else
                {
                    const SDL_Color& c = pal->colors[idx];
                    // BGRA layout: A<<24 | R<<16 | G<<8 | B (little-endian GL_BGRA)
                    pixels[row * w + col] = (0xFFu << 24) | (Uint32(c.r) << 16) | (Uint32(c.g) << 8) | Uint32(c.b);
                }
            }
        }
        SDL_UnlockSurface(surface);

        createTexture(pixels.data(), w, h, linear);
        return true;
    }

    // 32-bit surface: convert to destination format (BGRA), force full opacity
    SDL_Surface* converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, 0);
    if(!converted)
        return false;

    // Force alpha to opaque (LBM palette entries often have alpha=0)
    SDL_LockSurface(converted);
    for(int y = 0; y < converted->h; y++)
    {
        auto* row = (Uint32*)((Uint8*)converted->pixels + y * converted->pitch);
        for(int x = 0; x < converted->w; x++)
            row[x] |= 0xFF000000u; // set alpha bits
    }
    SDL_UnlockSurface(converted);

    createTexture(converted->pixels, converted->w, converted->h, linear);
    SDL_FreeSurface(converted);
    return true;
}

void GlTexture::DrawFull(const Rect& destRect) const
{
    if(!texture_)
        return;

    glBindTexture(GL_TEXTURE_2D, texture_);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2i(destRect.left, destRect.top);
    glTexCoord2f(1, 0);
    glVertex2i(destRect.right, destRect.top);
    glTexCoord2f(1, 1);
    glVertex2i(destRect.right, destRect.bottom);
    glTexCoord2f(0, 1);
    glVertex2i(destRect.left, destRect.bottom);
    glEnd();
}

void GlTexture::Draw(int x, int y) const
{
    if(!texture_)
        return;

    glBindTexture(GL_TEXTURE_2D, texture_);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2i(x, y);
    glTexCoord2f(1, 0);
    glVertex2i(x + width_, y);
    glTexCoord2f(1, 1);
    glVertex2i(x + width_, y + height_);
    glTexCoord2f(0, 1);
    glVertex2i(x, y + height_);
    glEnd();
}

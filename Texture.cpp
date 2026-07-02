// Copyright (C) 2026 - 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Texture.h"
#include <glad/glad.h>
#include <utility>
#include <vector>

Texture::~Texture()
{
    if(texture_)
        glDeleteTextures(1, &texture_);
}

Texture::Texture(Texture&& other) noexcept
    : texture_(std::exchange(other.texture_, 0)), size_(std::exchange(other.size_, {0, 0}))
{}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if(this != &other)
    {
        if(texture_)
            glDeleteTextures(1, &texture_);
        texture_ = std::exchange(other.texture_, 0);
        size_ = std::exchange(other.size_, {0, 0});
    }
    return *this;
}

void Texture::load(const void* bgraPixels, Extent size, bool filterLinear)
{
    if(texture_)
        glDeleteTextures(1, &texture_);
    texture_ = 0;
    size_ = {0, 0};

    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterLinear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterLinear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_BGRA, GL_UNSIGNED_BYTE, bgraPixels);
    size_ = size;
}

void Texture::createEmpty(Extent size, bool filterLinear)
{
    load(nullptr, size, filterLinear);
}

void Texture::upload(const void* bgraPixels)
{
    if(!texture_)
        return;
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size_.x, size_.y, GL_BGRA, GL_UNSIGNED_BYTE, bgraPixels);
}

bool Texture::load(SDL_Surface* surface, bool filterLinear)
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
        const bool hasCK = SDL_GetColorKey(surface, &ck) == 0;
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

        load(pixels.data(), Extent(w, h), filterLinear);
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

    load(converted->pixels, Extent(converted->w, converted->h), filterLinear);
    SDL_FreeSurface(converted);
    return true;
}

void Texture::Draw(const Rect& destRect) const
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

void Texture::Draw(Position pos) const
{
    if(!texture_)
        return;

    glBindTexture(GL_TEXTURE_2D, texture_);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2i(pos.x, pos.y);
    glTexCoord2f(1, 0);
    glVertex2i(pos.x + size_.x, pos.y);
    glTexCoord2f(1, 1);
    glVertex2i(pos.x + size_.x, pos.y + size_.y);
    glTexCoord2f(0, 1);
    glVertex2i(pos.x, pos.y + size_.y);
    glEnd();
}

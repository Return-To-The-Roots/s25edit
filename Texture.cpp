// Copyright (C) 2026 - 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Texture.h"
#include "defines.h"
#include "globals.h"
#include <glad/glad.h>
#include <algorithm>
#include <utility>
#include <vector>

Texture::~Texture()
{
    // The texture only lives as long as the GL context. If the context is gone, it already freed
    // the texture, and calling GL then would segfault.
    if(texture_ && SDL_GL_GetCurrentContext())
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

void Texture::upload(const void* bgraPixels) // NOLINT(readability-make-member-function-const)
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
            row[x] |= 0xFF000000u;
    }
    SDL_UnlockSurface(converted);

    load(converted->pixels, Extent(converted->w, converted->h), filterLinear);
    SDL_FreeSurface(converted);
    return true;
}

void Texture::draw(const Rect& destRect) const
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

void Texture::draw(Position pos) const
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

void Texture::drawTiled(const Rect& destRect) const
{
    if(!texture_)
        return;

    const Extent tileSize = getSize();
    if(tileSize.x == 0 || tileSize.y == 0)
        return;

    glBindTexture(GL_TEXTURE_2D, texture_);
    glBegin(GL_QUADS);
    for(int y = destRect.top; y < destRect.bottom; y += static_cast<int>(tileSize.y))
    {
        const int rowH = std::min(static_cast<int>(tileSize.y), destRect.bottom - y);
        const float v1 = float(rowH) / float(tileSize.y);
        for(int x = destRect.left; x < destRect.right; x += static_cast<int>(tileSize.x))
        {
            const int colW = std::min(static_cast<int>(tileSize.x), destRect.right - x);
            const float u1 = float(colW) / float(tileSize.x);

            glTexCoord2f(0, 0);
            glVertex2i(x, y);
            glTexCoord2f(u1, 0);
            glVertex2i(x + colW, y);
            glTexCoord2f(u1, v1);
            glVertex2i(x + colW, y + rowH);
            glTexCoord2f(0, v1);
            glVertex2i(x, y + rowH);
        }
    }
    glEnd();
}

void drawRect(const Rect& rect, unsigned color)
{
    glDisable(GL_TEXTURE_2D);
    glColor4ub((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, (color >> 24) & 0xFF);
    glBegin(GL_QUADS);
    glVertex2i(rect.left, rect.top);
    glVertex2i(rect.right, rect.top);
    glVertex2i(rect.right, rect.bottom);
    glVertex2i(rect.left, rect.bottom);
    glEnd();
    glEnable(GL_TEXTURE_2D);
    glColor4f(1, 1, 1, 1);
}

Texture& getBmpTexture(unsigned idx, bool filterLinear)
{
    static std::vector<Texture> cache;
    static std::vector<bool> linearFlags;
    if(idx >= global::bmpArray.size())
    {
        static Texture dummy;
        return dummy;
    }
    if(idx >= cache.size())
    {
        cache.resize(static_cast<size_t>(idx) + 1);
        linearFlags.resize(static_cast<size_t>(idx) + 1, false);
    }
    if(!cache[idx].isValid() || linearFlags[idx] != filterLinear)
    {
        linearFlags[idx] = filterLinear;
        auto& bmp = global::bmpArray[idx];
        if(bmp.surface)
            cache[idx].load(bmp.surface.get(), filterLinear);
    }
    return cache[idx];
}

void drawButtonBox(const Rect& area, bool pressed, unsigned baseTex, unsigned faceTex)
{
    getBmpTexture(baseTex).drawTiled(area);

    const auto sz = area.getSize();

    // 2px black frame: left+top if pressed, right+bottom otherwise
    if(pressed)
    {
        drawRect(Rect(area.getOrigin(), 2, sz.y), 0xFF000000);
        drawRect(Rect(area.getOrigin(), sz.x, 2), 0xFF000000);
    } else
    {
        drawRect(Rect(area.right - 2, area.top, 2, sz.y), 0xFF000000);
        drawRect(Rect(area.left, area.bottom - 2, sz.x, 2), 0xFF000000);
    }

    // Foreground inset by 2px
    const Rect fgRect(area.getOrigin() + Position(2, 2), sz - Extent(4, 4));
    getBmpTexture(faceTex).drawTiled(fgRect);
}

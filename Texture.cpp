// Copyright (C) 2026 - 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Texture.h"
#include "defines.h"
#include "globals.h"
#include <glad/glad.h>
#include <algorithm>
#include <array>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include <libsiedler2/ArchivItem_Bitmap.h>
#include <libsiedler2/ArchivItem_Palette.h>

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

void Texture::load(const uint8_t* bgraPixels, Extent size)
{
    if(texture_)
        glDeleteTextures(1, &texture_);
    texture_ = 0;
    size_ = {0, 0};

    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_BGRA, GL_UNSIGNED_BYTE, bgraPixels);
    size_ = size;
}

void Texture::createEmpty(Extent size)
{
    load(nullptr, size);
}

void Texture::upload(const void* bgraPixels)
{
    if(!texture_)
        return;
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size_.x, size_.y, GL_BGRA, GL_UNSIGNED_BYTE, bgraPixels);
}

bool Texture::load(const libsiedler2::baseArchivItem_Bitmap& bitmap)
{
    const auto w = bitmap.getWidth();
    const auto h = bitmap.getHeight();
    if(w == 0 || h == 0)
        return false;

    // Render the bitmap to a BGRA buffer via libsiedler2's print method.
    // This handles both paletted and BGRA source formats.
    std::vector<uint8_t> bgra(static_cast<size_t>(w) * h * 4);
    if(bitmap.print(bgra.data(), w, h, libsiedler2::TextureFormat::BGRA) != 0)
    {
        // If print fails (e.g. paletted bitmap without a palette), try manual conversion
        if(bitmap.getFormat() == libsiedler2::TextureFormat::Paletted)
        {
            const auto* pal = bitmap.getPalette();
            if(!pal)
                return false;
            const auto& src = bitmap.getPixelData();
            if(src.size() < static_cast<size_t>(w) * h)
                return false;
            for(unsigned y = 0; y < h; y++)
            {
                for(unsigned x = 0; x < w; x++)
                {
                    uint8_t idx = src[y * w + x];
                    auto c = pal->get(idx);
                    uint32_t& dst = reinterpret_cast<uint32_t*>(bgra.data())[y * w + x];
                    dst = (0xFFu << 24) | (uint32_t(c.r) << 16) | (uint32_t(c.g) << 8) | uint32_t(c.b);
                }
            }
        } else
            return false;
    }

    load(bgra.data(), Extent(w, h));
    return true;
}

void Texture::draw(const Rect& destRect) const
{
    if(!texture_)
        return;

    glColor4f(1, 1, 1, 1);
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

    glColor4f(1, 1, 1, 1);
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

void Texture::draw(const Rect& destRect, const Rect& srcRect) const
{
    if(!texture_)
        return;

    // Clamp source rect to texture bounds
    int srcL = std::max(0, srcRect.left);
    int srcT = std::max(0, srcRect.top);
    int srcR = std::min(static_cast<int>(size_.x), srcRect.right);
    int srcB = std::min(static_cast<int>(size_.y), srcRect.bottom);
    if(srcL >= srcR || srcT >= srcB)
        return;

    const float u0 = float(srcL) / float(size_.x);
    const float v0 = float(srcT) / float(size_.y);
    const float u1 = float(srcR) / float(size_.x);
    const float v1 = float(srcB) / float(size_.y);

    glColor4f(1, 1, 1, 1);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glBegin(GL_QUADS);
    glTexCoord2f(u0, v0);
    glVertex2i(destRect.left, destRect.top);
    glTexCoord2f(u1, v0);
    glVertex2i(destRect.right, destRect.top);
    glTexCoord2f(u1, v1);
    glVertex2i(destRect.right, destRect.bottom);
    glTexCoord2f(u0, v1);
    glVertex2i(destRect.left, destRect.bottom);
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

Texture& getTexture(ArchiveID archive, int index)
{
    return Texture::getTexture(archive, index);
}

void drawButtonBox(const Rect& area, bool pressed, unsigned baseTex, unsigned faceTex)
{
    getTexture(ArchiveID::EDITIO, baseTex).drawTiled(area);

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
    const Extent fgSz(sz.x - 4u, sz.y - 4u);
    const Rect fgRect(area.getOrigin() + Position(2, 2), fgSz);
    getTexture(ArchiveID::EDITIO, faceTex).drawTiled(fgRect);
}

// Bitmap-texture cache (static members)
// Cache for typed-archive textures: (archive, index) → Texture
static std::map<std::pair<ArchiveID, int>, std::unique_ptr<Texture>> s_typedTexCache;

Texture& Texture::getTexture(ArchiveID archive, int index)
{
    auto& archiv = global::typedArchives[archive];
    if(index < 0 || static_cast<unsigned>(index) >= archiv.size())
    {
        static Texture dummy;
        return dummy;
    }
    const auto key = std::make_pair(archive, index);
    auto it = s_typedTexCache.find(key);
    if(it == s_typedTexCache.end() || !it->second->isValid())
    {
        auto tex = std::make_unique<Texture>();
        const auto* bmp = dynamic_cast<const libsiedler2::baseArchivItem_Bitmap*>(archiv.get(index));
        if(bmp)
        {
            tex->load(*bmp);
            tex->anchor_ = {bmp->getNx(), bmp->getNy()};
        }
        it = s_typedTexCache.emplace(key, std::move(tex)).first;
    }
    return *it->second;
}

void Texture::drawSprite(Position pos) const
{
    if(!isValid())
        return;
    draw(pos - anchor_);
}

// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CFont.h"
#include "../CIO/CFile.h"
#include "../CSurface.h"
#include "../Texture.h"
#include "../globals.h"
#include "CollisionDetection.h"
#include <libsiedler2/ArchivItem_Bitmap.h>
#include <libsiedler2/ArchivItem_Bitmap_Player.h>
#include <libsiedler2/ArchivItem_Font.h>
#include <libsiedler2/PixelBufferBGRA.h>
#include <glad/glad.h>
#include <cassert>
#include <cmath>
#include <iostream>

CFont::CFont(std::string text, Position pos, FontSize fontsize, FontColor color)
    : pos_(pos), string_(std::move(text)), fontsize_(fontsize), color_(color), initialColor_(color), clickedParam(0)
{
    size_ = Extent(getTextWidth(string_, fontsize_), getLineHeight(fontsize_));
}

void CFont::setPos(Position pos)
{
    pos_ = pos;
}

void CFont::setFontsize(FontSize fontsize)
{
    fontsize_ = fontsize;
    size_ = Extent(getTextWidth(string_, fontsize_), getLineHeight(fontsize_));
}

void CFont::setColor(FontColor color)
{
    initialColor_ = color_ = color;
}

void CFont::setText(std::string text)
{
    if(text == string_)
        return;
    this->string_ = std::move(text);
    size_ = Extent(getTextWidth(string_, fontsize_), getLineHeight(fontsize_));
}

void CFont::setMouseData(SDL_MouseButtonEvent button)
{
    if(!callback)
        return;
    // left button is pressed
    if(button.button == SDL_BUTTON_LEFT)
    {
        if(IsPointInRect(button.x, button.y, Rect(pos_, size_)))
        {
            // if mouse button is pressed ON the text
            if(button.state == SDL_PRESSED && getColor() == initialColor_)
            {
                const auto tmpInitialColor = initialColor_;
                setColor(FontColor::Orange);
                initialColor_ = tmpInitialColor;
            } else if(button.state == SDL_RELEASED && getColor() == FontColor::Orange)
            {
                callback(clickedParam);
            }
        } else if(getColor() != initialColor_)
            setColor(initialColor_);
    }
}

// atlas-based rendering
struct GlyphPos
{
    unsigned x, y, w;
};
struct FontAtlas
{
    Texture tex;
    unsigned lineHeight = 0;
    unsigned maxWidth = 0;
    std::array<GlyphPos, 256> glyphs{};
};

static libsiedler2::ColorRGB getPlayerColor(FontColor color)
{
    switch(color)
    {
        case FontColor::Blue: return {64, 128, 255};
        case FontColor::Red: return {255, 64, 64};
        case FontColor::Orange: return {255, 165, 0};
        case FontColor::Green: return {64, 192, 64};
        case FontColor::MintGreen: return {64, 255, 160};
        case FontColor::Yellow: return {255, 255, 0};
        case FontColor::BrightRed: return {255, 32, 32};
    }
    return {255, 255, 0};
}

static FontAtlas& getAtlas(FontSize size, FontColor color)
{
    static FontAtlas atlases[3][7];
    int ci = static_cast<int>(color);
    if(ci < 0 || ci > 6)
        ci = 0;
    int si;
    switch(size)
    {
        case FontSize::Small: si = 0; break;
        case FontSize::Medium: si = 1; break;
        case FontSize::Large: si = 2; break;
        default: si = 0; break;
    }
    if(!atlases[si][ci].tex.isValid())
    {
        auto& a = atlases[si][ci];
        // Resolve the font from the EDITRES archive by height
        unsigned targetDy = static_cast<unsigned>(size);
        auto& archiv = global::typedArchives[ArchiveID::EDITRES];
        const libsiedler2::ArchivItem_Font* font = nullptr;
        for(unsigned i = 0; i < archiv.size(); i++)
        {
            auto* f = dynamic_cast<const libsiedler2::ArchivItem_Font*>(archiv.get(i));
            if(!f)
                continue;
            unsigned dy = f->getDy();
            if(dy + 1 >= targetDy && dy <= targetDy + 1)
            {
                font = f;
                break;
            }
        }
        if(!font)
            return a;

        a.lineHeight = font->getDy() + 1;
        unsigned advW = font->getDx();
        a.maxWidth = advW;

        unsigned numGlyphs = 0;
        for(unsigned i = 32; i < font->size(); ++i)
        {
            if(font->get(i))
                numGlyphs++;
        }
        if(numGlyphs == 0)
            return a;

        auto numCols = static_cast<unsigned>(std::sqrt(static_cast<double>(numGlyphs)));
        if(numCols < 1)
            numCols = 1;
        unsigned numRows = (numGlyphs + numCols - 1) / numCols;
        constexpr Extent spacing(1, 1);
        Extent cellSize(advW + spacing.x * 2, font->getDy() + spacing.y * 2);
        Extent texSize = cellSize * Extent(numCols, numRows) + spacing * 2u;

        libsiedler2::PixelBufferBGRA buffer(texSize.x, texSize.y);

        // Build a palette where player-color indices 128-135 map to the
        // requested FontColor, and everything else is dark outline.
        auto playerClr = getPlayerColor(color);
        auto atlasPal = std::make_unique<libsiedler2::ArchivItem_Palette>();
        {
            const auto* srcPal = global::currentPalette;
            if(!srcPal)
                return a;
            for(int i = 0; i < 256; i++)
            {
                if(i >= 128 && i < 136)
                    atlasPal->set(i, playerClr);
                else if(i == 0)
                    atlasPal->set(i, libsiedler2::ColorRGB(0, 0, 0)); // transparent background
                else
                    atlasPal->set(i, libsiedler2::ColorRGB(32, 32, 32)); // dark outline
            }
        }

        unsigned gi = 0;
        for(unsigned ci = 32; ci < font->size(); ++ci)
        {
            auto* sub = font->get(ci);
            const auto* pg = dynamic_cast<const libsiedler2::ArchivItem_Bitmap_Player*>(sub);
            if(!pg)
                continue;

            unsigned col = gi % numCols, row = gi / numCols;
            unsigned px = spacing.x + col * cellSize.x;
            unsigned py = spacing.y + row * cellSize.y;

            const_cast<libsiedler2::ArchivItem_Bitmap_Player*>(pg)->print(buffer, atlasPal.get(), 128, px, py, 0, 0, 0,
                                                                          0);
            a.glyphs[ci] = GlyphPos{px, py, pg->getWidth()};
            if(pg->getWidth() > a.maxWidth)
                a.maxWidth = pg->getWidth();
            gi++;
        }

        a.tex.load(buffer.getPixelPtr(), texSize);
    }
    return atlases[si][ci];
}

static unsigned getCharWidth(uint8_t c, FontSize fontsize)
{
    if(fontsize == FontSize::Small && c == 236)
        c = 109;
    auto& atlas = getAtlas(fontsize, FontColor::Yellow);
    if(!atlas.tex.isValid())
        return 8;
    auto& g = atlas.glyphs[c];
    if(g.w > 0)
        return g.w;
    return atlas.maxWidth; // fallback
}

void CFont::draw(const std::string& string, Position pos, FontSize fontsize, FontColor color, FontAlign align)
{
    if(string.empty())
        return;
    auto& atlas = getAtlas(fontsize, color);
    if(!atlas.tex.isValid())
        return;

    unsigned totalW = CFont::getTextWidth(string, fontsize);
    if(align == FontAlign::Middle)
        pos.x -= static_cast<int>(totalW / 2);
    else if(align == FontAlign::Right)
        pos.x -= static_cast<int>(totalW);

    float texW = static_cast<float>(atlas.tex.getSize().x);
    float texH = static_cast<float>(atlas.tex.getSize().y);

    glBindTexture(GL_TEXTURE_2D, atlas.tex.getHandle());
    glColor4f(1, 1, 1, 1);
    glBegin(GL_QUADS);

    int curX = pos.x;
    for(char c : string)
    {
        unsigned char uc = static_cast<unsigned char>(c);
        auto& g = atlas.glyphs[uc];
        if(g.w == 0)
        {
            curX += atlas.maxWidth / 2;
            continue;
        }
        int gh = static_cast<int>(atlas.lineHeight);
        float u0 = static_cast<float>(g.x) / texW;
        float v0 = static_cast<float>(g.y) / texH;
        float u1 = static_cast<float>(g.x + g.w) / texW;
        float v1 = static_cast<float>(g.y + gh) / texH;

        glTexCoord2f(u0, v0);
        glVertex2i(curX, pos.y);
        glTexCoord2f(u1, v0);
        glVertex2i(curX + g.w, pos.y);
        glTexCoord2f(u1, v1);
        glVertex2i(curX + g.w, pos.y + gh);
        glTexCoord2f(u0, v1);
        glVertex2i(curX, pos.y + gh);
        curX += g.w; // advance by glyph width
    }
    glEnd();
}

unsigned CFont::getTextWidth(const std::string& string, FontSize fontsize)
{
    unsigned w = 0;
    for(unsigned char c : string)
    {
        if(c == '\n')
            break;
        w += getCharWidth(c, fontsize);
    }
    return w;
}

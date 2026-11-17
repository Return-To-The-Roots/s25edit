// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CFont.h"
#include "../CSurface.h"
#include "../Texture.h"
#include "../globals.h"
#include "CollisionDetection.h"
#include <cassert>

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
        if(isPointInRect(button.x, button.y, Rect(pos_, size_)))
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

namespace {
unsigned getIndexForChar(uint8_t c)
{
    // subtract 32 shows that we start by spacebar as 'zero-position'
    if(c >= 32 && c <= 90)
        return c - 32;
    /* \ */
    else if(c == 92)
        return 59;
    // _
    else if(c == 95)
        return 60;
    // between 'a' and 'z'
    else if(c >= 97 && c <= 122)
        return c - 32 - 4;
    // ©
    else if(c == 169)
        return 114;
    // Ä
    else if(c == 196)
        return 100;
    // Ç
    else if(c == 199)
        return 87;
    // Ö
    else if(c == 214)
        return 106;
    // Ü
    else if(c == 220)
        return 107;
    // ß
    else if(c == 223)
        return 113;
    // à
    else if(c == 224)
        return 92;
    // á
    else if(c == 225)
        return 108;
    // â
    else if(c == 226)
        return 90;
    // ä
    else if(c == 228)
        return 91;
    // ç
    else if(c == 231)
        return 93;
    // è
    else if(c == 232)
        return 96;
    // é
    else if(c == 233)
        return 89;
    // ê
    else if(c == 234)
        return 94;
    // ë
    else if(c == 235)
        return 95;
    // ì
    else if(c == 236)
        return 99;
    // í
    else if(c == 237)
        return 109;
    // î
    else if(c == 238)
        return 98;
    // ï
    else if(c == 239)
        return 97;
    // ñ
    else if(c == 241)
        return 112;
    // ò
    else if(c == 242)
        return 103;
    // ó
    else if(c == 243)
        return 110;
    // ô
    else if(c == 244)
        return 101;
    // ö
    else if(c == 246)
        return 102;
    // ù
    else if(c == 249)
        return 105;
    // ú
    else if(c == 250)
        return 111;
    // û
    else if(c == 251)
        return 104;
    // ü
    else if(c == 252)
        return 88;
    // chiffre not available, use '_' instead
    return 60;
}

unsigned getIndexForChar(uint8_t c, FontSize fontsize, FontColor color)
{
    unsigned offset;
    switch(fontsize)
    {
        case FontSize::Small: offset = FONT9_SPACE; break;
        default:
        case FontSize::Medium: offset = FONT11_SPACE; break;
        case FontSize::Large: offset = FONT14_SPACE; break;
    }
    return offset + getIndexForChar(c) * NUM_FONT_COLORS + static_cast<unsigned>(color);
}

unsigned getCharWidth(uint8_t c, FontSize fontsize, FontColor color)
{
    // NOTE: there is a bug in the ansi 236 'ì' at fontsize 9, the width is 39, this is not useable, we will use the
    // width of ansi 237 'í' instead
    if(fontsize == FontSize::Small && c == 236)
        c = 109;
    return global::bmpArray[getIndexForChar(c, fontsize, color)].w;
}
} // namespace

void CFont::draw(Position parentOrigin) const
{
    if(string_.empty())
        return;
    draw(string_, parentOrigin + pos_, fontsize_, color_, FontAlign::Left);
}

void CFont::draw(const std::string& string, Position pos, FontSize fontsize, FontColor color, FontAlign align)
{
    if(string.empty())
        return;

    // Measure text width for alignment
    unsigned totalWidth = 0;
    for(unsigned char c : string)
        totalWidth += getCharWidth(c, fontsize, color);

    // Apply alignment
    switch(align)
    {
        case FontAlign::Middle: pos.x -= static_cast<int>(totalWidth / 2); break;
        case FontAlign::Right: pos.x -= static_cast<int>(totalWidth); break;
        case FontAlign::Left: break; // no adjustment
    }

    // Draw each character as a textured quad
    Position curPos = pos;
    for(unsigned char c : string)
    {
        auto& tex = getBmpTexture(getIndexForChar(c, fontsize, color));
        tex.draw(Rect(curPos, tex.getSize()));
        curPos.x += tex.getSize().x;
    }
}

bool CFont::writeText(SDL_Surface* Surf_Dest, const std::string& string, unsigned x, unsigned y, FontSize fontsize,
                      FontColor color, FontAlign align)
{
    // data for necessary counting pixels depending on alignment
    unsigned pixel_ctr_w = 0;
    // counter for the drawed pixels (cause we dont want to draw outside of the surface)
    unsigned pos_x = x;
    unsigned pos_y = y;

    if(!Surf_Dest || string.empty())
        return false;

    // are there enough vertical pixels to draw the chiffres?
    if(Surf_Dest->h < static_cast<int>(y + static_cast<unsigned>(fontsize)))
        return false;

    // in case of right or middle alignment we must count the pixels first
    auto pixel_count_loop = (align == FontAlign::Middle) || (align == FontAlign::Right);

    // now lets draw the chiffres
    auto chiffre = string.begin();
    while(chiffre != string.end())
    {
        const auto charW = getCharWidth(*chiffre, fontsize, color);
        // if we only count pixels in this round
        if(pixel_count_loop)
        {
            pixel_ctr_w += charW;

            // if text is to long to go further left, stop loop and begin writing at x=0
            if((align == FontAlign::Middle && pixel_ctr_w / 2 > x) || static_cast<int>(pixel_ctr_w) >= Surf_Dest->w)
            {
                pos_x = 0;
                chiffre = string.begin();
                pixel_count_loop = false;
                continue;
            }

            ++chiffre;

            // if this was the last chiffre go in normal mode and write the text to the specified position
            if(chiffre == string.end())
            {
                chiffre = string.begin();

                if(align == FontAlign::Middle)
                    pos_x = x - pixel_ctr_w / 2;
                else if(align == FontAlign::Right)
                    pos_x = Surf_Dest->w - pixel_ctr_w;

                pixel_count_loop = false;
            }
            continue;
        }

        // if right end of surface is reached, stop drawing chiffres
        if(Surf_Dest->w < static_cast<int>(pos_x + charW))
            break;

        // draw the chiffre to the destination
        CSurface::Draw(Surf_Dest, global::bmpArray[getIndexForChar(*chiffre, fontsize, color)].surface, pos_x, pos_y);

        // set position for next chiffre
        pos_x += charW;

        // go to next chiffre
        ++chiffre;
    }

    return true;
}

unsigned CFont::getTextWidth(const std::string& string, FontSize fontsize)
{
    unsigned w = 0;
    for(unsigned char c : string)
        w += getCharWidth(c, fontsize, FontColor::Yellow); // width is same for all colors

    return w;
}

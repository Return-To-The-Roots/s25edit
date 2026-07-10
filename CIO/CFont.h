// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"
#include <SDL.h>
#include <functional>
#include <memory>
#include <string>

class CFont
{
    friend class CDebug;

private:
    Position pos_; ///< Position of the text (top-left)
    Extent size_;  ///< Pixel extent of the rendered text
    std::string string_;
    FontSize fontsize_;
    FontColor color_, initialColor_;
    std::function<void(int)> callback;
    int clickedParam;

public:
    CFont(std::string text, Position pos = {0, 0}, FontSize fontsize = FontSize::Small,
          FontColor color = FontColor::Yellow);
    // Access
    Position getPos() const { return pos_; }
    Extent getSize() const { return size_; }
    void setPos(Position pos);
    void setFontsize(FontSize fontsize);
    void setColor(FontColor color);
    FontColor getColor() const { return color_; }
    void setText(std::string text);
    void setCallback(std::function<void(int)> callback, int param)
    {
        this->callback = std::move(callback);
        clickedParam = param;
    }
    void unsetCallback()
    {
        callback = nullptr;
        clickedParam = 0;
    }
    void setMouseData(SDL_MouseButtonEvent button);

    /// Draw this font's text at the given absolute position
    void draw(Position parentOrigin) const { draw(string_, parentOrigin + pos_, fontsize_, color_, FontAlign::Left); }

    /// Draw text directly
    /// @param pos  Absolute position (top-left of the text, adjusted for alignment).
    static void draw(const std::string& string, Position pos, FontSize fontsize = FontSize::Small,
                     FontColor color = FontColor::Yellow, FontAlign align = FontAlign::Left);

    /// Compute the pixel width of a string without drawing it.
    static unsigned getTextWidth(const std::string& string, FontSize fontsize);
};

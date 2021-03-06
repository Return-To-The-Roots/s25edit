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
    SdlSurface Surf_Font;
    Sint16 x_;
    Sint16 y_;
    Uint16 w;
    Uint16 h;
    std::string string_;
    FontSize fontsize_;
    FontColor color_, initialColor_;
    std::function<void(int)> callback;
    int clickedParam;

    void writeText();

public:
    CFont(std::string text, unsigned x = 0, unsigned y = 0, FontSize fontsize = FontSize::Small,
          FontColor color = FontColor::Yellow);
    // Access
    int getX() const { return x_; };
    int getY() const { return y_; };
    void setPos(Position pos);
    unsigned getW() const { return w; };
    unsigned getH() const { return static_cast<unsigned>(fontsize_); };
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
    SDL_Surface* getSurface();
    // Methods
    // fontsize can be 9, 11 or 14 (otherwise it will be set to 9) ---- '\n' is possible
    // this function can be used as CFont::writeText to write text directly to a surface without creating an object
    static bool writeText(SDL_Surface* Surf_Dest, const std::string& string, unsigned x = 0, unsigned y = 0,
                          FontSize fontsize = FontSize::Small, FontColor color = FontColor::Yellow,
                          FontAlign align = FontAlign::Left);
    static bool writeText(SdlSurface& Surf_Dest, const std::string& string, unsigned x = 0, unsigned y = 0,
                          FontSize fontsize = FontSize::Small, FontColor color = FontColor::Yellow,
                          FontAlign align = FontAlign::Left)
    {
        return writeText(Surf_Dest.get(), string, x, y, fontsize, color, align);
    }
};

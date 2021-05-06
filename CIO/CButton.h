// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "SdlSurface.h"
#include "defines.h"

class CButton
{
    friend class CDebug;

private:
    SdlSurface Surf_Button;
    bool needRender;
    Sint16 x_;
    Sint16 y_;
    Uint16 w;
    Uint16 h;
    int pic_normal;
    int pic_marked;
    int pic_background;
    int button_picture;
    const char* button_text;
    FontColor button_text_color;
    bool marked;
    bool clicked;
    void (*callback_)(int);
    int clickedParam;
    int motionEntryParam;
    int motionLeaveParam;

public:
    CButton(void callback(int), int clickedParam, Sint16 x = 0, Sint16 y = 0, Uint16 w = 20, Uint16 h = 20,
            int color = BUTTON_GREY, const char* text = nullptr, int button_picture = -1);
    // Access
    int getX() const { return x_; };
    int getY() const { return y_; };
    Point16 getPos() const { return {x_, y_}; }
    int getW() const { return w; };
    int getH() const { return h; };
    void setX(int x) { this->x_ = x; };
    void setY(int y) { this->y_ = y; };
    void setButtonPicture(int picture);
    void setButtonText(const char* text);
    void setMouseData(const SDL_MouseMotionEvent& motion);
    void setMouseData(const SDL_MouseButtonEvent& button);
    bool render();
    SDL_Surface* getSurface()
    {
        render();
        return Surf_Button.get();
    };
    void setColor(int color);
    void setTextColor(FontColor color)
    {
        button_text_color = color;
        needRender = true;
    };
    void setMotionParams(int entry, int leave)
    {
        motionEntryParam = entry;
        motionLeaveParam = leave;
    };
};

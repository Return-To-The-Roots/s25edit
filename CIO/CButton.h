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
    Position pos_;
    Extent size_;
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
    CButton(void callback(int), int clickedParam, Position pos = {0, 0}, Extent size = {20, 20},
            int color = BUTTON_GREY, const char* text = nullptr, int button_picture = -1);
    // Access
    int getX() const { return pos_.x; };
    int getY() const { return pos_.y; };
    const Position& getPos() const { return pos_; }
    const Extent& getSize() const { return size_; };
    void setX(int x) { pos_.x = x; };
    void setY(int y) { pos_.y = y; };
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

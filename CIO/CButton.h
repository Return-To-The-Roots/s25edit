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
    Point16 pos_;
    Extent16 size_;
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
    CButton(void callback(int), int clickedParam, Point16 pos = {0, 0}, Extent16 size = {20, 20},
            int color = BUTTON_GREY, const char* text = nullptr, int button_picture = -1);
    // Access
    int getX() const { return pos_.x; };
    int getY() const { return pos_.y; };
    Point16 getPos() const { return pos_; }
    int getW() const { return size_.x; };
    int getH() const { return size_.y; };
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

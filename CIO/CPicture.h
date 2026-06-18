// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "SdlSurface.h"
#include "defines.h"

class CPicture
{
    friend class CDebug;

private:
    SdlSurface Surf_Picture;
    bool needRender;
    Point16 pos_;
    Extent16 size_;
    int picture_;
    bool marked;
    bool clicked;
    void (*callback)(int);
    int clickedParam;
    int motionEntryParam;
    int motionLeaveParam;

public:
    CPicture(void callback(int), int clickedParam, Point16 pos = {0, 0}, int picture = -1);
    // Access
    int getX() const { return pos_.x; };
    int getY() const { return pos_.y; };
    int getW() const { return size_.x; };
    int getH() const { return size_.y; };
    void setX(int x) { pos_.x = x; };
    void setY(int y) { pos_.y = y; };
    void setMouseData(const SDL_MouseMotionEvent& motion);
    void setMouseData(const SDL_MouseButtonEvent& button);
    bool render();
    SDL_Surface* getSurface()
    {
        render();
        return Surf_Picture.get();
    };
    void setMotionParams(int entry, int leave)
    {
        motionEntryParam = entry;
        motionLeaveParam = leave;
    };
};

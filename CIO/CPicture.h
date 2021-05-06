// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "SdlSurface.h"

class CPicture
{
    friend class CDebug;

private:
    SdlSurface Surf_Picture;
    bool needRender;
    Uint16 x;
    Uint16 y;
    Uint16 w;
    Uint16 h;
    int picture_;
    bool marked;
    bool clicked;
    void (*callback)(int);
    int clickedParam;
    int motionEntryParam;
    int motionLeaveParam;

public:
    CPicture(void callback(int), int clickedParam, Uint16 x = 0, Uint16 y = 0, int picture = -1);
    // Access
    int getX() const { return x; };
    int getY() const { return y; };
    int getW() const { return w; };
    int getH() const { return h; };
    void setX(int x) { this->x = x; };
    void setY(int y) { this->y = y; };
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

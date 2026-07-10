// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ArchiveID.h"
#include "Point.h"
#include "defines.h"

class CPicture
{
    friend class CDebug;

private:
    Position pos_;
    Extent size_;
    ArchiveID archive_;
    int picture_;
    bool marked;
    bool clicked;
    void (*callback)(int);
    int clickedParam;
    int motionEntryParam;
    int motionLeaveParam;

public:
    CPicture(void callback(int), int clickedParam, Position pos, ArchiveID archive, int picture);
    // Access
    int getX() const { return pos_.x; };
    int getY() const { return pos_.y; };
    Position getPos() const { return pos_; }
    Extent getSize() const { return size_; };
    void setX(int x) { pos_.x = x; };
    void setY(int y) { pos_.y = y; };
    void setMouseData(const SDL_MouseMotionEvent& motion);
    void setMouseData(const SDL_MouseButtonEvent& button);
    void draw(Position parentOrigin) const;
    void setMotionParams(int entry, int leave)
    {
        motionEntryParam = entry;
        motionLeaveParam = leave;
    };
};

// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CPicture.h"
#include "../CSurface.h"
#include "../globals.h"

CPicture::CPicture(void callback(int), int clickedParam, Point16 pos, int picture)
{
    marked = false;
    clicked = false;
    this->pos_ = pos;
    if(picture >= 0)
        this->picture_ = picture;
    else
        this->picture_ = 0;
    this->size_.x = global::bmpArray[picture].w;
    this->size_.y = global::bmpArray[picture].h;
    this->callback = callback;
    this->clickedParam = clickedParam;
    motionEntryParam = -1;
    motionLeaveParam = -1;
    needRender = true;
}

void CPicture::setMouseData(const SDL_MouseMotionEvent& motion)
{
    // cursor is on the picture
    if((motion.x >= pos_.x) && (motion.x < pos_.x + size_.x) && (motion.y >= pos_.y) && (motion.y < pos_.y + size_.y))
    {
        if(motion.state == SDL_RELEASED)
        {
            marked = true;
            if(motionEntryParam >= 0 && callback)
                callback(motionEntryParam);
        }
    } else
    {
        // picture was marked before and mouse cursor is on the picture now, so do the callback
        if(motionLeaveParam >= 0 && callback && marked)
            callback(motionLeaveParam);
        marked = false;
    }
    needRender = true;
}

void CPicture::setMouseData(const SDL_MouseButtonEvent& button)
{
    // left button is pressed
    if(button.button == SDL_BUTTON_LEFT)
    {
        // if mouse button is pressed ON the button, set marked=true
        if((button.state == SDL_PRESSED) && (button.x >= pos_.x) && (button.x < pos_.x + size_.x) && (button.y >= pos_.y)
           && (button.y < pos_.y + size_.y))
        {
            marked = true;
            clicked = true;
        } else if(button.state == SDL_RELEASED)
        {
            clicked = false;
            // if mouse button is released ON the PICTURE (marked = true), then do the callback
            if(marked && callback)
                callback(clickedParam);
        }
    }
    needRender = true;
}

bool CPicture::render()
{
    // if we don't need to render, all is up to date, return true
    if(!needRender)
        return true;
    needRender = false;
    // if we need a new surface
    if(!Surf_Picture)
    {
        Surf_Picture = makeRGBSurface(size_.x, size_.y);
        if(!Surf_Picture)
            return false;
        SDL_SetColorKey(Surf_Picture.get(), SDL_TRUE, SDL_MapRGB(Surf_Picture->format, 0, 0, 0));
    }

    CSurface::Draw(Surf_Picture, global::bmpArray[picture_].surface);

    return true;
}

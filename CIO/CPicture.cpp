// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CPicture.h"
#include "../Texture.h"
#include "../globals.h"
#include "CollisionDetection.h"
#include <libsiedler2/Archiv.h>
#include <libsiedler2/ArchivItem_Bitmap.h>

CPicture::CPicture(void callback(int), int clickedParam, Position pos, ArchiveID archive, int picture)
    : pos_(pos), archive_(archive), picture_(picture >= 0 ? picture : 0)
{
    marked = false;
    clicked = false;
    auto& archiv = global::typedArchives[archive];
    if(picture >= 0 && static_cast<unsigned>(picture) < archiv.size())
    {
        if(auto* bmp = dynamic_cast<const libsiedler2::baseArchivItem_Bitmap*>(archiv.get(picture)))
            size_ = Extent(bmp->getWidth(), bmp->getHeight());
    }
    this->callback = callback;
    this->clickedParam = clickedParam;
    motionEntryParam = -1;
    motionLeaveParam = -1;
}

void CPicture::setMouseData(const SDL_MouseMotionEvent& motion)
{
    // cursor is on the picture
    if(IsPointInRect(motion.x, motion.y, Rect(pos_, size_)))
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
}

void CPicture::setMouseData(const SDL_MouseButtonEvent& button)
{
    // left button is pressed
    if(button.button == SDL_BUTTON_LEFT)
    {
        // if mouse button is pressed ON the button, set marked=true
        if(button.state == SDL_PRESSED && IsPointInRect(button.x, button.y, Rect(pos_, size_)))
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
}

void CPicture::draw(Position parentOrigin) const
{
    getTexture(archive_, picture_).draw(parentOrigin + pos_);
}

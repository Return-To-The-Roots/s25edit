// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CButton.h"
#include "../Texture.h"
#include "../globals.h"
#include "CFont.h"
#include "CollisionDetection.h"

CButton::CButton(void callback(int), int clickedParam, Position pos, Extent size, int color, const char* text,
                 int button_picture)
    : pos_(pos), size_(size)
{
    marked = false;
    clicked = false;
    setColor(color);
    this->button_picture = button_picture;
    button_text = text;
    button_text_color = FontColor::Yellow;
    this->callback_ = callback;
    this->clickedParam = clickedParam;
    motionEntryParam = -1;
    motionLeaveParam = -1;
}

void CButton::setButtonPicture(int picture)
{
    this->button_picture = picture;
}

void CButton::setButtonText(const char* text)
{
    button_text = text;
}

void CButton::setColor(int color)
{
    switch(color)
    {
        case BUTTON_GREY:
            pic_normal = BUTTON_GREY_DARK;
            pic_marked = BUTTON_GREY_BRIGHT;
            pic_background = BUTTON_GREY_BACKGROUND;
            break;

        case BUTTON_RED1:
            pic_normal = BUTTON_RED1_DARK;
            pic_marked = BUTTON_RED1_BRIGHT;
            pic_background = BUTTON_RED1_BACKGROUND;
            break;

        case BUTTON_GREEN1:
            pic_normal = BUTTON_GREEN1_DARK;
            pic_marked = BUTTON_GREEN1_BRIGHT;
            pic_background = BUTTON_GREEN1_BACKGROUND;
            break;

        case BUTTON_GREEN2:
            pic_normal = BUTTON_GREEN2_DARK;
            pic_marked = BUTTON_GREEN2_BRIGHT;
            pic_background = BUTTON_GREEN2_BACKGROUND;
            break;

        case BUTTON_RED2:
            pic_normal = BUTTON_RED2_DARK;
            pic_marked = BUTTON_RED2_BRIGHT;
            pic_background = BUTTON_RED2_BACKGROUND;
            break;

        case BUTTON_STONE:
            pic_normal = BUTTON_STONE_DARK;
            pic_marked = BUTTON_STONE_BRIGHT;
            pic_background = BUTTON_STONE_BACKGROUND;
            break;

        default:
            pic_normal = BUTTON_GREY_DARK;
            pic_marked = BUTTON_GREY_BRIGHT;
            pic_background = BUTTON_GREY_BACKGROUND;
            break;
    }
}

void CButton::setMouseData(const SDL_MouseMotionEvent& motion)
{
    // cursor is on the button (and mouse button not pressed while moving on the button)
    if(IsPointInRect(motion.x, motion.y, Rect(pos_, size_)))
    {
        if(motion.state == SDL_RELEASED)
        {
            marked = true;
            if(motionEntryParam >= 0 && callback_)
                callback_(motionEntryParam);
        }
    } else
    {
        // button was marked before and mouse cursor is on the button now, so do the callback
        if(motionLeaveParam >= 0 && callback_ && marked)
            callback_(motionLeaveParam);
        marked = false;
    }
}

void CButton::setMouseData(const SDL_MouseButtonEvent& button)
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
            // if mouse button is released ON the BUTTON (marked = true), then do the callback
            if(marked && callback_)
                callback_(clickedParam);
        }
    }
}

void CButton::draw(Position parentOrigin) const
{
    const Position absPos = parentOrigin + pos_;

    // Draw 3D button box
    const int foreground = (marked && !clicked) ? pic_marked : pic_normal;
    drawButtonBox(Rect(absPos, size_), clicked, pic_background, foreground);

    // 4. Draw picture or text centered inside the button
    if(button_picture >= 0)
    {
        auto& picTex = getTexture(ArchiveID::EDITIO, button_picture);
        const Position picPos = absPos + size_ / 2 - Position(picTex.getSize()) / 2;
        picTex.draw(picPos);
    } else if(button_text)
    {
        // Draw text centered (using native-size texture drawing for each character)
        const Extent textSize(CFont::getTextWidth(button_text, FontSize::Medium),
                              static_cast<unsigned>(FontSize::Medium));
        const Position textPos = absPos + (Position(size_) - textSize) / 2;
        CFont::draw(button_text, textPos, FontSize::Medium, button_text_color, FontAlign::Left);
    }
}

// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CButton.h"
#include "../CSurface.h"
#include "../globals.h"
#include "CFont.h"
#include "CollisionDetection.h"

CButton::CButton(void callback(int), int clickedParam, Point16 pos, Extent16 size, int color, const char* text,
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
    needRender = true;
}

void CButton::setButtonPicture(int picture)
{
    this->button_picture = picture;
    needRender = true;
}

void CButton::setButtonText(const char* text)
{
    button_text = text;
    needRender = true;
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

    needRender = true;
}

void CButton::setMouseData(const SDL_MouseMotionEvent& motion)
{
    // cursor is on the button (and mouse button not pressed while moving on the button)
    if(IsPointInRect(Position(motion.x, motion.y), Rect(Position(pos_), Extent(size_))))
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
    needRender = true;
}

void CButton::setMouseData(const SDL_MouseButtonEvent& button)
{
    // left button is pressed
    if(button.button == SDL_BUTTON_LEFT)
    {
        // if mouse button is pressed ON the button, set marked=true
        if((button.state == SDL_PRESSED) && IsPointInRect(Position(button.x, button.y), Rect(Position(pos_), Extent(size_))))
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
    needRender = true;
}

bool CButton::render()
{
    // position in the Surface 'Surf_Button'
    Uint16 pos_x = 0;
    Uint16 pos_y = 0;
    // width and height of the button color source picture
    Uint16 pic_w = 0;
    Uint16 pic_h = 0;
    // foreground of the button --> marked or unmarked, NOT the picture
    int foreground;

    // if we don't need to render, all is up to date, return true
    if(!needRender)
        return true;
    needRender = false;
    // if we need a new surface
    if(!Surf_Button)
    {
        if((Surf_Button = makeRGBSurface(size_.x, size_.y)) == nullptr)
            return false;
    }

    // at first completly fill the background (not the fastest way, but simplier)
    if(size_.x <= global::bmpArray[pic_background].w)
        pic_w = size_.x;
    else
        pic_w = global::bmpArray[pic_background].w;

    if(size_.y <= global::bmpArray[pic_background].h)
        pic_h = size_.y;
    else
        pic_h = global::bmpArray[pic_background].h;

    while(pos_x + pic_w <= Surf_Button->w)
    {
        while(pos_y + pic_h <= Surf_Button->h)
        {
            CSurface::Draw(Surf_Button, global::bmpArray[pic_background].surface, pos_x, pos_y, 0, 0, pic_w, pic_h);
            pos_y += pic_h;
        }

        if(Surf_Button->h - pos_y > 0)
            CSurface::Draw(Surf_Button, global::bmpArray[pic_background].surface, pos_x, pos_y, 0, 0, pic_w,
                           Surf_Button->h - pos_y);

        pos_y = 0;
        pos_x += pic_w;
    }

    if(Surf_Button->w - pos_x > 0)
    {
        while(pos_y + pic_h <= Surf_Button->h)
        {
            CSurface::Draw(Surf_Button, global::bmpArray[pic_background].surface, pos_x, pos_y, 0, 0,
                           Surf_Button->w - pos_x, pic_h);
            pos_y += pic_h;
        }

        if(Surf_Button->h - pos_y > 0)
            CSurface::Draw(Surf_Button, global::bmpArray[pic_background].surface, pos_x, pos_y, 0, 0,
                           Surf_Button->w - pos_x, Surf_Button->h - pos_y);
    }

    // draw partial black frame
    if(clicked)
    {
        // black frame is left and up
        // draw vertical line
        pos_x = 0;
        for(int y = 0; y < size_.y; y++)
            CSurface::DrawPixel_RGB(Surf_Button, pos_x, y, 0, 0, 0);

        // draw vertical line
        pos_x = 1;
        for(int y = 0; y < size_.y - 1; y++)
            CSurface::DrawPixel_RGB(Surf_Button, pos_x, y, 0, 0, 0);

        // draw horizontal line
        pos_y = 0;
        for(int x = 0; x < size_.x; x++)
            CSurface::DrawPixel_RGB(Surf_Button, x, pos_y, 0, 0, 0);

        // draw horizontal line
        pos_y = 1;
        for(int x = 0; x < size_.x - 1; x++)
            CSurface::DrawPixel_RGB(Surf_Button, x, pos_y, 0, 0, 0);
    } else
    {
        // black frame is right and down
        // draw vertical line
        pos_x = size_.x - 1;
        for(int y = 0; y < size_.y; y++)
            CSurface::DrawPixel_RGB(Surf_Button, pos_x, y, 0, 0, 0);

        // draw vertical line
        pos_x = size_.x - 2;
        for(int y = 1; y < size_.y; y++)
            CSurface::DrawPixel_RGB(Surf_Button, pos_x, y, 0, 0, 0);

        // draw horizontal line
        pos_y = size_.y - 1;
        for(int x = 0; x < size_.x; x++)
            CSurface::DrawPixel_RGB(Surf_Button, x, pos_y, 0, 0, 0);

        // draw horizontal line
        pos_y = size_.y - 2;
        for(int x = 1; x < size_.x; x++)
            CSurface::DrawPixel_RGB(Surf_Button, x, pos_y, 0, 0, 0);
    }

    // draw the foreground --> at first the color (marked or unmarked) and then the picture or text
    if(size_.x <= global::bmpArray[pic_normal].w)
        pic_w = size_.x;
    else
        pic_w = global::bmpArray[pic_normal].w;

    if(size_.y <= global::bmpArray[pic_normal].h)
        pic_h = size_.y;
    else
        pic_h = global::bmpArray[pic_normal].h;

    // beware overdrawing the left and upper frame
    pos_x = 2;
    pos_y = 2;

    // decide if button lights or not
    if(marked && !clicked)
        foreground = pic_marked;
    else
        foreground = pic_normal;

    // '-2' follows a few times, this means: beware overdrawing the right and lower frame
    while(pos_x + pic_w <= Surf_Button->w - 2)
    {
        while(pos_y + pic_h <= Surf_Button->h - 2)
        {
            CSurface::Draw(Surf_Button, global::bmpArray[foreground].surface, pos_x, pos_y, 0, 0, pic_w, pic_h);
            pos_y += pic_h;
        }

        if(Surf_Button->h - 2 - pos_y > 0)
            CSurface::Draw(Surf_Button, global::bmpArray[foreground].surface, pos_x, pos_y, 0, 0, pic_w,
                           Surf_Button->h - 2 - pos_y);

        pos_y = 2;
        pos_x += pic_w;
    }

    if(Surf_Button->w - 2 - pos_x > 0)
    {
        while(pos_y + pic_h <= Surf_Button->h - 2)
        {
            CSurface::Draw(Surf_Button, global::bmpArray[foreground].surface, pos_x, pos_y, 0, 0,
                           Surf_Button->w - 2 - pos_x, pic_h);
            pos_y += pic_h;
        }

        if(Surf_Button->h - 2 - pos_y > 0)
            CSurface::Draw(Surf_Button, global::bmpArray[foreground].surface, pos_x, pos_y, 0, 0,
                           Surf_Button->w - 2 - pos_x, Surf_Button->h - 2 - pos_y);
    }

    // positioning the picture or write text
    if(button_picture >= 0)
    {
        // picture may not be bigger than the button
        if(global::bmpArray[button_picture].w <= Surf_Button->w && global::bmpArray[button_picture].h <= Surf_Button->h)
        {
            // get coordinates of the left upper corner where to positionate the picture
            Position leftup = Position(Surf_Button->w, Surf_Button->h) / 2
                              - Position(global::bmpArray[button_picture].w, global::bmpArray[button_picture].h) / 2;
            // blit it
            CSurface::Draw(Surf_Button, global::bmpArray[button_picture].surface, leftup);
        } else
        {
            button_picture = -1;
            button_text = "PIC";
        }
    } else if(button_text)
        CFont::writeText(Surf_Button, button_text, Position(static_cast<int>(size_.x) / 2, static_cast<int>(size_.y - 11) / 2),
                         FontSize::Medium, button_text_color, FontAlign::Middle);

    return true;
}

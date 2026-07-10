// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CWindow.h"
#include "../CGame.h"
#include "../Texture.h"
#include "../globals.h"
#include "CButton.h"
#include "CFont.h"
#include "CPicture.h"
#include "CSelectBox.h"
#include "CTextfield.h"
#include "CollisionDetection.h"
#include "helpers/containerUtils.h"
#include <libsiedler2/ArchivItem_Bitmap.h>
#include <glad/glad.h>
#include <cassert>

CWindow::CWindow(void callback(int), int callbackQuitMessage, Position pos, Extent size, const char* title, int color,
                 Uint8 flags, ArchiveID bgArchive)
    : CControlContainer(
      color,
      BorderSizes(ArchiveID::EDITRES, WINDOW_LEFT_FRAME, WINDOW_UPPER_FRAME, WINDOW_RIGHT_FRAME, WINDOW_LOWER_FRAME),
      bgArchive),
      pos_(pos), size_(size), title(title), callback_(callback), callbackQuitMessage(callbackQuitMessage)
{
    assert(callback);
    canMove = (flags & WINDOW_MOVE) != 0;
    canClose = (flags & WINDOW_CLOSE) != 0;
    canMinimize = (flags & WINDOW_MINIMIZE) != 0;
    canResize = (flags & WINDOW_RESIZE) != 0;
}

static Position makePos(WindowPos pos, Extent size)
{
    if(pos == WindowPos::Center)
        return (global::s2->getRes() - size) / 2;
    else
        return {};
}

CWindow::CWindow(void callback(int), int callbackQuitMessage, WindowPos pos, Extent size,
                 const char* title /*= nullptr*/, int color /*= WINDOW_GREEN1*/, Uint8 flags /*= 0*/,
                 ArchiveID bgArchive /*= ArchiveID::EDITRES*/)
    : CWindow(callback, callbackQuitMessage, makePos(pos, size), size, title, color, flags, bgArchive)
{}

void CWindow::setTitle(const char* title)
{
    this->title = title;
}

bool CWindow::hasActiveInputElement()
{
    return helpers::contains_if(getTextFields(), [](const auto& text) { return text->isActive(); });
}

void CWindow::setMouseData(SDL_MouseMotionEvent motion)
{
    // cursor is on the title frame (+/-2 and +/-4 are only for a good optic)
    const Position titleFrameLT =
      pos_ + Position(static_cast<int>(global::getBitmapSize(ArchiveID::EDITRES, WINDOW_LEFT_UPPER_CORNER).x), 0)
      + Position(2, 4);
    const Position titleFrameRB =
      pos_
      + Position(static_cast<int>(size_.x),
                 static_cast<int>(global::getBitmapSize(ArchiveID::EDITRES, WINDOW_UPPER_FRAME).y))
      - Position(static_cast<int>(global::getBitmapSize(ArchiveID::EDITRES, WINDOW_RIGHT_UPPER_CORNER).x) + 2, 4);
    if(IsPointInRect(motion.x, motion.y, Rect(titleFrameLT, Extent(titleFrameRB - titleFrameLT))))
    {
        // left button was pressed while moving
        if(clicked)
            moving = true;
    } else if(!moving)
        clicked = false;

    if(!(SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT)))
        moving = false;
    if(moving && canMove)
    {
        pos_ += Position(motion.xrel, motion.yrel);
        // Clamp window position so it stays on screen
        const auto res = global::s2->getRes();
        const auto maxPos = res - elMin(size_, res);
        pos_ = elMin(elMax(pos_, Position(0, 0)), Position(maxPos));
    }

    // check whats happen to the close button
    if(canClose)
    {
        // cursor is on the button (+/-2 is only for the optic)
        const auto closeSize = global::getBitmapSize(ArchiveID::EDITRES, WINDOW_BUTTON_CLOSE);
        canClose_marked =
          IsPointInRect(Position(motion.x, motion.y), Rect(pos_ + Position(2, 2), closeSize - Extent(4, 4)));
    }
    // check whats happen to the minimize button
    if(canMinimize)
    {
        // cursor is on the button (+/-2 is only for the optic)
        const auto minSize = global::getBitmapSize(ArchiveID::EDITRES, WINDOW_BUTTON_MINIMIZE);
        canMinimize_marked =
          IsPointInRect(Position(motion.x, motion.y),
                        Rect(pos_ + Position(static_cast<int>(size_.x) - static_cast<int>(minSize.x) + 2, 2),
                             minSize - Extent(4, 4)));
    }
    // check whats happen to the resize button
    if(canResize)
    {
        // cursor is on the button (+/-2 is only for the optic)
        const auto resizeSize = global::getBitmapSize(ArchiveID::EDITRES, WINDOW_BUTTON_RESIZE);
        if(IsPointInRect(Position(motion.x, motion.y),
                         Rect(pos_
                                + Position(static_cast<int>(size_.x) - static_cast<int>(resizeSize.x) + 2,
                                           static_cast<int>(size_.y) - static_cast<int>(resizeSize.y) + 2),
                              resizeSize - Extent(4, 4))))
        {
            // left button was pressed while moving
            if(SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT))
                resizing = true;
            canResize_marked = true;
        } else if(!resizing)
            canResize_marked = false;

        if(!(SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT)))
            resizing = false;
        if(resizing)
        {
            // only resize if not minimized
            if(!minimized)
            {
                size_ = Extent(static_cast<int>(size_.x) + motion.xrel, static_cast<int>(size_.y) + motion.yrel);
                const auto res = global::s2->getRes();
                const auto maxSize = Extent(elMax(Position(res) - pos_, Position(1, 1)));
                size_ = elMin(elMax(size_, Extent::all(1u)), maxSize);
                callback_(WINDOW_RESIZED_CALL);
            }
        }
    }

    // deliver mouse data to the content objects of the window (if mouse cursor is inside the window)
    if(IsPointInRect(motion.x, motion.y, Rect(getPos(), getSize())))
    {
        // IMPORTANT: we use the left upper corner of the window as (x,y)=(0,0), so we have to manipulate
        //           the motion-structure before give it to buttons, pictures....: x_absolute - x_window, y_absolute -
        //           y_window
        motion.x -= pos_.x;
        motion.y -= pos_.y;
        CControlContainer::setMouseData(motion);
    }
}

void CWindow::setMouseData(SDL_MouseButtonEvent button)
{
    // at first check if the right mouse button was pressed, cause in this case we will close the window
    if(button.button == SDL_BUTTON_RIGHT && button.state == SDL_PRESSED)
    {
        callback_(callbackQuitMessage);
        return;
    }

    // save width and height in case we minimize the window (the initializing values are for preventing any mistakes and
    // compilerwarning --- in fact: uninitialized values are only a problem if the window is created minimized, but this
    // will not happen)
    static int maximized_h = global::getBitmapSize(ArchiveID::EDITRES, WINDOW_UPPER_FRAME).y
                             + global::getBitmapSize(ArchiveID::EDITRES, WINDOW_CORNER_RECTANGLE).y;
    if(!minimized)
        maximized_h = static_cast<int>(size_.y);

    // left button is pressed
    if(button.button == SDL_BUTTON_LEFT)
    {
        // cursor is on the title frame (+/-2 and +/-4 are only for a good optic)
        if((button.x
            >= pos_.x + static_cast<int>(global::getBitmapSize(ArchiveID::EDITRES, WINDOW_LEFT_UPPER_CORNER).x) + 2)
           && (button.x < pos_.x + static_cast<int>(size_.x)
                            - static_cast<int>(global::getBitmapSize(ArchiveID::EDITRES, WINDOW_RIGHT_UPPER_CORNER).x)
                            - 2)
           && (button.y >= pos_.y + 4)
           && (button.y
               < pos_.y + static_cast<int>(global::getBitmapSize(ArchiveID::EDITRES, WINDOW_UPPER_FRAME).y) - 4))
        {
            marked = true;
            clicked = true;
        }
        // pressed inside the window
        if(button.state == SDL_PRESSED && IsPointInRect(button.x, button.y, Rect(pos_, size_)))
            marked = true;
        // else pressed outside of the window
        else if(button.state == SDL_PRESSED)
            marked = false;

        // check whats happen to the close button
        // only set 'clicked' if pressed AND cursor is ON the button (marked == true)
        if(button.state == SDL_PRESSED && canClose_marked)
            canClose_clicked = true;
        else if(button.state == SDL_RELEASED)
        {
            canClose_clicked = false;
            // if mouse button is released ON the close button (marked = true), then send the quit message to the
            // callback
            if(canClose_marked)
            {
                callback_(callbackQuitMessage);
                return;
            }
        }
        // check whats happen to the minimize button
        // only set 'clicked' if pressed AND cursor is ON the button (marked == true)
        if(button.state == SDL_PRESSED && canMinimize_marked)
            canMinimize_clicked = true;
        else if(button.state == SDL_RELEASED)
        {
            canMinimize_clicked = false;
            // if mouse button is released ON the BUTTON (marked = true), then minimize or maximize the window
            if(canMinimize_marked)
            {
                if(minimized) // maximize now
                {
                    size_.y = static_cast<unsigned>(maximized_h);
                    minimized = false;
                } else // minimize now
                {
                    size_.y = global::getBitmapSize(ArchiveID::EDITRES, WINDOW_UPPER_FRAME).y
                              + global::getBitmapSize(ArchiveID::EDITRES, WINDOW_CORNER_RECTANGLE).y;
                    minimized = true;
                }
            }
        }
        // check whats happen to the resize button
        // only set 'clicked' if pressed AND cursor is ON the button (marked == true)
        if(button.state == SDL_PRESSED && canResize_marked)
            canResize_clicked = true;
        else if(button.state == SDL_RELEASED)
            canResize_clicked = false;
    }

    // deliver mouse data to the content objects of the window (if mouse cursor is inside the window)
    if(IsPointInRect(button.x, button.y, Rect(getPos(), getSize())))
    {
        // IMPORTANT: we use the left upper corner of the window as (x,y)=(0,0), so we have to manipulate
        //           the motion-structure before give it to buttons, pictures....: x_absolute - x_window, y_absolute -
        //           y_window
        button.x -= pos_.x;
        button.y -= pos_.y;
        CControlContainer::setMouseData(button);
    }

    // at least call the callback
    callback_(WINDOW_CLICKED_CALL);
}

void CWindow::draw(Position /*parentOrigin*/)
{
    // 1. Background fill (tiled)
    if(getBackground() != WINDOW_NOTHING)
        getTexture(backgroundArchive_, getBackground()).drawTiled(getRect());

    // 2. Content (if not minimized) — clipped to the area inside frames
    if(!minimized)
    {
        const auto viewH = global::s2->getRes().y;
        const auto& b = getBorderSizes();
        const auto contentOrigin = pos_ + Position(b.left, b.top);
        const auto contentSize = getSize() - getBorderSize();
        if(static_cast<int>(contentSize.x) > 0 && static_cast<int>(contentSize.y) > 0)
        {
            glEnable(GL_SCISSOR_TEST);
            glScissor(contentOrigin.x, viewH - (contentOrigin.y + static_cast<int>(contentSize.y)),
                      static_cast<int>(contentSize.x), static_cast<int>(contentSize.y));
            drawChildren(pos_);
            glDisable(GL_SCISSOR_TEST);
        }
    }

    // 3. Upper frame
    int upperframe;
    if(clicked)
        upperframe = WINDOW_UPPER_FRAME_CLICKED;
    else if(marked)
        upperframe = WINDOW_UPPER_FRAME_MARKED;
    else
        upperframe = WINDOW_UPPER_FRAME;

    // Draw upper frame tile across the top of the window
    {
        const Rect upperFrameRect(pos_, Extent(size_.x, getTexture(ArchiveID::EDITRES, upperframe).getSize().y));
        getTexture(ArchiveID::EDITRES, upperframe).drawTiled(upperFrameRect);
    }

    // 4. Title text
    if(title)
    {
        const int titleY = pos_.y + (getTexture(ArchiveID::EDITRES, WINDOW_UPPER_FRAME).getSize().y - 9) / 2;
        CFont::draw(title, Position(pos_.x + static_cast<int>(size_.x) / 2, titleY), FontSize::Small, FontColor::Yellow,
                    FontAlign::Middle);
    }

    // 5. Lower frame (tiled across bottom)
    {
        const int lowerH = getTexture(ArchiveID::EDITRES, WINDOW_LOWER_FRAME).getSize().y;
        const Rect lowerFrameRect(Position(pos_.x, pos_.y + static_cast<int>(size_.y) - lowerH),
                                  Extent(size_.x, lowerH));
        getTexture(ArchiveID::EDITRES, WINDOW_LOWER_FRAME).drawTiled(lowerFrameRect);
    }

    // 6. Left frame (tiled down left side)
    {
        const Rect leftFrameRect(pos_, Extent(getTexture(ArchiveID::EDITRES, WINDOW_LEFT_FRAME).getSize().x, size_.y));
        getTexture(ArchiveID::EDITRES, WINDOW_LEFT_FRAME).drawTiled(leftFrameRect);
    }

    // 7. Right frame (tiled down right side)
    {
        const int rightW = getTexture(ArchiveID::EDITRES, WINDOW_RIGHT_FRAME).getSize().x;
        const Rect rightFrameRect(Position(pos_.x + static_cast<int>(size_.x) - rightW, pos_.y),
                                  Extent(rightW, size_.y));
        getTexture(ArchiveID::EDITRES, WINDOW_RIGHT_FRAME).drawTiled(rightFrameRect);
    }

    // 8. Corners
    {
        getTexture(ArchiveID::EDITRES, WINDOW_LEFT_UPPER_CORNER).draw(pos_);

        const Extent ru = getTexture(ArchiveID::EDITRES, WINDOW_RIGHT_UPPER_CORNER).getSize();
        getTexture(ArchiveID::EDITRES, WINDOW_RIGHT_UPPER_CORNER)
          .draw(pos_ + Position(static_cast<int>(size_.x) - ru.x, 0));

        const Extent cr = getTexture(ArchiveID::EDITRES, WINDOW_CORNER_RECTANGLE).getSize();
        getTexture(ArchiveID::EDITRES, WINDOW_CORNER_RECTANGLE)
          .draw(pos_ + Position(0, static_cast<int>(size_.y) - cr.y));
        getTexture(ArchiveID::EDITRES, WINDOW_CORNER_RECTANGLE).draw(pos_ + size_ - cr);
    }

    // 9. Close button
    if(canClose)
    {
        int closebutton;
        if(canClose_clicked)
            closebutton = WINDOW_BUTTON_CLOSE_CLICKED;
        else if(canClose_marked)
            closebutton = WINDOW_BUTTON_CLOSE_MARKED;
        else
            closebutton = WINDOW_BUTTON_CLOSE;
        getTexture(ArchiveID::EDITRES, closebutton).draw(pos_);
    }

    // 10. Minimize button
    if(canMinimize)
    {
        int minimizebutton;
        if(canMinimize_clicked)
            minimizebutton = WINDOW_BUTTON_MINIMIZE_CLICKED;
        else if(canMinimize_marked)
            minimizebutton = WINDOW_BUTTON_MINIMIZE_MARKED;
        else
            minimizebutton = WINDOW_BUTTON_MINIMIZE;
        const Extent minBtnSize = getTexture(ArchiveID::EDITRES, minimizebutton).getSize();
        getTexture(ArchiveID::EDITRES, minimizebutton)
          .draw(pos_ + Position(static_cast<int>(size_.x) - minBtnSize.x, 0));
    }

    // 11. Resize button
    if(canResize)
    {
        int resizebutton;
        if(canResize_clicked)
            resizebutton = WINDOW_BUTTON_RESIZE_CLICKED;
        else if(canResize_marked)
            resizebutton = WINDOW_BUTTON_RESIZE_MARKED;
        else
            resizebutton = WINDOW_BUTTON_RESIZE;
        const Extent resBtnSize = getTexture(ArchiveID::EDITRES, resizebutton).getSize();
        getTexture(ArchiveID::EDITRES, resizebutton).draw(pos_ + size_ - resBtnSize);
    }
}

void CWindow::setInactive()
{
    active = false;
    clicked = false;
    marked = false;

    for(auto& textfield : getTextFields())
    {
        textfield->setInactive();
    }
}

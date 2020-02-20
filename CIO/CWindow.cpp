#include "CWindow.h"
#include "../CGame.h"
#include "../CSurface.h"
#include "../globals.h"
#include "CButton.h"
#include "CFont.h"
#include "CPicture.h"
#include "CSelectBox.h"
#include "CTextfield.h"
#include "CollisionDetection.h"
#include "helpers/containerUtils.h"
#include <cassert>

CWindow::CWindow(void callback(int), int callbackQuitMessage, Position pos, Extent size, const char* title, int color, Uint8 flags)
    : CControlContainer(color, {global::bmpArray[WINDOW_LEFT_FRAME].w, global::bmpArray[WINDOW_UPPER_FRAME].h}), x_(pos.x), y_(pos.y),
      w_(size.x), h_(size.y), title(title), callback_(callback), callbackQuitMessage(callbackQuitMessage)
{
    assert(callback);
    // ensure window is big enough to take all basic pictures needed
    // if ( w < (global::bmpArray[WINDOW_LEFT_UPPER_CORNER].w + global::bmpArray[WINDOW_UPPER_FRAME].w +
    // global::bmpArray[WINDOW_RIGHT_UPPER_CORNER].w) )
    //    this->w = global::bmpArray[WINDOW_LEFT_UPPER_CORNER].w + global::bmpArray[WINDOW_UPPER_FRAME].w +
    //    global::bmpArray[WINDOW_RIGHT_UPPER_CORNER].w;
    // else
    // if ( h < (global::bmpArray[WINDOW_UPPER_FRAME].h + global::bmpArray[WINDOW_CORNER_RECTANGLE].h) )
    //    this->h = global::bmpArray[WINDOW_UPPER_FRAME].h + global::bmpArray[WINDOW_CORNER_RECTANGLE].h;
    // else
    canMove = (flags & WINDOW_MOVE) != 0;
    canClose = (flags & WINDOW_CLOSE) != 0;
    canMinimize = (flags & WINDOW_MINIMIZE) != 0;
    canResize = (flags & WINDOW_RESIZE) != 0;
}

static Position makePos(WindowPos pos, Extent size)
{
    if(pos == WindowPos::Center)
        return Position(global::s2->getDisplaySurface()->w, global::s2->getDisplaySurface()->h) / 2 - size / 2;
    else
        return {};
}

CWindow::CWindow(void callback(int), int callbackQuitMessage, WindowPos pos, Extent size, const char* title /*= nullptr*/,
                 int color /*= WINDOW_GREEN1*/, Uint8 flags /*= 0*/)
    : CWindow(callback, callbackQuitMessage, makePos(pos, size), size, title, color, flags)
{}

void CWindow::setTitle(const char* title)
{
    this->title = title;
    needRender = true;
}

void CWindow::setColor(int color)
{
    setBackgroundPicture(color);
}

bool CWindow::hasActiveInputElement()
{
    return helpers::contains_if(getTextFields(), [](const auto& text) { return text->isActive(); });
}

void CWindow::setMouseData(SDL_MouseMotionEvent motion)
{
    // cursor is on the title frame (+/-2 and +/-4 are only for a good optic)
    const Position titleFrameLT = Position(x_, y_) + Position(global::bmpArray[WINDOW_LEFT_UPPER_CORNER].w + 2, 4);
    const Position titleFrameRB =
      Position(x_ + w_ - global::bmpArray[WINDOW_RIGHT_UPPER_CORNER].w - 2, y_ + global::bmpArray[WINDOW_UPPER_FRAME].h - 4);
    if(IsPointInRect(Position(motion.x, motion.y), Rect(titleFrameLT, Extent(titleFrameRB - titleFrameLT))))
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
        x_ += motion.xrel;
        y_ += motion.yrel;
        // make sure to not move the window outside the display surface
        if(x_ < 0)
            x_ = 0;
        if(x_ + w_ >= global::s2->getDisplaySurface()->w) //-V807
            x_ = global::s2->getDisplaySurface()->w - w_ - 1;
        if(y_ < 0)
            y_ = 0;
        if(y_ + h_ >= global::s2->getDisplaySurface()->h)
            y_ = global::s2->getDisplaySurface()->h - h_ - 1;
    }

    // check whats happen to the close button
    if(canClose)
    {
        // cursor is on the button (+/-2 is only for the optic)
        if((motion.x >= x_ + 2) && (motion.x < x_ + global::bmpArray[WINDOW_BUTTON_CLOSE].w - 2) && (motion.y >= y_ + 2)
           && (motion.y < y_ + global::bmpArray[WINDOW_BUTTON_CLOSE].h - 2))
            canClose_marked = true;
        else
            canClose_marked = false;
    }
    // check whats happen to the minimize button
    if(canMinimize)
    {
        // cursor is on the button (+/-2 is only for the optic)
        if((motion.x >= x_ + w_ - global::bmpArray[WINDOW_BUTTON_MINIMIZE].w + 2) && (motion.x < x_ + w_ - 2) && (motion.y >= y_ + 2)
           && (motion.y < y_ + global::bmpArray[WINDOW_BUTTON_MINIMIZE].h - 2))
            canMinimize_marked = true;
        else
            canMinimize_marked = false;
    }
    // check whats happen to the resize button
    if(canResize)
    {
        // cursor is on the button (+/-2 is only for the optic)
        if((motion.x >= x_ + w_ - global::bmpArray[WINDOW_BUTTON_RESIZE].w + 2) && (motion.x < x_ + w_ - 2)
           && (motion.y >= y_ + h_ - global::bmpArray[WINDOW_BUTTON_RESIZE].h + 2) && (motion.y < y_ + h_ - 2))
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
                w_ += motion.xrel;
                h_ += motion.yrel;

                // MISSING: we have to test if window size is under minimum

                // the window has resized, so we need a new surface
                surface.reset();
            }
        }
    }

    // deliver mouse data to the content objects of the window (if mouse cursor is inside the window)
    if(IsPointInRect(Position(motion.x, motion.y), Rect(getPos(), getSize())))
    {
        // IMPORTANT: we use the left upper corner of the window as (x,y)=(0,0), so we have to manipulate
        //           the motion-structure before give it to buttons, pictures....: x_absolute - x_window, y_absolute - y_window
        motion.x -= x_;
        motion.y -= y_;
        CControlContainer::setMouseData(motion);
    }

    needRender = true;
}

void CWindow::setMouseData(SDL_MouseButtonEvent button)
{
    // at first check if the right mouse button was pressed, cause in this case we will close the window
    if(button.button == SDL_BUTTON_RIGHT && button.state == SDL_PRESSED)
    {
        callback_(callbackQuitMessage);
        return;
    }

    // save width and height in case we minimize the window (the initializing values are for preventing any mistakes and compilerwarning ---
    // in fact: uninitialized values are only a problem if the window is created minimized, but this will not happen)
    static int maximized_h = global::bmpArray[WINDOW_UPPER_FRAME].h + global::bmpArray[WINDOW_CORNER_RECTANGLE].h;
    if(!minimized)
        maximized_h = h_;

    // left button is pressed
    if(button.button == SDL_BUTTON_LEFT)
    {
        // cursor is on the title frame (+/-2 and +/-4 are only for a good optic)
        if((button.x >= x_ + global::bmpArray[WINDOW_LEFT_UPPER_CORNER].w + 2)
           && (button.x < x_ + w_ - global::bmpArray[WINDOW_RIGHT_UPPER_CORNER].w - 2) && (button.y >= y_ + 4)
           && (button.y < y_ + +global::bmpArray[WINDOW_UPPER_FRAME].h - 4))
        {
            marked = true;
            clicked = true;
        }
        // pressed inside the window
        if((button.state == SDL_PRESSED) && (button.x >= x_) && (button.x <= x_ + w_) && (button.y >= y_) && (button.y <= y_ + h_))
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
            // if mouse button is released ON the close button (marked = true), then send the quit message to the callback
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
                    h_ = maximized_h;
                    // the window has resized, so we need a new surface
                    surface.reset();
                    minimized = false;
                } else // minimize now
                {
                    h_ = global::bmpArray[WINDOW_UPPER_FRAME].h + global::bmpArray[WINDOW_CORNER_RECTANGLE].h;
                    // the window has resized, so we need a new surface
                    surface.reset();
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
    if(IsPointInRect(Position(button.x, button.y), Rect(getPos(), getSize())))
    {
        // IMPORTANT: we use the left upper corner of the window as (x,y)=(0,0), so we have to manipulate
        //           the motion-structure before give it to buttons, pictures....: x_absolute - x_window, y_absolute - y_window
        button.x -= x_;
        button.y -= y_;
        CControlContainer::setMouseData(button);
    }

    // at least call the callback
    callback_(WINDOW_CLICKED_CALL);

    needRender = true;
}

bool CWindow::render()
{
    // position in the Surface 'surface'
    Uint16 pos_x = 0;
    Uint16 pos_y = 0;
    // width and height of the window background color source picture
    Uint16 pic_w = 0;
    Uint16 pic_h = 0;
    // upper frame (can be marked, clicked or normal)
    int upperframe;
    // close button (can be marked, clicked or normal)
    int closebutton = WINDOW_BUTTON_CLOSE;
    // minimize button (can be marked, clicked or normal)
    int minimizebutton = WINDOW_BUTTON_MINIMIZE;
    // resize button (can be marked, clicked or normal)
    int resizebutton = WINDOW_BUTTON_RESIZE;

    // test if a textfield has changed
    needRender |= helpers::contains_if(getTextFields(), [](const auto& textfield) { return textfield->hasRendered(); });

    // if we don't need to render, all is up to date, return true
    if(!needRender)
        return true;
    needRender = false;
    // if we need a new surface
    if(!surface)
    {
        if(!(surface = makeRGBSurface(w_, h_)))
            return false;
    }

    // at first completly fill the background (not the fastest way, but simpler)
    if(getBackground() != WINDOW_NOTHING)
    {
        pic_w = std::min(w_, global::bmpArray[getBackground()].w);
        pic_h = std::min(h_, global::bmpArray[getBackground()].h);

        while(pos_x + pic_w <= surface->w)
        {
            while(pos_y + pic_h <= surface->h)
            {
                CSurface::Draw(surface.get(), global::bmpArray[getBackground()].surface, pos_x, pos_y, 0, 0, pic_w, pic_h);
                pos_y += pic_h;
            }

            if(surface->h - pos_y > 0)
                CSurface::Draw(surface.get(), global::bmpArray[getBackground()].surface, pos_x, pos_y, 0, 0, pic_w, surface->h - pos_y);

            pos_y = 0;
            pos_x += pic_w;
        }

        if(surface->w - pos_x > 0)
        {
            while(pos_y + pic_h <= surface->h)
            {
                CSurface::Draw(surface.get(), global::bmpArray[getBackground()].surface, pos_x, pos_y, 0, 0, surface->w - pos_x, pic_h);
                pos_y += pic_h;
            }

            if(surface->h - pos_y > 0)
                CSurface::Draw(surface.get(), global::bmpArray[getBackground()].surface, pos_x, pos_y, 0, 0, surface->w - pos_x,
                               surface->h - pos_y);
        }
    }

    // if not minimized, draw the content now (this stands here to prevent the frames and corners from being overdrawn)
    if(!minimized)
    {
        renderElements();
    }

    // now draw the upper frame to the top
    if(clicked)
        upperframe = WINDOW_UPPER_FRAME_CLICKED;
    else if(marked)
        upperframe = WINDOW_UPPER_FRAME_MARKED;
    else
        upperframe = WINDOW_UPPER_FRAME;

    pic_w = std::min(w_, global::bmpArray[upperframe].w);

    pos_x = 0;
    pos_y = 0;
    while(pos_x + pic_w <= surface->w)
    {
        CSurface::Draw(surface, global::bmpArray[upperframe].surface, pos_x, pos_y);
        pos_x += pic_w;
    }

    if(surface->w - pos_x > 0)
        CSurface::Draw(surface.get(), global::bmpArray[upperframe].surface, pos_x, pos_y, 0, 0, surface->w - pos_x, pic_h);
    // write text in the upper frame
    if(title)
        CFont::writeText(surface.get(), title, (int)w_ / 2, (int)((global::bmpArray[WINDOW_UPPER_FRAME].h - 9) / 2), 9, FONT_YELLOW,
                         FontAlign::Middle);

    // now draw the other frames (left, right, down)
    // down
    pic_w = std::min(w_, global::bmpArray[WINDOW_LOWER_FRAME].w);
    pic_h = global::bmpArray[WINDOW_LOWER_FRAME].h;
    pos_x = 0;
    pos_y = h_ - global::bmpArray[WINDOW_LOWER_FRAME].h;
    while(pos_x + pic_w <= surface->w)
    {
        CSurface::Draw(surface, global::bmpArray[WINDOW_LOWER_FRAME].surface, pos_x, pos_y);
        pos_x += pic_w;
    }
    if(surface->w - pos_x > 0)
        CSurface::Draw(surface.get(), global::bmpArray[WINDOW_LOWER_FRAME].surface, pos_x, pos_y, 0, 0, surface->w - pos_x, pic_h);
    // left
    pic_h = std::min(h_, global::bmpArray[WINDOW_LEFT_FRAME].h);
    pos_x = 0;
    pos_y = 0;
    while(pos_y + pic_h <= surface->h)
    {
        CSurface::Draw(surface, global::bmpArray[WINDOW_LEFT_FRAME].surface, pos_x, pos_y);
        pos_y += pic_h;
    }
    if(surface->w - pos_x > 0)
        CSurface::Draw(surface.get(), global::bmpArray[WINDOW_LEFT_FRAME].surface, pos_x, pos_y, 0, 0, surface->w - pos_x, pic_h);
    // right
    pic_h = std::min(h_, global::bmpArray[WINDOW_RIGHT_FRAME].h);
    pos_x = w_ - global::bmpArray[WINDOW_RIGHT_FRAME].w;
    pos_y = 0;
    while(pos_y + pic_h <= surface->h)
    {
        CSurface::Draw(surface, global::bmpArray[WINDOW_RIGHT_FRAME].surface, pos_x, pos_y);
        pos_y += pic_h;
    }
    if(surface->w - pos_x > 0)
        CSurface::Draw(surface.get(), global::bmpArray[WINDOW_RIGHT_FRAME].surface, pos_x, pos_y, 0, 0, surface->w - pos_x, pic_h);

    // now draw the corners
    CSurface::Draw(surface, global::bmpArray[WINDOW_LEFT_UPPER_CORNER].surface, 0, 0);
    CSurface::Draw(surface, global::bmpArray[WINDOW_RIGHT_UPPER_CORNER].surface, w_ - global::bmpArray[WINDOW_RIGHT_UPPER_CORNER].w, 0);
    CSurface::Draw(surface, global::bmpArray[WINDOW_CORNER_RECTANGLE].surface, 0, h_ - global::bmpArray[WINDOW_CORNER_RECTANGLE].h);
    CSurface::Draw(surface, global::bmpArray[WINDOW_CORNER_RECTANGLE].surface, w_ - global::bmpArray[WINDOW_CORNER_RECTANGLE].w,
                   h_ - global::bmpArray[WINDOW_CORNER_RECTANGLE].h);
    // now the corner buttons
    // close
    if(canClose)
    {
        if(canClose_clicked)
            closebutton = WINDOW_BUTTON_CLOSE_CLICKED;
        else if(canClose_marked)
            closebutton = WINDOW_BUTTON_CLOSE_MARKED;
        else
            closebutton = WINDOW_BUTTON_CLOSE;
        CSurface::Draw(surface, global::bmpArray[closebutton].surface, 0, 0);
    }
    // minimize
    if(canMinimize)
    {
        if(canMinimize_clicked)
            minimizebutton = WINDOW_BUTTON_MINIMIZE_CLICKED;
        else if(canMinimize_marked)
            minimizebutton = WINDOW_BUTTON_MINIMIZE_MARKED;
        else
            minimizebutton = WINDOW_BUTTON_MINIMIZE;
        CSurface::Draw(surface, global::bmpArray[minimizebutton].surface, w_ - global::bmpArray[minimizebutton].w, 0);
    }
    // resize
    if(canResize)
    {
        if(canResize_clicked)
            resizebutton = WINDOW_BUTTON_RESIZE_CLICKED;
        else if(canResize_marked)
            resizebutton = WINDOW_BUTTON_RESIZE_MARKED;
        else
            resizebutton = WINDOW_BUTTON_RESIZE;
        CSurface::Draw(surface, global::bmpArray[resizebutton].surface, w_ - global::bmpArray[resizebutton].w,
                       h_ - global::bmpArray[resizebutton].h);
    }

    return true;
}

void CWindow::setInactive()
{
    active = false;
    clicked = false;
    marked = false;
    needRender = true;

    for(auto& textfield : getTextFields())
    {
        textfield->setInactive();
    }
}

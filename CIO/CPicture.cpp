#include "CPicture.h"
#include "../CSurface.h"
#include "../globals.h"

CPicture::CPicture(void callback(int), int clickedParam, Uint16 x, Uint16 y, int picture)
{
    marked = false;
    clicked = false;
    this->x = x;
    this->y = y;
    if(picture >= 0)
        this->picture_ = picture;
    else
        this->picture_ = 0;
    this->w = global::bmpArray[picture].w;
    this->h = global::bmpArray[picture].h;
    this->callback = callback;
    this->clickedParam = clickedParam;
    motionEntryParam = -1;
    motionLeaveParam = -1;
    needRender = true;
}

void CPicture::setMouseData(const SDL_MouseMotionEvent& motion)
{
    // cursor is on the picture
    if((motion.x >= x) && (motion.x < x + w) && (motion.y >= y) && (motion.y < y + h))
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
        if((button.state == SDL_PRESSED) && (button.x >= x) && (button.x < x + w) && (button.y >= y) && (button.y < y + h))
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
        Surf_Picture = makeRGBSurface(w, h);
        if(!Surf_Picture)
            return false;
        SDL_SetColorKey(Surf_Picture.get(), SDL_TRUE, SDL_MapRGB(Surf_Picture->format, 0, 0, 0));
    }

    CSurface::Draw(Surf_Picture, global::bmpArray[picture_].surface, 0, 0);

    return true;
}

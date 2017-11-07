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
        this->picture = picture;
    else
        this->picture = 0;
    this->w = global::bmpArray[picture].w;
    this->h = global::bmpArray[picture].h;
    this->callback = callback;
    this->clickedParam = clickedParam;
    motionEntryParam = -1;
    motionLeaveParam = -1;
    Surf_Picture = NULL;
    needSurface = true;
    needRender = true;
}

CPicture::~CPicture()
{
    SDL_FreeSurface(Surf_Picture);
}

void CPicture::setMouseData(SDL_MouseMotionEvent motion)
{
    // cursor is on the picture
    if((motion.x >= x) && (motion.x < x + w) && (motion.y >= y) && (motion.y < y + h))
    {
        if(motion.state != SDL_PRESSED || motion.state == SDL_RELEASED)
        {
            marked = true;
            if(motionEntryParam >= 0 && callback != NULL)
                callback(motionEntryParam);
        }
    } else
    {
        // picture was marked before and mouse cursor is on the picture now, so do the callback
        if(motionLeaveParam >= 0 && callback != NULL && marked == true)
            callback(motionLeaveParam);
        marked = false;
    }
    needRender = true;
}

void CPicture::setMouseData(SDL_MouseButtonEvent button)
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
            if(marked && callback != NULL)
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
    if(needSurface)
    {
        SDL_FreeSurface(Surf_Picture);
        Surf_Picture = NULL;
        if((Surf_Picture = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 0, 0, 0, 0)) == NULL)
            return false;
        SDL_SetColorKey(Surf_Picture, SDL_SRCCOLORKEY, SDL_MapRGB(Surf_Picture->format, 0, 0, 0));
        needSurface = false;
    }

    CSurface::Draw(Surf_Picture, global::bmpArray[picture].surface, 0, 0);

    return true;
}

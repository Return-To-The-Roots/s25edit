#include "CButton.h"
#include "../CSurface.h"
#include "../globals.h"
#include "CFont.h"

CButton::CButton(void callback(int), int clickedParam, Sint16 x, Sint16 y, Uint16 w, Uint16 h, int color, const char* text,
                 int button_picture)
{
    marked = false;
    clicked = false;
    this->x_ = x;
    this->y_ = y;
    this->w = w;
    this->h = h;
    setColor(color);
    this->button_picture = button_picture;
    button_text = text;
    button_text_color = FONT_YELLOW;
    this->callback_ = callback;
    this->clickedParam = clickedParam;
    motionEntryParam = -1;
    motionLeaveParam = -1;
    Surf_Button = nullptr;
    needSurface = true;
    needRender = true;
}

CButton::~CButton()
{
    SDL_FreeSurface(Surf_Button);
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
    if((motion.x >= x_) && (motion.x < x_ + w) && (motion.y >= y_) && (motion.y < y_ + h))
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
        if((button.state == SDL_PRESSED) && (button.x >= x_) && (button.x < x_ + w) && (button.y >= y_) && (button.y < y_ + h))
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
    if(needSurface)
    {
        SDL_FreeSurface(Surf_Button);
        Surf_Button = nullptr;
        if((Surf_Button = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 0, 0, 0, 0)) == nullptr)
            return false;
        needSurface = false;
    }

    // at first completly fill the background (not the fastest way, but simplier)
    if(w <= global::bmpArray[pic_background].w)
        pic_w = w;
    else
        pic_w = global::bmpArray[pic_background].w;

    if(h <= global::bmpArray[pic_background].h)
        pic_h = h;
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
            CSurface::Draw(Surf_Button, global::bmpArray[pic_background].surface, pos_x, pos_y, 0, 0, pic_w, Surf_Button->h - pos_y);

        pos_y = 0;
        pos_x += pic_w;
    }

    if(Surf_Button->w - pos_x > 0)
    {
        while(pos_y + pic_h <= Surf_Button->h)
        {
            CSurface::Draw(Surf_Button, global::bmpArray[pic_background].surface, pos_x, pos_y, 0, 0, Surf_Button->w - pos_x, pic_h);
            pos_y += pic_h;
        }

        if(Surf_Button->h - pos_y > 0)
            CSurface::Draw(Surf_Button, global::bmpArray[pic_background].surface, pos_x, pos_y, 0, 0, Surf_Button->w - pos_x,
                           Surf_Button->h - pos_y);
    }

    // draw partial black frame
    if(clicked)
    {
        // black frame is left and up
        // draw vertical line
        pos_x = 0;
        for(int y = 0; y < h; y++)
            CSurface::DrawPixel_RGB(Surf_Button, pos_x, y, 0, 0, 0);

        // draw vertical line
        pos_x = 1;
        for(int y = 0; y < h - 1; y++)
            CSurface::DrawPixel_RGB(Surf_Button, pos_x, y, 0, 0, 0);

        // draw horizontal line
        pos_y = 0;
        for(int x = 0; x < w; x++)
            CSurface::DrawPixel_RGB(Surf_Button, x, pos_y, 0, 0, 0);

        // draw horizontal line
        pos_y = 1;
        for(int x = 0; x < w - 1; x++)
            CSurface::DrawPixel_RGB(Surf_Button, x, pos_y, 0, 0, 0);
    } else
    {
        // black frame is right and down
        // draw vertical line
        pos_x = w - 1;
        for(int y = 0; y < h; y++)
            CSurface::DrawPixel_RGB(Surf_Button, pos_x, y, 0, 0, 0);

        // draw vertical line
        pos_x = w - 2;
        for(int y = 1; y < h; y++)
            CSurface::DrawPixel_RGB(Surf_Button, pos_x, y, 0, 0, 0);

        // draw horizontal line
        pos_y = h - 1;
        for(int x = 0; x < w; x++)
            CSurface::DrawPixel_RGB(Surf_Button, x, pos_y, 0, 0, 0);

        // draw horizontal line
        pos_y = h - 2;
        for(int x = 1; x < w; x++)
            CSurface::DrawPixel_RGB(Surf_Button, x, pos_y, 0, 0, 0);
    }

    // draw the foreground --> at first the color (marked or unmarked) and then the picture or text
    if(w <= global::bmpArray[pic_normal].w)
        pic_w = w;
    else
        pic_w = global::bmpArray[pic_normal].w;

    if(h <= global::bmpArray[pic_normal].h)
        pic_h = h;
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
            CSurface::Draw(Surf_Button, global::bmpArray[foreground].surface, pos_x, pos_y, 0, 0, pic_w, Surf_Button->h - 2 - pos_y);

        pos_y = 2;
        pos_x += pic_w;
    }

    if(Surf_Button->w - 2 - pos_x > 0)
    {
        while(pos_y + pic_h <= Surf_Button->h - 2)
        {
            CSurface::Draw(Surf_Button, global::bmpArray[foreground].surface, pos_x, pos_y, 0, 0, Surf_Button->w - 2 - pos_x, pic_h);
            pos_y += pic_h;
        }

        if(Surf_Button->h - 2 - pos_y > 0)
            CSurface::Draw(Surf_Button, global::bmpArray[foreground].surface, pos_x, pos_y, 0, 0, Surf_Button->w - 2 - pos_x,
                           Surf_Button->h - 2 - pos_y);
    }

    // positioning the picture or write text
    if(button_picture >= 0)
    {
        // picture may not be bigger than the button
        if(global::bmpArray[button_picture].w <= Surf_Button->w && global::bmpArray[button_picture].h <= Surf_Button->h)
        {
            // get coordinates of the left upper corner where to positionate the picture
            int leftup_x = (int)(Surf_Button->w / 2) - (int)(global::bmpArray[button_picture].w / 2);
            int leftup_y = (int)(Surf_Button->h / 2) - (int)(global::bmpArray[button_picture].h / 2);
            // blit it
            CSurface::Draw(Surf_Button, global::bmpArray[button_picture].surface, leftup_x, leftup_y);
        } else
        {
            button_picture = -1;
            button_text = "PIC";
        }
    } else if(button_text)
        CFont::writeText(Surf_Button, button_text, (int)w / 2, (int)((h - 11) / 2), 11, button_text_color, FontAlign::Middle);

    return true;
}

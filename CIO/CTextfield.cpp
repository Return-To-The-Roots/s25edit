#include "CTextfield.h"
#include "../CSurface.h"
#include "../globals.h"
#include "CFont.h"

CTextfield::CTextfield(Sint16 x, Sint16 y, Uint16 cols, Uint16 rows, int fontsize, int text_color, int bg_color, bool button_style)
{
    active = false;
    this->x_ = x;
    this->y_ = y;
    this->cols = (cols < 1 ? 1 : cols);
    this->rows = (rows < 1 ? 1 : rows);
    this->fontsize_ = fontsize;
    // calc width by maximum number of chiffres (cols) + one blinking chiffre * average pixel_width of a chiffre (fontsize-3) + tolerance
    // for borders
    this->w = (this->cols + 1) * (fontsize - 3) + 4;
    // calc height ----------------| this is the row_separator from CFont.cpp    |----        + tolerance for borders
    this->h = this->rows * (fontsize + (fontsize == 9 ? 1 : (fontsize == 11 ? 3 : 4))) + 4;
    this->textColor_ = text_color;
    setColor(bg_color);
    // allocate memory for the text: chiffres (cols) + '\n' for each line * rows + blinking chiffre + '\0'
    text_.resize((this->cols + 1) * this->rows + 2);

    needRender = true;
    rendered = false;
    this->button_style = button_style;
    textObj = std::make_unique<CFont>("", x, y, fontsize, text_color);
}

bool CTextfield::hasRendered()
{
    if(rendered)
    {
        rendered = false;
        return true;
    } else
        return false;
}

void CTextfield::setColor(int color)
{
    switch(color)
    {
        case BUTTON_GREY:
            pic_foreground = BUTTON_GREY_DARK;
            pic_background = BUTTON_GREY_BACKGROUND;
            break;

        case BUTTON_RED1:
            pic_foreground = BUTTON_RED1_DARK;
            pic_background = BUTTON_RED1_BACKGROUND;
            break;

        case BUTTON_GREEN1:
            pic_foreground = BUTTON_GREEN1_DARK;
            pic_background = BUTTON_GREEN1_BACKGROUND;
            break;

        case BUTTON_GREEN2:
            pic_foreground = BUTTON_GREEN2_DARK;
            pic_background = BUTTON_GREEN2_BACKGROUND;
            break;

        case BUTTON_RED2:
            pic_foreground = BUTTON_RED2_DARK;
            pic_background = BUTTON_RED2_BACKGROUND;
            break;

        case BUTTON_STONE:
            pic_foreground = BUTTON_STONE_DARK;
            pic_background = BUTTON_STONE_BACKGROUND;
            break;

        default:
            pic_foreground = -1;
            pic_background = -1;
            break;
    }

    needRender = true;
}

void CTextfield::setTextColor(int color)
{
    textColor_ = color;
    textObj->setColor(color);
    needRender = true;
}

void CTextfield::setX(int x)
{
    this->x_ = x;
    textObj->setPos(Position(x_, y_));
}

void CTextfield::setY(int y)
{
    this->y_ = y;
    textObj->setPos(Position(x_, y_));
}

void CTextfield::setText(const std::string& text)
{
    char* txtPtr = &this->text_[0];
    int col_ctr = 1, row_ctr = 1;

    for(char c : text)
    {
        if(txtPtr >= &this->text_.back() - 2)
            break;

        if(col_ctr > cols)
        {
            if(row_ctr < rows)
            {
                *txtPtr = '\n';
                txtPtr++;
                row_ctr++;
            } else
                break;
            col_ctr = 1;
        }

        *txtPtr++ = c;
        col_ctr++;
    }
    *txtPtr = '\0';
}

void CTextfield::setMouseData(SDL_MouseButtonEvent button)
{
    // left button is pressed
    if(button.button == SDL_BUTTON_LEFT)
    {
        // if mouse button is pressed ON the textfield, set active=true
        if(button.state == SDL_PRESSED)
        {
            if((button.x >= x_) && (button.x < x_ + w) && (button.y >= y_) && (button.y < y_ + h))
                active = true;
            else
                active = false;
        }
    }
    needRender = true;
}

void CTextfield::setKeyboardData(const SDL_KeyboardEvent& key)
{
    unsigned char chiffre = '\0';
    char* txtPtr = &text_[0];
    int col_ctr = 1, row_ctr = 1;

    if(!active)
        return;

    if(key.type == SDL_KEYDOWN)
    {
        // go to '\0'
        while(*txtPtr != '\0')
        {
            col_ctr++;
            if(*txtPtr == '\n')
            {
                row_ctr++;
                col_ctr = 1;
            }
            txtPtr++;
        }
        // decrement col_ctr cause '\0' is not counted
        col_ctr--;
        // end of text memory reached? ( 'cols'-chiffres from the user + '\n' in each row * rows + blinking chiffre + '\0' -1 for pointer
        // adress range
        if(txtPtr >= &text_.back() - 2)
        {
            // end reached, user may only delete chiffres
            if(key.keysym.sym != SDLK_BACKSPACE)
                return;
        }

        switch(key.keysym.sym)
        {
            case SDLK_BACKSPACE:
                if(txtPtr > &text_[0])
                {
                    txtPtr--;
                    *txtPtr = '\0';
                }
                break;

            case SDLK_RETURN:
                if(row_ctr < rows)
                {
                    *txtPtr = '\n';
                    txtPtr++;
                    *txtPtr = '\0';
                }
                break;

            default:
                if(col_ctr >= cols)
                {
                    if(row_ctr < rows)
                    {
                        *txtPtr = '\n';
                        txtPtr++;
                    } else
                        break;
                }
                // decide which chiffre to save
                if((key.keysym.sym >= 48 && key.keysym.sym <= 57) || key.keysym.sym == 32)
                    chiffre = (unsigned char)key.keysym.sym;
                else if(key.keysym.sym >= 97 && key.keysym.sym <= 122)
                {
                    chiffre = (unsigned char)key.keysym.sym;
                    // test for capital letters (small letter and shift pressed)
                    if(key.keysym.mod & KMOD_SHIFT)
                        chiffre -= 32;
                } else if(key.keysym.sym == 45)
                {
                    chiffre = (unsigned char)key.keysym.sym;
                    // test for '_' ('-' and shift pressed)
                    if(key.keysym.mod & KMOD_SHIFT)
                        chiffre = 95;
                } else if(key.keysym.sym == 46 || key.keysym.sym == 47)
                {
                    chiffre = (unsigned char)key.keysym.sym;
                }

                if(chiffre != '\0')
                {
                    *txtPtr = chiffre;
                    txtPtr++;
                    *txtPtr = '\0';
                }
                break;
        }
        needRender = true;
    }
}

bool CTextfield::render()
{
    // position in the Surface 'Surf_Button'
    Uint16 pos_x = 0;
    Uint16 pos_y = 0;
    // width and height of the button color source picture
    Uint16 pic_w = 0;
    Uint16 pic_h = 0;
    // we save the time to let a chiffre blink
    static Uint32 currentTime;
    static Uint32 lastTime = SDL_GetTicks();
    static bool blinking_chiffre = false;
    // if the textfield is active, we need to render to show the blinking chiffre
    if(active)
    {
        currentTime = SDL_GetTicks();
        if(currentTime - lastTime > 500)
        {
            lastTime = currentTime;

            blinking_chiffre = !blinking_chiffre;
        }
        needRender = true;
    }

    // if we don't need to render, all is up to date, return true
    if(!needRender)
        return true;
    needRender = false;
    // if we need a new surface
    if(!Surf_Text)
    {
        Surf_Text = makeSdlSurface(SDL_SWSURFACE, w, h, 32);
        if(!Surf_Text)
            return false;
    }

    // draw the pictures for background and foreground or, if not set, fill with black color
    if(pic_background >= 0 && pic_foreground >= 0)
    {
        // in case the textfield should look like a button, we do it, otherwise we use pic_foreground for the background
        int pic;
        if(button_style)
            pic = pic_background;
        else
            pic = pic_foreground;

        // at first completly fill the background (not the fastest way, but simplier)
        if(w <= global::bmpArray[pic].w)
            pic_w = w;
        else
            pic_w = global::bmpArray[pic].w;

        if(h <= global::bmpArray[pic].h)
            pic_h = h;
        else
            pic_h = global::bmpArray[pic].h;

        while(pos_x + pic_w <= Surf_Text->w)
        {
            while(pos_y + pic_h <= Surf_Text->h)
            {
                CSurface::Draw(Surf_Text, global::bmpArray[pic].surface, pos_x, pos_y, 0, 0, pic_w, pic_h);
                pos_y += pic_h;
            }

            if(Surf_Text->h - pos_y > 0)
                CSurface::Draw(Surf_Text, global::bmpArray[pic].surface, pos_x, pos_y, 0, 0, pic_w, Surf_Text->h - pos_y);

            pos_y = 0;
            pos_x += pic_w;
        }

        if(Surf_Text->w - pos_x > 0)
        {
            while(pos_y + pic_h <= Surf_Text->h)
            {
                CSurface::Draw(Surf_Text, global::bmpArray[pic].surface, pos_x, pos_y, 0, 0, Surf_Text->w - pos_x, pic_h);
                pos_y += pic_h;
            }

            if(Surf_Text->h - pos_y > 0)
                CSurface::Draw(Surf_Text, global::bmpArray[pic].surface, pos_x, pos_y, 0, 0, Surf_Text->w - pos_x, Surf_Text->h - pos_y);
        }

        // if not button_style, we are finished, otherwise continue drawing
        if(button_style)
        {
            // draw partial black frame
            if(active)
            {
                // black frame is left and up
                // draw vertical line
                pos_x = 0;
                for(int y = 0; y < h; y++)
                    CSurface::DrawPixel_RGB(Surf_Text, pos_x, y, 0, 0, 0);

                // draw vertical line
                pos_x = 1;
                for(int y = 0; y < h - 1; y++)
                    CSurface::DrawPixel_RGB(Surf_Text, pos_x, y, 0, 0, 0);

                // draw horizontal line
                pos_y = 0;
                for(int x = 0; x < w; x++)
                    CSurface::DrawPixel_RGB(Surf_Text, x, pos_y, 0, 0, 0);

                // draw horizontal line
                pos_y = 1;
                for(int x = 0; x < w - 1; x++)
                    CSurface::DrawPixel_RGB(Surf_Text, x, pos_y, 0, 0, 0);
            } else
            {
                // black frame is right and down
                // draw vertical line
                pos_x = w - 1;
                for(int y = 0; y < h; y++)
                    CSurface::DrawPixel_RGB(Surf_Text, pos_x, y, 0, 0, 0);

                // draw vertical line
                pos_x = w - 2;
                for(int y = 1; y < h; y++)
                    CSurface::DrawPixel_RGB(Surf_Text, pos_x, y, 0, 0, 0);

                // draw horizontal line
                pos_y = h - 1;
                for(int x = 0; x < w; x++)
                    CSurface::DrawPixel_RGB(Surf_Text, x, pos_y, 0, 0, 0);

                // draw horizontal line
                pos_y = h - 2;
                for(int x = 1; x < w; x++)
                    CSurface::DrawPixel_RGB(Surf_Text, x, pos_y, 0, 0, 0);
            }

            // draw the foreground --> at first the color (marked or unmarked) and then the picture or text
            if(w <= global::bmpArray[pic_foreground].w)
                pic_w = w;
            else
                pic_w = global::bmpArray[pic_foreground].w;

            if(h <= global::bmpArray[pic_foreground].h)
                pic_h = h;
            else
                pic_h = global::bmpArray[pic_foreground].h;

            // beware overdrawing the left and upper frame
            pos_x = 2;
            pos_y = 2;

            // '-2' follows a few times, this means: beware overdrawing the right and lower frame
            while(pos_x + pic_w <= Surf_Text->w - 2)
            {
                while(pos_y + pic_h <= Surf_Text->h - 2)
                {
                    CSurface::Draw(Surf_Text, global::bmpArray[pic_foreground].surface, pos_x, pos_y, 0, 0, pic_w, pic_h);
                    pos_y += pic_h;
                }

                if(Surf_Text->h - 2 - pos_y > 0)
                    CSurface::Draw(Surf_Text, global::bmpArray[pic_foreground].surface, pos_x, pos_y, 0, 0, pic_w,
                                   Surf_Text->h - 2 - pos_y);

                pos_y = 2;
                pos_x += pic_w;
            }

            if(Surf_Text->w - 2 - pos_x > 0)
            {
                while(pos_y + pic_h <= Surf_Text->h - 2)
                {
                    CSurface::Draw(Surf_Text, global::bmpArray[pic_foreground].surface, pos_x, pos_y, 0, 0, Surf_Text->w - 2 - pos_x,
                                   pic_h);
                    pos_y += pic_h;
                }

                if(Surf_Text->h - 2 - pos_y > 0)
                    CSurface::Draw(Surf_Text, global::bmpArray[pic_foreground].surface, pos_x, pos_y, 0, 0, Surf_Text->w - 2 - pos_x,
                                   Surf_Text->h - 2 - pos_y);
            }
        }
    } else
        SDL_FillRect(Surf_Text.get(), nullptr, SDL_MapRGB(Surf_Text->format, 0, 0, 0));

    char* txtPtr = &text_[0];

    // go to '\0'
    while(*txtPtr != '\0')
        txtPtr++;
    // add blinking chiffre if necessary
    if(blinking_chiffre && active)
    {
        *txtPtr = '>';
        txtPtr++;
        *txtPtr = '\0';
    }

    // write text
    textObj->setText(text_.data());
    textObj = std::make_unique<CFont>(&text_[0], x_, y_, fontsize_, textColor_);

    // delete blinking chiffre (otherwise it could be written between user input chiffres)
    if(blinking_chiffre && active)
    {
        txtPtr--;
        *txtPtr = '\0';
    }

    // blit text surface
    CSurface::Draw(Surf_Text, textObj->getSurface(), 2, 2);

    rendered = true;

    return true;
}

// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CTextfield.h"
#include "../Texture.h"
#include "../globals.h"
#include "CFont.h"
#include "CollisionDetection.h"
#include <glad/glad.h>
#include <cmath>

CTextfield::CTextfield(Position pos, Uint16 cols, Uint16 rows, FontSize fontsize, FontColor text_color, int bg_color,
                       bool button_style)
{
    active = false;
    this->cols = (cols < 1 ? 1 : cols);
    this->rows = (rows < 1 ? 1 : rows);
    // calc width by maximum number of chiffres (cols) + one blinking chiffre * average pixel_width of a chiffre
    // (fontsize-3) + tolerance for borders
    this->size_.x = (this->cols + 1) * (static_cast<unsigned>(fontsize) - 3) + 4;
    // calc height ----------------| this is the row_separator from CFont.cpp    |----        + tolerance for borders
    this->size_.y = this->rows * getLineHeight(fontsize) + 4;
    setColor(bg_color);
    // allocate memory for the text: chiffres (cols) + '\n' for each line * rows + blinking chiffre + '\0'
    text_.resize((this->cols + 1) * this->rows + 2);

    this->button_style = button_style;
    textObj = std::make_unique<CFont>("", pos, fontsize, text_color);
}

Position CTextfield::getPos() const
{
    return textObj->getPos();
}

void CTextfield::setPos(Position pos)
{
    textObj->setPos(pos);
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
}

void CTextfield::setTextColor(FontColor color)
{
    textObj->setColor(color);
}

void CTextfield::setText(const std::string& text)
{
    char* txtPtr = this->text_.data();
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
            active = IsPointInRect(button.x, button.y, Rect(getPos(), size_));
        }
    }
}

void CTextfield::setKeyboardData(const SDL_KeyboardEvent& key)
{
    unsigned char chiffre = '\0';
    char* txtPtr = text_.data();
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
        // end of text memory reached? ( 'cols'-chiffres from the user + '\n' in each row * rows + blinking chiffre +
        // '\0' -1 for pointer adress range
        if(txtPtr >= &text_.back() - 2)
        {
            // end reached, user may only delete chiffres
            if(key.keysym.sym != SDLK_BACKSPACE)
                return;
        }

        switch(key.keysym.sym)
        {
            case SDLK_BACKSPACE:
                if(txtPtr > text_.data())
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
                if((key.keysym.sym >= 48 && key.keysym.sym <= 57) || key.keysym.sym == 32 || key.keysym.sym == 46
                   || key.keysym.sym == 47)
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
                }

                if(chiffre != '\0')
                {
                    *txtPtr = chiffre;
                    txtPtr++;
                    *txtPtr = '\0';
                }
                break;
        }
    }
}

void CTextfield::draw(Position parentOrigin)
{
    const Position absPos = parentOrigin + getPos();
    const Rect area(absPos, size_);

    // Update cursor blink state
    static Uint32 lastTime = 0; // Shared timer is OK here
    if(active)
    {
        const Uint32 currentTime = SDL_GetTicks();
        if(lastTime == 0)
            lastTime = currentTime;
        if(currentTime - lastTime > 500)
        {
            lastTime = currentTime;
            blinking_chiffre = !blinking_chiffre;
        }
    } else
    {
        blinking_chiffre = false;
    }

    // Draw the background / foreground
    if(pic_background >= 0 && pic_foreground >= 0)
    {
        if(button_style)
            drawButtonBox(area, active, pic_background, pic_foreground);
        else
            getTexture(ArchiveID::EDITIO, pic_foreground).drawTiled(area);
    } else
    {
        // Fill with black
        drawRect(area, 0xFF000000);
    }

    // Prepare text with cursor
    std::string displayText = getText();

    // Add blinking cursor if active
    if(blinking_chiffre && active)
    {
        displayText += '>';
    }

    // Draw the text
    if(!displayText.empty())
    {
        textObj->setText(displayText);
        textObj->draw(Position(parentOrigin.x + 2, parentOrigin.y + 4));
    }
}

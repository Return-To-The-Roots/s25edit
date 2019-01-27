#include "CFont.h"
#include "../CSurface.h"
#include "../globals.h"

CFont::CFont(std::string text, int x, int y, int fontsize, int color)
{
    this->x_ = x;
    this->y_ = y;
    this->string_ = std::move(text);
    // only three sizes are available (in pixels)
    if(fontsize != 9 && fontsize != 11 && fontsize != 14)
        this->fontsize_ = 9;
    else
        this->fontsize_ = fontsize;
    this->color_ = color;
    Surf_Font = nullptr;
    callback = nullptr;
    clickedParam = 0;
    // create surface and write text to it
    writeText();
}

CFont::~CFont()
{
    SDL_FreeSurface(Surf_Font);
}

void CFont::setFontsize(int fontsize)
{
    if(fontsize != 9 && fontsize != 11 && fontsize != 14)
        this->fontsize_ = 9;
    else
        this->fontsize_ = fontsize;
    writeText();
}

void CFont::setColor(int color)
{
    this->color_ = color;
    writeText();
}

void CFont::setText(std::string text)
{
    if(text == string_)
        return;
    SDL_FreeSurface(Surf_Font);
    Surf_Font = nullptr;
    this->string_ = std::move(text);
    writeText();
}

void CFont::setMouseData(SDL_MouseButtonEvent button)
{
    // left button is pressed
    if(button.button == SDL_BUTTON_LEFT)
    {
        if((button.x >= x_) && (button.x < x_ + w) && (button.y >= y_) && (button.y < y_ + h))
        {
            // if mouse button is pressed ON the text
            if((button.state == SDL_PRESSED) && callback)
            {
                setColor(FONT_ORANGE);
            } else if(button.state == SDL_RELEASED)
            {
                if(color_ == FONT_ORANGE && callback)
                    callback(clickedParam);
            }
        }
    }
}

unsigned getIndexForChar(uint8_t c)
{
    // subtract 32 shows that we start by spacebar as 'zero-position'
    // subtract another value after subtracting 32 means the skipped chiffres in ansi in compare to our enumeration
    // (cause we dont have all ansi-values as pictures)
    if(c >= 32 && c <= 90)
        return c - 32;
    /* \ */
    else if(c == 92)
        return 59;
    // _
    else if(c == 95)
        return 60;
    // between 'a' and 'z'
    else if(c >= 97 && c <= 122)
        return c - 32 - 4;
    // ©
    else if(c == 169)
        return 114;
    // Ä
    else if(c == 196)
        return 100;
    // Ç
    else if(c == 199)
        return 87;
    // Ö
    else if(c == 214)
        return 106;
    // Ü
    else if(c == 220)
        return 107;
    // ß
    else if(c == 223)
        return 113;
    // à
    else if(c == 224)
        return 92;
    // á
    else if(c == 225)
        return 108;
    // â
    else if(c == 226)
        return 90;
    // ä
    else if(c == 228)
        return 91;
    // ç
    else if(c == 231)
        return 93;
    // è
    else if(c == 232)
        return 96;
    // é
    else if(c == 233)
        return 89;
    // ê
    else if(c == 234)
        return 94;
    // ë
    else if(c == 235)
        return 95;
    // ì
    else if(c == 236)
        return 99;
    // í
    else if(c == 237)
        return 109;
    // î
    else if(c == 238)
        return 98;
    // ï
    else if(c == 239)
        return 97;
    // ñ
    else if(c == 241)
        return 112;
    // ò
    else if(c == 242)
        return 103;
    // ó
    else if(c == 243)
        return 110;
    // ô
    else if(c == 244)
        return 101;
    // ö
    else if(c == 246)
        return 102;
    // ù
    else if(c == 249)
        return 105;
    // ú
    else if(c == 250)
        return 111;
    // û
    else if(c == 251)
        return 104;
    // ü
    else if(c == 252)
        return 88;
    // chiffre not available, use '_' instead
    else
        return 60;
}

bool CFont::writeText()
{
    if(string_.empty())
        return true;
    // data for counting pixels to create the surface
    unsigned int pixel_ctr_w = 0;
    unsigned int pixel_ctr_w_tmp = 0;
    // ROW_SEPARATOR IS ALSO USED IN CTEXTFIELD-CLASS, SO DO NOT CHANGE!!
    int row_separator = (fontsize_ == 9 ? 1 : (fontsize_ == 11 ? 3 : 4));
    unsigned int pixel_ctr_h = fontsize_ + row_separator;
    bool pixel_count_loop = true;
    // counter for the drawed pixels (cause we dont want to draw outside of the surface)
    int pos_x = 0;
    int pos_y = 0;

    // now lets draw the chiffres
    auto chiffre = string_.begin();
    while(chiffre != string_.end())
    {
        // the index for the chiffre-picture in the global::bmpArray
        unsigned int chiffre_index;
        // set chiffre_index to the first chiffre (spacebar) depending on the fontsize
        switch(fontsize_)
        {
            case 9: chiffre_index = FONT9_SPACE; break;
            default: // in fact not necessary, cause this case is handled by the constructor
            case 11: chiffre_index = FONT11_SPACE; break;
            case 14: chiffre_index = FONT14_SPACE; break;
        }
        chiffre_index += getIndexForChar(*chiffre) * NUM_FONT_COLORS + color_;

        // if we only count pixels in this round
        if(pixel_count_loop)
        {
            if(*chiffre == '\n')
            {
                pixel_ctr_h += row_separator + fontsize_;
                if(pixel_ctr_w_tmp > pixel_ctr_w)
                    pixel_ctr_w = pixel_ctr_w_tmp;
                pixel_ctr_w_tmp = 0;
                ++chiffre;
            } else
            {
                pixel_ctr_w_tmp += global::bmpArray[chiffre_index].w;
                ++chiffre;
            }

            // if this was the last chiffre setup width, create surface and go in normal mode to write text to the surface
            if(chiffre == string_.end())
            {
                if(pixel_ctr_w_tmp > pixel_ctr_w)
                    pixel_ctr_w = pixel_ctr_w_tmp;
                w = pixel_ctr_w;
                h = pixel_ctr_h;
                if(Surf_Font)
                    SDL_FreeSurface(Surf_Font);
                if(!(Surf_Font = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 0, 0, 0, 0)))
                    return false;
                SDL_SetColorKey(Surf_Font, SDL_SRCCOLORKEY, SDL_MapRGB(Surf_Font->format, 0, 0, 0));
                chiffre = string_.begin();
                pixel_count_loop = false;
                continue;
            } else
                continue;
        }

        // now we have our index and can use global::bmpArray[chiffre_index] to get the picture

        // test for new line
        if(*chiffre == '\n')
        {
            pos_y += row_separator + fontsize_;
            pos_x = 0;
            ++chiffre;
            continue;
        }

        // if right end of surface is reached, stop drawing chiffres
        if(Surf_Font->w < pos_x + global::bmpArray[chiffre_index].w)
            break;
        // if lower end of surface is reached, stop drawing chiffres
        if(Surf_Font->h < pos_y + row_separator + global::bmpArray[chiffre_index].h)
            break;

        // draw the chiffre to the destination
        CSurface::Draw(Surf_Font, global::bmpArray[chiffre_index].surface, pos_x, pos_y);

        // set position for next chiffre depending on the width of the actual drawn
        // NOTE: there is a bug in the ansi 236 'ì' at fontsize 9, the width is 39, this is not useable, we will use the width of ansi 237
        // 'í' instead
        if(fontsize_ == 9 && static_cast<uint8_t>(*chiffre) == 236)
            pos_x += global::bmpArray[FONT9_SPACE + 109 * NUM_FONT_COLORS + color_].w;
        else
            pos_x += global::bmpArray[chiffre_index].w;

        // go to next chiffre
        ++chiffre;
    }
    return true;
}

bool CFont::writeText(SDL_Surface* Surf_Dest, const char* string, int x, int y, int fontsize, int color, int align)
{
    // data for necessary counting pixels depending on alignment
    unsigned int pixel_ctr_w = 0;
    bool pixel_count_loop;
    // the index for the chiffre-picture in the global::bmpArray
    unsigned int chiffre_index = 0;
    // pointer to the chiffres
    unsigned char* chiffre = (unsigned char*)string;
    // counter for the drawed pixels (cause we dont want to draw outside of the surface)
    int pos_x = x;
    int pos_y = y;

    if(!Surf_Dest || !string)
        return false;

    // only three sizes are available (in pixels)
    if(fontsize != 9 && fontsize != 11 && fontsize != 14)
        fontsize = 9;

    // are there enough vertical pixels to draw the chiffres?
    if(Surf_Dest->h < y + fontsize)
        return false;

    // in case of right or middle alignment we must count the pixels first
    switch(align)
    {
        case ALIGN_LEFT: pixel_count_loop = false; break;

        case ALIGN_MIDDLE: pixel_count_loop = true; break;

        case ALIGN_RIGHT: pixel_count_loop = true; break;

        default: // in default: align = ALIGN_LEFT
            pixel_count_loop = false;
            break;
    }

    // now lets draw the chiffres
    while(*chiffre != '\0')
    {
        // set chiffre_index to the first chiffre (spacebar) depending on the fontsize
        switch(fontsize)
        {
            case 9: chiffre_index = FONT9_SPACE; break;

            case 11: chiffre_index = FONT11_SPACE; break;

            case 14: chiffre_index = FONT14_SPACE; break;

            default: // in fact not necessary, cause this case is handled before
                break;
        }

        // subtract 32 shows that we start by spacebar as 'zero-position'
        // subtract another value after subtracting 32 means the skipped chiffres in ansi in compare to our enumeration (cause we dont have
        // all ansi-values as pictures)

        // between 'spacebar' and the 'Z'
        if(*chiffre >= 32 && *chiffre <= 90)
            chiffre_index += (*chiffre - 32) * NUM_FONT_COLORS + color;
        /* \ */
        else if(*chiffre == 92)
            chiffre_index += 59 * NUM_FONT_COLORS + color;
        // _
        else if(*chiffre == 95)
            chiffre_index += 60 * NUM_FONT_COLORS + color;
        // between 'a' and 'z'
        else if(*chiffre >= 97 && *chiffre <= 122)
            chiffre_index += (*chiffre - 32 - 4) * NUM_FONT_COLORS + color;
        // ©
        else if(*chiffre == 169)
            chiffre_index += 114 * NUM_FONT_COLORS + color;
        // Ä
        else if(*chiffre == 196)
            chiffre_index += 100 * NUM_FONT_COLORS + color;
        // Ç
        else if(*chiffre == 199)
            chiffre_index += 87 * NUM_FONT_COLORS + color;
        // Ö
        else if(*chiffre == 214)
            chiffre_index += 106 * NUM_FONT_COLORS + color;
        // Ü
        else if(*chiffre == 220)
            chiffre_index += 107 * NUM_FONT_COLORS + color;
        // ß
        else if(*chiffre == 223)
            chiffre_index += 113 * NUM_FONT_COLORS + color;
        // à
        else if(*chiffre == 224)
            chiffre_index += 92 * NUM_FONT_COLORS + color;
        // á
        else if(*chiffre == 225)
            chiffre_index += 108 * NUM_FONT_COLORS + color;
        // â
        else if(*chiffre == 226)
            chiffre_index += 90 * NUM_FONT_COLORS + color;
        // ä
        else if(*chiffre == 228)
            chiffre_index += 91 * NUM_FONT_COLORS + color;
        // ç
        else if(*chiffre == 231)
            chiffre_index += 93 * NUM_FONT_COLORS + color;
        // è
        else if(*chiffre == 232)
            chiffre_index += 96 * NUM_FONT_COLORS + color;
        // é
        else if(*chiffre == 233)
            chiffre_index += 89 * NUM_FONT_COLORS + color;
        // ê
        else if(*chiffre == 234)
            chiffre_index += 94 * NUM_FONT_COLORS + color;
        // ë
        else if(*chiffre == 235)
            chiffre_index += 95 * NUM_FONT_COLORS + color;
        // ì
        else if(*chiffre == 236)
            chiffre_index += 99 * NUM_FONT_COLORS + color;
        // í
        else if(*chiffre == 237)
            chiffre_index += 109 * NUM_FONT_COLORS + color;
        // î
        else if(*chiffre == 238)
            chiffre_index += 98 * NUM_FONT_COLORS + color;
        // ï
        else if(*chiffre == 239)
            chiffre_index += 97 * NUM_FONT_COLORS + color;
        // ñ
        else if(*chiffre == 241)
            chiffre_index += 112 * NUM_FONT_COLORS + color;
        // ò
        else if(*chiffre == 242)
            chiffre_index += 103 * NUM_FONT_COLORS + color;
        // ó
        else if(*chiffre == 243)
            chiffre_index += 110 * NUM_FONT_COLORS + color;
        // ô
        else if(*chiffre == 244)
            chiffre_index += 101 * NUM_FONT_COLORS + color;
        // ö
        else if(*chiffre == 246)
            chiffre_index += 102 * NUM_FONT_COLORS + color;
        // ù
        else if(*chiffre == 249)
            chiffre_index += 105 * NUM_FONT_COLORS + color;
        // ú
        else if(*chiffre == 250)
            chiffre_index += 111 * NUM_FONT_COLORS + color;
        // û
        else if(*chiffre == 251)
            chiffre_index += 104 * NUM_FONT_COLORS + color;
        // ü
        else if(*chiffre == 252)
            chiffre_index += 88 * NUM_FONT_COLORS + color;
        // chiffre not available, use '_' instead
        else
            chiffre_index += 60 * NUM_FONT_COLORS + color;

        // if we only count pixels in this round
        if(pixel_count_loop)
        {
            pixel_ctr_w += global::bmpArray[chiffre_index].w;

            // if text is to long to go further left, stop loop and begin writing at x=0
            if((align == ALIGN_MIDDLE) && (x - (unsigned int)(pixel_ctr_w / 2) <= 0))
                pos_x = 0;
            else if((align == ALIGN_RIGHT) && (Surf_Dest->w - 1 - pixel_ctr_w <= 0))
                pos_x = 0;

            ++chiffre;

            // if this was the last chiffre go in normal mode and write the text to the specified position
            if(*chiffre == '\0')
            {
                chiffre = (unsigned char*)string;

                if(align == ALIGN_MIDDLE)
                    pos_x = x - (unsigned int)(pixel_ctr_w / 2);
                else if(align == ALIGN_RIGHT)
                    pos_x = Surf_Dest->w - 1 - pixel_ctr_w;

                pixel_count_loop = false;
                continue;
            } else
                continue;
        }

        // now we have our index and can use global::bmpArray[chiffre_index] to get the picture

        // if right end of surface is reached, stop drawing chiffres
        if(Surf_Dest->w < pos_x + global::bmpArray[chiffre_index].w)
            break;

        // draw the chiffre to the destination
        CSurface::Draw(Surf_Dest, global::bmpArray[chiffre_index].surface, pos_x, pos_y);

        // set position for next chiffre depending on the width of the actual drawn
        // NOTE: there is a bug in the ansi 236 'ì' at fontsize 9, the width is 39, this is not useable, we will use the width of ansi 237
        // 'í' instead
        if(fontsize == 9 && *chiffre == 236)
            pos_x += global::bmpArray[FONT9_SPACE + 109 * NUM_FONT_COLORS + color].w;
        else
            pos_x += global::bmpArray[chiffre_index].w;

        // go to next chiffre
        ++chiffre;
    }

    return true;
}

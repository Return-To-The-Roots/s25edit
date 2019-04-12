#include "CFont.h"
#include "../CSurface.h"
#include "../globals.h"
#include <cassert>

CFont::CFont(std::string text, unsigned x, unsigned y, unsigned fontsize, unsigned color)
    : Surf_Font(nullptr), x_(x), y_(y), string_(std::move(text))
{
    // only three sizes are available (in pixels)
    if(fontsize != 9 && fontsize != 11 && fontsize != 14)
        this->fontsize_ = 9;
    else
        this->fontsize_ = fontsize;
    this->color_ = color;
    callback = nullptr;
    clickedParam = 0;
    // create surface and write text to it
    writeText();
}

CFont::~CFont()
{
    SDL_FreeSurface(Surf_Font);
}

void CFont::setFontsize(unsigned fontsize)
{
    if(fontsize != 9 && fontsize != 11 && fontsize != 14)
        this->fontsize_ = 9;
    else
        this->fontsize_ = fontsize;
    writeText();
}

void CFont::setColor(unsigned color)
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

namespace {
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

unsigned getIndexForChar(uint8_t c, unsigned fontsize, unsigned color)
{
    assert(color < NUM_FONT_COLORS);
    unsigned offset;
    switch(fontsize)
    {
        case 9: offset = FONT9_SPACE; break;
        default:
        case 11: offset = FONT11_SPACE; break;
        case 14: offset = FONT14_SPACE; break;
    }
    return offset + getIndexForChar(c) * NUM_FONT_COLORS + color;
}

unsigned getCharWidth(uint8_t c, unsigned fontsize, unsigned color)
{ // NOTE: there is a bug in the ansi 236 'ì' at fontsize 9, the width is 39, this is not useable, we will use the width of ansi 237
    // 'í' instead
    if(fontsize == 9 && c == 236)
        c = 109;
    return global::bmpArray[getIndexForChar(c, fontsize, color)].w;
}
} // namespace

bool CFont::writeText()
{
    if(string_.empty())
        return true;
    // data for counting pixels to create the surface
    unsigned pixel_ctr_w = 0;
    unsigned pixel_ctr_w_tmp = 0;
    // ROW_SEPARATOR IS ALSO USED IN CTEXTFIELD-CLASS, SO DO NOT CHANGE!!
    unsigned row_separator = (fontsize_ == 9 ? 1 : (fontsize_ == 11 ? 3 : 4));
    unsigned pixel_ctr_h = fontsize_ + row_separator;
    bool pixel_count_loop = true;
    // counter for the drawed pixels (cause we dont want to draw outside of the surface)
    unsigned pos_x = 0;
    unsigned pos_y = 0;

    // now lets draw the chiffres
    auto chiffre = string_.begin();
    while(chiffre != string_.end())
    {
        const auto charW = getCharWidth(*chiffre, fontsize_, color_);
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
                pixel_ctr_w_tmp += charW;
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
        if(Surf_Font->w < static_cast<int>(pos_x + charW))
            break;

        const auto chiffre_index = getIndexForChar(*chiffre, fontsize_, color_);

        // if lower end of surface is reached, stop drawing chiffres
        if(Surf_Font->h < static_cast<int>(pos_y + row_separator + global::bmpArray[chiffre_index].h))
            break;

        // draw the chiffre to the destination
        CSurface::Draw(Surf_Font, global::bmpArray[chiffre_index].surface, pos_x, pos_y);

        // set position for next chiffre
        pos_x += charW;

        // go to next chiffre
        ++chiffre;
    }
    return true;
}

bool CFont::writeText(SDL_Surface* Surf_Dest, const std::string& string, unsigned x, unsigned y, unsigned fontsize, unsigned color,
                      FontAlign align)
{
    // data for necessary counting pixels depending on alignment
    unsigned pixel_ctr_w = 0;
    // counter for the drawed pixels (cause we dont want to draw outside of the surface)
    unsigned pos_x = x;
    unsigned pos_y = y;

    if(!Surf_Dest || string.empty())
        return false;

    // only three sizes are available (in pixels)
    if(fontsize != 9 && fontsize != 11 && fontsize != 14)
        fontsize = 9;

    // are there enough vertical pixels to draw the chiffres?
    if(Surf_Dest->h < static_cast<int>(y + fontsize))
        return false;

    // in case of right or middle alignment we must count the pixels first
    auto pixel_count_loop = (align == ALIGN_MIDDLE) || (align == ALIGN_RIGHT);

    // now lets draw the chiffres
    auto chiffre = string.begin();
    while(chiffre != string.end())
    {
        const auto charW = getCharWidth(*chiffre, fontsize, color);
        // if we only count pixels in this round
        if(pixel_count_loop)
        {
            pixel_ctr_w += charW;

            // if text is to long to go further left, stop loop and begin writing at x=0
            if((align == ALIGN_MIDDLE && pixel_ctr_w / 2 > x) || static_cast<int>(pixel_ctr_w) >= Surf_Dest->w)
            {
                pos_x = 0;
                chiffre = string.begin();
                pixel_count_loop = false;
                continue;
            }

            ++chiffre;

            // if this was the last chiffre go in normal mode and write the text to the specified position
            if(chiffre == string.end())
            {
                chiffre = string.begin();

                if(align == ALIGN_MIDDLE)
                    pos_x = x - pixel_ctr_w / 2;
                else if(align == ALIGN_RIGHT)
                    pos_x = Surf_Dest->w - pixel_ctr_w;

                pixel_count_loop = false;
            }
            continue;
        }

        // if right end of surface is reached, stop drawing chiffres
        if(Surf_Dest->w < static_cast<int>(pos_x + charW))
            break;

        // draw the chiffre to the destination
        CSurface::Draw(Surf_Dest, global::bmpArray[getIndexForChar(*chiffre, fontsize, color)].surface, pos_x, pos_y);

        // set position for next chiffre
        pos_x += charW;

        // go to next chiffre
        ++chiffre;
    }

    return true;
}

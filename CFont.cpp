#include "CFont.h"

CFont::CFont(const char *string, int x, int y, int fontsize, int color)
{
    this->x = x;
    this->y = y;
    this->string = (unsigned char*) string;
    //only three sizes are available (in pixels)
    if (fontsize != 9 && fontsize != 11 && fontsize != 14)
        this->fontsize = 9;
    else
        this->fontsize = fontsize;
    this->color = color;
    Surf_Font = NULL;
    //create surface and write text to it
    if ( writeText(this->string) == false)
        throw "Could not create font surface!\n";
}

CFont::CFont(unsigned char *string, int x, int y, int fontsize, int color)
{
    this->x = x;
    this->y = y;
    this->string = string;
    //only three sizes are available (in pixels)
    if (fontsize != 9 && fontsize != 11 && fontsize != 14)
        this->fontsize = 9;
    else
        this->fontsize = fontsize;
    this->color = color;
    Surf_Font = NULL;
    //create surface and write text to it
    if ( writeText(this->string) == false)
        throw "Could not create font surface!\n";
}

CFont::~CFont()
{
    SDL_FreeSurface(Surf_Font);
}

void CFont::setFontsize(int fontsize)
{
    if (fontsize != 9 && fontsize != 11 && fontsize != 14)
        this->fontsize = 9;
    else
        this->fontsize = fontsize;
    writeText(string);
}

void CFont::setColor(int color)
{
    this->color = color;
    writeText(string);
}

bool CFont::writeText(const char *string)
{
    //data for counting pixels to create the surface
    unsigned int pixel_ctr = 0;
    bool pixel_count_loop = true;
    //the index for the chiffre-picture in the global::bmpArray
    unsigned int chiffre_index = 0;
    //pointer to the chiffres
    unsigned char *chiffre = (string == NULL ? this->string : (unsigned char*) string);
    //counter for the drawed pixels (cause we dont want to draw outside of the surface)
    int pos_x = 0;

    if (string == NULL && this->string == NULL)
        return false;

    //now lets draw the chiffres
    while (*chiffre != '\0')
    {
        //set chiffre_index to the first chiffre (spacebar) depending on the fontsize
        switch (fontsize)
        {
            case 9:     chiffre_index = FONT9_SPACE;
                        break;

            case 11:    chiffre_index = FONT11_SPACE;
                        break;

            case 14:    chiffre_index = FONT14_SPACE;
                        break;

            default:    //in fact not necessary, cause this case is handled by the constructor
                        break;
        }

        //subtract 32 shows that we start by spacebar as 'zero-position'
        //subtract another value after subtracting 32 means the skipped chiffres in ansi in compare to our enumeration (cause we dont have all ansi-values as pictures)

        // between 'spacebar' and the 'Z'
        if (*chiffre >= 32 && *chiffre <= 90)
            chiffre_index += (*chiffre - 32)*FONT_COLOR_COUNT+color;
        /* \ */
        else if (*chiffre == 92)
            chiffre_index += 59*FONT_COLOR_COUNT+color;
        // _
        else if (*chiffre == 95)
            chiffre_index += 60*FONT_COLOR_COUNT+color;
        // between 'a' and 'z'
        else if (*chiffre >= 97 && *chiffre <= 122)
            chiffre_index += (*chiffre - 32 - 4)*FONT_COLOR_COUNT+color;
        // ©
        else if (*chiffre == 169)
            chiffre_index += 114*FONT_COLOR_COUNT+color;
        // Ä
        else if (*chiffre == 196)
            chiffre_index += 100*FONT_COLOR_COUNT+color;
        // Ç
        else if (*chiffre == 199)
            chiffre_index += 87*FONT_COLOR_COUNT+color;
        // Ö
        else if (*chiffre == 214)
            chiffre_index += 106*FONT_COLOR_COUNT+color;
        // Ü
        else if (*chiffre == 220)
            chiffre_index += 107*FONT_COLOR_COUNT+color;
        // ß
        else if (*chiffre == 223)
            chiffre_index += 113*FONT_COLOR_COUNT+color;
        // à
        else if (*chiffre == 224)
            chiffre_index += 92*FONT_COLOR_COUNT+color;
        // á
        else if (*chiffre == 225)
            chiffre_index += 108*FONT_COLOR_COUNT+color;
        // â
        else if (*chiffre == 226)
            chiffre_index += 90*FONT_COLOR_COUNT+color;
        // ä
        else if (*chiffre == 228)
            chiffre_index += 91*FONT_COLOR_COUNT+color;
        // ç
        else if (*chiffre == 231)
            chiffre_index += 93*FONT_COLOR_COUNT+color;
        // è
        else if (*chiffre == 232)
            chiffre_index += 96*FONT_COLOR_COUNT+color;
        // é
        else if (*chiffre == 233)
            chiffre_index += 89*FONT_COLOR_COUNT+color;
        // ê
        else if (*chiffre == 234)
            chiffre_index += 94*FONT_COLOR_COUNT+color;
        // ë
        else if (*chiffre == 235)
            chiffre_index += 95*FONT_COLOR_COUNT+color;
        // ì
        else if (*chiffre == 236)
            chiffre_index += 99*FONT_COLOR_COUNT+color;
        // í
        else if (*chiffre == 237)
            chiffre_index += 109*FONT_COLOR_COUNT+color;
        // î
        else if (*chiffre == 238)
            chiffre_index += 98*FONT_COLOR_COUNT+color;
        // ï
        else if (*chiffre == 239)
            chiffre_index += 97*FONT_COLOR_COUNT+color;
        // ñ
        else if (*chiffre == 241)
            chiffre_index += 112*FONT_COLOR_COUNT+color;
        // ò
        else if (*chiffre == 242)
            chiffre_index += 103*FONT_COLOR_COUNT+color;
        // ó
        else if (*chiffre == 243)
            chiffre_index += 110*FONT_COLOR_COUNT+color;
        // ô
        else if (*chiffre == 244)
            chiffre_index += 101*FONT_COLOR_COUNT+color;
        // ö
        else if (*chiffre == 246)
            chiffre_index += 102*FONT_COLOR_COUNT+color;
        // ù
        else if (*chiffre == 249)
            chiffre_index += 105*FONT_COLOR_COUNT+color;
        // ú
        else if (*chiffre == 250)
            chiffre_index += 111*FONT_COLOR_COUNT+color;
        // û
        else if (*chiffre == 251)
            chiffre_index += 104*FONT_COLOR_COUNT+color;
        // ü
        else if (*chiffre == 252)
            chiffre_index += 88*FONT_COLOR_COUNT+color;
        //chiffre not available, use '_' instead
        else
            chiffre_index += 60*FONT_COLOR_COUNT+color;


        //if we only count pixels in this round
        if (pixel_count_loop)
        {
            pixel_ctr += global::bmpArray[chiffre_index].w;
            chiffre++;

            //if this was the last chiffre setup width, create surface and go in normal mode to write text to the surface
            if (*chiffre == '\0')
            {
                w = pixel_ctr;
                if (Surf_Font != NULL)
                    SDL_FreeSurface(Surf_Font);
                if ( (Surf_Font = SDL_CreateRGBSurface(SDL_SWSURFACE, w, fontsize, 32, 0, 0, 0, 0)) == NULL )
                    return false;
                SDL_SetColorKey(Surf_Font, SDL_SRCCOLORKEY, SDL_MapRGB(Surf_Font->format, 0, 0, 0));
                chiffre = this->string;
                pixel_count_loop = false;
                continue;
            }
            else
                continue;
        }

        //now we have our index and can use global::bmpArray[chiffre_index] to get the picture

        //if right end of surface is reached, stop drawing chiffres
        if (Surf_Font->w < pos_x + global::bmpArray[chiffre_index].w)
            break;

        //draw the chiffre to the destination
        CSurface::Draw(Surf_Font, global::bmpArray[chiffre_index].surface, pos_x, 0);

        //set position for next chiffre depending on the width of the actual drawn
        //NOTE: there is a bug in the ansi 236 'ì' at fontsize 9, the width is 39, this is not useable, we will use the width of ansi 237 'í' instead
        if (fontsize == 9 && *chiffre == 236)
            pos_x += global::bmpArray[FONT9_SPACE+109*FONT_COLOR_COUNT+color].w;
        else
            pos_x += global::bmpArray[chiffre_index].w;

        //go to next chiffre
        chiffre++;
    }
    return true;
}

bool CFont::writeText(unsigned char *string)
{
    return CFont::writeText((const char*)string);
}

bool CFont::writeText(SDL_Surface *Surf_Dest, const char *string, int x, int y, int fontsize, int color, int align)
{
    //data for necessary counting pixels depending on alignment
    unsigned int pixel_ctr = 0;
    bool pixel_count_loop;
    //the index for the chiffre-picture in the global::bmpArray
    unsigned int chiffre_index = 0;
    //pointer to the chiffres
    unsigned char *chiffre = (unsigned char*) string;
    //counter for the drawed pixels (cause we dont want to draw outside of the surface)
    int pos_x = x;

    if (global::bmpArray == NULL || Surf_Dest == NULL || string == NULL)
        return false;

    //only three sizes are available (in pixels)
    if (fontsize != 9 && fontsize != 11 && fontsize != 14)
        fontsize = 9;

    //are there enough vertical pixels to draw the chiffres?
    if (Surf_Dest->h < y + fontsize)
        return false;


    //in case of right or middle alignment we must count the pixels first
    switch (align)
    {
        case ALIGN_LEFT:        pixel_count_loop = false;
                                break;

        case ALIGN_MIDDLE:      pixel_count_loop = true;
                                break;

        case ALIGN_RIGHT:       pixel_count_loop = true;
                                break;

        default:                //in default: align = ALIGN_LEFT
                                pixel_count_loop = false;
                                break;
    }

    //now lets draw the chiffres
    while (*chiffre != '\0')
    {
        //set chiffre_index to the first chiffre (spacebar) depending on the fontsize
        switch (fontsize)
        {
            case 9:     chiffre_index = FONT9_SPACE;
                        break;

            case 11:    chiffre_index = FONT11_SPACE;
                        break;

            case 14:    chiffre_index = FONT14_SPACE;
                        break;

            default:    //in fact not necessary, cause this case is handled before
                        break;
        }

        //subtract 32 shows that we start by spacebar as 'zero-position'
        //subtract another value after subtracting 32 means the skipped chiffres in ansi in compare to our enumeration (cause we dont have all ansi-values as pictures)

        // between 'spacebar' and the 'Z'
        if (*chiffre >= 32 && *chiffre <= 90)
            chiffre_index += (*chiffre - 32)*FONT_COLOR_COUNT+color;
        /* \ */
        else if (*chiffre == 92)
            chiffre_index += 59*FONT_COLOR_COUNT+color;
        // _
        else if (*chiffre == 95)
            chiffre_index += 60*FONT_COLOR_COUNT+color;
        // between 'a' and 'z'
        else if (*chiffre >= 97 && *chiffre <= 122)
            chiffre_index += (*chiffre - 32 - 4)*FONT_COLOR_COUNT+color;
        // ©
        else if (*chiffre == 169)
            chiffre_index += 114*FONT_COLOR_COUNT+color;
        // Ä
        else if (*chiffre == 196)
            chiffre_index += 100*FONT_COLOR_COUNT+color;
        // Ç
        else if (*chiffre == 199)
            chiffre_index += 87*FONT_COLOR_COUNT+color;
        // Ö
        else if (*chiffre == 214)
            chiffre_index += 106*FONT_COLOR_COUNT+color;
        // Ü
        else if (*chiffre == 220)
            chiffre_index += 107*FONT_COLOR_COUNT+color;
        // ß
        else if (*chiffre == 223)
            chiffre_index += 113*FONT_COLOR_COUNT+color;
        // à
        else if (*chiffre == 224)
            chiffre_index += 92*FONT_COLOR_COUNT+color;
        // á
        else if (*chiffre == 225)
            chiffre_index += 108*FONT_COLOR_COUNT+color;
        // â
        else if (*chiffre == 226)
            chiffre_index += 90*FONT_COLOR_COUNT+color;
        // ä
        else if (*chiffre == 228)
            chiffre_index += 91*FONT_COLOR_COUNT+color;
        // ç
        else if (*chiffre == 231)
            chiffre_index += 93*FONT_COLOR_COUNT+color;
        // è
        else if (*chiffre == 232)
            chiffre_index += 96*FONT_COLOR_COUNT+color;
        // é
        else if (*chiffre == 233)
            chiffre_index += 89*FONT_COLOR_COUNT+color;
        // ê
        else if (*chiffre == 234)
            chiffre_index += 94*FONT_COLOR_COUNT+color;
        // ë
        else if (*chiffre == 235)
            chiffre_index += 95*FONT_COLOR_COUNT+color;
        // ì
        else if (*chiffre == 236)
            chiffre_index += 99*FONT_COLOR_COUNT+color;
        // í
        else if (*chiffre == 237)
            chiffre_index += 109*FONT_COLOR_COUNT+color;
        // î
        else if (*chiffre == 238)
            chiffre_index += 98*FONT_COLOR_COUNT+color;
        // ï
        else if (*chiffre == 239)
            chiffre_index += 97*FONT_COLOR_COUNT+color;
        // ñ
        else if (*chiffre == 241)
            chiffre_index += 112*FONT_COLOR_COUNT+color;
        // ò
        else if (*chiffre == 242)
            chiffre_index += 103*FONT_COLOR_COUNT+color;
        // ó
        else if (*chiffre == 243)
            chiffre_index += 110*FONT_COLOR_COUNT+color;
        // ô
        else if (*chiffre == 244)
            chiffre_index += 101*FONT_COLOR_COUNT+color;
        // ö
        else if (*chiffre == 246)
            chiffre_index += 102*FONT_COLOR_COUNT+color;
        // ù
        else if (*chiffre == 249)
            chiffre_index += 105*FONT_COLOR_COUNT+color;
        // ú
        else if (*chiffre == 250)
            chiffre_index += 111*FONT_COLOR_COUNT+color;
        // û
        else if (*chiffre == 251)
            chiffre_index += 104*FONT_COLOR_COUNT+color;
        // ü
        else if (*chiffre == 252)
            chiffre_index += 88*FONT_COLOR_COUNT+color;
        //chiffre not available, use '_' instead
        else
            chiffre_index += 60*FONT_COLOR_COUNT+color;


        //if we only count pixels in this round
        if (pixel_count_loop)
        {
            pixel_ctr += global::bmpArray[chiffre_index].w;
            //if text is to long to go further left, stop loop and begin writing at x=0
            if ( (align == ALIGN_MIDDLE) && (x - (unsigned int)(pixel_ctr / 2) <= 0) )
                pos_x = 0;
            else if ( (align == ALIGN_RIGHT) && (Surf_Dest->w-1 - pixel_ctr <= 0) )
                pos_x = 0;

            chiffre++;

            //if this was the last chiffre go in normal mode and write the text to the specified position
            if (*chiffre == '\0')
            {
                chiffre = (unsigned char*) string;

                if (align == ALIGN_MIDDLE)
                    pos_x = x - (unsigned int)(pixel_ctr / 2);
                else if (align == ALIGN_RIGHT)
                    pos_x = Surf_Dest->w-1 - pixel_ctr;

                pixel_count_loop = false;
                continue;
            }
            else
                continue;
        }

        //now we have our index and can use global::bmpArray[chiffre_index] to get the picture

        //if right end of surface is reached, stop drawing chiffres
        if (Surf_Dest->w < pos_x + global::bmpArray[chiffre_index].w)
            break;

        //draw the chiffre to the destination
        CSurface::Draw(Surf_Dest, global::bmpArray[chiffre_index].surface, pos_x, y);

        //set position for next chiffre depending on the width of the actual drawn
        //NOTE: there is a bug in the ansi 236 'ì' at fontsize 9, the width is 39, this is not useable, we will use the width of ansi 237 'í' instead
        if (fontsize == 9 && *chiffre == 236)
            pos_x += global::bmpArray[FONT9_SPACE+109*FONT_COLOR_COUNT+color].w;
        else
            pos_x += global::bmpArray[chiffre_index].w;

        //go to next chiffre
        chiffre++;
    }

    return true;
}

bool CFont::writeText(SDL_Surface *Surf_Dest, unsigned char *string, int x, int y, int fontsize, int color, int align)
{
    return CFont::writeText(Surf_Dest, (const char*)string, x, y, fontsize, color, align);
}

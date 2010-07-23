#ifndef _CFONT_H
    #define _CFONT_H

#include "includes.h"

class CFont
{
    friend class CDebug;

    private:
        SDL_Surface *Surf_Font;
        Uint16 x;
        Uint16 y;
        Uint16 w;
        unsigned char *string;
        int fontsize;   //== Uint16 h;
        int color;

    public:
        //Constructor - Destructor
        CFont(const char *string, int x = 0, int y = 0, int fontsize = 9, int color = FONT_BLUE);
        CFont(unsigned char *string, int x = 0, int y = 0, int fontsize = 9, int color = FONT_BLUE);
        ~CFont();
        //Access
        int getX(void) { return x; };
        int getY(void) { return y; };
        void setX(int x) { this->x = x; };
        void setY(int y) { this->y = y; };
        int getW(void) { return w; };
        int getH(void) { return fontsize; };
        void setFontsize(int fontsize);
        void setColor(int color);
        SDL_Surface* getSurface(void) { return Surf_Font; };
        //Methods
        //fontsize can be 9, 11 or 14 (otherwise it will be set to 9)
        bool writeText(const char *string);
        bool writeText(unsigned char *string);
        //this functions can used as CFont::writeText to write text directly to a surface without creating an object
        static bool writeText(SDL_Surface *Surf_Dest, const char *string, int x = 0, int y = 0, int fontsize = 9, int color = FONT_BLUE, int align = LEFT);
        static bool writeText(SDL_Surface *Surf_Dest, unsigned char *string, int x = 0, int y = 0, int fontsize = 9, int color = FONT_BLUE, int align = LEFT);
};

#endif
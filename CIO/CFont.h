#ifndef _CFONT_H
#define _CFONT_H

#include "../defines.h"
#include <SDL.h>
#include <string>

class CFont
{
    friend class CDebug;

private:
    SDL_Surface* Surf_Font;
    Uint16 x_;
    Sint16 y_;
    Uint16 w;
    Uint16 h;
    std::string string_;
    int fontsize_; //== Uint16 h;
    int color_;
    void (*callback)(int);
    int clickedParam;

public:
    // Constructor - Destructor
    CFont(std::string text, int x = 0, int y = 0, int fontsize = 9, int color = FONT_YELLOW);
    ~CFont();
    // Access
    int getX() { return x_; };
    int getY() { return y_; };
    void setX(int x) { this->x_ = x; };
    void setY(int y) { this->y_ = y; };
    int getW() { return w; };
    int getH() { return fontsize_; };
    void setFontsize(int fontsize);
    void setColor(int color);
    int getColor() { return color_; }
    void setText(std::string text);
    void setCallback(void (*callback)(int), int param)
    {
        this->callback = callback;
        clickedParam = param;
    }
    void unsetCallback()
    {
        callback = nullptr;
        clickedParam = 0;
    }
    void setMouseData(SDL_MouseButtonEvent button);
    SDL_Surface* getSurface() { return Surf_Font; };
    // Methods
    // fontsize can be 9, 11 or 14 (otherwise it will be set to 9) ---- '\n' is possible
    bool writeText();
    // this function can be used as CFont::writeText to write text directly to a surface without creating an object
    static bool writeText(SDL_Surface* Surf_Dest, const char* string, int x = 0, int y = 0, int fontsize = 9, int color = FONT_YELLOW,
                          int align = ALIGN_LEFT);
};

#endif

#ifndef _CFONT_H
#define _CFONT_H

#include "../defines.h"
#include <SDL.h>
#include <functional>
#include <string>

class CFont
{
    friend class CDebug;

private:
    SDL_Surface* Surf_Font;
    Uint16 x_;
    Uint16 y_;
    Uint16 w;
    Uint16 h;
    std::string string_;
    unsigned fontsize_; //== Uint16 h;
    unsigned color_, initialColor_;
    std::function<void(int)> callback;
    int clickedParam;

public:
    // Constructor - Destructor
    CFont(std::string text, unsigned x = 0, unsigned y = 0, unsigned fontsize = 9, unsigned color = FONT_YELLOW);
    ~CFont();
    // Access
    int getX() { return x_; };
    int getY() { return y_; };
    void setX(unsigned x) { this->x_ = x; };
    void setY(unsigned y) { this->y_ = y; };
    unsigned getW() { return w; };
    unsigned getH() { return fontsize_; };
    void setFontsize(unsigned fontsize);
    void setColor(unsigned color);
    unsigned getColor() { return color_; }
    void setText(std::string text);
    void setCallback(std::function<void(int)> callback, int param)
    {
        this->callback = std::move(callback);
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
    static bool writeText(SDL_Surface* Surf_Dest, const std::string& string, unsigned x = 0, unsigned y = 0, unsigned fontsize = 9,
                          unsigned color = FONT_YELLOW, FontAlign align = ALIGN_LEFT);
};

#endif

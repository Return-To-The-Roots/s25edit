#ifndef _CTEXTFIELD_H
#define _CTEXTFIELD_H

#include "../includes.h"
#include <vector>

class CFont;

class CTextfield
{
    friend class CDebug;

private:
    SDL_Surface* Surf_Text;
    CFont* textObj;
    bool needSurface;
    bool needRender;
    Uint16 x;
    Uint16 y;
    Uint16 w;
    Uint16 h;
    Uint16 cols;
    Uint16 rows;
    int fontsize;
    int pic_background;
    int pic_foreground;
    std::vector<char> text;
    int text_color;
    // if active, keyboard data will be delivered and the cursor is blinking
    bool active;
    // we need this to say the window if it needs to render, otherwise no blinking cursor and no chiffres are shown
    bool rendered;
    // if true, the textfield looks like a button
    bool button_style;

public:
    // Constructor - Destructor
    CTextfield(Uint16 x = 0, Uint16 y = 0, Uint16 cols = 10, Uint16 rows = 1, int fontsize = 14, int text_color = FONT_YELLOW,
               int bg_color = -1, bool button_style = false);
    ~CTextfield();
    // Access
    int getX() { return x; };
    int getY() { return y; };
    int getW() { return w; };
    int getH() { return h; };
    int getCols() { return cols; };
    int getRows() { return rows; };
    void setX(int x) { this->x = x; }
    void setY(int y) { this->y = y; }
    void setText(const char* text);
    void setActive() { active = true; }
    void setInactive() { active = false; }
    bool isActive() { return active; }
    bool hasRendered();
    void setMouseData(SDL_MouseButtonEvent button);
    void setKeyboardData(SDL_KeyboardEvent key);
    bool render();
    SDL_Surface* getSurface()
    {
        render();
        return Surf_Text;
    }
    void setColor(int color);
    void setTextColor(int color)
    {
        text_color = color;
        needRender = true;
    }
    const char* getText() { return &text[0]; }
};

#endif

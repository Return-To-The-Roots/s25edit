#ifndef _CTEXTFIELD_H
#define _CTEXTFIELD_H

#include "defines.h"
#include <SDL.h>
#include <vector>

class CFont;

class CTextfield
{
    friend class CDebug;

private:
    SdlSurface Surf_Text;
    std::unique_ptr<CFont> textObj;
    bool needRender;
    Sint16 x_;
    Sint16 y_;
    Uint16 w;
    Uint16 h;
    Uint16 cols;
    Uint16 rows;
    int fontsize_;
    int pic_background;
    int pic_foreground;
    std::vector<char> text_;
    int textColor_;
    // if active, keyboard data will be delivered and the cursor is blinking
    bool active;
    // we need this to say the window if it needs to render, otherwise no blinking cursor and no chiffres are shown
    bool rendered;
    // if true, the textfield looks like a button
    bool button_style;

public:
    // Constructor - Destructor
    CTextfield(Sint16 x = 0, Sint16 y = 0, Uint16 cols = 10, Uint16 rows = 1, int fontsize = 14, int text_color = FONT_YELLOW,
               int bg_color = -1, bool button_style = false);
    // Access
    int getX() { return x_; };
    int getY() { return y_; };
    int getW() { return w; };
    int getH() { return h; };
    int getCols() { return cols; };
    int getRows() { return rows; };
    void setX(int x);
    void setY(int y);
    void setText(const std::string& text);
    void setActive() { active = true; }
    void setInactive() { active = false; }
    bool isActive() { return active; }
    bool hasRendered();
    void setMouseData(SDL_MouseButtonEvent button);
    void setKeyboardData(const SDL_KeyboardEvent& key);
    bool render();
    SDL_Surface* getSurface()
    {
        render();
        return Surf_Text.get();
    }
    void setColor(int color);
    void setTextColor(int color);
    std::string getText() const { return &text_.front(); }
};

#endif

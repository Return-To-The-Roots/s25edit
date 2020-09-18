#pragma once

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
    Uint16 w;
    Uint16 h;
    Uint16 cols;
    Uint16 rows;
    int pic_background;
    int pic_foreground;
    std::vector<char> text_;
    // if active, keyboard data will be delivered and the cursor is blinking
    bool active;
    // we need this to say the window if it needs to render, otherwise no blinking cursor and no chiffres are shown
    bool rendered;
    // if true, the textfield looks like a button
    bool button_style;

public:
    // Constructor - Destructor
    CTextfield(Sint16 x = 0, Sint16 y = 0, Uint16 cols = 10, Uint16 rows = 1, FontSize fontsize = FontSize::Large,
               FontColor text_color = FontColor::Yellow, int bg_color = -1, bool button_style = false);
    // Access
    int getX() const;
    int getY() const;
    int getW() const { return w; }
    int getH() const { return h; }
    int getCols() const { return cols; }
    int getRows() const { return rows; }
    void setX(int x);
    void setY(int y);
    void setText(const std::string& text);
    void setActive() { active = true; }
    void setInactive() { active = false; }
    bool isActive() const { return active; }
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
    void setTextColor(FontColor color);
    std::string getText() const { return text_.data(); }
};

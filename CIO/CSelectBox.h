#ifndef _CSELECTBOX_H
#define _CSELECTBOX_H

#include "../defines.h"
#include <SDL.h>
#include <functional>
#include <memory>
#include <vector>

class CFont;
class CButton;

class CSelectBox
{
    friend class CDebug;

private:
    SDL_Surface* Surf_SelectBox;
    std::vector<std::unique_ptr<CFont>> Entries;
    bool needSurface;
    bool needRender;
    Uint16 x_;
    Uint16 y_;
    Uint16 w_;
    Uint16 h_;
    Uint16 last_text_pos_y;
    int fontsize;
    int pic_background;
    int pic_foreground;
    int text_color;
    // we need this to say the window if it needs to render, otherwise no chiffre are shown
    bool rendered;
    std::unique_ptr<CButton> ScrollUpButton;
    std::unique_ptr<CButton> ScrollDownButton;

public:
    // Constructor - Destructor
    CSelectBox(Uint16 x = 0, Uint16 y = 0, Uint16 w = 100, Uint16 h = 100, int fontsize = 14, int text_color = FONT_YELLOW,
               int bg_color = -1);
    ~CSelectBox();
    // Access;
    int getX() { return x_; }
    int getY() { return y_; }
    int getW() { return w_; }
    int getH() { return h_; }
    void setX(int x) { this->x_ = x; }
    void setY(int y) { this->y_ = y; }
    bool hasRendered();
    void setMouseData(SDL_MouseButtonEvent button);
    void setMouseData(SDL_MouseMotionEvent motion);
    bool render();
    SDL_Surface* getSurface()
    {
        render();
        return Surf_SelectBox;
    }
    void setColor(int color);
    void setTextColor(int color)
    {
        text_color = color;
        needRender = true;
    }
    void setOption(const std::string& string, std::function<void(int)> callback = nullptr, int param = 0);
};

#endif

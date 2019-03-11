#ifndef _CBUTTON_H
#define _CBUTTON_H

#include "../includes.h"

class CButton
{
    friend class CDebug;

private:
    SDL_Surface* Surf_Button;
    bool needSurface;
    bool needRender;
    Uint16 x_;
    Uint16 y_;
    Uint16 w;
    Uint16 h;
    int pic_normal;
    int pic_marked;
    int pic_background;
    int button_picture;
    const char* button_text;
    int button_text_color;
    bool marked;
    bool clicked;
    void (*callback_)(int);
    int clickedParam;
    int motionEntryParam;
    int motionLeaveParam;

public:
    // Constructor - Destructor
    CButton(void callback(int), int clickedParam, Uint16 x = 0, Uint16 y = 0, Uint16 w = 20, Uint16 h = 20, int color = BUTTON_GREY,
            const char* text = nullptr, int button_picture = -1);
    ~CButton();
    // Access
    int getX() { return x_; };
    int getY() { return y_; };
    int getW() { return w; };
    int getH() { return h; };
    void setX(int x) { this->x_ = x; };
    void setY(int y) { this->y_ = y; };
    void setButtonPicture(int picture);
    void setButtonText(const char* text);
    void setMouseData(const SDL_MouseMotionEvent& motion);
    void setMouseData(const SDL_MouseButtonEvent& button);
    bool render();
    SDL_Surface* getSurface()
    {
        render();
        return Surf_Button;
    };
    void setColor(int color);
    void setTextColor(int color)
    {
        button_text_color = color;
        needRender = true;
    };
    void setMotionParams(int entry, int leave)
    {
        motionEntryParam = entry;
        motionLeaveParam = leave;
    };
};

#endif

#ifndef _CSELECTBOX_H
#define _CSELECTBOX_H

#include "defines.h"
#include "SdlSurface.h"
#include <functional>
#include <memory>
#include <vector>

class CFont;
class CButton;

class CSelectBox
{
    friend class CDebug;

private:
    SdlSurface Surf_SelectBox;
    std::vector<std::unique_ptr<CFont>> Entries;
    Point16 pos_;
    Extent16 size_;
    int fontsize;
    int pic_background;
    int pic_foreground;
    int text_color;
    std::unique_ptr<CButton> ScrollUpButton;
    std::unique_ptr<CButton> ScrollDownButton;
    Uint16 last_text_pos_y = 10;
    // we need this to say the window if it needs to render, otherwise no chiffre are shown
    bool rendered = false;
    bool needRender = true;

public:
    CSelectBox(Point16 pos, Extent16 size, int fontsize = 14, int text_color = FONT_YELLOW, int bg_color = -1);
    const Point16& getPos() const { return pos_; }
    const Extent16& getSize() const { return size_; }
    bool hasRendered();
    void setMouseData(SDL_MouseButtonEvent button);
    void setMouseData(SDL_MouseMotionEvent motion);
    bool render();
    SdlSurface& getSurface()
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

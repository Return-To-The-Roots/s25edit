#pragma once

#include "CControlContainer.h"

enum class WindowPos
{
    Center
};

class CWindow final : public CControlContainer
{
    friend class CDebug;

private:
    // if active is false, the window will be render behind the active windows within the game loop
    bool active = true;
    Sint16 x_;
    Sint16 y_;
    Uint16 w_;
    Uint16 h_;
    const char* title;
    bool marked = true;
    bool clicked = false;
    bool canMove;
    bool canClose;
    bool canClose_marked = false;
    bool canClose_clicked = false;
    bool canMinimize;
    bool canMinimize_marked = false;
    bool canMinimize_clicked = false;
    bool canResize;
    bool canResize_marked = false;
    bool canResize_clicked = false;
    bool minimized = false;
    bool moving = false;
    bool resizing = false;
    int priority = 0; // for register, blit, event
    void (*callback_)(int);
    int callbackQuitMessage;

    bool render() final;

public:
    CWindow(void callback(int), int callbackQuitMessage, Position pos, Extent size, const char* title = nullptr,
            int color = WINDOW_GREEN1, Uint8 flags = 0);
    CWindow(void callback(int), int callbackQuitMessage, WindowPos pos, Extent size, const char* title = nullptr,
            int color = WINDOW_GREEN1, Uint8 flags = 0);
    // Access
    Position getPos() const { return {x_, y_}; }
    Extent getSize() const { return {w_, h_}; }
    int getX() const { return x_; };
    int getY() const { return y_; };
    int getW() const { return w_; };
    int getH() const { return h_; };
    Rect getRect() const { return Rect(x_, y_, w_, h_); }
    int getPriority() const { return priority; }
    void setPriority(int priority) { this->priority = priority; }
    void setTitle(const char* title);
    void setMouseData(SDL_MouseMotionEvent motion) final;
    void setMouseData(SDL_MouseButtonEvent button) final;
    void setActive()
    {
        active = true;
        marked = true;
        needRender = true;
    }
    void setInactive();
    bool isActive() const { return active; }
    bool isMoving() const { return moving; }
    bool isResizing() const { return resizing; }
    bool isMarked() const { return marked; }
    void setDirty() { needRender = true; }
    // we can not trust this information, cause if minimized is false, it is possible, that we still have the old
    // minimized surface bool isMinimized() { return minimized; }; we need an information if a input-element (textfield
    // etc.) is active to not deliver the input to other gui-element in the event system
    bool hasActiveInputElement();
    void setColor(int color);
};

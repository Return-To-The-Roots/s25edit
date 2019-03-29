#ifndef _CWINDOW_H
#define _CWINDOW_H

#include "../includes.h"

class CPicture;
class CTextfield;
class CSelectBox;
class CFont;
class CButton;

class CWindow
{
    friend class CDebug;

private:
    // if active is false, the window will be render behind the active windows within the game loop
    bool active;
    // if waste is true, the window will be delete within the game loop
    bool waste;
    SDL_Surface* Surf_Window;
    bool needSurface;
    bool needRender;
    Sint16 x_;
    Sint16 y_;
    Uint16 w_;
    Uint16 h_;
    int pic_background;
    CButton* buttons[MAXBUTTONS];
    CFont* texts[MAXTEXTS];
    CPicture* pictures[MAXPICTURES];
    struct
    {
        int x_, y_, pic;
    } static_pictures[MAXPICTURES];
    CTextfield* textfields[MAXTEXTFIELDS];
    CSelectBox* selectboxes[MAXSELECTBOXES];
    const char* title;
    bool marked;
    bool clicked;
    bool canMove;
    bool canClose;
    bool canClose_marked;
    bool canClose_clicked;
    bool canMinimize;
    bool canMinimize_marked;
    bool canMinimize_clicked;
    bool canResize;
    bool canResize_marked;
    bool canResize_clicked;
    bool minimized;
    bool moving;
    bool resizing;
    int priority; // for register, blit, event
    void (*callback_)(int);
    int callbackQuitMessage;

public:
    // Constructor - Destructor
    CWindow(void callback(int), int callbackQuitMessage, Uint16 x = 0, Uint16 y = 0, Uint16 w = 200, Uint16 h = 200,
            const char* title = nullptr, int color = WINDOW_GREEN1, Uint8 flags = 0);
    ~CWindow();
    // Access
    int getX() const { return x_; };
    int getY() const { return y_; };
    int getW() const { return w_; };
    int getH() const { return h_; };
    Rect getRect() const { return Rect(x_, y_, w_, h_); }
    int getPriority() const { return priority; }
    void setPriority(int priority) { this->priority = priority; }
    void setTitle(const char* title);
    void setMouseData(SDL_MouseMotionEvent motion);
    void setMouseData(SDL_MouseButtonEvent button);
    void setKeyboardData(const SDL_KeyboardEvent& key);
    SDL_Surface* getSurface()
    {
        render();
        return Surf_Window;
    }
    void setActive()
    {
        active = true;
        marked = true;
        needRender = true;
    }
    void setInactive();
    bool isActive() const { return active; }
    void setWaste() { waste = true; }
    bool isWaste() const { return waste; }
    bool isMoving() const { return moving; }
    bool isResizing() const { return resizing; }
    bool isMarked() const { return marked; }
    void setDirty() { needRender = true; }
    // we can not trust this information, cause if minimized is false, it is possible, that we still have the old minimized surface
    // bool isMinimized() { return minimized; };
    // we need an information if a input-element (textfield etc.) is active to not deliver the input to other gui-element in the event
    // system
    bool hasActiveInputElement();
    void setColor(int color);
    // Methods
    CButton* addButton(void callback(int), int clickedParam, Uint16 x = 0, Uint16 y = 0, Uint16 w = 20, Uint16 h = 20,
                       int color = BUTTON_GREY, const char* text = nullptr, int picture = -1);
    bool delButton(CButton* ButtonToDelete);
    CFont* addText(std::string string, int x = 0, int y = 0, int fontsize = 9, int color = FONT_YELLOW);
    bool delText(CFont* TextToDelete);
    CPicture* addPicture(void callback(int), int clickedParam, Uint16 x, Uint16 y, int picture);
    bool delPicture(CPicture* PictureToDelete);
    int addStaticPicture(int x, int y, int picture);
    bool delStaticPicture(int ArrayIndex);
    CTextfield* addTextfield(Uint16 x = 0, Uint16 y = 0, Uint16 cols = 10, Uint16 rows = 1, int fontsize = 14, int text_color = FONT_YELLOW,
                             int bg_color = -1, bool button_style = false);
    bool delTextfield(CTextfield* TextfieldToDelete);
    CSelectBox* addSelectBox(Uint16 x = 0, Uint16 y = 0, Uint16 w = 100, Uint16 h = 100, int fontsize = 14, int text_color = FONT_YELLOW,
                             int bg_color = -1);
    bool delSelectBox(CSelectBox* SelectBoxToDelete);
    bool render();
};

#endif

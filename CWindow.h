#ifndef _CWINDOW_H
    #define _CWINDOW_H

#include "includes.h"

class CWindow
{
    friend class CDebug;

    private:
        //if active is false, the window will be render behind the active windows within the game loop
        bool active;
        //if waste is true, the window will be delete within the game loop
        bool waste;
        SDL_Surface *Surf_Window;
        bool needSurface;
        bool needRender;
        Sint16 x;
        Sint16 y;
        Uint16 w;
        Uint16 h;
        int pic_background;
        CButton *buttons[MAXBUTTONS];
        CFont *texts[MAXTEXTS];
        CPicture *pictures[MAXPICTURES];
        struct { int x, y, pic; } static_pictures[MAXPICTURES];
        unsigned char *title;
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
        int priority; //for register, blit, event
        void (*callback)(int);
        int callbackQuitMessage;

    public:
        //Constructor - Destructor
        CWindow(void callback(int), int callbackQuitMessage, Uint16 x = 0, Uint16 y = 0, Uint16 w = 200, Uint16 h = 200, const char *title = NULL, int color = WINDOW_GREEN1, Uint8 flags = 0);
        ~CWindow();
        //Access
        int getX(void) { return x; };
        int getY(void) { return y; };
        int getW(void) { return w; };
        int getH(void) { return h; };
        int getPriority(void) { return priority; };
        void setPriority(int priority) { this->priority = priority; };
        void setTitle(const char *title);
        void setTitle(unsigned char *title);
        void setMouseData(SDL_MouseMotionEvent motion);
        void setMouseData(SDL_MouseButtonEvent button);
        SDL_Surface* getSurface(void) { render(); return Surf_Window; };
        void setActive(void) { active = true; marked = true; needRender = true; };
        void setInactive(void) { active = false; clicked = false; marked = false; needRender = true; };
        bool isActive(void) { return active; };
        void setWaste(void) { waste = true; };
        bool isWaste(void) { return waste; };
        bool isMoving(void) { return moving; };
        bool isResizing(void) { return resizing; };
        //we can not trust this information, cause if minimized is false, it is possible, that we still have the old minimized surface
        //bool isMinimized(void) { return minimized; };
        void setColor(int color);
        //Methods
        CButton* addButton(void callback(int), int clickedParam, Uint16 x = 0, Uint16 y = 0, Uint16 width = 20, Uint16 height = 20, int color = BUTTON_GREY, const char *text = NULL, int picture = -1);
        bool delButton(CButton *ButtonToDelete);
        CFont* addText(const char *string, int x = 0, int y = 0, int fontsize = 9, int color = FONT_YELLOW);
        CFont* addText(unsigned char *string, int x = 0, int y = 0, int fontsize = 9, int color = FONT_YELLOW);
        bool delText(CFont *TextToDelete);
        CPicture* addPicture(void callback(int), int clickedParam, Uint16 x, Uint16 y, int picture);
        bool delPicture(CPicture *PictureToDelete);
        int addStaticPicture(int x, int y, int picture);
        bool delStaticPicture(int ArrayIndex);
        bool render(void);
};

#endif


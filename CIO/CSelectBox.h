#ifndef _CSELECTBOX_H
    #define _CSELECTBOX_H

#include "../includes.h"

//maximum number of entries for a selectbox
#define MAXSELECTBOXENTRIES 20000

class CSelectBox
{
    friend class CDebug;

    private:
        SDL_Surface *Surf_SelectBox;
        CFont* Entries[MAXSELECTBOXENTRIES];
        bool needSurface;
        bool needRender;
        Uint16 x;
        Uint16 y;
        Uint16 w;
        Uint16 h;
        Uint16 last_text_pos_y;
        int fontsize;
        int pic_background;
        int pic_foreground;
        int text_color;
        //we need this to say the window if it needs to render, otherwise no chiffres are shown
        bool rendered;
        CButton *ScrollUpButton;
        CButton *ScrollDownButton;

    public:
        //Constructor - Destructor
        CSelectBox(Uint16 x = 0, Uint16 y = 0, Uint16 w = 100, Uint16 h = 100, int fontsize = 14, int text_color = FONT_YELLOW, int bg_color = -1);
        ~CSelectBox();
        //Access;
        int getX(void) { return x; }
        int getY(void) { return y; }
        int getW(void) { return w; }
        int getH(void) { return h; }
        void setX(int x) { this->x = x; }
        void setY(int y) { this->y = y; }
        bool hasRendered(void);
        void setMouseData(SDL_MouseButtonEvent button);
        void setMouseData(SDL_MouseMotionEvent motion);
        bool render(void);
        SDL_Surface* getSurface(void) { render(); return Surf_SelectBox; }
        void setColor(int color);
        void setTextColor(int color) { text_color = color; needRender = true;}
        void setOption(const char *string, void (*callback)(int) = NULL, int param = 0);
        void setOption(unsigned char *string, void (*callback)(int) = NULL, int param = 0);
};

#endif

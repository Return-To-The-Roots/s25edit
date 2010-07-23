#ifndef _CMAP_H
    #define _CMAP_H

#include "includes.h"

class CMap
{
    friend class CDebug;

    private:
        SDL_Surface *Surf_Map;
        bobMAP *map;
        SDL_Rect displayRect;
        bool active;
        bool needSurface;
        int VertexX, VertexY;
        int mode;
        //get the number of the triangle nearest to cursor and save it to VertexX and VertexY
        void saveVertex(Uint16 MouseX, Uint16 MouseY, Uint8 MouseState);
        //blitting coords for the mouse cursor
        int MouseBlitX;
        int MouseBlitY;

    public:
        CMap(char *filename);
        ~CMap();
        void setMouseData(SDL_MouseMotionEvent motion);
        void setMouseData(SDL_MouseButtonEvent button);
        void setKeyboardData(SDL_KeyboardEvent key);
        void setActive(void) { active = true; };
        void setInactive(void) { active = false; };
        bool isActive(void) { return active; };
        int getVertexX(void) { return VertexX; };
        int getVertexY(void) { return VertexY; };
        int getMode(void) { return mode; };
        void setMode(int mode) { this->mode = mode; };
        SDL_Surface* getSurface(void) { render(); return Surf_Map; };
        bool render(void);
};

#endif

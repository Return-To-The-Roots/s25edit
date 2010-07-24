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
        bool modify;
        //get the number of the triangle nearest to cursor and save it to VertexX and VertexY
        void saveVertex(Uint16 MouseX, Uint16 MouseY, Uint8 MouseState);
        //blitting coords for the mouse cursor
        int MouseBlitX;
        int MouseBlitY;
        //counts the distance from the cursor vertex to the farest vertex that can be involved in changes (0 - only cursor vertex, 1 - six vertices around the cursor vertex ....) (editor mode)
        int ChangeSection;
        //counts how many vertices around the cursor are involved in changes (changeable by user by pressing + or -) (editor mode)
        int VertexCounter;
        //array to store all these vertices (editor mode)
        struct vertexPoint Vertices[37];

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

    private:
        //returns count of the vertices that are involved in changes (editor mode)
        int getActiveVertices(int ChangeSection);
        void actualizeVertices(void);
        int correctMouseBlitX(int MouseBlitX, int VertexX, int VertexY);
        int correctMouseBlitY(int MouseBlitY, int VertexX, int VertexY);
        void modifyVertex(void);
        void modifyHeight(int VertexX, int VertexY);
};

#endif

#ifndef _CSURFACE_H
    #define _CSURFACE_H

#include "includes.h"

class CSurface
{
    friend class CDebug;
    public:
        CSurface();
        //blits from source on destination to position X,Y
        static bool Draw(SDL_Surface *Surf_Dest, SDL_Surface *Surf_Src, int X, int Y);
        //blits from source on destination to position X,Y and rotates (angle --> degrees --> 90, 180, 270)
        static bool Draw(SDL_Surface *Surf_Dest, SDL_Surface *Surf_Src, int X, int Y, int angle);
        //blits rectangle (X2,Y2,W,H) from source on destination to position X,Y
        static bool Draw(SDL_Surface *Surf_Dest, SDL_Surface *Surf_Src, int X, int Y, int X2, int Y2, int W, int H);
        //blits rectangle (X2,Y2,W,H) from source on destination to position X,Y  and rotates (angle --> degrees --> 90, 180, 270)
        static bool Draw(SDL_Surface *Surf_Dest, SDL_Surface *Surf_Src, int X, int Y, int X2, int Y2, int W, int H, int angle);
        static void DrawPixel_Color(SDL_Surface *screen, int x, int y, Uint32 color);
        static void DrawPixel_RGB(SDL_Surface *screen, int x, int y, Uint8 R, Uint8 G, Uint8 B);
        static void DrawPixel_RGBA(SDL_Surface *screen, int x, int y, Uint8 R, Uint8 G, Uint8 B, Uint8 A);
        static Uint32 GetPixel(SDL_Surface *surface, int x, int y);
        static void DrawTriangleField(SDL_Surface *display, struct DisplayRectangle displayRect, bobMAP *myMap);
        static void DrawTriangle(SDL_Surface *display, struct DisplayRectangle displayRect, bobMAP *myMap, Uint8 type, struct point P1, struct point P2, struct point P3);
        static void get_nodeVectors(bobMAP *myMap);
        static void update_shading(bobMAP *myMap, int VertexX, int VertexY);

    private:
        //to count the rounds (round != gameloop) --> important to makes trees and water moving --> walks from 0 to 7
        static char roundCount;
        static Uint32 roundTime;

        static struct vector get_nodeVector(struct vector v1, struct vector v2, struct vector v3);
        static struct vector get_normVector(struct vector v);
        static struct vector get_flatVector(struct point *P1, struct point *P2, struct point *P3);
        static Sint32 get_LightIntensity(struct vector node);
        static float absf(float a);
        //update flatVectors around a vertex
        static void update_flatVectors(bobMAP *myMap, int VertexX, int VertexY);
        //update nodeVector based on new flatVectors around it
        static void update_nodeVector(bobMAP *myMap, int VertexX, int VertexY);
};

#endif

#ifndef _CSURFACE_H
    #define _CSURFACE_H

#include "includes.h"

class CSurface
{
    friend class CDebug;
    public:
        CSurface();

    public:
        static bool Draw(SDL_Surface *Surf_Dest, SDL_Surface *Surf_Src, int X, int Y);
        static bool Draw(SDL_Surface *Surf_Dest, SDL_Surface *Surf_Src, int X, int Y, int X2, int Y2, int W, int H);
        static void DrawPixel_Color(SDL_Surface *screen, int x, int y, Uint32 color);
        static void DrawPixel_RGB(SDL_Surface *screen, int x, int y, Uint8 R, Uint8 G, Uint8 B);
        static void DrawPixel_RGBA(SDL_Surface *screen, int x, int y, Uint8 R, Uint8 G, Uint8 B, Uint8 A);
        static Uint32 GetPixel(SDL_Surface *surface, int x, int y);
        static void DrawTriangleField(SDL_Surface *display, SDL_Rect displayRect, bobMAP *myMap);
        static void DrawTriangle(SDL_Surface *display, SDL_Rect displayRect, bobMAP *myMap, Uint8 type, struct point P1, struct point P2, struct point P3);
        static void get_normVectors(bobMAP *myMap);

    private:
        static bool gouraud;

        static struct vector get_nodeVector(struct vector v1, struct vector v2, struct vector v3);
        static struct vector get_normVector(struct vector v);
        static struct vector get_flatVector(struct point *P1, struct point *P2, struct point *P3);
        static Sint32 get_LightIntensity(struct vector node);
        static float absf(float a);
};

#endif

#ifndef _CSURFACE_H
#define _CSURFACE_H

#include "SdlSurface.h"
#include "defines.h"
#include <SDL.h>

struct vector;

class CSurface
{
    friend class CDebug;

public:
    // blits from source on destination to position X,Y
    static bool Draw(SDL_Surface* Surf_Dest, SDL_Surface* Surf_Src, int X, int Y);
    static bool Draw(SDL_Surface* Surf_Dest, SdlSurface& Surf_Src, int X, int Y);
    static bool Draw(SdlSurface& Surf_Dest, SdlSurface& Surf_Src, Position pos = {0, 0});
    static bool Draw(SdlSurface& Surf_Dest, SDL_Surface* Surf_Src, int X = 0, int Y = 0);
    // blits from source on destination to position X,Y and rotates (angle --> degrees --> 90, 180, 270)
    static bool Draw(SDL_Surface* Surf_Dest, SDL_Surface* Surf_Src, int X, int Y, int angle);
    static bool Draw(SdlSurface& Surf_Dest, SdlSurface& Surf_Src, int X, int Y, int angle);
    // blits rectangle (X2,Y2,W,H) from source on destination to position X,Y
    static bool Draw(SDL_Surface* Surf_Dest, SDL_Surface* Surf_Src, int X, int Y, int X2, int Y2, int W, int H);
    static bool Draw(SDL_Surface* Surf_Dest, SdlSurface& Surf_Src, int X, int Y, int X2, int Y2, int W, int H);
    static bool Draw(SdlSurface& Surf_Dest, SdlSurface& Surf_Src, int X, int Y, int X2, int Y2, int W, int H)
    {
        return Draw(Surf_Dest.get(), Surf_Src.get(), X, Y, X2, Y2, W, H);
    }
    static void DrawPixel_Color(SDL_Surface* screen, int x, int y, Uint32 color);
    static void DrawPixel_RGB(SDL_Surface* screen, int x, int y, Uint8 R, Uint8 G, Uint8 B);
    static void DrawPixel_RGB(SdlSurface& screen, int x, int y, Uint8 R, Uint8 G, Uint8 B) { DrawPixel_RGB(screen.get(), x, y, R, G, B); }
    static void DrawPixel_RGBA(SDL_Surface* screen, int x, int y, Uint8 R, Uint8 G, Uint8 B, Uint8 A);
    static Uint32 GetPixel(SDL_Surface* surface, int x, int y);
    static void DrawTriangleField(SDL_Surface* display, const DisplayRectangle& displayRect, const bobMAP& myMap);
    static void DrawTriangle(SDL_Surface* display, const DisplayRectangle& displayRect, const bobMAP& myMap, MapType type,
                             const MapNode& P1, const MapNode& P2, const MapNode& P3);

    static void get_nodeVectors(bobMAP& myMap);
    static void update_shading(bobMAP& myMap, int VertexX, int VertexY);

private:
    // to decide what to draw, triangle-textures or objects and texture-borders
    static bool drawTextures;

    static vector get_nodeVector(const vector& v1, const vector& v2, const vector& v3);
    static vector get_normVector(const vector& v);
    static vector get_flatVector(const IntVector& P1, const IntVector& P2, const IntVector& P3);
    static Sint32 get_LightIntensity(const vector& node);
    static float absf(float a);
    // update flatVectors around a vertex
    static void update_flatVectors(bobMAP& myMap, int VertexX, int VertexY);
    // update nodeVector based on new flatVectors around it
    static void update_nodeVector(bobMAP& myMap, int VertexX, int VertexY);
    static void GetTerrainTextureCoords(MapType mapType, TriangleTerrainType texture, bool isRSU, int texture_move, Point16& upper,
                                        Point16& left, Point16& right, Point16& upper2, Point16& left2, Point16& right2);
};

#endif

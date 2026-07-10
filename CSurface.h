// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"

struct vector;

class CSurface
{
public:
    /// Render terrain into the current GL framebuffer.
    /// displayRect specifies the visible area in map-pixel coordinates.
    static void DrawTriangleField(const DisplayRectangle& displayRect, const bobMAP& myMap);

    /// Draw a single triangle given its three vertices.
    static void DrawTriangle(const DisplayRectangle& displayRect, const bobMAP& myMap, MapType type, const MapNode& P1,
                             const MapNode& P2, const MapNode& P3);

    static void get_nodeVectors(bobMAP& myMap);
    static void update_shading(bobMAP& myMap, Position pos);

private:
    // to decide what to draw, triangle-textures or objects and texture-borders
    static bool drawTextures;

    static vector get_nodeVector(const vector& v1, const vector& v2, const vector& v3);
    static vector get_normVector(const vector& v);
    static vector get_flatVector(const IntVector& P1, const IntVector& P2, const IntVector& P3);
    static Sint32 get_LightIntensity(const vector& node);
    static float absf(float a);
    // update flatVectors around a vertex
    static void update_flatVectors(bobMAP& myMap, Position pos);
    // update nodeVector based on new flatVectors around it
    static void update_nodeVector(bobMAP& myMap, Position pos);

    static void GetTerrainTextureCoords(MapType mapType, TriangleTerrainType texture, bool isRSU, Point16& upper,
                                        Point16& left, Point16& right, Point16& upper2, Point16& left2,
                                        Point16& right2);
};

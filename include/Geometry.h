// Copyright (C) 2009 - 2025 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"

/// ---------------------------------------------------------------------------
/// USD triangle coordinate conventions — aligning with s25client.
///
/// RSU triangles are identical in both.  USD differs in odd rows only:
///
/// s25client TerrainRenderer::UpdateTrianglePos[1]:
///   gl_vertices[pos+1][0] = GetVertexPos(pt)          = (x, y)
///   gl_vertices[pos+1][1] = GetNeighbour SE(pt)        = (x + (y&1), y+1)
///   gl_vertices[pos+1][2] = GetNeighbour E(pt)         = (x+1, y)
///   For odd rows: (x, y+1), (x, y), (x+1, y)
///
/// s25edit old
/// (CSurface.cpp master, even row USD loop, parity case):
///   DrawTriangle(..., (x+1, y+1), (x, y), (x+1, y))
///   Vertex 0 = (x+1, y+1) = SE(x+1, y) instead of SE(x, y).
///
/// s25client MapLoader::InitNodes assigns USD texture:
///   node.t2 = MapLayer::Terrain2 at (pt.x, pt.y) — same visual as node.t1.
/// s25edit old: vertex(i,j).usdTexture read from file(i,j) without shift;
///   render-time: visual USD(x,y) reads vertex(x - !(y&1), y).usdTexture.
/// s25edit now: CFile.cpp shifts during I/O so vertex(i,j).usdTexture
///   = USD at visual (i,j), matching node.t2.
///
/// The client* identity helpers mark "reviewed for client convention."
/// editor* helpers document the old convention for reference.
/// ---------------------------------------------------------------------------

inline Position clientUsdVertexPos(Position visualPos)
{
    return visualPos;
}
inline Position clientUsdVertexPos(int x, int y)
{
    return {x, y};
}

/// Old editor: USD(x,y) ← vertex(x - !(y&1), y).usdTexture.
inline Position editorUsdVertexPos(Position visualPos)
{
    return {visualPos.x - !(visualPos.y & 1), visualPos.y};
}
inline Position editorUsdVertexPos(int x, int y)
{
    return {x - !(y & 1), y};
}

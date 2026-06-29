// Copyright (C) 2025 Settlers Freaks <sf-team at siedler25.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"

/// Identity marker for s25client coordinate convention.
///
/// s25client TerrainRenderer::UpdateTriangleTerrain:\n
///   terrain[nodeIdx][0] = RSU, terrain[nodeIdx][1] = USD\n
///   Both read from the same vertex (x, y).\n
///\n
/// After CFile.cpp's I/O shift, vertex(x, y).usdTexture = USD at visual (x, y),\n
/// matching terrain[nodeIdx][1].
inline Position clientUsdVertexPos(Position visualPos)
{
    return visualPos;
}
inline Position clientUsdVertexPos(int x, int y)
{
    return {x, y};
}

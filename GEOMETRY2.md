<!--
Copyright (C) 2025 Settlers Freaks <sf-team at siedler25.org>
SPDX-License-Identifier: GPL-3.0-or-later
-->

# s25edit vs s25client coordinate alignment

Both use the same pixel positions (`bobMAP::updateVertexCoords` /
`TerrainRenderer::GetVertexPos`) and identical RSU triangles.

## 1. USD triangle — odd row vertex 0 differs

**s25edit** USD at `(x, y)`, odd rows (`CSurface.cpp:DrawTriangleField` loop
body):

    (x+1, y+1), (x, y), (x+1, y)

**s25client** USD at `(x, y)`, odd rows
(`TerrainRenderer::UpdateTrianglePos[1]`):

    (x, y+1), (x, y), (x+1, y)

The only difference: vertex 0 is `(x+1, y+1)` (right-lower) in s25edit vs
`(x, y+1)` (left-lower) in s25client.

## 2. Fix: shift USD textures during file I/O

File read (`CIO/CFile.cpp:read_wld`) maps file rows to memory:

    memory(i, j).usdTexture = file((i + !(j&1)) % width, j).usdTexture

File write (`CIO/CFile.cpp:save_wld`) maps back:

    file(i, j).usdTexture = memory((i + !(j&1)) % width, j).usdTexture

After this shift, `vertex(x, y).usdTexture` in memory always contains the USD
texture for visual position `(x, y)` — matching `terrain[pos][1]` in
`s25client::TerrainRenderer::UpdateTriangleTerrain` which reads from the
same vertex `(x, y)`.

## 3. Rendered triangle vertices now match

**s25edit** (`DrawTriangleField` via `clientUsdTriangleVertices`) produces
the same USD vertex set as **s25client** (`UpdateTrianglePos[1]`):

    (x, y+1), (x, y), (x+1, y)   // odd rows  — was (x+1, y+1), (x, y), (x+1, y)
    (x-1, y+1), (x-1, y), (x, y) // even rows — unchanged

## 4. Border texture sources now match

s25client reads four terrain samples per vertex
(`TerrainRenderer::GenerateOpenGL`):

    t1 = terrain[pos][0]  // RSU at (x, y)
    t2 = terrain[pos][1]  // USD at (x, y)
    t3 = terrain[E(pos)][0]  // RSU at (x+1, y)
    t4 = terrain[SW(pos)][1] // USD at SW(x, y)

With the I/O shift, `vertex(x, y).usdTexture` is now `t2`, and
`vertex(x, y).rsuTexture` is `t1` — same indexing as s25client.

Note: s25client still checks 6 border edges (three pairs, both directions)
while s25edit checks 3 (one direction per edge, determined by
`edgePriority`).  The textures being compared are now the same, but the set
of emitted border triangles differs in which direction check fires.

## 5. Migrated call sites

All `.usdTexture` writes pass through `clientUsdVertexPos()` (identity
marker — see `include/Geometry.h`).  Reads need no helper because memory is
already in client convention.

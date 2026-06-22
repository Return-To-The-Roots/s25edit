// Copyright (C) 2025 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Point.h"
#include "Rect.h"
#include "defines.h"
#include <SDL.h>

struct bobMAP;
class EditorGLTexture;

/// Screen/game resolution used by the editor (matches CGame::GameResolution).
using Extent = Point<unsigned>;

/// OpenGL terrain rendering adapted from s25client::TerrainRenderer.
///
/// s25client ref: libs/s25main/TerrainRenderer.cpp
///
/// Design (matches s25client):
///  - `GenerateOpenGL()` pre-computes ALL triangle data into VBOs once.
///  - `Draw(viewport)` renders the visible region from VBOs each frame.
///  - `AltitudeChanged()` incrementally updates triangles after a map edit.
///
/// Differences from s25client:
///  - Single tileset atlas texture vs individual terrain textures — no
///    texture-sort batching needed; one glDrawArrays call covers all terrain.
///  - No fog-of-war / visibility system.
///  - Water animation via texcoord shifting (matches original SGE code), not
///    palette animation.
///  - Map wrapping via glTranslate passes instead of sorted texture batches.
namespace TerrainRenderer {

/// Initialize OpenGL state and pre-allocate renderer structures.
/// s25client: libs/s25main/TerrainRenderer.cpp TerrainRenderer::Init
void Init(const Extent& screenSize);

/// Set the viewport and map-space ortho camera.
/// s25client: libs/s25main/GameWorldViewer.cpp Draw -> setViewport
void setCamera(int camX, int camY, int screenW, int screenH);

/// Reset to screen-space projection (for UI overlay).
void setScreenProjection(int screenW, int screenH);

// ---------------------------------------------------------------------------
// Pre-compute  (call once on map load)
// ---------------------------------------------------------------------------

/// Pre-compute ALL terrain triangles into VBOs.  After this, Draw() can be
/// called repeatedly without regeneration.
/// s25client: libs/s25main/TerrainRenderer.cpp TerrainRenderer::GenerateOpenGL()
void GenerateOpenGL(const bobMAP& map, SDL_Surface* tileset);

/// Incrementally update triangles around vertex (x,y) after a height/texture
/// edit.  Rebuilds VBOs for the affected neighbourhood.
/// s25client: libs/s25main/TerrainRenderer.cpp TerrainRenderer::AltitudeChanged()
void AltitudeChanged(int x, int y, const bobMAP& map);

// ---------------------------------------------------------------------------
// Per-frame drawing
// ---------------------------------------------------------------------------

/// Render terrain for the given viewport.  Uses pre-computed VBOs.
/// Handles map wrapping internally via glTranslate passes.
/// s25client: libs/s25main/TerrainRenderer.cpp TerrainRenderer::Draw()
void Draw(const DisplayRectangle& displayRect);

// ---------------------------------------------------------------------------
// State queries
// ---------------------------------------------------------------------------

/// Check if terrain has been generated.
bool isTerrainValid();

/// Mark terrain as needing regeneration (new map).
void invalidateTerrain();

/// Get tileset texture dimensions (needed by border drawing).
float texWidth();
float texHeight();

/// Get map pixel dimensions (needed by border wrap passes).
int mapWidthPx();
int mapHeightPx();

// ---------------------------------------------------------------------------
// Animation
// ---------------------------------------------------------------------------

/// Advance water/lava animation timers.  Call once per frame.
/// s25client: libs/s25main/TerrainRenderer.cpp  Draw  (water animation)
void updateAnimation();

/// Get current texture animation offset (pixels).
int textureOffset();

/// Get current tree-animation round count (0-7).
int roundCount();

} // namespace TerrainRenderer

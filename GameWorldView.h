// Copyright (C) 2009 - 2025 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"

struct SDL_Surface;
struct bobMAP;

namespace EditorWorldView {

/// Draw editor overlay sprites (trees, resources, buildings, animals) to target.
///
/// This is the editor equivalent of s25client::GameWorldView::Draw: terrain and
/// coastline borders are rendered separately by TerrainRenderer, and this pass
/// blits the per-vertex editor overlays to the UI overlay surface.
void Draw(SDL_Surface* target, const DisplayRectangle& viewRect, const bobMAP& map);

} // namespace EditorWorldView

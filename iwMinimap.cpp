// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iwMinimap.h"
#include "Loader.h"

iwMinimap::iwMinimap()
    : IngameWindow(107, IngameWindow::posLastOrCenter, Extent(160, 160), "Minimap",
                   LOADER.GetImageN("resource", 41))
{
    // Minimap rendering will draw the terrain overview into this window
    // TODO: Render minimap content using the EditorWorld/GameWorldEditor
}

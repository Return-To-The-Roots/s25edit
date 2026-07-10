// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iwEditorCursor.h"
#include "Loader.h"
#include "controls/ctrlButton.h"
#include "defines.h"

iwEditorCursor::iwEditorCursor()
    : IngameWindow(109, IngameWindow::posLastOrCenter, Extent(210, 130), "Cursor",
                   LOADER.GetImageN("resource", 41))
{
    const int x0 = contentOffset.x;
    const int y0 = contentOffset.y;

    AddTextButton(1, DrawPoint(x0 + 2, y0 + 2),  Extent(96, 32), TextureColor::Grey, "Hexagon", NormalFont);
    AddTextButton(2, DrawPoint(x0 + 2, y0 + 34), Extent(196, 32), TextureColor::Grey, "Cursor-Activity: static", NormalFont);

    auto* arrowUp = LOADER.GetImageN("editbob", CURSOR_SYMBOL_ARROW_UP);
    if(arrowUp)
        AddImageButton(3, DrawPoint(x0 + 2, y0 + 66), Extent(32, 32),
                       TextureColor::Invisible, arrowUp, "")->SetBorder(false);
}

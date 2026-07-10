// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iwEditorMenu.h"
#include "CGame.h"
#include "WindowManager.h"
#include "controls/ctrlButton.h"
#include "iwLoadMap.h"
#include "iwSaveMap.h"
#include "defines.h"
#include "dskMainMenu.h"
#include "globals.h"
#include "Loader.h"

iwEditorMenu::iwEditorMenu(EditorWorld& world)
    : IngameWindow(100, IngameWindow::posLastOrCenter, Extent(220, 320), "Main menu",
                   LOADER.GetImageN("resource", 41)),
      world_(world)
{
    auto off = DrawPoint(contentOffset.x, contentOffset.y);
    int cx = (GetSize().x - contentOffset.x - contentOffsetEnd.x) / 2 + contentOffset.x;
    AddTextButton(ID_btLoad, DrawPoint(cx - 95, off.y + 100), Extent(190, 22), TextureColor::Green2, "Load map", NormalFont);
    AddTextButton(ID_btSave, DrawPoint(cx - 95, off.y + 125), Extent(190, 22), TextureColor::Green2, "Save map", NormalFont);
    AddTextButton(ID_btQuit, DrawPoint(cx - 95, off.y + 260), Extent(190, 22), TextureColor::Green2, "Leave editor", NormalFont);
}

void iwEditorMenu::Msg_ButtonClick(unsigned ctrl_id)
{
    switch(ctrl_id)
    {
        case ID_btLoad:
            Close();
            WINDOWMANAGER.Show(std::make_unique<iwLoadMap>());
            break;
        case ID_btSave:
            Close();
            WINDOWMANAGER.Show(std::make_unique<iwSaveMap>(world_));
            break;
        case ID_btQuit:
            Close();
            WINDOWMANAGER.Switch(std::make_unique<dskMainMenu>());
            break;
    }
}

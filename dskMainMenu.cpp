// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dskMainMenu.h"
#include "CGame.h"
#include "EditorWorld.h"
#include "Loader.h"
#include "WindowManager.h"
#include "controls/ctrlButton.h"
#include "defines.h"
#include "dskEditorInterface.h"
#include "dskOptions.h"
#include "globals.h"
#include "ogl/glArchivItem_Bitmap.h"
#include "iwLoadMap.h"

dskMainMenu::dskMainMenu()
    : Desktop(nullptr)
{
    AddTextButton(ID_btStartEditor, DrawPoint(50, 160), Extent(200, 22), TextureColor::Red1, "Start editor",
                  NormalFont);
    AddTextButton(ID_btLoadMap, DrawPoint(50, 200), Extent(200, 22), TextureColor::Green2, "Load map", NormalFont);
    AddTextButton(ID_btOptions, DrawPoint(50, 370), Extent(200, 22), TextureColor::Green2, "Options", NormalFont);
    AddTextButton(ID_btQuit, DrawPoint(50, 400), Extent(200, 22), TextureColor::Red1, "Quit program", NormalFont);
}

void dskMainMenu::Draw_()
{
    if(auto* bg = LOADER.GetImageN("setup010", 0))
        bg->DrawFull(GetDrawRect());
    Desktop::Draw_();
}

void dskMainMenu::Msg_ButtonClick(unsigned ctrl_id)
{
    switch(ctrl_id)
    {
        case ID_btStartEditor:
        {
            // Create a blank EditorWorld (32x32 greenland)
            auto world = std::make_unique<EditorWorld>(MapExtent(32, 32), 1);
            WINDOWMANAGER.Switch(std::make_unique<dskEditorInterface>(std::move(world)));
            break;
        }
        case ID_btLoadMap:
            WINDOWMANAGER.Show(std::make_unique<iwLoadMap>());
            break;
        case ID_btOptions:
            WINDOWMANAGER.Switch(std::make_unique<dskOptions>());
            break;
        case ID_btQuit:
            global::s2->Running = false;
            break;
    }
}

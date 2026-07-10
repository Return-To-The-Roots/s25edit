// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iwLoadMap.h"
#include "EditorWorld.h"
#include "WindowManager.h"
#include "controls/ctrlButton.h"
#include "controls/ctrlList.h"
#include "defines.h"
#include "globals.h"
#include "dskEditorInterface.h"
#include "helpers/format.hpp"
#include "s25util/strAlgos.h"
#include "Loader.h"
#include <boost/filesystem.hpp>
#include <iostream>

namespace bfs = boost::filesystem;

iwLoadMap::iwLoadMap()
    : IngameWindow(101, IngameWindow::posLastOrCenter, Extent(280, 320), "Load map",
                   LOADER.GetImageN("resource", 41))
{
    auto off = DrawPoint(contentOffset.x, contentOffset.y) + DrawPoint(10, 5);
    auto* list = AddList(ID_lstFiles, off, Extent(150, 260), TextureColor::Grey, NormalFont);

    // Populate with .SWD and .WLD files
    if(bfs::exists(global::userMapsPath))
    {
        for(auto& entry : bfs::directory_iterator(global::userMapsPath))
        {
            if(!bfs::is_regular_file(entry.status()))
                continue;
            auto ext = s25util::toLower(entry.path().extension().string());
            if(ext != ".swd" && ext != ".wld")
                continue;
            list->AddItem(entry.path().filename().string());
        }
    }

    int btnX = off.x + 150 + 8;
    AddTextButton(ID_btLoad, DrawPoint(btnX, off.y + 130), Extent(85, 22), TextureColor::Green2, "Load", NormalFont);
    AddTextButton(ID_btAbort, DrawPoint(btnX, off.y + 155), Extent(85, 22), TextureColor::Red1, "Abort", NormalFont);
}

void iwLoadMap::Msg_ListSelectItem(unsigned /*ctrl_id*/, int selection)
{
    auto* list = GetCtrl<ctrlList>(ID_lstFiles);
    if(list && selection >= 0)
        selectedFile_ = list->GetItemText(selection);
}

void iwLoadMap::Msg_ButtonClick(unsigned ctrl_id)
{
    switch(ctrl_id)
    {
        case ID_btLoad:
        {
            if(selectedFile_.empty())
            {
                Close();
                break;
            }
            bfs::path path = global::userMapsPath / selectedFile_;
            if(!bfs::exists(path))
            {
                // Try both extensions
                auto tryPath = global::userMapsPath / (selectedFile_ + ".swd");
                if(!bfs::exists(tryPath))
                    tryPath = global::userMapsPath / (selectedFile_ + ".wld");
                path = tryPath;
            }
            if(bfs::exists(path))
            {
                auto world = EditorWorld::loadFromSwd(path);
                if(world)
                {
                    WINDOWMANAGER.Switch(std::make_unique<dskEditorInterface>(std::move(world)));
                } else
                {
                    std::cerr << "Failed to load map from " << path << "\n";
                }
            }
            Close();
            break;
        }
        case ID_btAbort:
            Close();
            break;
    }
}

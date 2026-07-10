// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dskOptions.h"
#include "CGame.h"
#include "Loader.h"
#include "WindowManager.h"
#include "controls/ctrlButton.h"
#include "controls/ctrlList.h"
#include "controls/ctrlText.h"
#include "defines.h"
#include "dskMainMenu.h"
#include "globals.h"
#include "helpers/format.hpp"
#include "ogl/FontStyle.h"
#include "ogl/glArchivItem_Bitmap.h"

static constexpr std::pair<int, int> resolutions[] = {
  {800, 600},   {832, 624},  {960, 540},  {964, 544},   {960, 640},   {960, 720},   {1024, 576}, {1024, 600},
  {1072, 600},  {1152, 768}, {1024, 768}, {1152, 864},  {1152, 870},  {1152, 900},  {1200, 800}, {1200, 900},
  {1280, 720},  {1280, 768}, {1280, 800}, {1280, 854},  {1360, 768},  {1366, 768},  {1376, 768}, {1400, 900},
  {1440, 900},  {1440, 960}, {1280, 960}, {1280, 1024}, {1360, 1024}, {1366, 1024}, {1600, 768}, {1600, 900},
  {1600, 1024}, {1400, 1050},{1680, 1050},{1600, 1200}, {1920, 1080}, {1920, 1200}, {1920, 1400},{1920, 1440},
  {2048, 1152}, {2048, 1536},{3840, 2160},
};

dskOptions::dskOptions()
    : Desktop(nullptr)
{
    AddTextButton(ID_btBack, DrawPoint(global::s2->GameResolution.x / 2 - 100, 440), Extent(200, 22),
                  TextureColor::Red1, "back", NormalFont);
    AddText(ID_txtResolution, DrawPoint(global::s2->GameResolution.x / 2, 10), "Options", COLOR_YELLOW,
            FontStyle::CENTER, NormalFont);
    AddTextButton(ID_btFullscreen,
                  DrawPoint(global::s2->GameResolution.x / 2 - 100, 410), Extent(200, 22), TextureColor::Red1,
                  global::s2->fullscreen ? "WINDOW" : "FULLSCREEN", NormalFont);

    auto* list = AddList(ID_lstResolution, DrawPoint(global::s2->GameResolution.x / 2 - 100, 80),
                         Extent(200, 310), TextureColor::Grey, NormalFont);
    int selectedIdx = -1;
    for(unsigned i = 0; i < sizeof(resolutions)/sizeof(resolutions[0]); ++i)
    {
        list->AddItem(helpers::format("%d x %d", resolutions[i].first, resolutions[i].second));
        if(static_cast<unsigned>(resolutions[i].first) == global::s2->GameResolution.x
           && static_cast<unsigned>(resolutions[i].second) == global::s2->GameResolution.y)
            selectedIdx = static_cast<int>(i);
    }
    if(selectedIdx >= 0)
        list->SetSelection(static_cast<unsigned>(selectedIdx));
}

void dskOptions::Draw_()
{
    if(auto* bg = LOADER.GetImageN("setup013", 0))
        bg->DrawFull(GetDrawRect());
    Desktop::Draw_();
}

void dskOptions::Msg_ButtonClick(unsigned ctrl_id)
{
    switch(ctrl_id)
    {
        case ID_btBack:
            WINDOWMANAGER.Switch(std::make_unique<dskMainMenu>());
            break;
        case ID_btFullscreen:
            global::s2->fullscreen = !global::s2->fullscreen;
            global::s2->ApplyWindowChanges();
            global::s2->SaveSettings();
            WINDOWMANAGER.Switch(std::make_unique<dskOptions>());
            break;
    }
}

void dskOptions::Msg_ListSelectItem(unsigned ctrl_id, int selection)
{
    if(ctrl_id != ID_lstResolution || selection < 0)
        return;
    const auto sel = static_cast<unsigned>(selection);
    if(sel < sizeof(resolutions)/sizeof(resolutions[0]))
    {
        // Skip if same as current — avoids infinite loop from SetSelection in ctor
        if(static_cast<unsigned>(resolutions[sel].first) == global::s2->GameResolution.x
           && static_cast<unsigned>(resolutions[sel].second) == global::s2->GameResolution.y)
            return;
        global::s2->GameResolution.x = resolutions[sel].first;
        global::s2->GameResolution.y = resolutions[sel].second;
        global::s2->ApplyWindowChanges();
        global::s2->SaveSettings();
        WINDOWMANAGER.Switch(std::make_unique<dskOptions>());
    }
}

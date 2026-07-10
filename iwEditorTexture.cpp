// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iwEditorTexture.h"
#include "Loader.h"
#include "controls/ctrlButton.h"
#include "defines.h"

iwEditorTexture::iwEditorTexture(CbFunc onSelect)
    : IngameWindow(101, IngameWindow::posLastOrCenter, Extent(220, 133), "Terrain",
                   LOADER.GetImageN("resource", 41)), onSelect_(std::move(onSelect))
{
    const int x0 = contentOffset.x;
    const int y0 = contentOffset.y;
    const int step = 34;
    const int picSize = 32;

    // Map button ID (1..17) to TRIANGLE_TEXTURE_* value (the legacy s2Id)
    static constexpr struct { unsigned id; int texIdx; int s2Id; } entries[] = {
        {1,  PICTURE_GREENLAND_TEXTURE_SNOW,           TRIANGLE_TEXTURE_SNOW},
        {2,  PICTURE_GREENLAND_TEXTURE_STEPPE,          TRIANGLE_TEXTURE_STEPPE},
        {3,  PICTURE_GREENLAND_TEXTURE_SWAMP,           TRIANGLE_TEXTURE_SWAMP},
        {4,  PICTURE_GREENLAND_TEXTURE_FLOWER,          TRIANGLE_TEXTURE_FLOWER},
        {5,  PICTURE_GREENLAND_TEXTURE_MINING1,         TRIANGLE_TEXTURE_MINING1},
        {6,  PICTURE_GREENLAND_TEXTURE_MINING2,         TRIANGLE_TEXTURE_MINING2},
        {7,  PICTURE_GREENLAND_TEXTURE_MINING3,         TRIANGLE_TEXTURE_MINING3},
        {8,  PICTURE_GREENLAND_TEXTURE_MINING4,         TRIANGLE_TEXTURE_MINING4},
        {9,  PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW1,  TRIANGLE_TEXTURE_STEPPE_MEADOW1},
        {10, PICTURE_GREENLAND_TEXTURE_MEADOW1,         TRIANGLE_TEXTURE_MEADOW1},
        {11, PICTURE_GREENLAND_TEXTURE_MEADOW2,         TRIANGLE_TEXTURE_MEADOW2},
        {12, PICTURE_GREENLAND_TEXTURE_MEADOW3,         TRIANGLE_TEXTURE_MEADOW3},
        {13, PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW2,  TRIANGLE_TEXTURE_STEPPE_MEADOW2},
        {14, PICTURE_GREENLAND_TEXTURE_MINING_MEADOW,   TRIANGLE_TEXTURE_MINING_MEADOW},
        {15, PICTURE_GREENLAND_TEXTURE_WATER,           TRIANGLE_TEXTURE_WATER},
        {16, PICTURE_GREENLAND_TEXTURE_LAVA,            TRIANGLE_TEXTURE_LAVA},
        {17, PICTURE_GREENLAND_TEXTURE_MEADOW_MIXED,    TRIANGLE_TEXTURE_MEADOW_MIXED},
    };

    for(const auto& e : entries)
    {
        auto* img = LOADER.GetImageN("editio", e.texIdx);
        if(!img) continue;
        int col = (e.id - 1) % 6;
        int row = (e.id - 1) / 6;
        AddImageButton(e.id, DrawPoint(x0 + col * step, y0 + row * step), Extent(picSize, picSize),
                       TextureColor::Invisible, img, "")->SetBorder(false);
    }
}

void iwEditorTexture::Msg_ButtonClick(unsigned ctrl_id)
{
    static constexpr int s2IdForId[] = {
        0, // unused index 0
        TRIANGLE_TEXTURE_SNOW,
        TRIANGLE_TEXTURE_STEPPE,
        TRIANGLE_TEXTURE_SWAMP,
        TRIANGLE_TEXTURE_FLOWER,
        TRIANGLE_TEXTURE_MINING1,
        TRIANGLE_TEXTURE_MINING2,
        TRIANGLE_TEXTURE_MINING3,
        TRIANGLE_TEXTURE_MINING4,
        TRIANGLE_TEXTURE_STEPPE_MEADOW1,
        TRIANGLE_TEXTURE_MEADOW1,
        TRIANGLE_TEXTURE_MEADOW2,
        TRIANGLE_TEXTURE_MEADOW3,
        TRIANGLE_TEXTURE_STEPPE_MEADOW2,
        TRIANGLE_TEXTURE_MINING_MEADOW,
        TRIANGLE_TEXTURE_WATER,
        TRIANGLE_TEXTURE_LAVA,
        TRIANGLE_TEXTURE_MEADOW_MIXED,
    };
    if(ctrl_id < sizeof(s2IdForId) / sizeof(s2IdForId[0]) && onSelect_)
        onSelect_(s2IdForId[ctrl_id]);
    Close();
}

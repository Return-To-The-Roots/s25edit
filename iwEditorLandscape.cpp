// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iwEditorLandscape.h"
#include "Loader.h"
#include "controls/ctrlButton.h"
#include "defines.h"

/// Map from button index (0-based) to MAPPIC clean index for the corresponding landscape object.
static constexpr int buttonToMapPic[] = {
    MAPPIC_GRANITE_1_6,  // 0: granite (largest size)
    MAPPIC_TREE_DEAD,    // 1: dead tree
    MAPPIC_STONE1,       // 2: stone
    MAPPIC_CACTUS1,      // 3: cactus
    MAPPIC_PEBBLE1,      // 4: pebble
    MAPPIC_BUSH1,        // 5: bush
    MAPPIC_SHRUB1,       // 6: shrub
    MAPPIC_BONE1,        // 7: bone
    MAPPIC_MUSHROOM1,    // 8: mushroom
};

iwEditorLandscape::iwEditorLandscape(CbFunc onSelect)
    : IngameWindow(104, IngameWindow::posLastOrCenter, Extent(112, 174), "Landscape",
                   LOADER.GetImageN("resource", 41)), onSelect_(std::move(onSelect))
{
    const int x0 = contentOffset.x;
    const int y0 = contentOffset.y;
    const int step = 34;
    const int picSize = 32;

    // Greenland landscape objects
    static constexpr int landscapeIcons[] = {
        PICTURE_LANDSCAPE_GRANITE,
        PICTURE_LANDSCAPE_TREE_DEAD,
        PICTURE_LANDSCAPE_STONE,
        PICTURE_LANDSCAPE_CACTUS,
        PICTURE_LANDSCAPE_PEBBLE,
        PICTURE_LANDSCAPE_BUSH,
        PICTURE_LANDSCAPE_SHRUB,
        PICTURE_LANDSCAPE_BONE,
        PICTURE_LANDSCAPE_MUSHROOM,
    };

    for(unsigned i = 0; i < sizeof(landscapeIcons) / sizeof(landscapeIcons[0]); i++)
    {
        auto* img = LOADER.GetImageN("editio", landscapeIcons[i]);
        if(!img) continue;
        int col = i % 3;
        int row = i / 3;
        AddImageButton(i + 1, DrawPoint(x0 + col * step, y0 + row * step), Extent(picSize, picSize),
                       TextureColor::Invisible, img, "")->SetBorder(false);
    }
}

void iwEditorLandscape::Msg_ButtonClick(unsigned ctrl_id)
{
    if(ctrl_id >= 1 && ctrl_id <= sizeof(buttonToMapPic) / sizeof(buttonToMapPic[0]) && onSelect_)
    {
        onSelect_(buttonToMapPic[ctrl_id - 1]);
    }
    Close();
}

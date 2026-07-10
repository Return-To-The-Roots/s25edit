// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iwEditorAnimal.h"
#include "Loader.h"
#include "controls/ctrlButton.h"
#include "defines.h"

/// Map from button index (0-based) to Species enum value (as int).
static constexpr int buttonToSpecies[] = {
    1, // 0: rabbit     → Species::RabbitWhite
    3, // 1: fox        → Species::Fox
    4, // 2: stag       → Species::Stag
    5, // 3: roe (deer) → Species::Deer
    6, // 4: duck       → Species::Duck
    7, // 5: sheep      → Species::Sheep
};

iwEditorAnimal::iwEditorAnimal(CbFunc onSelect)
    : IngameWindow(105, IngameWindow::posLastOrCenter, Extent(116, 106), "Animals",
                   LOADER.GetImageN("resource", 41)), onSelect_(std::move(onSelect))
{
    const int x0 = contentOffset.x;
    const int y0 = contentOffset.y;
    const int step = 34;
    const int picSize = 32;

    static constexpr int animalIcons[] = {
        PICTURE_ANIMAL_RABBIT,
        PICTURE_ANIMAL_FOX,
        PICTURE_ANIMAL_STAG,
        PICTURE_ANIMAL_ROE,
        PICTURE_ANIMAL_DUCK,
        PICTURE_ANIMAL_SHEEP,
    };

    for(unsigned i = 0; i < sizeof(animalIcons) / sizeof(animalIcons[0]); i++)
    {
        auto* img = LOADER.GetImageN("editio", animalIcons[i]);
        if(!img) continue;
        int col = i % 3;
        int row = i / 3;
        AddImageButton(i + 1, DrawPoint(x0 + col * step, y0 + row * step), Extent(picSize, picSize),
                       TextureColor::Invisible, img, "")->SetBorder(false);
    }
}

void iwEditorAnimal::Msg_ButtonClick(unsigned ctrl_id)
{
    if(ctrl_id >= 1 && ctrl_id <= sizeof(buttonToSpecies) / sizeof(buttonToSpecies[0]) && onSelect_)
        onSelect_(buttonToSpecies[ctrl_id - 1]);
    Close();
}

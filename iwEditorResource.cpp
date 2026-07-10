// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iwEditorResource.h"
#include "Loader.h"
#include "controls/ctrlButton.h"
#include "defines.h"

iwEditorResource::iwEditorResource()
    : IngameWindow(103, IngameWindow::posLastOrCenter, Extent(148, 55), "Resources",
                   LOADER.GetImageN("resource", 41))
{
    const int x0 = contentOffset.x;
    const int y0 = contentOffset.y;
    const int step = 34;
    const int picSize = 32;

    static constexpr int resourceIds[] = {
        PICTURE_RESOURCE_GOLD,
        PICTURE_RESOURCE_ORE,
        PICTURE_RESOURCE_COAL,
        PICTURE_RESOURCE_GRANITE,
    };

    for(unsigned i = 0; i < sizeof(resourceIds) / sizeof(resourceIds[0]); i++)
    {
        auto* img = LOADER.GetImageN("editio", resourceIds[i]);
        if(!img) continue;
        AddImageButton(i + 1, DrawPoint(x0 + i * step, y0), Extent(picSize, picSize),
                       TextureColor::Invisible, img, "")->SetBorder(false);
    }
}

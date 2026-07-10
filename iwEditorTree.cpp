// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iwEditorTree.h"
#include "Loader.h"
#include "controls/ctrlButton.h"
#include "defines.h"

iwEditorTree::iwEditorTree(CbFunc onSelect)
    : IngameWindow(102, IngameWindow::posLastOrCenter, Extent(148, 140), "Trees",
                   LOADER.GetImageN("resource", 41)), onSelect_(std::move(onSelect))
{
    const int x0 = contentOffset.x;
    const int y0 = contentOffset.y;
    const int step = 34;
    const int picSize = 32;

    // Map button ID to noTree type (0-8) or sentinel for mixed
    static constexpr struct { unsigned id; int texIdx; int treeType; } entries[] = {
        {1,  PICTURE_TREE_PINE,       0},
        {2,  PICTURE_TREE_BIRCH,      1},
        {3,  PICTURE_TREE_OAK,        2},
        {4,  PICTURE_TREE_PALM1,      3},
        {5,  PICTURE_TREE_PALM2,      4},
        {6,  PICTURE_TREE_PINEAPPLE,  5},
        {7,  PICTURE_TREE_CYPRESS,    6},
        {8,  PICTURE_TREE_CHERRY,     7},
        {9,  PICTURE_TREE_FIR,        8},
        {10, PICTURE_TREE_WOOD_MIXED, -1}, // mixed wood: random pine/birch/oak
        {11, PICTURE_TREE_PALM_MIXED, -2}, // mixed palm: random palm1/palm2/pineapple/cypress/cherry
    };

    for(const auto& e : entries)
    {
        auto* img = LOADER.GetImageN("editio", e.texIdx);
        if(!img) continue;
        int col = (e.id - 1) % 4;
        int row = (e.id - 1) / 4;
        AddImageButton(e.id, DrawPoint(x0 + col * step, y0 + row * step), Extent(picSize, picSize),
                       TextureColor::Invisible, img, "")->SetBorder(false);
    }
}

void iwEditorTree::Msg_ButtonClick(unsigned ctrl_id)
{
    // treeTypeForId[ctrl_id] = noTree type (0-8), -1 for mixed wood, -2 for mixed palm, 0 for invalid
    static constexpr int treeTypeForId[] = {
        0, // unused index 0
        0, 1, 2, 3, 4, 5, 6, 7, 8, -1, -2
    };
    if(ctrl_id < sizeof(treeTypeForId) / sizeof(treeTypeForId[0]) && onSelect_)
        onSelect_(treeTypeForId[ctrl_id]);
    Close();
}

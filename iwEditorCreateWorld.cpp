// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iwEditorCreateWorld.h"
#include "Loader.h"
#include "controls/ctrlButton.h"

iwEditorCreateWorld::iwEditorCreateWorld()
    : IngameWindow(108, IngameWindow::posLastOrCenter, Extent(250, 350), "Create world",
                   LOADER.GetImageN("resource", 41))
{
    const int x0 = contentOffset.x;
    const int y0 = contentOffset.y;
    const int btnW = 35;
    const int btnH = 20;
    const int row1Y = 15;
    const int row2Y = 49;
    const int spacing = 2;

    // Width controls
    AddTextButton(1, DrawPoint(x0, y0 + row1Y), Extent(btnW, btnH), TextureColor::Grey, "128<-", NormalFont);
    AddTextButton(2, DrawPoint(x0 + btnW + spacing, y0 + row1Y), Extent(btnW, btnH), TextureColor::Grey, "16<-", NormalFont);
    AddTextButton(3, DrawPoint(x0 + 2 * (btnW + spacing), y0 + row1Y), Extent(btnW - 10, btnH), TextureColor::Grey, "2<-", NormalFont);
    AddTextButton(4, DrawPoint(x0 + 3 * (btnW + spacing) + 5, y0 + row1Y), Extent(btnW - 10, btnH), TextureColor::Grey, "->2", NormalFont);
    AddTextButton(5, DrawPoint(x0 + 4 * (btnW + spacing), y0 + row1Y), Extent(btnW, btnH), TextureColor::Grey, "->16", NormalFont);
    AddTextButton(6, DrawPoint(x0 + 5 * (btnW + spacing), y0 + row1Y), Extent(btnW, btnH), TextureColor::Grey, "->128", NormalFont);

    // Height controls
    AddTextButton(7,  DrawPoint(x0, y0 + row2Y), Extent(btnW, btnH), TextureColor::Grey, "128<-", NormalFont);
    AddTextButton(8,  DrawPoint(x0 + btnW + spacing, y0 + row2Y), Extent(btnW, btnH), TextureColor::Grey, "16<-", NormalFont);
    AddTextButton(9,  DrawPoint(x0 + 2 * (btnW + spacing), y0 + row2Y), Extent(btnW - 10, btnH), TextureColor::Grey, "2<-", NormalFont);
    AddTextButton(10, DrawPoint(x0 + 3 * (btnW + spacing) + 5, y0 + row2Y), Extent(btnW - 10, btnH), TextureColor::Grey, "->2", NormalFont);
    AddTextButton(11, DrawPoint(x0 + 4 * (btnW + spacing), y0 + row2Y), Extent(btnW, btnH), TextureColor::Grey, "->16", NormalFont);
    AddTextButton(12, DrawPoint(x0 + 5 * (btnW + spacing), y0 + row2Y), Extent(btnW, btnH), TextureColor::Grey, "->128", NormalFont);

    // Landscape type
    AddTextButton(13, DrawPoint(x0 + 50, y0 + 80), Extent(110, 20), TextureColor::Grey, "Greenland", NormalFont);

    // Main texture prev/next
    AddTextButton(14, DrawPoint(x0 + 30, y0 + 120), Extent(35, 20), TextureColor::Grey, "-", NormalFont);
    AddTextButton(15, DrawPoint(x0 + 140, y0 + 120), Extent(35, 20), TextureColor::Grey, "+", NormalFont);

    // Border size
    AddTextButton(16, DrawPoint(x0 + 30, y0 + 170), Extent(35, 20), TextureColor::Grey, "-", NormalFont);
    AddTextButton(17, DrawPoint(x0 + 140, y0 + 170), Extent(35, 20), TextureColor::Grey, "+", NormalFont);

    // Border texture prev/next
    AddTextButton(18, DrawPoint(x0 + 30, y0 + 210), Extent(35, 20), TextureColor::Grey, "-", NormalFont);
    AddTextButton(19, DrawPoint(x0 + 140, y0 + 210), Extent(35, 20), TextureColor::Grey, "+", NormalFont);

    // Create world button
    AddTextButton(20, DrawPoint(x0 + 30, y0 + 260), Extent(150, 40), TextureColor::Green2, "Create world", NormalFont);
}

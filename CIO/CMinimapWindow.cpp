// Copyright (C) 2026 - 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CMinimapWindow.h"
#include "../CGame.h"
#include "../CMap.h"
#include "../Texture.h"
#include "../globals.h"
#include "CFont.h"
#include <libsiedler2/ArchivItem_Bitmap.h>

void CMinimapWindow::draw(Position /*parentOrigin*/)
{
    // Draw window chrome (frame, title, close button, background, child elements)
    CWindow::draw(pos_);

    // Compute content area (inside the frames)
    const auto& b = getBorderSizes();
    const Position contentPos = pos_ + Position(b.left, b.top);
    const auto contentSize = getSize() - getBorderSize();
    if(static_cast<int>(contentSize.x) <= 0 || static_cast<int>(contentSize.y) <= 0)
        return;

    auto* map = global::s2->getMapObj();
    if(!map)
        return;

    // Fill pixel buffer with minimap terrain
    int scale = 1;
    map->drawMinimap(pixels_, static_cast<int>(contentSize.x), static_cast<int>(contentSize.y), scale);

    // Upload to texture and draw
    if(!minimapTex_.isValid() || minimapTex_.getSize() != contentSize)
        minimapTex_.createEmpty(contentSize);
    minimapTex_.upload(pixels_.data());
    minimapTex_.draw(Rect(contentPos, contentSize));

    // Draw player flags and numbers on top
    for(int i = 0; i < MAXPLAYERS; i++)
    {
        const auto hqX = map->getPlayerHQx()[i];
        const auto hqY = map->getPlayerHQy()[i];
        if(hqX == 0xFFFF || hqY == 0xFFFF)
            continue;

        const int flagIdx = FLAG_BLUE_DARK + i % 7;
        const Position hqPos = Position(hqX, hqY) / scale;
        Texture::getTexture(ArchiveID::EDITBOB, flagIdx).drawSprite(contentPos + hqPos);

        // Player number
        CFont::draw(std::to_string(i + 1), contentPos + hqPos, FontSize::Small, FontColor::MintGreen);
    }

    // Draw the position arrow
    {
        const int arrowIdx = MAPPIC_ARROWCROSS_ORANGE;
        const auto& dispRect = map->getDisplayRect();
        const Position arrowCenter = dispRect.getOrigin() + dispRect.getSize() / 2u;
        auto& tex = Texture::getTexture(ArchiveID::MAP00, arrowIdx);
        const Position arrowPos =
          contentPos + arrowCenter / Position(triangleWidth, triangleHeight) / scale - tex.anchor();
        tex.draw(arrowPos);
    }
}

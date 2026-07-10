// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CMenu.h"
#include "../CGame.h"
#include "../Texture.h"
#include "../globals.h"

CMenu::CMenu(int pic_background, ArchiveID archive) : CControlContainer(pic_background, BorderSizes{}, archive) {}

void CMenu::draw(Position /*parentOrigin*/)
{
    // Draw full-screen background texture
    const auto res = global::s2->getRes();
    getTexture(backgroundArchive_, getBackground()).draw(Rect(0, 0, res.x, res.y));

    drawChildren(Position(0, 0));
}

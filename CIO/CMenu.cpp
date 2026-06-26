// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CMenu.h"
#include "../CGame.h"
#include "../globals.h"

CMenu::CMenu(int pic_background) : CControlContainer(pic_background) {}

bool CMenu::render()
{
    if(getBackground() < 0)
        return false;

    // if we don't need to render, all is up to date, return true
    if(!needRender)
        return true;
    needRender = false;
    // if we need a new surface
    if(!surface)
    {
        surface = makeRGBSurface(global::s2->getRes().x, global::s2->getRes().y, true);
        if(!surface)
            return false;
    }

    // Background is drawn via OpenGL in CGame::Render(); only draw UI elements here.
    renderElements();
    return true;
}

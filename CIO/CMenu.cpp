// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CMenu.h"
#include "../CGame.h"
#include "../Texture.h"
#include "../globals.h"

CMenu::CMenu(int pic_background) : CControlContainer(pic_background) {}

bool CMenu::render()
{
    if(getBackground() < 0)
        return false;

    if(!bgTexture_)
    {
        const int picIdx = getBackground();
        if(picIdx >= 0 && picIdx < static_cast<int>(global::bmpArray.size()) && global::bmpArray[picIdx].surface)
        {
            bgTexture_ = std::make_unique<Texture>();
            bgTexture_->load(global::bmpArray[picIdx].surface.get(), true);
        }
    }

    if(bgTexture_)
        bgTexture_->Draw(Rect(0, 0, global::s2->getRes().x, global::s2->getRes().y));

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

    renderElements();
    return true;
}

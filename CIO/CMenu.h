// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "CControlContainer.h"

class CMenu final : public CControlContainer
{
    // if active is false, the menu will not be render within the game loop
    bool active = true;

    bool render() final;

public:
    CMenu(int pic_background);
    void setActive() { active = true; };
    void setInactive() { active = false; };
    bool isActive() const { return active; };
};

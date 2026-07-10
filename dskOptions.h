// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "desktops/Desktop.h"

class dskOptions : public Desktop
{
public:
    dskOptions();

    void Msg_ButtonClick(unsigned ctrl_id) override;
    void Msg_ListSelectItem(unsigned ctrl_id, int selection) override;

protected:
    void Draw_() override;

private:
    enum ControlIds
    {
        ID_btBack,
        ID_btFullscreen,
        ID_lstResolution,
        ID_txtResolution
    };
};

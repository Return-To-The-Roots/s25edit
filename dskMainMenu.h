// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "desktops/Desktop.h"

class dskMainMenu : public Desktop
{
public:
    dskMainMenu();

    void Msg_ButtonClick(unsigned ctrl_id) override;

protected:
    void Draw_() override;

private:
    enum ControlIds
    {
        ID_btStartEditor,
        ID_btLoadMap,
        ID_btOptions,
        ID_btQuit
    };
};

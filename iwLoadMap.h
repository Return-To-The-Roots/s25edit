// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ingameWindows/IngameWindow.h"

class iwLoadMap : public IngameWindow
{
public:
    iwLoadMap();

    void Msg_ButtonClick(unsigned ctrl_id) override;
    void Msg_ListSelectItem(unsigned ctrl_id, int selection) override;

private:
    enum
    {
        ID_lstFiles,
        ID_btLoad,
        ID_btAbort
    };
    std::string selectedFile_;
};

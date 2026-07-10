// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ingameWindows/IngameWindow.h"
#include <functional>

class iwEditorPlayer : public IngameWindow
{
public:
    using CbFunc = std::function<void(int)>;
    iwEditorPlayer(int currentPlayer, CbFunc onPlayerChanged);

    void Msg_ButtonClick(unsigned ctrl_id) override;

private:
    enum
    {
        ID_btPrev,
        ID_txtPlayer,
        ID_btNext,
        ID_btGoTo
    };
    void updateDisplay();

    int currentPlayer_;
    CbFunc onPlayerChanged_;
};

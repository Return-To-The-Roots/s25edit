// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ingameWindows/IngameWindow.h"
#include <functional>

class iwEditorTree : public IngameWindow
{
public:
    /// Callback receives the noTree type (0-8) or -1 for mixed wood, -2 for mixed palm
    using CbFunc = std::function<void(int)>;
    iwEditorTree(CbFunc onSelect);

    void Msg_ButtonClick(unsigned ctrl_id) override;

private:
    CbFunc onSelect_;
};

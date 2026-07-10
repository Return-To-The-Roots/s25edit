// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ingameWindows/IngameWindow.h"
#include <functional>

class iwEditorTexture : public IngameWindow
{
public:
    /// Callback receives the selected terrain s2Id (TRIANGLE_TEXTURE_* value & ~0x40)
    using CbFunc = std::function<void(int)>;
    iwEditorTexture(CbFunc onSelect);

    void Msg_ButtonClick(unsigned ctrl_id) override;

private:
    CbFunc onSelect_;
};

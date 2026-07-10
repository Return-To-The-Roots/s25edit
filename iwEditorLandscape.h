// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ingameWindows/IngameWindow.h"
#include <functional>

/// Window for selecting landscape objects (rocks, cactuses, bushes, etc.).
/// onSelect receives the MAPPIC clean index of the chosen object.
class iwEditorLandscape : public IngameWindow
{
public:
    using CbFunc = std::function<void(int mapPicId)>;
    iwEditorLandscape(CbFunc onSelect);

    void Msg_ButtonClick(unsigned ctrl_id) override;

private:
    CbFunc onSelect_;
};

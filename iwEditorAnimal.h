// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ingameWindows/IngameWindow.h"
#include "gameTypes/AnimalTypes.h"
#include <functional>

/// Window for selecting animals to place on the map.
/// onSelect receives the Species enum value cast to int.
class iwEditorAnimal : public IngameWindow
{
public:
    using CbFunc = std::function<void(int species)>;
    iwEditorAnimal(CbFunc onSelect);

    void Msg_ButtonClick(unsigned ctrl_id) override;

private:
    CbFunc onSelect_;
};

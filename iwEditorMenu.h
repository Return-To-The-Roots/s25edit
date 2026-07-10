// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ingameWindows/IngameWindow.h"

class EditorWorld;

class iwEditorMenu : public IngameWindow
{
public:
    explicit iwEditorMenu(EditorWorld& world);

    void Msg_ButtonClick(unsigned ctrl_id) override;

private:
    enum
    {
        ID_btLoad,
        ID_btSave,
        ID_btQuit
    };
    EditorWorld& world_;
};

// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ingameWindows/IngameWindow.h"

class EditorWorld;

class iwSaveMap : public IngameWindow
{
public:
    explicit iwSaveMap(EditorWorld& world);

    void Msg_ButtonClick(unsigned ctrl_id) override;

private:
    enum
    {
        ID_lblFilename,
        ID_lblMapname,
        ID_lblAuthor,
        ID_edtFilename,
        ID_edtMapname,
        ID_edtAuthor,
        ID_btSave,
        ID_btAbort
    };

    EditorWorld& world_;
};

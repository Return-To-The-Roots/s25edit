// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iwEditorPlayer.h"
#include "Loader.h"
#include "controls/ctrlButton.h"
#include "controls/ctrlText.h"
#include "defines.h"
#include "s25util/colors.h"
#include <string>

iwEditorPlayer::iwEditorPlayer(int currentPlayer, CbFunc onPlayerChanged)
    : IngameWindow(106, IngameWindow::posLastOrCenter, Extent(100, 80), "Players",
                   LOADER.GetImageN("resource", 41)),
      currentPlayer_(currentPlayer), onPlayerChanged_(std::move(onPlayerChanged))
{
    const int x0 = contentOffset.x;
    const int y0 = contentOffset.y;

    AddTextButton(ID_btPrev, DrawPoint(x0, y0), Extent(20, 20), TextureColor::Grey, "-", NormalFont);
    AddText(ID_txtPlayer, DrawPoint(x0 + 26, y0 + 4), std::to_string(currentPlayer_ + 1),
            COLOR_ORANGE, FontStyle::CENTER, LargeFont);
    AddTextButton(ID_btNext, DrawPoint(x0 + 60, y0), Extent(20, 20), TextureColor::Grey, "+", NormalFont);
    AddTextButton(ID_btGoTo, DrawPoint(x0, y0 + 20), Extent(60, 20), TextureColor::Grey, "Go to", NormalFont);
}

void iwEditorPlayer::updateDisplay()
{
    auto* txt = GetCtrl<ctrlText>(ID_txtPlayer);
    if(txt)
        txt->SetText(std::to_string(currentPlayer_ + 1));
}

void iwEditorPlayer::Msg_ButtonClick(unsigned ctrl_id)
{
    switch(ctrl_id)
    {
        case ID_btPrev:
            if(currentPlayer_ > 0)
            {
                currentPlayer_--;
                updateDisplay();
                if(onPlayerChanged_)
                    onPlayerChanged_(currentPlayer_);
            }
            break;
        case ID_btNext:
            if(currentPlayer_ < 6)
            {
                currentPlayer_++;
                updateDisplay();
                if(onPlayerChanged_)
                    onPlayerChanged_(currentPlayer_);
            }
            break;
        case ID_btGoTo:
            break;
    }
}

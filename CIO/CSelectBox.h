// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"
#include <functional>
#include <memory>
#include <vector>

class CFont;
class CButton;

class CSelectBox
{
    friend class CDebug;

private:
    /// Width and height of the scroll buttons, also the width of the scrollbar
    static constexpr int scrollButtonSize = 20;
    /// Distance of the entries from the left and top border
    static constexpr int textPadding = 10;

    std::vector<std::unique_ptr<CFont>> Entries;
    Position pos_;
    Extent size_;
    FontSize fontsize;
    int pic_background;
    int pic_foreground;
    FontColor text_color;
    std::unique_ptr<CButton> ScrollUpButton;
    std::unique_ptr<CButton> ScrollDownButton;
    bool scrollUpButtonPressed_ = false;
    bool scrollDownButtonPressed_ = false;
    int scrollPos_ = 0; ///< Index of the first visible entry
    // Scrollbar slider between the scroll buttons
    bool sliderDragging_ = false;
    int sliderPos_ = 0; ///< Position of the slider in the track while dragging
    int dragStartSliderPos_ = 0;
    int dragStartScrollPos_ = 0;
    int dragStartMouseY_ = 0;

    /// Number of fully visible entries
    int getPageSize() const;
    bool isEntryVisible(int index) const;
    bool hasScrollbar() const;
    /// Area between the scroll buttons, relative to the select box
    Rect getScrollTrack() const;
    int getSliderHeight() const;
    int getSliderPosFromScroll() const;
    int clampSliderPos(int sliderPos) const;
    /// Pass the event (relative to the select box) to the entries, only the visible ones are clickable
    void setEntriesMouseData(const SDL_MouseButtonEvent& button);

public:
    CSelectBox(Position pos, Extent size, FontSize fontsize = FontSize::Large, FontColor text_color = FontColor::Yellow,
               int bg_color = -1);
    Position getPos() const { return pos_; }
    Extent getSize() const { return size_; }
    void setMouseData(SDL_MouseButtonEvent button);
    void setMouseData(SDL_MouseMotionEvent motion);
    void setMouseWheel(const SDL_MouseWheelEvent& wheel, Position mousePos);
    void scroll(int lines);
    void draw(Position parentOrigin);
    void setColor(int color);
    void setTextColor(FontColor color) { text_color = color; }
    void addOption(const std::string& string, std::function<void(int)> callback = nullptr, int param = 0);
    void setSize(Extent size);
    void setPos(Position pos);
};

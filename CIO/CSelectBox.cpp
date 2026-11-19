// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CSelectBox.h"
#include "../CGame.h"
#include "../Texture.h"
#include "../globals.h"
#include "CButton.h"
#include "CFont.h"
#include "CollisionDetection.h"
#include <glad/glad.h>
#include <algorithm>
#include <cmath>

CSelectBox::CSelectBox(Position pos, Extent size, FontSize fontsize, FontColor text_color, int bg_color)
    : pos_(pos), size_(size), fontsize(fontsize), text_color(text_color)
{
    setColor(bg_color);

    // button position is relative to the selectbox
    const int scrollButtonX = static_cast<int>(size_.x) - 1 - scrollButtonSize;
    const Extent scrollButtonExtent(scrollButtonSize, scrollButtonSize);
    ScrollUpButton = std::make_unique<CButton>(nullptr, 0, Position(scrollButtonX, 0), scrollButtonExtent, BUTTON_GREY,
                                               nullptr, PICTURE_SMALL_ARROW_UP);
    ScrollDownButton =
      std::make_unique<CButton>(nullptr, 0, Position(scrollButtonX, static_cast<int>(size_.y) - 1 - scrollButtonSize),
                                scrollButtonExtent, BUTTON_GREY, nullptr, PICTURE_SMALL_ARROW_DOWN);
}

void CSelectBox::addOption(const std::string& string, std::function<void(int)> callback, int param)
{
    // The entries are moved when scrolling, so place the new one relative to the current scroll position
    const int index = static_cast<int>(Entries.size());
    const int y = textPadding + (index - scrollPos_) * static_cast<int>(getLineHeight(fontsize));

    auto Entry = std::make_unique<CFont>(string, Position(textPadding, y), fontsize, FontColor::Yellow);
    Entry->setCallback(std::move(callback), param);
    Entries.emplace_back(std::move(Entry));
}

void CSelectBox::setColor(int color)
{
    switch(color)
    {
        case BUTTON_GREY:
            pic_foreground = BUTTON_GREY_DARK;
            pic_background = BUTTON_GREY_BACKGROUND;
            break;

        case BUTTON_RED1:
            pic_foreground = BUTTON_RED1_DARK;
            pic_background = BUTTON_RED1_BACKGROUND;
            break;

        case BUTTON_GREEN1:
            pic_foreground = BUTTON_GREEN1_DARK;
            pic_background = BUTTON_GREEN1_BACKGROUND;
            break;

        case BUTTON_GREEN2:
            pic_foreground = BUTTON_GREEN2_DARK;
            pic_background = BUTTON_GREEN2_BACKGROUND;
            break;

        case BUTTON_RED2:
            pic_foreground = BUTTON_RED2_DARK;
            pic_background = BUTTON_RED2_BACKGROUND;
            break;

        case BUTTON_STONE:
            pic_foreground = BUTTON_STONE_DARK;
            pic_background = BUTTON_STONE_BACKGROUND;
            break;

        default:
            pic_foreground = -1;
            pic_background = -1;
            break;
    }
}

int CSelectBox::getPageSize() const
{
    return std::max(1, (static_cast<int>(size_.y) - textPadding) / static_cast<int>(getLineHeight(fontsize)));
}

bool CSelectBox::isEntryVisible(int index) const
{
    return index >= scrollPos_ && index < scrollPos_ + getPageSize();
}

void CSelectBox::scroll(int lines)
{
    const auto rowHeight = static_cast<int>(getLineHeight(fontsize));
    const int maxScrollPos = std::max(0, static_cast<int>(Entries.size()) - getPageSize());
    const int newScrollPos = std::clamp(scrollPos_ + lines, 0, maxScrollPos);
    const int offset = (scrollPos_ - newScrollPos) * rowHeight;
    scrollPos_ = newScrollPos;
    for(auto& entry : Entries)
        entry->setPos(Position(entry->getPos().x, entry->getPos().y + offset));
}

bool CSelectBox::hasScrollbar() const
{
    return static_cast<int>(Entries.size()) > getPageSize();
}

Rect CSelectBox::getScrollTrack() const
{
    const int height = std::max(0, static_cast<int>(size_.y) - 1 - 2 * scrollButtonSize);
    return Rect(Position(static_cast<int>(size_.x) - 1 - scrollButtonSize, scrollButtonSize),
                Extent(scrollButtonSize, height));
}

int CSelectBox::getSliderHeight() const
{
    const int range = static_cast<int>(Entries.size());
    return std::max(1, static_cast<int>(getScrollTrack().getSize().y) * getPageSize() / range);
}

int CSelectBox::getSliderPosFromScroll() const
{
    const int range = static_cast<int>(Entries.size());
    const int trackHeight = static_cast<int>(getScrollTrack().getSize().y);
    if(scrollPos_ + getPageSize() >= range)
        return trackHeight - getSliderHeight();
    return trackHeight * scrollPos_ / range;
}

int CSelectBox::clampSliderPos(int sliderPos) const
{
    const int maxSliderPos = static_cast<int>(getScrollTrack().getSize().y) - getSliderHeight();
    return std::clamp(sliderPos, 0, std::max(0, maxSliderPos));
}

void CSelectBox::setMouseWheel(const SDL_MouseWheelEvent& wheel, Position mousePos)
{
    if(!isPointInRect(mousePos, Rect(pos_, size_)))
        return;
    const int lines = (wheel.direction == SDL_MOUSEWHEEL_FLIPPED) ? wheel.y : -wheel.y;
    scroll(lines);
}

void CSelectBox::setMouseData(SDL_MouseMotionEvent motion)
{
    // Make the coordinates relative to the select box, like the positions of the scroll buttons and entries
    motion.x -= pos_.x;
    motion.y -= pos_.y;
    // The release of the button might have been missed, e.g. when it happened outside of the parent window
    if((motion.state & SDL_BUTTON_LMASK) == 0)
        sliderDragging_ = false;
    if(sliderDragging_)
    {
        const int dy = motion.y - dragStartMouseY_;
        sliderPos_ = clampSliderPos(dragStartSliderPos_ + dy);
        // Derive the scroll position from the distance moved since grabbing the slider. Converting the slider position
        // back and forth would lose a line due to rounding as soon as the slider is grabbed.
        const int trackHeight = std::max(1, static_cast<int>(getScrollTrack().getSize().y));
        const auto lines =
          static_cast<int>(std::lround(static_cast<double>(dy) * static_cast<double>(Entries.size()) / trackHeight));
        scroll(dragStartScrollPos_ + lines - scrollPos_);
    }
    if(!hasScrollbar())
        return;
    ScrollUpButton->setMouseData(motion);
    ScrollDownButton->setMouseData(motion);
}

void CSelectBox::setEntriesMouseData(const SDL_MouseButtonEvent& button)
{
    // The whole row is clickable: From the left border to the scrollbar or right border
    const int rowWidth = hasScrollbar() ? getScrollTrack().left : static_cast<int>(size_.x);
    const Extent rowSize(static_cast<unsigned>(std::max(0, rowWidth)), getLineHeight(fontsize));
    for(int i = 0; i < static_cast<int>(Entries.size()); i++)
    {
        // Hidden entries get an empty area: They can't be clicked but still lose their highlight on a click elsewhere
        const Extent clickSize = isEntryVisible(i) ? rowSize : Extent(0, 0);
        Entries[i]->setMouseData(button, Rect(Position(0, Entries[i]->getPos().y), clickSize));
    }
}

void CSelectBox::setMouseData(SDL_MouseButtonEvent button)
{
    if(button.button != SDL_BUTTON_LEFT)
        return;

    // Make the coordinates relative to the select box, like the positions of the scroll buttons and entries
    button.x -= pos_.x;
    button.y -= pos_.y;
    const Position mousePos(button.x, button.y);
    const bool scrollbar = hasScrollbar();
    const auto isOnButton = [mousePos](const CButton& btn) {
        return isPointInRect(mousePos, Rect(btn.getPos(), btn.getSize()));
    };

    if(button.state == SDL_PRESSED)
    {
        if(scrollbar && isPointInRect(mousePos, getScrollTrack()))
        {
            const int sliderPos = getSliderPosFromScroll();
            const int sliderHeight = getSliderHeight();
            const int y = mousePos.y - getScrollTrack().top;
            if(y >= sliderPos && y < sliderPos + sliderHeight)
            {
                sliderDragging_ = true;
                sliderPos_ = dragStartSliderPos_ = sliderPos;
                dragStartScrollPos_ = scrollPos_;
                dragStartMouseY_ = mousePos.y;
            } else
            {
                // Clicked above/below the slider -> Move half a slider height, i.e. half a page, up/down
                const int halfPage = std::max(1, getPageSize() / 2);
                scroll(y < sliderPos ? -halfPage : halfPage);
            }
            return;
        }
        scrollUpButtonPressed_ = scrollbar && isOnButton(*ScrollUpButton);
        scrollDownButtonPressed_ = scrollbar && isOnButton(*ScrollDownButton);
    } else
    {
        sliderDragging_ = false;
        if(scrollUpButtonPressed_ && isOnButton(*ScrollUpButton))
            scroll(-1);
        else if(scrollDownButtonPressed_ && isOnButton(*ScrollDownButton))
            scroll(1);
        scrollUpButtonPressed_ = scrollDownButtonPressed_ = false;
    }

    if(isPointInRect(mousePos, Rect(Position(0, 0), size_)))
        setEntriesMouseData(button);
    if(scrollbar)
    {
        ScrollUpButton->setMouseData(button);
        ScrollDownButton->setMouseData(button);
    }
}

void CSelectBox::setSize(Extent size)
{
    if(size_ != size)
    {
        size_ = size;
        const int scrollButtonX = static_cast<int>(size_.x) - 1 - scrollButtonSize;
        ScrollUpButton->setX(scrollButtonX);
        ScrollDownButton->setX(scrollButtonX);
        ScrollDownButton->setY(static_cast<int>(size_.y) - 1 - scrollButtonSize);
        // scroll() keeps the scroll position in the valid range, so scrolling by 0 lines only corrects it for the new
        // number of visible entries. E.g. a taller box can't stay scrolled as far down, else rows would stay empty.
        scroll(0);
    }
}

void CSelectBox::setPos(Position pos)
{
    pos_ = pos;
}

void CSelectBox::draw(Position parentOrigin)
{
    const Position absPos = parentOrigin + pos_;
    const Rect area(absPos, size_);

    // Draw background
    if(pic_background >= 0 && pic_foreground >= 0)
    {
        getBmpTexture(pic_foreground).drawTiled(area);
    } else
    {
        // Fill with black
        drawRect(area, 0xFF000000);
    }

    // Clip entries to the select box area
    const auto viewH = global::s2->getRes().y;
    glEnable(GL_SCISSOR_TEST);
    glScissor(area.left, viewH - (area.top + static_cast<int>(size_.y)), static_cast<int>(size_.x),
              static_cast<int>(size_.y));

    // Draw entries
    for(int i = 0; i < static_cast<int>(Entries.size()); i++)
    {
        if(isEntryVisible(i))
            Entries[i]->draw(absPos);
    }

    glDisable(GL_SCISSOR_TEST);

    if(hasScrollbar())
    {
        const Rect track = getScrollTrack();
        drawButtonBox(Rect(absPos + track.getOrigin(), track.getSize()), true, BUTTON_GREY_BACKGROUND,
                      BUTTON_GREY_DARK);
        const int sliderPos = sliderDragging_ ? sliderPos_ : getSliderPosFromScroll();
        drawButtonBox(Rect(absPos + track.getOrigin() + Position(0, sliderPos),
                           Extent(track.getSize().x, static_cast<unsigned>(getSliderHeight()))),
                      false, BUTTON_GREY_BACKGROUND, BUTTON_GREY_DARK);
        ScrollUpButton->draw(absPos);
        ScrollDownButton->draw(absPos);
    }
}

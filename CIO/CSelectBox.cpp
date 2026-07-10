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
#include <glad/glad.h>

CSelectBox::CSelectBox(Position pos, Extent size, FontSize fontsize, FontColor text_color, int bg_color)
    : pos_(pos), size_(size), fontsize(fontsize), text_color(text_color)
{
    setColor(bg_color);

    // button position is relative to the selectbox
    ScrollUpButton = std::make_unique<CButton>(nullptr, 0, Position(static_cast<int>(size_.x) - 1 - 20, 0),
                                               Extent(20, 20), BUTTON_GREY, nullptr, PICTURE_SMALL_ARROW_UP);
    ScrollDownButton = std::make_unique<CButton>(
      nullptr, 0, Position(static_cast<int>(size_.x) - 1 - 20, static_cast<int>(size_.y) - 1 - 20), Extent(20, 20),
      BUTTON_GREY, nullptr, PICTURE_SMALL_ARROW_DOWN);
}

void CSelectBox::addOption(const std::string& string, std::function<void(int)> callback, int param)
{
    // explanation: row_height = row_separator + fontsize
    const unsigned row_height = getLineHeight(fontsize);

    auto Entry = std::make_unique<CFont>(string, Position(10, last_text_pos_y), fontsize, FontColor::Yellow);
    Entry->setCallback(std::move(callback), param);
    Entries.emplace_back(std::move(Entry));
    last_text_pos_y += row_height;
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

void CSelectBox::setMouseData(SDL_MouseMotionEvent motion)
{
    // IMPORTANT: we use the left upper corner of the selectbox as (x,y)=(0,0), so we have to manipulate
    //           the motion-structure before give it to the buttons: x_absolute - x_selectbox, y_absolute - y_selectbox
    motion.x -= pos_.x;
    motion.y -= pos_.y;
    ScrollUpButton->setMouseData(motion);
    ScrollDownButton->setMouseData(motion);
}

void CSelectBox::setMouseData(SDL_MouseButtonEvent button)
{
    bool manipulated = false;
    static bool scroll_up_button_marked = false;
    static bool scroll_down_button_marked = false;

    // left button is pressed
    if(button.button == SDL_BUTTON_LEFT)
    {
        // if mouse button is pressed ON the selectbox
        if(button.state == SDL_PRESSED)
        {
            if((button.x >= pos_.x) && (button.x < pos_.x + static_cast<int>(size_.x)) && (button.y >= pos_.y)
               && (button.y < pos_.y + static_cast<int>(size_.y)))
            {
                // scroll up button
                if((button.x > pos_.x + static_cast<int>(size_.x) - 20) && (button.y < pos_.y + 20))
                {
                    scroll_up_button_marked = true;
                }
                // scroll down button
                else if((button.x > pos_.x + static_cast<int>(size_.x) - 20)
                        && (button.y > pos_.y + static_cast<int>(size_.y) - 20))
                {
                    scroll_down_button_marked = true;
                }

                // IMPORTANT: we use the left upper corner of the selectbox as (x,y)=(0,0), so we have to manipulate
                //           the motion-structure before give it to buttons and entries: x_absolute - x_selectbox,
                //           y_absolute - y_selectbox
                button.x -= pos_.x;
                button.y -= pos_.y;
                manipulated = true;

                for(auto& entry : Entries)
                {
                    entry->setMouseData(button);
                }
            }
        } else if(button.state == SDL_RELEASED)
        {
            if((button.x >= pos_.x) && (button.x < pos_.x + static_cast<int>(size_.x)) && (button.y >= pos_.y)
               && (button.y < pos_.y + static_cast<int>(size_.y)))
            {
                // scroll up button
                if(scroll_up_button_marked)
                {
                    if((button.x > pos_.x + static_cast<int>(size_.x) - 20) && (button.y < pos_.y + 20))
                    {
                        // test if first entry is on the most upper position
                        if(!Entries.empty() && Entries.front()->getPos().y < 10)
                        {
                            for(auto& entry : Entries)
                            {
                                entry->setPos(Position(entry->getPos().x, entry->getPos().y + 10));
                            }
                        }
                    }
                }
                // scroll down button
                else if(scroll_down_button_marked)
                {
                    if((button.x > pos_.x + static_cast<int>(size_.x) - 20)
                       && (button.y > pos_.y + static_cast<int>(size_.y) - 20))
                    {
                        // test if last entry is on the most lower position
                        if(!Entries.empty() && Entries.back()->getPos().y > static_cast<int>(size_.y) - 10)
                        {
                            for(auto& entry : Entries)
                            {
                                entry->setPos(Position(entry->getPos().x, entry->getPos().y - 10));
                            }
                        }
                    }
                }

                // IMPORTANT: we use the left upper corner of the selectbox as (x,y)=(0,0), so we have to manipulate
                //           the motion-structure before give it to buttons and entries: x_absolute - x_selectbox,
                //           y_absolute - y_selectbox
                button.x -= pos_.x;
                button.y -= pos_.y;
                manipulated = true;

                for(auto& entry : Entries)
                {
                    entry->setMouseData(button);
                }
            }
            scroll_up_button_marked = false;
            scroll_down_button_marked = false;
        }
        // IMPORTANT: we use the left upper corner of the selectbox as (x,y)=(0,0), so we have to manipulate
        //           the motion-structure before give it to buttons and entries: x_absolute - x_selectbox, y_absolute -
        //           y_selectbox
        if(!manipulated)
        {
            button.x -= pos_.x;
            button.y -= pos_.y;
        }
        ScrollUpButton->setMouseData(button);
        ScrollDownButton->setMouseData(button);
    }
}

void CSelectBox::setSize(Extent size)
{
    if(size_ != size)
    {
        size_ = size;
        // update scroll down button position
        ScrollDownButton->setY(size_.y - 1 - 20);
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
        getTexture(ArchiveID::EDITIO, pic_foreground).drawTiled(area);
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
    for(const auto& entry : Entries)
    {
        entry->draw(absPos);
    }

    glDisable(GL_SCISSOR_TEST);

    // Draw scroll buttons (on top, within the select box)
    ScrollUpButton->draw(absPos);
    ScrollDownButton->draw(absPos);
}

// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CControlContainer.h"
#include "../Texture.h"
#include "../globals.h"
#include "CButton.h"
#include "CFont.h"
#include "CPicture.h"
#include "CSelectBox.h"
#include "CTextfield.h"
#include "helpers/containerUtils.h"

BorderSizes::BorderSizes(ArchiveID archive, int leftIdx, int topIdx, int rightIdx, int bottomIdx)
    : left(static_cast<int>(global::getBitmapSize(archive, leftIdx).x)),
      top(static_cast<int>(global::getBitmapSize(archive, topIdx).y)),
      right(static_cast<int>(global::getBitmapSize(archive, rightIdx).x)),
      bottom(static_cast<int>(global::getBitmapSize(archive, bottomIdx).y))
{}

CControlContainer::CControlContainer(int pic_background, ArchiveID archive)
    : CControlContainer(pic_background, BorderSizes{}, archive)
{}
CControlContainer::CControlContainer(int pic_background, BorderSizes border, ArchiveID archive)
    : backgroundArchive_(archive), border(border), pic_background(pic_background)
{}

CControlContainer::~CControlContainer() noexcept = default;

void CControlContainer::setBackgroundPicture(int pic_background, ArchiveID archive)
{
    this->pic_background = pic_background;
    backgroundArchive_ = archive;
}

void CControlContainer::setMouseData(const SDL_MouseMotionEvent motion)
{
    for(const auto& picture : pictures)
    {
        picture->setMouseData(motion);
    }
    for(const auto& button : buttons)
    {
        button->setMouseData(motion);
    }
    for(const auto& selectbox : selectboxes)
    {
        selectbox->setMouseData(motion);
    }
}

void CControlContainer::setMouseData(const SDL_MouseButtonEvent button)
{
    for(const auto& picture : pictures)
    {
        picture->setMouseData(button);
    }
    for(const auto& i : buttons)
    {
        i->setMouseData(button);
    }
    for(const auto& textfield : textfields)
    {
        textfield->setMouseData(button);
    }
    for(const auto& selectbox : selectboxes)
    {
        selectbox->setMouseData(button);
    }
}

void CControlContainer::setKeyboardData(const SDL_KeyboardEvent& key)
{
    for(const auto& textfield : textfields)
    {
        textfield->setKeyboardData(key);
    }
}

template<class T, class U>
bool CControlContainer::eraseElement(T& collection, const U* element)
{
    const auto it = helpers::find_if(collection, [element](const auto& cur) { return cur.get() == element; });
    if(it != collection.end())
    {
        collection.erase(it);
        return true;
    }
    return false;
}

CButton* CControlContainer::addButton(void callback(int), int clickedParam, Position pos, Extent size, int color,
                                      const char* text, int picture)
{
    pos = pos + Position(border.left, border.top);

    buttons.emplace_back(std::make_unique<CButton>(callback, clickedParam, pos, size, color, text, picture));
    return buttons.back().get();
}

bool CControlContainer::delButton(CButton* ButtonToDelete)
{
    return eraseElement(buttons, ButtonToDelete);
}

CFont* CControlContainer::addText(std::string string, Position pos, FontSize fontsize, FontColor color)
{
    pos = pos + Position(border.left, border.top);

    texts.emplace_back(std::make_unique<CFont>(std::move(string), pos, fontsize, color));
    return texts.back().get();
}

bool CControlContainer::delText(CFont* TextToDelete)
{
    return eraseElement(texts, TextToDelete);
}

CPicture* CControlContainer::addPicture(void callback(int), int clickedParam, Position pos, ArchiveID archive,
                                        int localIndex)
{
    pos = pos + Position(border.left, border.top);

    pictures.emplace_back(std::make_unique<CPicture>(callback, clickedParam, pos, archive, localIndex));
    return pictures.back().get();
}

bool CControlContainer::delPicture(CPicture* PictureToDelete)
{
    return eraseElement(pictures, PictureToDelete);
}

int CControlContainer::addStaticPicture(Position pos, ArchiveID archive, int localIndex)
{
    if(localIndex < 0)
        return -1;
    pos = pos + Position(border.left, border.top);

    unsigned id = static_pictures.empty() ? 0u : static_pictures.back().id + 1u;
    static_pictures.emplace_back(Picture{pos, archive, localIndex, id});
    return id;
}

bool CControlContainer::delStaticPicture(int picId)
{
    if(picId < 0)
        return false;
    const auto it =
      helpers::find_if(static_pictures, [picId](const auto& pic) { return static_cast<unsigned>(picId) == pic.id; });
    if(it != static_pictures.end())
    {
        static_pictures.erase(it);
        return true;
    }
    return false;
}

CTextfield* CControlContainer::addTextfield(Position pos, Uint16 cols, Uint16 rows, FontSize fontsize,
                                            FontColor text_color, int bg_color, bool button_style)
{
    pos = pos + Position(border.left, border.top);

    textfields.emplace_back(
      std::make_unique<CTextfield>(pos, cols, rows, fontsize, text_color, bg_color, button_style));
    return textfields.back().get();
}

bool CControlContainer::delTextfield(CTextfield* TextfieldToDelete)
{
    return eraseElement(textfields, TextfieldToDelete);
}

CSelectBox* CControlContainer::addSelectBox(Position pos, Extent size, FontSize fontsize, FontColor text_color,
                                            int bg_color)
{
    pos += Position(border.left, border.top);

    selectboxes.emplace_back(std::make_unique<CSelectBox>(pos, size, fontsize, text_color, bg_color));
    return selectboxes.back().get();
}

bool CControlContainer::delSelectBox(CSelectBox* SelectBoxToDelete)
{
    return eraseElement(selectboxes, SelectBoxToDelete);
}

void CControlContainer::drawChildren(Position origin)
{
    for(const auto& picture : pictures)
        picture->draw(origin);
    for(const auto& text : texts)
        text->draw(origin);
    for(const auto& textfield : textfields)
        textfield->draw(origin);
    for(const auto& selectbox : selectboxes)
        selectbox->draw(origin);
    for(const auto& button : buttons)
        button->draw(origin);
    for(const auto& static_picture : static_pictures)
    {
        getTexture(static_picture.archive, static_picture.pic).draw(origin + static_picture.pos);
    }
}

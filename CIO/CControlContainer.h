// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"
#include <memory>
#include <vector>

class CButton;
class CFont;
class CPicture;
class CTextfield;
class CSelectBox;

class CControlContainer
{
    friend class CDebug;

private:
    struct Picture
    {
        Position pos;
        int pic;
        unsigned id;
    };

    // if waste is true, the menu will be delete within the game loop
    Extent borderBeginSize, borderEndSize; // Width and height of border at left/top and right/bottom
    bool waste = false;
    int pic_background;
    std::vector<std::unique_ptr<CButton>> buttons;
    std::vector<std::unique_ptr<CFont>> texts;
    std::vector<std::unique_ptr<CPicture>> pictures;
    std::vector<std::unique_ptr<CTextfield>> textfields;
    std::vector<std::unique_ptr<CSelectBox>> selectboxes;
    std::vector<Picture> static_pictures;

    template<class T, class U>
    bool eraseElement(T& collection, const U* element);
    virtual bool render() = 0;

protected:
    SdlSurface surface;
    bool needRender = true;

    void renderElements();
    auto& getTextFields() { return textfields; }
    const auto& getTextFields() const { return textfields; }
    int getBackground() const { return pic_background; }

public:
    CControlContainer(int pic_background);
    CControlContainer(int pic_background, Extent borderBeginSize, Extent borderEndSize);
    ~CControlContainer() noexcept;
    // Access
    Extent getBorderSize() const { return borderBeginSize + borderEndSize; }
    void setBackgroundPicture(int pic_background);
    virtual void setMouseData(SDL_MouseMotionEvent motion);
    virtual void setMouseData(SDL_MouseButtonEvent button);
    void setKeyboardData(const SDL_KeyboardEvent& key);
    SDL_Surface* getSurface()
    {
        render();
        return surface.get();
    }
    void setWaste() { waste = true; }
    bool isWaste() const { return waste; }
    // Methods
    CButton* addButton(void callback(int), int clickedParam, Position pos = {0, 0}, Extent size = {20, 20},
                       int color = BUTTON_GREY, const char* text = nullptr, int picture = -1);
    bool delButton(CButton* ButtonToDelete);
    CFont* addText(std::string string, Position pos, FontSize fontsize, FontColor color = FontColor::Yellow);
    bool delText(CFont* TextToDelete);
    CPicture* addPicture(void callback(int), int clickedParam, Position pos, int picture);
    bool delPicture(CPicture* PictureToDelete);
    int addStaticPicture(Position pos, int picture);
    bool delStaticPicture(int picId);
    CTextfield* addTextfield(Position pos = {0, 0}, Uint16 cols = 10, Uint16 rows = 1,
                             FontSize fontsize = FontSize::Large, FontColor text_color = FontColor::Yellow,
                             int bg_color = -1, bool button_style = false);
    bool delTextfield(CTextfield* TextfieldToDelete);
    CSelectBox* addSelectBox(Position pos, Extent size, FontSize fontsize = FontSize::Large,
                             FontColor text_color = FontColor::Yellow, int bg_color = -1);
    bool delSelectBox(CSelectBox* SelectBoxToDelete);
};

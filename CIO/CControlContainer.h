#ifndef _CCONTROLCONTAINER_H
#define _CCONTROLCONTAINER_H

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
        int x, y, pic;
        unsigned id;
    };

    // if waste is true, the menu will be delete within the game loop
    Point<uint16_t> borderSize; // Width and height of border
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
    // Constructor - Destructor
    CControlContainer(int pic_background, Point<uint16_t> borderSize);
    ~CControlContainer();
    // Access
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
    bool isWaste() { return waste; }
    // Methods
    CButton* addButton(void callback(int), int clickedParam, Uint16 x = 0, Uint16 y = 0, Uint16 w = 20, Uint16 h = 20,
                       int color = BUTTON_GREY, const char* text = nullptr, int picture = -1);
    bool delButton(CButton* ButtonToDelete);
    CFont* addText(std::string string, int x = 0, int y = 0, int fontsize = 9, int color = FONT_YELLOW);
    bool delText(CFont* TextToDelete);
    CPicture* addPicture(void callback(int), int clickedParam, Uint16 x, Uint16 y, int picture);
    bool delPicture(CPicture* PictureToDelete);
    int addStaticPicture(int x, int y, int picture);
    bool delStaticPicture(int picId);
    CTextfield* addTextfield(Uint16 x = 0, Uint16 y = 0, Uint16 cols = 10, Uint16 rows = 1, int fontsize = 14, int text_color = FONT_YELLOW,
                             int bg_color = -1, bool button_style = false);
    bool delTextfield(CTextfield* TextfieldToDelete);
    CSelectBox* addSelectBox(Uint16 x = 0, Uint16 y = 0, Uint16 w = 100, Uint16 h = 100, int fontsize = 14, int text_color = FONT_YELLOW,
                             int bg_color = -1);
    bool delSelectBox(CSelectBox* SelectBoxToDelete);
};

#endif
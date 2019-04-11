#ifndef _CMENU_H
#define _CMENU_H

#include "../defines.h"

class CButton;
class CFont;
class CPicture;
class CTextfield;
class CSelectBox;

class CMenu
{
    friend class CDebug;

private:
    // if active is false, the menu will not be render within the game loop
    bool active;
    // if waste is true, the menu will be delete within the game loop
    bool waste;
    SDL_Surface* Surf_Menu;
    bool needSurface;
    bool needRender;
    int pic_background;
    CButton* buttons[MAXBUTTONS];
    CFont* texts[MAXTEXTS];
    CPicture* pictures[MAXPICTURES];
    CTextfield* textfields[MAXTEXTFIELDS];
    CSelectBox* selectboxes[MAXSELECTBOXES];
    struct
    {
        int x, y, pic;
    } static_pictures[MAXPICTURES];

public:
    // Constructor - Destructor
    CMenu(int pic_background);
    ~CMenu();
    // Access
    void setBackgroundPicture(int pic_background);
    void setMouseData(const SDL_MouseMotionEvent& motion);
    void setMouseData(const SDL_MouseButtonEvent& button);
    void setKeyboardData(const SDL_KeyboardEvent& key);
    SDL_Surface* getSurface()
    {
        render();
        return Surf_Menu;
    };
    void setActive() { active = true; };
    void setInactive() { active = false; };
    bool isActive() { return active; };
    void setWaste() { waste = true; };
    bool isWaste() { return waste; };
    // Methods
    CButton* addButton(void callback(int), int clickedParam, Uint16 x = 0, Uint16 y = 0, Uint16 w = 20, Uint16 h = 20,
                       int color = BUTTON_GREY, const char* text = nullptr, int picture = -1);
    bool delButton(CButton* ButtonToDelete);
    CFont* addText(std::string string, int x = 0, int y = 0, int fontsize = 9, int color = FONT_YELLOW);
    bool delText(CFont* TextToDelete);
    CPicture* addPicture(void callback(int), int clickedParam, Uint16 x, Uint16 y, int picture);
    bool delPicture(CPicture* PictureToDelete);
    int addStaticPicture(int x, int y, int picture);
    bool delStaticPicture(int ArrayIndex);
    CTextfield* addTextfield(Uint16 x = 0, Uint16 y = 0, Uint16 cols = 10, Uint16 rows = 1, int fontsize = 14, int text_color = FONT_YELLOW,
                             int bg_color = -1, bool button_style = false);
    bool delTextfield(CTextfield* TextfieldToDelete);
    CSelectBox* addSelectBox(Uint16 x = 0, Uint16 y = 0, Uint16 w = 100, Uint16 h = 100, int fontsize = 14, int text_color = FONT_YELLOW,
                             int bg_color = -1);
    bool delSelectBox(CSelectBox* SelectBoxToDelete);
    bool render();
};

#endif

#ifndef _CMENU_H
    #define _CMENU_H

#include "includes.h"

class CMenu
{
    friend class CDebug;

    private:
        //if active is false, the menu will not be render within the game loop
        bool active;
        //if waste is true, the menu will be delete within the game loop
        bool waste;
        SDL_Surface *Surf_Menu;
        bool needSurface;
        bool needRender;
        int pic_background;
        CButton *buttons[MAXBUTTONS];
        CFont *texts[MAXTEXTS];
        CPicture *pictures[MAXPICTURES];
        struct { int x, y, pic; } static_pictures[MAXPICTURES];

    public:
        //Constructor - Destructor
        CMenu(int pic_background);
        ~CMenu();
        //Access
        void setBackgroundPicture(int pic_background);
        void setMouseData(SDL_MouseMotionEvent motion);
        void setMouseData(SDL_MouseButtonEvent button);
        SDL_Surface* getSurface(void) { render(); return Surf_Menu; };
        void setActive(void) { active = true; };
        void setInactive(void) { active = false; };
        bool isActive(void) { return active; };
        void setWaste(void) { waste = true; };
        bool isWaste(void) { return waste; };
        //Methods
        CButton* addButton(void callback(int), int clickedParam, Uint16 x = 0, Uint16 y = 0, Uint16 width = 20, Uint16 height = 20, int color = BUTTON_GREY, const char *text = NULL, int picture = -1);
        bool delButton(CButton *ButtonToDelete);
        CFont* addText(const char *string, int x = 0, int y = 0, int fontsize = 9, int color = FONT_YELLOW);
        CFont* addText(unsigned char *string, int x = 0, int y = 0, int fontsize = 9, int color = FONT_YELLOW);
        bool delText(CFont *TextToDelete);
        CPicture* addPicture(void callback(int), int clickedParam, Uint16 x, Uint16 y, int picture);
        bool delPicture(CPicture *PictureToDelete);
        int addStaticPicture(int x, int y, int picture);
        bool delStaticPicture(int ArrayIndex);
        bool render(void);
};

#endif

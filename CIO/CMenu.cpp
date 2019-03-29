#include "CMenu.h"
#include "../CGame.h"
#include "../CSurface.h"
#include "../globals.h"
#include "CButton.h"
#include "CFont.h"
#include "CPicture.h"
#include "CSelectBox.h"
#include "CTextfield.h"

CMenu::CMenu(int pic_background)
{
    this->pic_background = pic_background;
    for(auto& button : buttons)
        button = nullptr;
    for(auto& text : texts)
        text = nullptr;
    for(auto& picture : pictures)
        picture = nullptr;
    for(auto& static_picture : static_pictures)
    {
        static_picture.x = 0;
        static_picture.y = 0;
        static_picture.pic = -1;
    }
    for(auto& textfield : textfields)
        textfield = nullptr;
    for(auto& selectbox : selectboxes)
        selectbox = nullptr;

    Surf_Menu = nullptr;
    needSurface = true;
    needRender = true;
    active = true;
    waste = false;
    render();
}

CMenu::~CMenu()
{
    for(auto& button : buttons)
        delete button;
    for(auto& text : texts)
        delete text;
    for(auto& picture : pictures)
        delete picture;
    for(auto& textfield : textfields)
        delete textfield;
    for(auto& selectbox : selectboxes)
        delete selectbox;
    SDL_FreeSurface(Surf_Menu);
}

void CMenu::setBackgroundPicture(int pic_background)
{
    this->pic_background = pic_background;
    needRender = true;
}

void CMenu::setMouseData(const SDL_MouseMotionEvent& motion)
{
    for(auto& picture : pictures)
    {
        if(picture)
            picture->setMouseData(motion);
    }
    for(auto& button : buttons)
    {
        if(button)
            button->setMouseData(motion);
    }
    for(auto& selectbox : selectboxes)
    {
        if(selectbox)
            selectbox->setMouseData(motion);
    }
    needRender = true;
}

void CMenu::setMouseData(const SDL_MouseButtonEvent& button)
{
    for(auto& picture : pictures)
    {
        if(picture)
            picture->setMouseData(button);
    }
    for(auto& i : buttons)
    {
        if(i)
            i->setMouseData(button);
    }
    for(auto& textfield : textfields)
    {
        if(textfield)
            textfield->setMouseData(button);
    }
    for(auto& selectboxe : selectboxes)
    {
        if(selectboxe)
            selectboxe->setMouseData(button);
    }
    needRender = true;
}

void CMenu::setKeyboardData(const SDL_KeyboardEvent& key)
{
    for(auto& textfield : textfields)
    {
        if(textfield)
            textfield->setKeyboardData(key);
    }
}

CButton* CMenu::addButton(void callback(int), int clickedParam, Uint16 x, Uint16 y, Uint16 w, Uint16 h, int color, const char* text,
                          int picture)
{
    if(x >= Surf_Menu->w || y >= Surf_Menu->h)
        return nullptr;

    for(auto& button : buttons)
    {
        if(!button)
        {
            button = new CButton(callback, clickedParam, x, y, w, h, color, text, picture);
            needRender = true;
            return button;
        }
    }
    return nullptr;
}

bool CMenu::delButton(CButton* ButtonToDelete)
{
    if(!ButtonToDelete)
        return false;

    for(auto& button : buttons)
    {
        if(button == ButtonToDelete)
        {
            delete button;
            button = nullptr;
            needRender = true;
            return true;
        }
    }
    return false;
}

CFont* CMenu::addText(std::string string, int x, int y, int fontsize, int color)
{
    if(x >= Surf_Menu->w || y >= Surf_Menu->h)
        return nullptr;

    for(auto& text : texts)
    {
        if(!text)
        {
            text = new CFont(std::move(string), x, y, fontsize, color);
            needRender = true;
            return text;
        }
    }
    return nullptr;
}

bool CMenu::delText(CFont* TextToDelete)
{
    if(!TextToDelete)
        return false;

    for(auto& text : texts)
    {
        if(text == TextToDelete)
        {
            delete text;
            text = nullptr;
            needRender = true;
            return true;
        }
    }
    return false;
}

CPicture* CMenu::addPicture(void callback(int), int clickedParam, Uint16 x, Uint16 y, int picture)
{
    for(auto& i : pictures)
    {
        if(!i)
        {
            i = new CPicture(callback, clickedParam, x, y, picture);
            needRender = true;
            return i;
        }
    }
    return nullptr;
}

bool CMenu::delPicture(CPicture* PictureToDelete)
{
    if(!PictureToDelete)
        return false;

    for(auto& picture : pictures)
    {
        if(picture == PictureToDelete)
        {
            delete picture;
            picture = nullptr;
            needRender = true;
            return true;
        }
    }
    return false;
}

int CMenu::addStaticPicture(int x, int y, int picture)
{
    if(picture < 0)
        return -1;

    for(int i = 0; i < MAXPICTURES; i++)
    {
        if(static_pictures[i].pic == -1)
        {
            static_pictures[i].pic = picture;
            static_pictures[i].x = x;
            static_pictures[i].y = y;
            needRender = true;
            return i;
        }
    }
    return -1;
}

bool CMenu::delStaticPicture(int ArrayIndex)
{
    if(ArrayIndex < 0 || ArrayIndex >= MAXPICTURES)
        return false;

    static_pictures[ArrayIndex].pic = -1;
    static_pictures[ArrayIndex].x = 0;
    static_pictures[ArrayIndex].y = 0;
    needRender = true;

    return true;
}

CTextfield* CMenu::addTextfield(Uint16 x, Uint16 y, Uint16 cols, Uint16 rows, int fontsize, int text_color, int bg_color, bool button_style)
{
    if(x >= Surf_Menu->w || y >= Surf_Menu->h)
        return nullptr;

    for(auto& textfield : textfields)
    {
        if(!textfield)
        {
            textfield = new CTextfield(x, y, cols, rows, fontsize, text_color, bg_color, button_style);
            needRender = true;
            return textfield;
        }
    }
    return nullptr;
}

bool CMenu::delTextfield(CTextfield* TextfieldToDelete)
{
    if(!TextfieldToDelete)
        return false;

    for(auto& textfield : textfields)
    {
        if(textfield == TextfieldToDelete)
        {
            delete textfield;
            textfield = nullptr;
            needRender = true;
            return true;
        }
    }
    return false;
}

CSelectBox* CMenu::addSelectBox(Uint16 x, Uint16 y, Uint16 w, Uint16 h, int fontsize, int text_color, int bg_color)
{
    if(x >= Surf_Menu->w || y >= Surf_Menu->h)
        return nullptr;

    for(auto& selectboxe : selectboxes)
    {
        if(!selectboxe)
        {
            selectboxe = new CSelectBox(x, y, w, h, fontsize, text_color, bg_color);
            needRender = true;
            return selectboxe;
        }
    }
    return nullptr;
}

bool CMenu::delSelectBox(CSelectBox* SelectBoxToDelete)
{
    if(!SelectBoxToDelete)
        return false;

    for(auto& selectbox : selectboxes)
    {
        if(selectbox == SelectBoxToDelete)
        {
            delete selectbox;
            selectbox = nullptr;
            needRender = true;
            return true;
        }
    }
    return false;
}

bool CMenu::render()
{
    if(pic_background < 0)
        return false;

    // if we don't need to render, all is up to date, return true
    if(!needRender)
        return true;
    needRender = false;
    // if we need a new surface
    if(needSurface)
    {
        SDL_FreeSurface(Surf_Menu);
        Surf_Menu = SDL_CreateRGBSurface(SDL_SWSURFACE, global::s2->getRes().x, global::s2->getRes().y, 32, 0, 0, 0, 0);
        if(!Surf_Menu)
            return false;
        needSurface = false;
    }

    // CSurface::Draw(Surf_Menu, global::bmpArray[pic_background].surface, 0, 0);
    SDL_Surface* surfBG = global::bmpArray[pic_background].surface;
    sge_TexturedRect(Surf_Menu, 0, 0, Surf_Menu->w - 1, 0, 0, Surf_Menu->h - 1, Surf_Menu->w - 1, Surf_Menu->h - 1, surfBG, 0, 0,
                     surfBG->w - 1, 0, 0, surfBG->h - 1, surfBG->w - 1, surfBG->h - 1);

    for(auto& static_picture : static_pictures)
    {
        if(static_picture.pic >= 0)
            CSurface::Draw(Surf_Menu, global::bmpArray[static_picture.pic].surface, static_picture.x, static_picture.y);
    }
    for(auto& picture : pictures)
    {
        if(picture)
            CSurface::Draw(Surf_Menu, picture->getSurface(), picture->getX(), picture->getY());
    }
    for(auto& text : texts)
    {
        if(text)
            CSurface::Draw(Surf_Menu, text->getSurface(), text->getX(), text->getY());
    }
    for(auto& textfield : textfields)
    {
        if(textfield)
            CSurface::Draw(Surf_Menu, textfield->getSurface(), textfield->getX(), textfield->getY());
    }
    for(auto& selectbox : selectboxes)
    {
        if(selectbox)
            CSurface::Draw(Surf_Menu, selectbox->getSurface(), selectbox->getX(), selectbox->getY());
    }
    for(auto& button : buttons)
    {
        if(button)
            CSurface::Draw(Surf_Menu, button->getSurface(), button->getX(), button->getY());
    }

    return true;
}

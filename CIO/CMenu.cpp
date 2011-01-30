#include "CMenu.h"

CMenu::CMenu(int pic_background)
{
    this->pic_background = pic_background;
    for (int i = 0; i < MAXBUTTONS; i++)
        buttons[i] = NULL;
    for (int i = 0; i < MAXTEXTS; i++)
        texts[i] = NULL;
    for (int i = 0; i < MAXPICTURES; i++)
        pictures[i] = NULL;
    for (int i = 0; i < MAXPICTURES; i++)
    {
        static_pictures[i].x = 0;
        static_pictures[i].y = 0;
        static_pictures[i].pic = -1;
    }
    for (int i = 0; i < MAXTEXTFIELDS; i++)
        textfields[i] = NULL;

    Surf_Menu = NULL;
    needSurface = true;
    needRender = true;
    active = true;
    waste = false;
    render();
}

CMenu::~CMenu()
{
    for (int i = 0; i < MAXBUTTONS; i++)
    {
        if (buttons[i] != NULL)
            delete buttons[i];
    }
    for (int i = 0; i < MAXTEXTS; i++)
    {
        if (texts[i] != NULL)
            delete texts[i];
    }
    for (int i = 0; i < MAXPICTURES; i++)
    {
        if (pictures[i] != NULL)
            delete pictures[i];
    }
    for (int i = 0; i < MAXTEXTFIELDS; i++)
    {
        if (textfields[i] != NULL)
            delete textfields[i];
    }
    SDL_FreeSurface(Surf_Menu);
}

void CMenu::setBackgroundPicture(int pic_background)
{
    this->pic_background = pic_background;
    needRender = true;
}

void CMenu::setMouseData(SDL_MouseMotionEvent motion)
{
    for (int i = 0; i < MAXPICTURES; i++)
    {
        if (pictures[i] != NULL)
            pictures[i]->setMouseData(motion);
    }
    for (int i = 0; i < MAXBUTTONS; i++)
    {
        if (buttons[i] != NULL)
            buttons[i]->setMouseData(motion);
    }
    needRender = true;
}

void CMenu::setMouseData(SDL_MouseButtonEvent button)
{
    for (int i = 0; i < MAXPICTURES; i++)
    {
        if (pictures[i] != NULL)
            pictures[i]->setMouseData(button);
    }
    for (int i = 0; i < MAXBUTTONS; i++)
    {
        if (buttons[i] != NULL)
            buttons[i]->setMouseData(button);
    }
    for (int i = 0; i < MAXTEXTFIELDS; i++)
    {
        if (textfields[i] != NULL)
            textfields[i]->setMouseData(button);
    }
    needRender = true;
}

void CMenu::setKeyboardData(SDL_KeyboardEvent key)
{
    for (int i = 0; i < MAXTEXTFIELDS; i++)
    {
        if (textfields[i] != NULL)
            textfields[i]->setKeyboardData(key);
    }
}

CButton* CMenu::addButton(void callback(int), int clickedParam, Uint16 x, Uint16 y, Uint16 w, Uint16 h, int color, const char *text, int picture)
{
    if (x >= Surf_Menu->w || y >= Surf_Menu->h)
        return false;

    for (int i = 0; i < MAXBUTTONS; i++)
    {
        if (buttons[i] == NULL)
        {
            buttons[i] = new CButton(callback, clickedParam, x, y, w, h, color, text, picture);
            needRender = true;
            return buttons[i];
        }
    }
    return NULL;
}

bool CMenu::delButton(CButton *ButtonToDelete)
{
    if (ButtonToDelete == NULL)
        return false;

    for (int i = 0; i < MAXBUTTONS; i++)
    {
        if (buttons[i] == ButtonToDelete)
        {
            delete buttons[i];
            buttons[i] = NULL;
            needRender = true;
            return true;
        }
    }
    return false;
}

CFont* CMenu::addText(const char *string, int x, int y, int fontsize, int color)
{
    if (x >= Surf_Menu->w || y >= Surf_Menu->h)
        return false;

    for (int i = 0; i < MAXTEXTS; i++)
    {
        if (texts[i] == NULL)
        {
            texts[i] = new CFont(string, x, y, fontsize, color);
            needRender = true;
            return texts[i];
        }
    }
    return NULL;
}

CFont* CMenu::addText(unsigned char *string, int x, int y, int fontsize, int color)
{
    if (x >= Surf_Menu->w || y >= Surf_Menu->h)
        return false;

    for (int i = 0; i < MAXTEXTS; i++)
    {
        if (texts[i] == NULL)
        {
            texts[i] = new CFont(string, x, y, fontsize, color);
            needRender = true;
            return texts[i];
        }
    }
    return NULL;
}

bool CMenu::delText(CFont *TextToDelete)
{
    if (TextToDelete == NULL)
        return false;

    for (int i = 0; i < MAXTEXTS; i++)
    {
        if (texts[i] == TextToDelete)
        {
            delete texts[i];
            texts[i] = NULL;
            needRender = true;
            return true;
        }
    }
    return false;
}

CPicture* CMenu::addPicture(void callback(int), int clickedParam, Uint16 x, Uint16 y, int picture)
{
    if (x >= global::s2->MenuResolutionX || y >= global::s2->MenuResolutionY)
        return false;

    for (int i = 0; i < MAXPICTURES; i++)
    {
        if (pictures[i] == NULL)
        {
            pictures[i] = new CPicture(callback, clickedParam, x, y, picture);
            needRender = true;
            return pictures[i];
        }
    }
    return NULL;
}

bool CMenu::delPicture(CPicture *PictureToDelete)
{
    if (PictureToDelete == NULL)
        return false;

    for (int i = 0; i < MAXPICTURES; i++)
    {
        if (pictures[i] == PictureToDelete)
        {
            delete pictures[i];
            pictures[i] = NULL;
            needRender = true;
            return true;
        }
    }
    return false;
}

int CMenu::addStaticPicture(int x, int y, int picture)
{
    if (x >= global::s2->MenuResolutionX || y >= global::s2->MenuResolutionY)
        return false;

    if (picture < 0)
        return -1;

    for (int i = 0; i < MAXPICTURES; i++)
    {
        if (static_pictures[i].pic == -1)
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
    if (ArrayIndex < 0 || ArrayIndex >= MAXPICTURES)
        return false;

    static_pictures[ArrayIndex].pic = -1;
    static_pictures[ArrayIndex].x = 0;
    static_pictures[ArrayIndex].y = 0;
    needRender = true;

    return true;
}

CTextfield* CMenu::addTextfield(Uint16 x, Uint16 y, Uint16 cols, Uint16 rows, int fontsize, int text_color, int bg_color, bool button_style)
{
    if (x >= Surf_Menu->w || y >= Surf_Menu->h)
        return false;

    for (int i = 0; i < MAXTEXTFIELDS; i++)
    {
        if (textfields[i] == NULL)
        {
            textfields[i] = new CTextfield(x, y, cols, rows, fontsize, text_color, bg_color, button_style);
            needRender = true;
            return textfields[i];
        }
    }
    return NULL;
}

bool CMenu::delTextfield(CTextfield* TextfieldToDelete)
{
    if (TextfieldToDelete == NULL)
        return false;

    for (int i = 0; i < MAXTEXTFIELDS; i++)
    {
        if (textfields[i] == TextfieldToDelete)
        {
            delete textfields[i];
            textfields[i] = NULL;
            needRender = true;
            return true;
        }
    }
    return false;
}

bool CMenu::render(void)
{
    if (pic_background < 0)
        return false;

    //if we don't need to render, all is up to date, return true
    if (!needRender)
        return true;
    needRender = false;
    //if we need a new surface
    if (needSurface)
    {
        SDL_FreeSurface(Surf_Menu);
        Surf_Menu = NULL;
        if ( (Surf_Menu = SDL_CreateRGBSurface(SDL_SWSURFACE, global::s2->MenuResolutionX, global::s2->MenuResolutionY, 32, 0, 0, 0, 0)) == NULL )
            return false;
        needSurface = false;
    }

    CSurface::Draw(Surf_Menu, global::bmpArray[pic_background].surface, 0, 0);
    for (int i = 0; i < MAXPICTURES; i++)
    {
        if (static_pictures[i].pic >= 0)
            CSurface::Draw(Surf_Menu, global::bmpArray[static_pictures[i].pic].surface, static_pictures[i].x, static_pictures[i].y);
    }
    for (int i = 0; i < MAXPICTURES; i++)
    {
        if (pictures[i] != NULL)
            CSurface::Draw(Surf_Menu, pictures[i]->getSurface(), pictures[i]->getX(), pictures[i]->getY());
    }
    for (int i = 0; i < MAXTEXTS; i++)
    {
        if (texts[i] != NULL)
            CSurface::Draw(Surf_Menu, texts[i]->getSurface(), texts[i]->getX(), texts[i]->getY());
    }
    for (int i = 0; i < MAXTEXTFIELDS; i++)
    {
        if (textfields[i] != NULL)
            CSurface::Draw(Surf_Menu, textfields[i]->getSurface(), textfields[i]->getX(), textfields[i]->getY());
    }
    for (int i = 0; i < MAXBUTTONS; i++)
    {
        if (buttons[i] != NULL)
            CSurface::Draw(Surf_Menu, buttons[i]->getSurface(), buttons[i]->getX(), buttons[i]->getY());
    }

    return true;
}

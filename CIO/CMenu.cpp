#include "CMenu.h"
#include "../CGame.h"
#include "../globals.h"
#include <SGE/sge_blib.h>

CMenu::CMenu(int pic_background) : CControlContainer(pic_background) {}

bool CMenu::render()
{
    if(getBackground() < 0)
        return false;

    // if we don't need to render, all is up to date, return true
    if(!needRender)
        return true;
    needRender = false;
    // if we need a new surface
    if(!surface)
    {
        surface = makeRGBSurface(global::s2->getRes().x, global::s2->getRes().y);
        if(!surface)
            return false;
    }

    // CSurface::Draw(surface, global::bmpArray[pic_background].surface, 0, 0);
    auto& surfBG = global::bmpArray[getBackground()].surface;
    sge_TexturedRect(surface.get(), 0, 0, surface->w - 1, 0, 0, surface->h - 1, surface->w - 1, surface->h - 1, surfBG.get(), 0, 0,
                     surfBG->w - 1, 0, 0, surfBG->h - 1, surfBG->w - 1, surfBG->h - 1);
    renderElements();
    return true;
}

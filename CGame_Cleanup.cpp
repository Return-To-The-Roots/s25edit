#include "CGame.h"

void CGame::Cleanup()
{
    //unregister menus
    for (int i = 0; i < MAXMENUS; i++)
    {
        if (Menus[i] != NULL)
            CSurface::Draw(Surf_Display, Menus[i]->getSurface(), 0, 0);
    }
    //unregister windows
    for (int i = 0; i < MAXWINDOWS; i++)
    {
        if (Windows[i] != NULL)
            CSurface::Draw(Surf_Display, Windows[i]->getSurface(), 0, 0);
    }

    //free all picture surfaces
    for (int i = 0; i < MAXBOBBMP; i++)
    {
        if (global::bmpArray[i].surface != NULL)
            SDL_FreeSurface(global::bmpArray[i].surface);
    }
    //free all shadow surfaces
    for (int i = 0; i < MAXBOBSHADOW; i++)
    {
        if (global::shadowArray[i].surface != NULL)
            SDL_FreeSurface(global::shadowArray[i].surface);
    }

    SDL_FreeSurface(Surf_Display);

    SDL_Quit();
}

#include "CGame.h"
#include "CIO/CMenu.h"
#include "CIO/CWindow.h"
#include "CSurface.h"
#include "globals.h"

void CGame::Cleanup()
{
    // unregister menus
    for(int i = 0; i < MAXMENUS; i++)
    {
        if(Menus[i] != nullptr)
            CSurface::Draw(Surf_Display, Menus[i]->getSurface(), 0, 0);
    }
    // unregister windows
    for(int i = 0; i < MAXWINDOWS; i++)
    {
        if(Windows[i] != nullptr)
            CSurface::Draw(Surf_Display, Windows[i]->getSurface(), 0, 0);
    }

    // free all picture surfaces
    for(int i = 0; i < MAXBOBBMP; i++)
    {
        if(global::bmpArray[i].surface != nullptr)
            SDL_FreeSurface(global::bmpArray[i].surface);
    }
    // free all shadow surfaces
    for(int i = 0; i < MAXBOBSHADOW; i++)
    {
        if(global::shadowArray[i].surface != nullptr)
            SDL_FreeSurface(global::shadowArray[i].surface);
    }

    SDL_FreeSurface(Surf_Display);
    if(Surf_DisplayGL != nullptr)
        SDL_FreeSurface(Surf_DisplayGL);

    SDL_Quit();
}

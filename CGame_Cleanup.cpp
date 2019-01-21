#include "CGame.h"
#include "CIO/CMenu.h"
#include "CIO/CWindow.h"
#include "CSurface.h"
#include "globals.h"

void CGame::Cleanup()
{
    // unregister menus
    for(auto& Menu : Menus)
    {
        if(Menu)
            CGame::UnregisterMenu(Menu);
    }
    // unregister windows
    for(auto& Window : Windows)
    {
        if(Window)
            CGame::UnregisterWindow(Window);
    }

    // free all picture surfaces
    for(int i = 0; i < MAXBOBBMP; i++)
    {
        if(global::bmpArray[i].surface)
            SDL_FreeSurface(global::bmpArray[i].surface);
    }
    // free all shadow surfaces
    for(int i = 0; i < MAXBOBSHADOW; i++)
    {
        if(global::shadowArray[i].surface)
            SDL_FreeSurface(global::shadowArray[i].surface);
    }

    SDL_FreeSurface(Surf_Display);
    if(Surf_DisplayGL)
        SDL_FreeSurface(Surf_DisplayGL);

    SDL_Quit();
}

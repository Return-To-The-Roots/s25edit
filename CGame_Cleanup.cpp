#include "defines.h"
#include "CGame.h"
#include "globals.h"
#include <SDL.h>

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
    for(auto& i : global::bmpArray)
    {
        if(i.surface)
            SDL_FreeSurface(i.surface);
    }
    // free all shadow surfaces
    for(auto& i : global::shadowArray)
    {
        if(i.surface)
            SDL_FreeSurface(i.surface);
    }

    SDL_FreeSurface(Surf_Display);
    if(Surf_DisplayGL)
        SDL_FreeSurface(Surf_DisplayGL);

    SDL_Quit();
}

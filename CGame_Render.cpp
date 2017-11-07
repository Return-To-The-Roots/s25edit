#include "CGame.h"
#include "CIO/CMenu.h"
#include "CIO/CWindow.h"
#include "CMap.h"
#include "CSurface.h"
#include "globals.h"
#include <cstdio>
#ifdef _WIN32
#include "s25editResource.h"
#include <SDL_syswm.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

void CGame::SetAppIcon()
{
#ifdef _WIN32
    LPARAM icon = (LPARAM)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_SYMBOL));
    SendMessage(GetConsoleWindow(), WM_SETICON, ICON_BIG, icon);
    SendMessage(GetConsoleWindow(), WM_SETICON, ICON_SMALL, icon);

    SDL_SysWMinfo info;
    // get window handle from SDL
    SDL_VERSION(&info.version);
    if(SDL_GetWMInfo(&info) != 1)
        return;
    SendMessage(info.window, WM_SETICON, ICON_BIG, icon);
    SendMessage(info.window, WM_SETICON, ICON_SMALL, icon);
#endif // _WIN32
}

void CGame::Render()
{
    // clear the surface before drawing new (in normal case not needed)
    // SDL_FillRect( Surf_Display, NULL, SDL_MapRGB(Surf_Display->format,0,0,0) );

    // check resolution
    /*
    if (MapObj == NULL || !MapObj->isActive())
    {
        //we are in menu
        if ( (Surf_Display->w != MenuResolutionX || Surf_Display->h != MenuResolutionY) ||
            ( (fullscreen && !(Surf_Display->flags&SDL_FULLSCREEN)) || (!fullscreen && (Surf_Display->flags&SDL_FULLSCREEN)) )
           )
        {
            SDL_FreeSurface(Surf_Display);
            Surf_Display = SDL_SetVideoMode(MenuResolutionX, MenuResolutionY, 32, SDL_SWSURFACE | SDL_DOUBLEBUF | (fullscreen ?
    SDL_FULLSCREEN : 0));
        }
    }
    else
    {
    */
    // we are in game
    if((Surf_Display->w != GameResolutionX || Surf_Display->h != GameResolutionY)
       || ((fullscreen && !(Surf_Display->flags & SDL_FULLSCREEN)) || (!fullscreen && (Surf_Display->flags & SDL_FULLSCREEN))))
    {
        SDL_FreeSurface(Surf_Display);
        Surf_Display = NULL;

        if(CSurface::useOpenGL)
        {
            SDL_FreeSurface(Surf_DisplayGL);

            Surf_DisplayGL = SDL_SetVideoMode(GameResolutionX, GameResolutionY, 32, SDL_OPENGL | (fullscreen ? SDL_FULLSCREEN : 0));
            Surf_Display = SDL_CreateRGBSurface(SDL_SWSURFACE, GameResolutionX, GameResolutionY, 32, 0, 0, 0, 0);
        } else
        {
            Surf_Display =
              SDL_SetVideoMode(GameResolutionX, GameResolutionY, 32, SDL_SWSURFACE | SDL_DOUBLEBUF | (fullscreen ? SDL_FULLSCREEN : 0));
        }
        SetAppIcon();
    }
    //}

    // if the S2 loading screen is shown, render only this until user clicks a mouse button
    if(showLoadScreen)
    {
        // CSurface::Draw(Surf_Display, global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface, 0, 0);
        sge_TexturedRect(
          Surf_Display, 0, 0, Surf_Display->w - 1, 0, 0, Surf_Display->h - 1, Surf_Display->w - 1, Surf_Display->h - 1,
          global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface, 0, 0, global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface->w - 1, 0,
          0, global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface->h - 1, global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface->w - 1,
          global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface->h - 1);

        if(CSurface::useOpenGL)
        {
            // SDL_BlitSurface(Surf_Display, NULL, Surf_DisplayGL, NULL);
            // SDL_Flip(Surf_DisplayGL);
            SDL_GL_SwapBuffers();
        } else
            SDL_Flip(Surf_Display);
        return;
    }

    // render the map if active
    if(MapObj != NULL && MapObj->isActive())
        CSurface::Draw(Surf_Display, MapObj->getSurface(), 0, 0);

    // render active menus
    for(int i = 0; i < MAXMENUS; i++)
    {
        if(Menus[i] != NULL && Menus[i]->isActive())
            CSurface::Draw(Surf_Display, Menus[i]->getSurface(), 0, 0);
    }

    // render windows ordered by priority
    int highestPriority = 0;
    // first find the highest priority
    for(int i = 0; i < MAXWINDOWS; i++)
    {
        if(Windows[i] != NULL && Windows[i]->getPriority() > highestPriority)
            highestPriority = Windows[i]->getPriority();
    }
    // render from lowest priority to highest
    for(int actualPriority = 0; actualPriority <= highestPriority; actualPriority++)
    {
        for(int i = 0; i < MAXWINDOWS; i++)
        {
            if(Windows[i] != NULL && Windows[i]->getPriority() == actualPriority)
                CSurface::Draw(Surf_Display, Windows[i]->getSurface(), Windows[i]->getX(), Windows[i]->getY());
        }
    }

    // render mouse cursor
    if(Cursor.clicked == true)
    {
        if(Cursor.button.right)
            CSurface::Draw(Surf_Display, global::bmpArray[CROSS].surface, Cursor.x, Cursor.y);
        else
            CSurface::Draw(Surf_Display, global::bmpArray[CURSOR_CLICKED].surface, Cursor.x, Cursor.y);
    } else
        CSurface::Draw(Surf_Display, global::bmpArray[CURSOR].surface, Cursor.x, Cursor.y);

#ifdef _ADMINMODE
    FrameCounter++;
#endif

    if(CSurface::useOpenGL)
    {
        SDL_BlitSurface(Surf_Display, NULL, Surf_DisplayGL, NULL);
        SDL_Flip(Surf_DisplayGL);
        SDL_GL_SwapBuffers();
    } else
        SDL_Flip(Surf_Display);

    SDL_Delay(msWait);
}

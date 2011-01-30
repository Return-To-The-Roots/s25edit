#include "CGame.h"
#include <stdio.h>

void CGame::Render()
{
    //clear the surface before drawing new (in normal case not needed)
    //SDL_FillRect( Surf_Display, NULL, SDL_MapRGB(Surf_Display->format,0,0,0) );

    //check resolution
    if (MapObj == NULL || !MapObj->isActive())
    {
        //we are in menu
        if ( (Surf_Display->w != MenuResolutionX || Surf_Display->h != MenuResolutionY) ||
            ( (fullscreen && !(Surf_Display->flags&SDL_FULLSCREEN)) || (!fullscreen && (Surf_Display->flags&SDL_FULLSCREEN)) )
           )
        {
            SDL_FreeSurface(Surf_Display);
            Surf_Display = SDL_SetVideoMode(MenuResolutionX, MenuResolutionY, 32, SDL_SWSURFACE | SDL_DOUBLEBUF | (fullscreen ? SDL_FULLSCREEN : 0));
        }
    }
    else
    {
        //we are in game
        if ( (Surf_Display->w != GameResolutionX || Surf_Display->h != GameResolutionY) ||
            ( (fullscreen && !(Surf_Display->flags&SDL_FULLSCREEN)) || (!fullscreen && (Surf_Display->flags&SDL_FULLSCREEN)) )
           )
        {
            SDL_FreeSurface(Surf_Display);
            Surf_Display = SDL_SetVideoMode(GameResolutionX, GameResolutionY, 32, SDL_SWSURFACE | SDL_DOUBLEBUF | (fullscreen ? SDL_FULLSCREEN : 0));
        }
    }

#ifndef _VIEWERMODE
    //if the S2 loading screen is shown, render only this until user clicks a mouse button
    if (showLoadScreen)
    {
        CSurface::Draw(Surf_Display, global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface, 0, 0);
        SDL_Flip(Surf_Display);
        return;
    }
#endif

    //render the map if active
    if (MapObj != NULL && MapObj->isActive())
        CSurface::Draw(Surf_Display, MapObj->getSurface(), 0, 0);

    //render active menus
    for (int i = 0; i < MAXMENUS; i++)
    {
        if (Menus[i] != NULL && Menus[i]->isActive())
            CSurface::Draw(Surf_Display, Menus[i]->getSurface(), 0, 0);
    }

    //render windows ordered by priority
    int highestPriority = 0;
    //first find the highest priority
    for (int i = 0; i < MAXWINDOWS; i++)
    {
        if (Windows[i] != NULL && Windows[i]->getPriority() > highestPriority)
            highestPriority = Windows[i]->getPriority();
    }
    //render from lowest priority to highest
    for (int actualPriority = 0; actualPriority <= highestPriority; actualPriority++)
    {
        for (int i = 0; i < MAXWINDOWS; i++)
        {
            if (Windows[i] != NULL && Windows[i]->getPriority() == actualPriority)
                CSurface::Draw(Surf_Display, Windows[i]->getSurface(), Windows[i]->getX(), Windows[i]->getY());
        }
    }

#ifdef _VIEWERMODE
    SDL_FillRect(Surf_Display, NULL, SDL_MapRGB(Surf_Display->format, 40,40,40));
    CSurface::Draw(Surf_Display, global::bmpArray[index].surface, 0, 0);
    static char infos[50];
    sprintf(infos, "index=%d, w=%d, h=%d, nx=%d, ny=%d", index, global::bmpArray[index].w, global::bmpArray[index].h, global::bmpArray[index].nx, global::bmpArray[index].ny);
    CFont::writeText(Surf_Display, infos, 350, 400, 14);
#endif

    //render mouse cursor
    if (Cursor.clicked == true)
    {
        if (Cursor.button.right)
            CSurface::Draw(Surf_Display, global::bmpArray[CROSS].surface, Cursor.x, Cursor.y);
        else
            CSurface::Draw(Surf_Display, global::bmpArray[CURSOR_CLICKED].surface, Cursor.x, Cursor.y);
    }
    else
        CSurface::Draw(Surf_Display, global::bmpArray[CURSOR].surface, Cursor.x, Cursor.y);

#ifdef _ADMINMODE
    FrameCounter++;
#endif

    //CSurface::Draw(Surf_Display, bmpArray[0].surface, 0, 0);
    //CFont::writeText(Surf_Display, "!@#$%&*()-_=+,<.>/?;:'\"\\1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ©ÄÇÖÜßàáâäçèéêëìíîïñòóôöùúûü", 10, 150, 9);
    //CFont::writeText(Surf_Display, "!!!!", 10, 150, 14, FONT_YELLOW);
    //char text[] = "hasdfsdf34!!!!";
    //CFont::writeText(Surf_Display, (unsigned char*)text, 10, 150, 14, FONT_MINTGREEN, ALIGN_RIGHT);

    SDL_Flip(Surf_Display);

    SDL_Delay(msWait);
}

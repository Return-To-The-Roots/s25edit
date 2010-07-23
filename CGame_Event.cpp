#include "CGame.h"

void CGame::EventHandling(SDL_Event *Event)
{
    switch(Event->type) {
        case SDL_ACTIVEEVENT: {
            switch(Event->active.state) {
                case SDL_APPMOUSEFOCUS: {


                    break;
                }
                case SDL_APPINPUTFOCUS: {


                    break;
                }
                case SDL_APPACTIVE: {


                    break;
                }
            }
            break;
        }

        case SDL_KEYDOWN: {

            //deliver keyboard data to map
            if (Map != NULL)
                Map->setKeyboardData(Event->key);

            switch (Event->key.keysym.sym)
            {
                case SDLK_F1:       if (fullscreen)
                                        fullscreen = false;
                                    else
                                        fullscreen = true;
                                    break;
                case SDLK_F5:       GameResolutionX = 1440;
                                    GameResolutionY = 900;
                                    break;

#ifdef _ADMINMODE
                case SDLK_F12:      //if CTRL and ALT are pressed
                                    if (SDL_GetModState() == (KMOD_LCTRL | KMOD_LALT))
                                        callback::debugger(INITIALIZING_CALL);
                                    break;
/*#else
                case SDLK_F12:
                                    break;

*/#endif

#ifdef _VIEWERMODE
                case SDLK_RIGHT:    index++;
                                    break;

                case SDLK_LEFT:     (index > 0 ? index-- : 0);
                                    break;

                case SDLK_UP:       index+=10;
                                    break;

                case SDLK_DOWN:     (index >= 10 ? index-=10 : index = 0);
                                    break;
/*#else
                case SDLK_RIGHT:
                                    break;

                case SDLK_LEFT:
                                    break;

                case SDLK_UP:
                                    break;

                case SDLK_DOWN:
                                    break;
*/#endif

                default:            break;
            }

            break;
        }

        case SDL_KEYUP: {

            //deliver keyboard data to map
            if (Map != NULL)
                Map->setKeyboardData(Event->key);

            break;
        }

        case SDL_MOUSEMOTION:
        {
            //setup mouse cursor data
            if ((Event->motion.state&SDL_BUTTON(SDL_BUTTON_RIGHT))==0)
            {
                Cursor.x = Event->motion.x;
                Cursor.y = Event->motion.y;
            }
/*
            //NOTE: we will now deliver the data to menus, windows, map etc., sometimes we have to break the switch and stop
            //      delivering earlier, for doing this we make use of a variable showing us the deliver status
            int delivered = false;
            //deliver mouse motion data to the active window
            for (int i = 0; i < MAXWINDOWS; i++)
            {
                if (Windows[i] != NULL && Windows[i]->isActive() && !Windows[i]->isWaste())
                {
                    Windows[i]->setMouseData(Event->motion);
                    if ( (Event->motion.x >= Windows[i]->getX()) && (Event->motion.x < Windows[i]->getX() + Windows[i]->getW()) && (Event->motion.y >= Windows[i]->getY()) && (Event->motion.y < Windows[i]->getY() + Windows[i]->getH()) )
                        delivered = true;
                }
            }
            if (delivered)
                break;
*/

            //NOTE: we will now deliver the data to menus, windows, map etc., sometimes we have to break the switch and stop
            //      delivering earlier, for doing this we make use of a variable showing us the deliver status
            bool delivered = false;
            //now we walk through the windows and find out, if cursor is on one of these (ordered by priority)
            //we have to change the prioritys of the windows (for rendering), so find the highest one
            int highestPriority = 0;
            for (int j = 0; j < MAXWINDOWS; j++)
            {
                if (Windows[j] != NULL && Windows[j]->getPriority() > highestPriority)
                    highestPriority = Windows[j]->getPriority();
            }

            for (int actualPriority = highestPriority; actualPriority >= 0; actualPriority--)
            {
                for (int i = 0; i < MAXWINDOWS; i++)
                {
                    if (Windows[i] != NULL && !Windows[i]->isWaste() && Windows[i]->getPriority() == actualPriority)
                    {
                            //Windows[i]->setActive();
                            //Windows[i]->setPriority(highestPriority+1);
                            Windows[i]->setMouseData(Event->motion);
                            //is the cursor INSIDE the window?
                        if ( (Event->motion.x >= Windows[i]->getX()) && (Event->motion.x < Windows[i]->getX() + Windows[i]->getW()) && (Event->motion.y >= Windows[i]->getY()) && (Event->motion.y < Windows[i]->getY() + Windows[i]->getH()) )
                        {
                            delivered = true;
                            break;
                        }
                    }
                }
                if (delivered)
                    break;
            }
            //if mouse data has been deliverd, stop delivering anymore
            if (delivered)
                break;

            //deliver mouse motion data to map if active
            if (Map != NULL && Map->isActive())
            {
                Map->setMouseData(Event->motion);
                //data has been delivered to map, so no menu is in the foreground --> stop delivering
                break;
            }

            //deliver mouse motion data to active menus
            for (int i = 0; i < MAXMENUS; i++)
            {
                if (Menus[i] != NULL && Menus[i]->isActive() && !Menus[i]->isWaste())
                    Menus[i]->setMouseData(Event->motion);
            }

            /*if (Button != NULL)
            {
                Button->setMouseData(Event->motion);
                CSurface::Draw(Surf_Display, Button->getSurface(), 100, 100);
            }*/

            break;
        }

        case SDL_MOUSEBUTTONDOWN:
        {
            //setup mouse cursor data
            Cursor.clicked = true;
            Cursor.button.left = false;
            Cursor.button.right = false;
            if (Event->button.button == SDL_BUTTON_LEFT)
                Cursor.button.left = true;
            else if (Event->button.button == SDL_BUTTON_RIGHT)
                Cursor.button.right = true;

            //clicking a mouse button will close the S2 loading screen if it is shown
            if (showLoadScreen)
            {
                showLoadScreen = false;
                //prevent pressing another object "behind" the loading screen
                break;
            }

            //NOTE: we will now deliver the data to menus, windows, map etc., sometimes we have to break the switch and stop
            //      delivering earlier, for doing this we make use of a variable showing us the deliver status
            bool delivered = false;
            //now we walk through the windows and find out, if cursor is on one of these (ordered by priority)
            //we have to change the prioritys of the windows (for rendering), so find the highest one
            int highestPriority = 0;
            for (int j = 0; j < MAXWINDOWS; j++)
            {
                if (Windows[j] != NULL && Windows[j]->getPriority() > highestPriority)
                    highestPriority = Windows[j]->getPriority();
            }

            for (int actualPriority = highestPriority; actualPriority >= 0; actualPriority--)
            {
                for (int i = 0; i < MAXWINDOWS; i++)
                {
                    if (Windows[i] != NULL && !Windows[i]->isWaste() && Windows[i]->getPriority() == actualPriority)
                    {
                        //is the cursor INSIDE the window?
                        if ( (Event->button.x >= Windows[i]->getX()) && (Event->button.x < Windows[i]->getX() + Windows[i]->getW()) && (Event->button.y >= Windows[i]->getY()) && (Event->button.y < Windows[i]->getY() + Windows[i]->getH()) )
                        {
                            Windows[i]->setActive();
                            Windows[i]->setPriority(highestPriority+1);
                            Windows[i]->setMouseData(Event->button);
                            delivered = true;
                            break;
                        }
                        else if (Windows[i]->isActive())
                            Windows[i]->setInactive();
                    }
                }
                if (delivered)
                    break;
            }
            //if mouse data has been deliverd, stop delivering anymore
            if (delivered)
                break;

            //deliver mouse button data to map if active
            if (Map != NULL && Map->isActive())
            {
                Map->setMouseData(Event->button);
                //data has been delivered to map, so no menu is in the foreground --> stop delivering
                break;
            }

            //deliver mouse button data to active menus
            for (int i = 0; i < MAXMENUS; i++)
            {
                if (Menus[i] != NULL && Menus[i]->isActive() && !Menus[i]->isWaste())
                    Menus[i]->setMouseData(Event->button);
            }

            /*if (Button != NULL)
            {
                Button->setMouseData(Event->button);
                CSurface::Draw(Surf_Display, Button->getSurface(), 100, 100);
            }*/

            switch(Event->button.button) {
                case SDL_BUTTON_LEFT: {

                    break;
                }
                case SDL_BUTTON_RIGHT: {

                    break;
                }
                case SDL_BUTTON_MIDDLE: {

                    break;
                }
            }
            break;
        }

        case SDL_MOUSEBUTTONUP:
        {
            //setup mouse cursor data
            Cursor.clicked = false;

            //NOTE: we will now deliver the data to menus, windows, map etc., sometimes we have to break the switch and stop
            //      delivering earlier, for doing this we make use of a variable showing us the deliver status
            bool delivered = false;
            //now we walk through the windows and find out, if cursor is on one of these (ordered by priority)
            //we have to change the prioritys of the windows (for rendering), so find the highest one
            int highestPriority = 0;
            for (int j = 0; j < MAXWINDOWS; j++)
            {
                if (Windows[j] != NULL && Windows[j]->getPriority() > highestPriority)
                    highestPriority = Windows[j]->getPriority();
            }

            for (int actualPriority = highestPriority; actualPriority >= 0; actualPriority--)
            {
                for (int i = 0; i < MAXWINDOWS; i++)
                {
                    if (Windows[i] != NULL && !Windows[i]->isWaste() && Windows[i]->getPriority() == actualPriority)
                    {
                        //is the cursor INSIDE the window?
                        if ( (Event->button.x >= Windows[i]->getX()) && (Event->button.x < Windows[i]->getX() + Windows[i]->getW()) && (Event->button.y >= Windows[i]->getY()) && (Event->button.y < Windows[i]->getY() + Windows[i]->getH()) )
                        {
                            //Windows[i]->setActive();
                            //Windows[i]->setPriority(highestPriority+1);
                            Windows[i]->setMouseData(Event->button);
                            delivered = true;
                            break;
                        }
                        //else if (Windows[i]->isActive())
                            //Windows[i]->setInactive();
                    }
                }
                if (delivered)
                    break;
            }
            //if mouse data has been deliverd, stop delivering anymore
            if (delivered)
                break;

            //if still not delivered, keep delivering to secondary elements like menu or map

            //deliver mouse button data to map if active
            if (Map != NULL && Map->isActive())
            {
                Map->setMouseData(Event->button);
                //data has been delivered to map, so no menu is in the foreground --> stop delivering
                break;
            }

            //deliver mouse button data to active menus
            for (int i = 0; i < MAXMENUS; i++)
            {
                if (Menus[i] != NULL && Menus[i]->isActive() && !Menus[i]->isWaste())
                    Menus[i]->setMouseData(Event->button);
            }

            /*if (Button != NULL)
            {
                Button->setMouseData(Event->button);
                CSurface::Draw(Surf_Display, Button->getSurface(), 100, 100);
            }*/

            switch(Event->button.button) {
                case SDL_BUTTON_LEFT: {

                    break;
                }
                case SDL_BUTTON_RIGHT: {

                    break;
                }
                case SDL_BUTTON_MIDDLE: {

                    break;
                }
            }
            break;
        }

        case SDL_QUIT: {
            Running = false;
            break;
        }

        case SDL_SYSWMEVENT: {
            //Ignore
            break;
        }

        case SDL_VIDEORESIZE: {

            break;
        }

        case SDL_VIDEOEXPOSE: {

            break;
        }

        default: {

            break;
        }
    }
}
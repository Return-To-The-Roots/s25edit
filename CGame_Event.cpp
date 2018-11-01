#include "CGame.h"
#include "CIO/CMenu.h"
#include "CIO/CWindow.h"
#include "CMap.h"
#include "CSurface.h"
#include "callbacks.h"
#include "globals.h"

void CGame::EventHandling(SDL_Event* Event)
{
    switch(Event->type)
    {
        case SDL_KEYDOWN:
        {
            // NOTE: we will now deliver the data to menus, windows, map etc., sometimes we have to break the switch and stop
            //      delivering earlier, for doing this we make use of a variable showing us the deliver status
            bool delivered = false;
            // now we walk through the windows and find out, if cursor is on one of these (ordered by priority)
            // we have to change the prioritys of the windows (for rendering), so find the highest one
            int highestPriority = 0;
            for(int j = 0; j < MAXWINDOWS; j++)
            {
                if(Windows[j] != NULL && Windows[j]->getPriority() > highestPriority)
                    highestPriority = Windows[j]->getPriority();
            }

            for(int i = 0; i < MAXWINDOWS; i++)
            {
                if(Windows[i] != NULL && !Windows[i]->isWaste() && Windows[i]->isMarked() && Windows[i]->getPriority() == highestPriority
                   && Windows[i]->hasActiveInputElement())
                {
                    Windows[i]->setKeyboardData(Event->key);
                    delivered = true;
                    break;
                }
            }
            // if (delivered)
            //    break;

            // deliver keyboard data to map if active
            if(!delivered)
            {
                if(MapObj != NULL && MapObj->isActive())
                {
                    MapObj->setKeyboardData(Event->key);
                    // data has been delivered to map, so no menu is in the foreground --> stop delivering
                    // break;
                }

                // deliver keyboard data to active menus
                for(int i = 0; i < MAXMENUS; i++)
                {
                    if(Menus[i] != NULL && Menus[i]->isActive() && !Menus[i]->isWaste())
                        Menus[i]->setKeyboardData(Event->key);
                }
            }

            switch(Event->key.keysym.sym)
            {
                case SDLK_F2:
                    if(fullscreen)
                        fullscreen = false;
                    else
                        fullscreen = true;
                    break;

#ifdef _ADMINMODE
                case SDLK_F3: // if CTRL and ALT are pressed
                    // if (SDL_GetModState() == (KMOD_LCTRL | KMOD_LALT))
                    callback::debugger(INITIALIZING_CALL);
                    break;
                case SDLK_F4: // if CTRL and ALT are pressed
                    // if (SDL_GetModState() == (KMOD_LCTRL | KMOD_LALT))
                    callback::viewer(INITIALIZING_CALL);
                    break;
#endif
                // F5 - F7 is ZOOM, F5 = zoom in, F6 = normal view, F7 = zoom out
                case SDLK_F5:
                    if(TRIANGLE_INCREASE < 10 && MapObj->getMap())
                    {
                        callback::PleaseWait(INITIALIZING_CALL);
                        TRIANGLE_HEIGHT += 5;
                        TRIANGLE_WIDTH += 11;
                        TRIANGLE_INCREASE += 1;
                        bobMAP* myMap = MapObj->getMap();
                        myMap->updateVertexCoords();
                        CSurface::get_nodeVectors(myMap);
                        callback::PleaseWait(WINDOW_QUIT_MESSAGE);
                    }
                    break;
                case SDLK_F6:
                {
                    if(MapObj->getMap())
                    {
                        callback::PleaseWait(INITIALIZING_CALL);
                        TRIANGLE_HEIGHT = 28;
                        TRIANGLE_WIDTH = 56;
                        TRIANGLE_INCREASE = 5;
                        bobMAP* myMap = MapObj->getMap();
                        myMap->updateVertexCoords();
                        CSurface::get_nodeVectors(myMap);
                        callback::PleaseWait(WINDOW_QUIT_MESSAGE);
                    }
                }
                break;
                case SDLK_F7:
                    if(TRIANGLE_INCREASE > 1 && MapObj->getMap())
                    {
                        callback::PleaseWait(INITIALIZING_CALL);
                        TRIANGLE_HEIGHT -= 5;
                        TRIANGLE_WIDTH -= 11;
                        TRIANGLE_INCREASE -= 1;
                        bobMAP* myMap = MapObj->getMap();
                        myMap->updateVertexCoords();
                        CSurface::get_nodeVectors(myMap);
                        callback::PleaseWait(WINDOW_QUIT_MESSAGE);
                    }
                    break;

                default: break;
            }

            break;
        }

        case SDL_KEYUP:
        {
            // deliver keyboard data to map
            if(MapObj != NULL)
                MapObj->setKeyboardData(Event->key);

            break;
        }

        case SDL_MOUSEMOTION:
        {
            // setup mouse cursor data
            if(MapObj != NULL && MapObj->isActive())
            {
                if((Event->motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT)) == 0)
                {
                    Cursor.x = Event->motion.x;
                    Cursor.y = Event->motion.y;
                }
            } else
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
                                if ( (Event->motion.x >= Windows[i]->getX()) && (Event->motion.x < Windows[i]->getX() + Windows[i]->getW())
               && (Event->motion.y >= Windows[i]->getY()) && (Event->motion.y < Windows[i]->getY() + Windows[i]->getH()) ) delivered = true;
                            }
                        }
                        if (delivered)
                            break;
            */

            // NOTE: we will now deliver the data to menus, windows, map etc., sometimes we have to break the switch and stop
            //      delivering earlier, for doing this we make use of a variable showing us the deliver status
            bool delivered = false;
            // now we walk through the windows and find out, if cursor is on one of these (ordered by priority)
            // we have to change the prioritys of the windows (for rendering), so find the highest one
            int highestPriority = 0;
            for(int j = 0; j < MAXWINDOWS; j++)
            {
                if(Windows[j] != NULL && Windows[j]->getPriority() > highestPriority)
                    highestPriority = Windows[j]->getPriority();
            }

            for(int actualPriority = highestPriority; actualPriority >= 0; actualPriority--)
            {
                for(int i = 0; i < MAXWINDOWS; i++)
                {
                    if(Windows[i] != NULL && !Windows[i]->isWaste() && Windows[i]->getPriority() == actualPriority)
                    {
                        // is the cursor INSIDE the window or does the user move or resize the window?
                        if(((Event->motion.x >= Windows[i]->getX()) && (Event->motion.x < Windows[i]->getX() + Windows[i]->getW())
                            && (Event->motion.y >= Windows[i]->getY()) && (Event->motion.y < Windows[i]->getY() + Windows[i]->getH()))
                           || Windows[i]->isMoving() || Windows[i]->isResizing())
                        {
                            // Windows[i]->setActive();
                            // Windows[i]->setPriority(highestPriority+1);
                            Windows[i]->setMouseData(Event->motion);
                            delivered = true;
                            break;
                        }
                    }
                }
                if(delivered)
                    break;
            }
            // if mouse data has been delivered, stop delivering anymore
            if(delivered)
                break;

            // deliver mouse motion data to map if active
            if(MapObj != NULL && MapObj->isActive())
            {
                MapObj->setMouseData(Event->motion);
                // data has been delivered to map, so no menu is in the foreground --> stop delivering
                break;
            }

            // deliver mouse motion data to active menus
            for(int i = 0; i < MAXMENUS; i++)
            {
                if(Menus[i] != NULL && Menus[i]->isActive() && !Menus[i]->isWaste())
                {
                    Menus[i]->setMouseData(Event->motion);
                    break;
                }
            }

            break;
        }

        case SDL_MOUSEBUTTONDOWN:
        {
            // setup mouse cursor data
            Cursor.clicked = true;
            Cursor.button.left = false;
            Cursor.button.right = false;
            if(Event->button.button == SDL_BUTTON_LEFT)
                Cursor.button.left = true;
            else if(Event->button.button == SDL_BUTTON_RIGHT)
                Cursor.button.right = true;

            // clicking a mouse button will close the S2 loading screen if it is shown
            if(showLoadScreen)
            {
                showLoadScreen = false;
                // prevent pressing another object "behind" the loading screen
                break;
            }

            // NOTE: we will now deliver the data to menus, windows, map etc., sometimes we have to break the switch and stop
            //      delivering earlier, for doing this we make use of a variable showing us the deliver status
            bool delivered = false;
            // now we walk through the windows and find out, if cursor is on one of these (ordered by priority)
            // we have to change the prioritys of the windows (for rendering), so find the highest one
            int highestPriority = 0;
            for(int j = 0; j < MAXWINDOWS; j++)
            {
                if(Windows[j] != NULL && Windows[j]->getPriority() > highestPriority)
                    highestPriority = Windows[j]->getPriority();
            }

            for(int actualPriority = highestPriority; actualPriority >= 0; actualPriority--)
            {
                for(int i = 0; i < MAXWINDOWS; i++)
                {
                    if(Windows[i] != NULL && !Windows[i]->isWaste() && Windows[i]->getPriority() == actualPriority)
                    {
                        // is the cursor INSIDE the window?
                        if((Event->button.x >= Windows[i]->getX()) && (Event->button.x < Windows[i]->getX() + Windows[i]->getW())
                           && (Event->button.y >= Windows[i]->getY()) && (Event->button.y < Windows[i]->getY() + Windows[i]->getH()))
                        {
                            Windows[i]->setActive();
                            Windows[i]->setPriority(highestPriority + 1);
                            Windows[i]->setMouseData(Event->button);
                            delivered = true;
                            break;
                        } else if(Windows[i]->isActive())
                            Windows[i]->setInactive();
                    }
                }
                if(delivered)
                    break;
            }
            // if mouse data has been deliverd, stop delivering anymore
            if(delivered)
                break;

            // deliver mouse button data to map if active
            if(MapObj != NULL && MapObj->isActive())
            {
                MapObj->setMouseData(Event->button);
                // data has been delivered to map, so no menu is in the foreground --> stop delivering
                break;
            }

            // deliver mouse button data to active menus
            for(int i = 0; i < MAXMENUS; i++)
            {
                if(Menus[i] != NULL && Menus[i]->isActive() && !Menus[i]->isWaste())
                    Menus[i]->setMouseData(Event->button);
            }

            break;
        }

        case SDL_MOUSEBUTTONUP:
        {
            // setup mouse cursor data
            Cursor.clicked = false;

            // NOTE: we will now deliver the data to menus, windows, map etc., sometimes we have to break the switch and stop
            //      delivering earlier, for doing this we make use of a variable showing us the deliver status
            bool delivered = false;
            // now we walk through the windows and find out, if cursor is on one of these (ordered by priority)
            // we have to change the prioritys of the windows (for rendering), so find the highest one
            int highestPriority = 0;
            for(int j = 0; j < MAXWINDOWS; j++)
            {
                if(Windows[j] != NULL && Windows[j]->getPriority() > highestPriority)
                    highestPriority = Windows[j]->getPriority();
            }

            for(int actualPriority = highestPriority; actualPriority >= 0; actualPriority--)
            {
                for(int i = 0; i < MAXWINDOWS; i++)
                {
                    if(Windows[i] != NULL && !Windows[i]->isWaste() && Windows[i]->getPriority() == actualPriority)
                    {
                        // is the cursor INSIDE the window?
                        if((Event->button.x >= Windows[i]->getX()) && (Event->button.x < Windows[i]->getX() + Windows[i]->getW())
                           && (Event->button.y >= Windows[i]->getY()) && (Event->button.y < Windows[i]->getY() + Windows[i]->getH()))
                        {
                            // Windows[i]->setActive();
                            // Windows[i]->setPriority(highestPriority+1);
                            Windows[i]->setMouseData(Event->button);
                            delivered = true;
                            break;
                        }
                        // else if (Windows[i]->isActive())
                        // Windows[i]->setInactive();
                    }
                }
                if(delivered)
                    break;
            }
            // if mouse data has been deliverd, stop delivering anymore
            /// We can't stop here cause of problems with the map. If user has the left mouse button pressed and modifies the vertices,
            /// it will cause a problem if he walks over a window with pressed mouse button and releases it in the window.
            /// So the MapObj needs the "release-event" of the mouse button.
            // if (delivered)
            // break;

            // if still not delivered, keep delivering to secondary elements like menu or map

            // deliver mouse button data to map if active
            if(MapObj != NULL && MapObj->isActive())
            {
                MapObj->setMouseData(Event->button);
                // data has been delivered to map, so no menu is in the foreground --> stop delivering
                break;
            }

            /// now we do what we commented out a few lines before
            if(delivered)
                break;

            // deliver mouse button data to active menus
            for(int i = 0; i < MAXMENUS; i++)
            {
                if(Menus[i] != NULL && Menus[i]->isActive() && !Menus[i]->isWaste())
                    Menus[i]->setMouseData(Event->button);
            }
            break;
        }

        case SDL_QUIT: Running = false; break;

        default: break;
    }
}

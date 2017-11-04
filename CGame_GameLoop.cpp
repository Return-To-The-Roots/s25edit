#include "CGame.h"

void CGame::GameLoop()
{
    for(int i = 0; i < MAXCALLBACKS; i++)
    {
        if(Callbacks[i] != NULL)
            Callbacks[i](CALL_FROM_GAMELOOP);
    }
    for(int i = 0; i < MAXMENUS; i++)
    {
        if(Menus[i] != NULL && Menus[i]->isWaste())
            UnregisterMenu(Menus[i]);
    }
    for(int i = 0; i < MAXWINDOWS; i++)
    {
        if(Windows[i] != NULL && Windows[i]->isWaste())
            UnregisterWindow(Windows[i]);
    }
}

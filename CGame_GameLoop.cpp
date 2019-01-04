#include "CGame.h"
#include "CIO/CMenu.h"
#include "CIO/CWindow.h"

void CGame::GameLoop()
{
    for(int i = 0; i < MAXCALLBACKS; i++)
    {
        if(Callbacks[i] != nullptr)
            Callbacks[i](CALL_FROM_GAMELOOP);
    }
    for(int i = 0; i < MAXMENUS; i++)
    {
        if(Menus[i] != nullptr && Menus[i]->isWaste())
            UnregisterMenu(Menus[i]);
    }
    for(int i = 0; i < MAXWINDOWS; i++)
    {
        if(Windows[i] != nullptr && Windows[i]->isWaste())
            UnregisterWindow(Windows[i]);
    }
}

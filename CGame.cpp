#include "CGame.h"

CGame::CGame()
{
    GameResolutionX = 1024;
    GameResolutionY = 768;
    MenuResolutionX = 640;
    MenuResolutionY = 480;

#if defined (_ADMINMODE)
    fullscreen = false;
#elif defined(_VIEWERMODE)
    fullscreen = false;
#else
    fullscreen = true;
#endif

#ifdef _ADMINMODE
    FrameCounter = 0;
    RegisteredCallbacks = 0;
    RegisteredWindows = 0;
    RegisteredMenus = 0;
#endif
#ifdef _VIEWERMODE
    msWait = 40;
#else
    msWait = 0;
#endif
    Surf_Display = NULL;
	Running = true;
	//mouse cursor data
	Cursor.x = 0;
	Cursor.y = 0;
	Cursor.clicked = false;
	Cursor.button.left = false;
	Cursor.button.right = false;

#ifdef _VIEWERMODE
	index = 0;
#endif

	for (int i = 0; i < MAXMENUS; i++)
        Menus[i] = NULL;
    for (int i = 0; i < MAXWINDOWS; i++)
        Windows[i] = NULL;
    for (int i = 0; i < MAXCALLBACKS; i++)
        Callbacks[i] = NULL;
    MapObj = NULL;
}

CGame::~CGame()
{
}

int CGame::Execute()
{
    if (Init() == false)
		return -1;

    SDL_Event Event;

	while (Running)
	{
		while (SDL_PollEvent(&Event))
			EventHandling(&Event);

		GameLoop();
		Render();
	}

	Cleanup();

	return 0;
}

bool CGame::RegisterMenu(CMenu *Menu)
{
    bool success = false;

    if (Menu == NULL)
        return success;
    for (int i = 0; i < MAXMENUS; i++)
    {
        if (!success && Menus[i] == NULL)
        {
            Menus[i] = Menu;
            Menus[i]->setActive();
            success = true;
#ifdef _ADMINMODE
            RegisteredMenus++;
#endif
        }
        else if (Menus[i] != NULL)
            Menus[i]->setInactive();
    }
    return success;
}

bool CGame::UnregisterMenu(CMenu *Menu)
{
    if (Menu == NULL)
        return false;
    for (int i = 0; i < MAXMENUS; i++)
    {
        if (Menus[i] == Menu)
        {
            for (int j = i-1; j >= 0; j--)
            {
                if (Menus[j] != NULL)
                {
                    Menus[i-1]->setActive();
                    break;
                }
            }
            delete Menus[i];
            Menus[i] = NULL;
#ifdef _ADMINMODE
            RegisteredMenus--;
#endif
            return true;
        }
    }
    return false;
}

bool CGame::RegisterWindow(CWindow *Window)
{
    bool success = false;
    int highestPriority = 0;

    //first find the highest priority
    for (int i = 0; i < MAXWINDOWS; i++)
    {
        if (Windows[i] != NULL && Windows[i]->getPriority() > highestPriority)
            highestPriority = Windows[i]->getPriority();
    }

    if (Window == NULL)
        return success;
    for (int i = 0; i < MAXWINDOWS; i++)
    {
        if (!success && Windows[i] == NULL)
        {
            Windows[i] = Window;
            Windows[i]->setActive();
            Windows[i]->setPriority(highestPriority+1);
            success = true;
#ifdef _ADMINMODE
            RegisteredWindows++;
#endif
        }
        else if (Windows[i] != NULL)
            Windows[i]->setInactive();
    }
    return success;
}

bool CGame::UnregisterWindow(CWindow *Window)
{
    if (Window == NULL)
        return false;
    for (int i = 0; i < MAXWINDOWS; i++)
    {
        if (Windows[i] == Window)
        {
            for (int j = i-1; j >= 0; j--)
            {
                if (Windows[j] != NULL)
                {
                    Windows[j]->setActive();
                    break;
                }
            }
            delete Windows[i];
            Windows[i] = NULL;
#ifdef _ADMINMODE
            RegisteredWindows--;
#endif
            return true;
        }
    }
    return false;
}

bool CGame::RegisterCallback(void (*callback)(int))
{
    if (callback == NULL)
        return false;
    for (int i = 0; i < MAXCALLBACKS; i++)
    {
        if (Callbacks[i] == NULL)
        {
            Callbacks[i] = callback;
#ifdef _ADMINMODE
            RegisteredCallbacks++;
#endif
            return true;
        }
    }
    return false;
}

bool CGame::UnregisterCallback(void (*callback)(int))
{
    if (callback == NULL)
        return false;
    for (int i = 0; i < MAXCALLBACKS; i++)
    {
        if (Callbacks[i] == callback)
        {
            Callbacks[i] = NULL;
#ifdef _ADMINMODE
            RegisteredCallbacks--;
#endif
            return true;
        }
    }
    return false;
}

void CGame::delMapObj(void)
{
    delete MapObj;
    MapObj = NULL;
}

int main(int argc, char* argv[])
{
    global::s2 = new CGame;

    return global::s2->Execute();
}

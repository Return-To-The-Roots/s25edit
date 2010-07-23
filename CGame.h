#ifndef _CGAME_H
    #define _CGAME_H

#include "includes.h"

class CGame
{
    friend class CDebug;

    public:
        int GameResolutionX;
        int GameResolutionY;
        int MenuResolutionX;
        int MenuResolutionY;
        bool fullscreen;

        bool Running;
        bool showLoadScreen;
        SDL_Surface *Surf_Display;

	private:
#ifdef _ADMINMODE
        //some debugging variables
        unsigned long int FrameCounter;
        int RegisteredCallbacks;
        int RegisteredWindows;
        int RegisteredMenus;
#endif
        //milliseconds for SDL_Delay()
        Uint32 msWait;
        //structure for mouse cursor
        struct
        {
            Uint16 x, y;
            bool clicked;
            struct
            {
                bool left;
                bool right;
            } button;
        } Cursor;

        //Object for Menu Screens
        CMenu *Menus[MAXMENUS];
        //Object for Windows
        CWindow *Windows[MAXWINDOWS];
        //Object for Callbacks
        void (*Callbacks[MAXCALLBACKS])(int);
        //Object for the Map
        CMap *Map;

#ifdef _VIEWERMODE
		//counter for surfing through pics
		int index;
#endif

    public:

        CGame();
        ~CGame();

        int Execute();

		bool Init();

		void EventHandling(SDL_Event *Event);

		void Exit();

		void GameLoop();

		void Render();

		void Cleanup();

		bool RegisterMenu(CMenu *Menu);
		bool UnregisterMenu(CMenu *Menu);
		bool RegisterWindow(CWindow *Window);
		bool UnregisterWindow(CWindow *Window);
		bool RegisterCallback(void (*callback)(int));
		bool UnregisterCallback(void (*callback)(int));
		void setMap(CMap *Map) { this->Map = Map; };
		CMap *getMap(void) { return Map; };
		void delMap(void);
};

#endif
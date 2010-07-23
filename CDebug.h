#ifndef _CDEBUG_H
    #define _CDEBUG_H

#include "includes.h"

class CDebug
{
    private:
        //callback fuction that is constructing the Debugger-Object
        void (*dbgCallback)(int);
        //debugger window
        CWindow *dbgWnd;
        //text for FrameCounter
        CFont *FrameCounterText;
        //text for Frames per Second
        CFont *FramesPerSecText;
        //text for msWait (milliseconds to wait --> SDL_Delay())
        CFont *msWaitText;
        //text for Registered Menus
        CFont *RegisteredMenusText;
        //text for Registered Windows
        CFont *RegisteredWindowsText;
        //text for Registered Callbacks
        CFont *RegisteredCallbacksText;
        //text for mouse cursor data
        CFont *MouseText;
        //text for map name
        CFont *MapNameText;
        //text for map width and height
        CFont *MapSizeText;
        //text for map author
        CFont *MapAuthorText;
        //text for map type
        CFont *MapTypeText;
        //text for map players
        CFont *MapPlayerText;
        //text for position of cursor (position means the number of the triangle/vertex)
        CFont *VertexText;
        //text for data at the vertex the cursor is on
        CFont *VertexDataText;
        //text for vector at the vertex the cursor is on
        CFont *VertexVectorText;
        //text for vector at the triangle below the vertex the cursor is on
        CFont *FlatVectorText;
        //button for shading model
        CButton *ShadingButton;
        //some puffers to write texts with sprintf()
        char puffer1[100];
        //char puffer2[100];
        //char puffer3[100];
        //fontsize for debugging window (remember: only 9, 11 or 14)
        int fontsize;

        //enumeration for messages sent to the debugger
        enum
        {
            WNDQUIT = 1,    //debugger window closed
            INCREMENT_MSWAIT,
            DECREMENT_MSWAIT,
            SETZERO_MSWAIT,
            CHANGE_SHADING
        };

    public:
        //Constructor, Destructor
        CDebug(void dbgCallback(int), int quitParam);
        ~CDebug();
        //Methods
        void sendParam(int Param);
        void actualizeData(void);
};

#endif
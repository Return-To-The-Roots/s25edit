#ifndef _CDEBUG_H
#define _CDEBUG_H

#include "defines.h"

class CFont;
class CWindow;
class CMap;
struct bobMAP;

class CDebug
{
private:
    // callback fuction that is constructing the Debugger-Object
    void (*dbgCallback_)(int);
    // debugger window
    CWindow* dbgWnd;
    // text for FrameCounter
    CFont* FrameCounterText;
    // text for Frames per Second
    CFont* FramesPerSecText;
    // text for msWait (milliseconds to wait --> SDL_Delay())
    CFont* msWaitText;
    // text for Registered Menus
    CFont* RegisteredMenusText;
    // text for Registered Windows
    CFont* RegisteredWindowsText;
    // text for Registered Callbacks
    CFont* RegisteredCallbacksText;
    CFont* DisplayRectText;
    // text for mouse cursor data
    CFont* MouseText;
    // text for map name
    CFont* MapNameText;
    // text for map width and height
    CFont* MapSizeText;
    // text for map author
    CFont* MapAuthorText;
    // text for map type
    CFont* MapTypeText;
    // text for map players
    CFont* MapPlayerText;
    // text for position of cursor (position means the number of the triangle/vertex)
    CFont* VertexText;
    // text for data at the vertex the cursor is on
    CFont* VertexDataText;
    // text for vector at the vertex the cursor is on
    CFont* VertexVectorText;
    // text for vector at the triangle below the vertex the cursor is on
    CFont* FlatVectorText;
    // texts for map data at the vertex the cursor is on
    CFont* rsuTextureText;
    CFont* usdTextureText;
    CFont* roadText;
    CFont* objectTypeText;
    CFont* objectInfoText;
    CFont* animalText;
    CFont* unknown1Text;
    CFont* buildText;
    CFont* unknown2Text;
    CFont* unknown3Text;
    CFont* resourceText;
    CFont* shadingText;
    CFont* unknown5Text;
    CFont* editorModeText;
    // fontsize for debugging window (remember: only 9, 11 or 14)
    FontSize fontsize;
    // temporary pointer to Map-Object
    CMap* MapObj;
    // temporary pointer to map
    bobMAP* map;

    // enumeration for messages sent to the debugger
    enum
    {
        WNDQUIT = 1, // debugger window closed
        INCREMENT_MSWAIT,
        DECREMENT_MSWAIT,
        SETZERO_MSWAIT
    };

public:
    // Constructor, Destructor
    CDebug(void dbgCallback_(int), int quitParam);
    ~CDebug();
    // Methods
    void sendParam(int Param);
    void actualizeData();
};

#endif

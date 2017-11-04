// NOTE: negative callbackParams are reserved: -1 = callback is called first time, -2 = used by gameloop for registered
// callbacks (callbacks that will additionally execute WITHIN the gameloop)

// NOTE: don't forget that if the map quits (Param: MAP_QUIT), there are many windows that have to be closed.
// This happens for example if a new Map will be loaded or user goes to main menu. So if you add a new window, don't forget
// to add it to this "close lists" if it's necessary (also in the file CMap.cpp, function setMouseData(Button)).

#ifndef _CALLBACKS_H
#define _CALLBACKS_H

#include "includes.h"

namespace callback {
// PleaseWait creates a small window (not moveable, not resizeable, not minimizable, not closeable) with the String "Please wait..."
void PleaseWait(int Param);
void mainmenu(int Param);
void submenuOptions(int Param);
void MinimapMenu(int Param);
#ifdef _EDITORMODE
void EditorHelpMenu(int Param);
void EditorMainMenu(int Param);
void EditorLoadMenu(int Param);
void EditorSaveMenu(int Param);
void EditorQuitMenu(int Param);
void EditorTextureMenu(int Param);
void EditorTreeMenu(int Param);
void EditorResourceMenu(int Param);
void EditorLandscapeMenu(int Param);
void EditorAnimalMenu(int Param);
void EditorPlayerMenu(int Param);
void EditorCreateMenu(int Param);
void EditorCursorMenu(int Param);
#else
void GameMenu(int Param);
#endif

#ifdef _ADMINMODE
void debugger(int Param);
void viewer(int Param);
void submenu1(int Param);
#endif
} // namespace callback

#endif

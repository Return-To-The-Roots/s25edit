#ifndef _CALLBACKS_H
    #define _CALLBACKS_H

#include "includes.h"

namespace callback
{
    void mainmenu(int Param);
    void submenuOptions(int Param);
#ifdef _EDITORMODE
    void EditorMainMenu(int Param);
    void EditorQuitMenu(int Param);
    void EditorTextureMenu(int Param);
#else
    void GameMenu(int Param);
#endif

#ifdef _ADMINMODE
    void debugger(int Param);
    void submenu1(int Param);
#endif
}

#endif

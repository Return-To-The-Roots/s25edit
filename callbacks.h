#ifndef _CALLBACKS_H
    #define _CALLBACKS_H

#include "includes.h"

namespace callback
{
    void mainmenu(int Param);
    void submenuOptions(int Param);
    void GameMenu(int Param);
#ifdef _ADMINMODE
    void debugger(int Param);
    void submenu1(int Param);
#endif
}

#endif

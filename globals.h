#ifndef _GLOBALS_H
    #define _GLOBALS_H

#include "includes.h"

namespace global
{
    //array for all pictures
    extern bobBMP bmpArray[];
    //array for all shadows
    extern bobSHADOW shadowArray[];
    //array for all palettes
    extern bobPAL palArray[];
    //the game object
    extern CGame *s2;
}

extern unsigned char TRIANGLE_HEIGHT;
extern unsigned char TRIANGLE_WIDTH;
extern unsigned char TRIANGLE_INCREASE;

#endif

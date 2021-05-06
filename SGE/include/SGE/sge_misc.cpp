// Copyright (C) 1999 - 2003 Anders Lindström
// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 *	SDL Graphics Extension
 *	Misc functions
 *
 *	Started 990819
 */

#include "sge_misc.h"
#include <cstdlib>
#include <ctime>

Uint32 delay_res = 10;

//==================================================================================
// Returns a random integer between min and max
//==================================================================================
int sge_Random(int min, int max)
{
    return min + (rand() % (max - min + 1));
}

//==================================================================================
// Seed the random number generator with a number from the system clock.
// Should be called once before the first use of sge_Random.
//==================================================================================
void sge_Randomize()
{
    srand(static_cast<unsigned>(time(nullptr)));
}

//==================================================================================
// Test the resolution of SDL_Delay()
//==================================================================================
Uint32 sge_CalibrateDelay()
{
    SDL_Delay(10);
    delay_res = SDL_GetTicks();
    SDL_Delay(1);
    delay_res = SDL_GetTicks() - delay_res;

    return delay_res;
}

//==================================================================================
// Get the resolution of SDL_Delay()
//==================================================================================
Uint32 sge_DelayRes()
{
    return delay_res;
}

//==================================================================================
// Delay 'ticks' ms.
// Tries to burn time in SDL_Delay() if possible
// Returns the exact delay time
//==================================================================================
Uint32 sge_Delay(Uint32 ticks)
{
    Uint32 start = SDL_GetTicks();
    auto time_left = (Sint32)ticks;
    Uint32 tmp;

    if(ticks >= delay_res)
    {
        tmp = ticks - (ticks % delay_res);
        SDL_Delay(tmp);
        time_left = (Sint32)(ticks - (SDL_GetTicks() - start)); // Possible error for large ticks... nah
    }

    while(time_left > 0)
    {
        time_left = (Sint32)(ticks - (SDL_GetTicks() - start));
    }

    return SDL_GetTicks() - start;
}

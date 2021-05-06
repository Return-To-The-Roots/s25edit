// Copyright (C) 1999 - 2003 Anders Lindström
// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 *	SDL Graphics Extension
 *	Misc functions (header)
 *
 *	Started 990819
 */

#pragma once

#include "sge_internal.h"

#ifdef _SGE_C
extern "C"
{
#endif
    DECLSPEC int sge_Random(int min, int max);
    DECLSPEC void sge_Randomize();

    DECLSPEC Uint32 sge_CalibrateDelay();
    DECLSPEC Uint32 sge_DelayRes();
    DECLSPEC Uint32 sge_Delay(Uint32 ticks);
#ifdef _SGE_C
}
#endif

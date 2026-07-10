// Copyright (C) 2009 - 2025 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <SDL.h>
#include <memory>

struct SDLWindowDestroyer
{
    void operator()(SDL_Window* p) const { SDL_DestroyWindow(p); }
};
using SdlWindow = std::unique_ptr<SDL_Window, SDLWindowDestroyer>;

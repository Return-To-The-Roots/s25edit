// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "gameData/WorldDescription.h"
#include <boost/filesystem/path.hpp>
#include <SDL.h>
#include <vector>

class CGame;
struct bobBMP;
struct bobSHADOW;
struct bobPAL;

namespace global {
// array for all pictures
extern std::vector<bobBMP> bmpArray;
// array for all shadows
extern std::vector<bobSHADOW> shadowArray;
// array for all palettes
extern std::vector<bobPAL> palArray;
// the game object
extern CGame* s2;
// Path to game data (must not be empty!)
extern boost::filesystem::path gameDataFilePath;
// Path where maps will be stored (must not be empty!)
extern boost::filesystem::path userMapsPath;
extern WorldDescription worldDesc;
} // namespace global

extern unsigned char TRIANGLE_HEIGHT;
extern unsigned char TRIANGLE_WIDTH;
extern unsigned char TRIANGLE_INCREASE;

extern Uint8 gouData[3][256][256];

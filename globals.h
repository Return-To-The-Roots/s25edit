// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "gameData/WorldDescription.h"
#include <boost/filesystem/path.hpp>
#include <SDL.h>
#include <array>
#include <vector>

class CGame;
struct bobBMP;
struct bobSHADOW;
struct bobPAL;
struct PaletteAnimation;

namespace global {
// array for all pictures
extern std::vector<bobBMP> bmpArray;
// array for all shadows
extern std::vector<bobSHADOW> shadowArray;
// array for all palettes
extern std::vector<bobPAL> palArray;
// Palette animations per tileset, indexed by MapType (0=Greenland, 1=Wasteland, 2=Winterland).
// Each entry is a list of CRNG chunk animations applied to the whole tileset palette.
extern std::array<std::vector<PaletteAnimation>, 3> paletteAnimations;
// the game object
extern CGame* s2;
// Path to game data (must not be empty!)
extern boost::filesystem::path gameDataFilePath;
// Path where maps will be stored (must not be empty!)
extern boost::filesystem::path userMapsPath;
extern WorldDescription worldDesc;
} // namespace global

extern unsigned char triangleHeight;
extern unsigned char triangleWidth;
extern unsigned char triangleIncrease;

// Default / reset values for zoom
constexpr unsigned char TRIANGLE_HEIGHT_DEFAULT = 28;
constexpr unsigned char TRIANGLE_WIDTH_DEFAULT = 56;
constexpr unsigned char TRIANGLE_INCREASE_DEFAULT = 5;

// Zoom step increments/decrements
constexpr unsigned char ZOOM_STEP_HEIGHT = 5;
constexpr unsigned char ZOOM_STEP_WIDTH = 11;
constexpr unsigned char ZOOM_STEP_INCREASE = 1;

// Zoom boundaries (based on triangleIncrease)
constexpr unsigned char ZOOM_INCREASE_MAX = 9;
constexpr unsigned char ZOOM_INCREASE_MIN = 1;

extern Uint8 gouData[3][256][256];

// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "gameData/WorldDescription.h"
#include <boost/filesystem/path.hpp>
#include <array>

class CGame;

namespace global {

// the game object
extern CGame* s2;
// Path to game data (must not be empty!)
extern boost::filesystem::path gameDataFilePath;
// Path where maps will be stored (must not be empty!)
extern boost::filesystem::path userMapsPath;
extern WorldDescription worldDesc;
} // namespace global

#include "gameData/MapConsts.h"
// Triangle/zoom constants from s25main: TR_W=56, TR_H=28, HEIGHT_FACTOR=5

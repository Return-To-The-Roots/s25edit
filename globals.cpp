// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "globals.h"

// the game object
CGame* global::s2;

boost::filesystem::path global::gameDataFilePath(".");
boost::filesystem::path global::userMapsPath;
WorldDescription global::worldDesc;

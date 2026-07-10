// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2024 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"
#include <boost/filesystem/path.hpp>
#include <cstdio>
#include <string>

struct bobMAP;

class CFile
{
private:
    static bobMAP* read_wld(FILE* fp);
    static bobMAP* read_swd(FILE* fp);
    static bool save_wld(FILE* fp, void* data);
    static bool save_swd(FILE* fp, void* data);

public:
    static void* open_file(const boost::filesystem::path& filepath, char filetype);
    static bool save_file(const boost::filesystem::path& filepath, char filetype, void* data);
};

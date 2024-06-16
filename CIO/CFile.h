// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2024 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// handling of the files and data types is mostly based on the file specification from the 'Return to the Roots'-Team

#pragma once

#include "defines.h"
#include <boost/filesystem/path.hpp>
#include <cstdio>
#include <string>

struct bobBMP;
struct bobSHADOW;
struct bobPAL;
struct bobMAP;

class CFile
{
private:
    static bool loadPAL;
    static bobBMP* bmpArray;
    static bobSHADOW* shadowArray;
    static bobPAL* palArray;
    static bobPAL* palActual; // surfaces for new pictures will use this palette
public:
    // Access Methods
    static void set_palActual(bobPAL* Actual) { palActual = Actual; }
    static bobPAL* get_palArray() { return palArray; }
    static void set_palArray(bobPAL* Array) { palArray = Array; }
    static void set_bmpArray(bobBMP* new_bmpArray) { bmpArray = new_bmpArray; }

private:
    // Methods
    static bool read_lst(FILE* fp);
    static bool read_bob(FILE* fp); // not implemented yet
    static bool open_idx(const boost::filesystem::path& filepath);
    static bool read_bbm(FILE* fp);
    static bool read_lbm(FILE* fp, const boost::filesystem::path& filepath);
    static bool read_gou(FILE* fp);
    static bobMAP* read_wld(FILE* fp);
    static bobMAP* read_swd(FILE* fp);
    static bool save_lst(FILE* fp, void* data); // not implemented yet
    static bool save_bob(FILE* fp, void* data); // not implemented yet
    static bool save_idx(FILE* fp, void* data); // not implemented yet
    static bool save_bbm(FILE* fp, void* data); // not implemented yet
    static bool save_lbm(FILE* fp, void* data); // not implemented yet
    static bool save_wld(FILE* fp, void* data);
    static bool save_swd(FILE* fp, void* data);
    static bool read_bob01(FILE* fp); // not implemented yet
    static bool read_bob02(FILE* fp);
    static bool read_bob03(FILE* fp);
    static bool read_bob04(FILE* fp, int player_color = PLAYER_BLUE);
    static bool read_bob05(FILE* fp);
    static bool read_bob07(FILE* fp);
    static bool read_bob14(FILE* fp);

public:
    static void init();
    static void* open_file(const boost::filesystem::path& filepath, char filetype, bool only_loadPAL = false);
    static bool save_file(const boost::filesystem::path& filepath, char filetype, void* data);
};

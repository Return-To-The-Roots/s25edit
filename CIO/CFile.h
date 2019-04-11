// handling of the files and data types is mostly based on the file specification from the 'Return to the Roots'-Team

#ifndef _CFILE_H
#define _CFILE_H

#include "../defines.h"
#include <cstdio>
#include <string>

struct bobBMP;
struct bobSHADOW;
struct bobPAL;
struct bobMAP;

class CFile
{
private:
    static FILE* fp;
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
    static bool open_lst();
    static bool open_bob(); // not implemented yet
    static bool open_idx(const std::string& filename);
    static bool open_bbm();
    static bool open_lbm(const std::string& filename);
    static bool open_gou();
    static bobMAP* open_wld();
    static bobMAP* open_swd();
    static bool save_lst(void* data);                              // not implemented yet
    static bool save_bob(void* data);                              // not implemented yet
    static bool save_idx(void* data, const std::string& filename); // not implemented yet
    static bool save_bbm(void* data);                              // not implemented yet
    static bool save_lbm(void* data);                              // not implemented yet
    static bool save_wld(void* data);
    static bool save_swd(void* data);
    static bool read_bob01(); // not implemented yet
    static bool read_bob02();
    static bool read_bob03();
    static bool read_bob04(int player_color = PLAYER_BLUE);
    static bool read_bob05();
    static bool read_bob07();
    static bool read_bob14();

public:
    static void init();
    static void* open_file(const std::string& filename, char filetype, bool only_loadPAL = false);
    static bool save_file(const std::string& filename, char filetype, void* data);
};

#endif

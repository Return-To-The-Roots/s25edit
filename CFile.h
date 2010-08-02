//handling of the files and data types is mostly based on the file specification from the 'Return to the Roots'-Team

#ifndef _CFILE_H
    #define _CFILE_H

#include "includes.h"

class CFile
{
    private:
        static FILE *fp;
        static bool loadPAL;
        static bobBMP *bmpArray;
        static bobSHADOW *shadowArray;
        static bobPAL *palArray;
        static bobPAL *palActual;   //surfaces for new pictures will use this palette
    public:
        //Access Methods
        static void set_palActual(bobPAL *Actual) { palActual = Actual; };
        static bobPAL* get_palArray(void) { return palArray; };
        static void set_bmpArray(bobBMP *new_bmpArray) { bmpArray = new_bmpArray; };

    private:
        //Methods
        static bool open_lst(void);
        static bool open_bob(void);                         //not implemented yet
        static bool open_idx(char *filename);
        static bool open_bbm(void);
        static bool open_lbm(char *filename);
        static bobMAP* open_wld(void);
        static bobMAP* open_swd(void);
        static bool save_lst(void *data);                   //not implemented yet
        static bool save_bob(void *data);                   //not implemented yet
        static bool save_idx(void *data, char *filename);   //not implemented yet
        static bool save_bbm(void *data);                   //not implemented yet
        static bool save_lbm(void *data);                   //not implemented yet
        static bool save_wld(void *data);
        static bool save_swd(void *data);
        static bool read_bob01(void);                       //not implemented yet
        static bool read_bob02(void);
        static bool read_bob03(void);
        static bool read_bob04(void);
        static bool read_bob05(void);
        static bool read_bob07(void);
        static bool read_bob14(void);
        //convert between big- and little-endian
        static inline void endian_swap(Uint16& x);
        static inline void endian_swap(Uint32& x);

    public:
        CFile();
        ~CFile();
        static void* open_file(char *filename, char filetype, bool only_loadPAL = false);
        static void* open_file(const char *filename, char filetype, bool only_loadPAL = false);
        static bool save_file(char *filename, char filetype, void *data);
        static bool save_file(const char *filename, char filetype, void *data);
};








#endif

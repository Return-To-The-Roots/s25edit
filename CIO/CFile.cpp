// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2024 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CFile.h"
#include "../CSurface.h"
#include "../globals.h"
#include "libendian/libendian.h"
#include "s25util/file_handle.h"
#include <boost/endian/conversion.hpp>
#include <boost/nowide/cstdio.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>

//-V:fseek:303
//-V:ftell:303

bobBMP* CFile::bmpArray = nullptr;
bobSHADOW* CFile::shadowArray = nullptr;
bobPAL* CFile::palArray = nullptr;
bobPAL* CFile::palActual = nullptr;
bool CFile::loadPAL = false;

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
#define CHECK_READ(readCmd) \
    if(!(readCmd))          \
    throw std::runtime_error("Read failed at line " LINE_STRING)

void CFile::init()
{
    bmpArray = &global::bmpArray[0];
    shadowArray = &global::shadowArray[0];
    palArray = &global::palArray[0];
    palActual = &global::palArray[0];
    loadPAL = false;
}

void* CFile::open_file(const boost::filesystem::path& filepath, char filetype, bool only_loadPAL)
{
    void* return_value = nullptr;

    if(filepath.empty() || !bmpArray || !shadowArray || !palArray || !palActual)
        return nullptr;
    s25util::file_handle fh(boost::nowide::fopen(filepath.string().c_str(), "rb"));
    if(!fh)
        return nullptr;

    if(only_loadPAL)
        loadPAL = true;

    try
    {
        switch(filetype)
        {
            case LST:
                if(read_lst(*fh))
                    return_value = (void*)-1;

                break;

            case BOB:
                if(read_bob(*fh))
                    return_value = (void*)-1;

                break;

            case IDX:
                if(open_idx(filepath))
                    return_value = (void*)-1;

                break;

            case BBM:
                if(read_bbm(*fh))
                    return_value = (void*)-1;

                break;

            case LBM:
                if(read_lbm(*fh, filepath))
                    return_value = (void*)-1;

                break;

            case GOU:
                if(read_gou(*fh))
                    return_value = (void*)-1;

                break;

            case WLD: return_value = read_wld(*fh); break;

            case SWD: return_value = read_swd(*fh); break;

            default: // no valid data type
                break;
        }
    } catch(const std::exception& e)
    {
        std::cerr << "Error while reading " << filepath << ": " << e.what() << std::endl;
    }

    loadPAL = false;

    return return_value;
}

bool CFile::read_lst(FILE* fp)
{
    // type of entry (used or unused entry)
    Uint16 entrytype;
    // bobtype of the entry
    Uint16 bobtype;

    // skip: id (2x 1 Bytes) + count (1x 4 Bytes) = 6 Bytes
    fseek(fp, 6, SEEK_SET);

    // main loop for reading entries
    while(!feof(fp))
    {
        // entry type (2 Bytes) - unused (=0x0000) or used (=0x0001) -
        if(!libendian::le_read_us(&entrytype, fp))
        {
            if(feof(fp))
                break;
            CHECK_READ(false);
        }

        // if entry is unused, go back to 'while' --- and by the way: after the last entry there are always zeros in the
        // file, so the following case will happen till we have reached the end of the file and the 'while' will break -
        // PERFECT!
        if(entrytype == 0x0000)
            continue;

        // bobtype (2 Bytes)
        CHECK_READ(libendian::le_read_us(&bobtype, fp));

        switch(bobtype)
        {
            case BOBTYPE01:
                if(!read_bob01(fp))
                    return false;
                break;

            case BOBTYPE02:
                if(!read_bob02(fp))
                    return false;
                break;

            case BOBTYPE03:
                if(!read_bob03(fp))
                    return false;
                break;

            case BOBTYPE04:
                if(!read_bob04(fp, PLAYER_BLUE))
                    return false;
                break;

            case BOBTYPE05:
                if(!read_bob05(fp))
                    return false;
                break;

            case BOBTYPE07:
                if(!read_bob07(fp))
                    return false;
                break;

            case BOBTYPE14:
                if(!read_bob14(fp))
                    return false;
                break;

            default: // Something is wrong? Maybe the last entry was really the LAST, so we should not return false
                break;
        }
    }

    return true;
}

bool CFile::read_bob(FILE*)
{
    return false;
}

bool CFile::open_idx(const boost::filesystem::path& filepath)
{
    // starting adress of data in the corresponding '******.DAT'-File
    Uint32 offset;
    // bobtype of the entry
    Uint16 bobtype;
    // bobtype is repeated in the corresponding '******.DAT'-File, so we have to check this is correct
    Uint16 bobtype_check;

    // '******.IDX'-File
    const s25util::file_handle fh_idx(boost::nowide::fopen(filepath.string().c_str(), "rb"));
    // corresponding '******.DAT'-File
    auto filename_dat = filepath;
    filename_dat.replace_extension(".DAT");
    const s25util::file_handle fh_dat(boost::nowide::fopen(filename_dat.string().c_str(), "rb"));
    if(!fh_idx || !fh_dat)
        return false;

    // we are finished opening the files now we can handle the content

    // skip: unknown data (1x 4 Bytes) at the beginning of the file
    fseek(*fh_idx, 4, SEEK_SET);

    // main loop for reading entries
    while(!feof(*fh_idx) && !feof(*fh_dat))
    {
        // skip: name (1x 16 Bytes)
        fseek(*fh_idx, 16, SEEK_CUR);
        // offset (4 Bytes)
        if(!libendian::le_read_ui(&offset, *fh_idx))
        {
            if(feof(*fh_idx))
                break;
            CHECK_READ(false);
        }
        // skip unknown data (6x 1 Byte)
        fseek(*fh_idx, 6, SEEK_CUR);
        // bobtype (2 Bytes)
        CHECK_READ(libendian::le_read_us(&bobtype, *fh_idx));
        // set fp_dat to the position in 'offset'
        fseek(*fh_dat, offset, SEEK_SET);
        // read bobtype again, now from 'DAT'-File
        CHECK_READ(libendian::le_read_us(&bobtype_check, *fh_dat));
        // check if data in 'DAT'-File is the data that it should be (bobtypes are equal)
        if(bobtype != bobtype_check)
            return false;

        switch(bobtype)
        {
            case BOBTYPE01:
                if(!read_bob01(*fh_dat))
                    return false;
                break;

            case BOBTYPE02:
                if(!read_bob02(*fh_dat))
                    return false;
                break;

            case BOBTYPE03:
                if(!read_bob03(*fh_dat))
                    return false;
                break;

            case BOBTYPE04:
                if(!read_bob04(*fh_dat))
                    return false;
                break;

            case BOBTYPE05:
                if(!read_bob05(*fh_dat))
                    return false;
                break;

            case BOBTYPE07:
                if(!read_bob07(*fh_dat))
                    return false;
                break;

            case BOBTYPE14:
                if(!read_bob14(*fh_dat))
                    return false;
                break;

            default: // Something is wrong? Maybe the last entry was really the LAST, so we should not return false
                break;
        }
    }

    return true;
}

bool CFile::read_bbm(FILE* fp)
{
    // skip header (48 Bytes)
    fseek(fp, 48, SEEK_CUR);

    for(auto& color : palArray->colors)
    {
        CHECK_READ(libendian::read(&(color.r), 1, fp));
        CHECK_READ(libendian::read(&(color.g), 1, fp));
        CHECK_READ(libendian::read(&(color.b), 1, fp));
    }

    palArray++;

    return true;
}

bool CFile::read_lbm(FILE* fp, const boost::filesystem::path& filepath)
{
    // IMPORTANT NOTE: LBM (also ILBM) is the Interchange File Format (IFF) and the 4-byte-blocks are originally
    // organized as Big Endian, so a convertion is needed

    // identifier for the kind of data follows (FORM, BMHD, CMAP, BODY)
    std::array<char, 5> chunk_identifier;

    // length of data block
    Uint32 length;
    // color depth of the picture
    Uint16 color_depth;
    // is the picture rle-compressed?
    Uint8 compression_flag;
    // compression type
    Sint8 ctype;
    // array for palette colors
    std::array<SDL_Color, 256> colors;
    // color value for read pixel
    Uint8 color_value;

    // skip File-Identifier "FORM" (1x 4 Bytes) + unknown data (4x 1 Byte) + Header-Identifier "PBM " (1x 4 Bytes) = 12
    // Bytes
    fseek(fp, 12, SEEK_CUR);

    /* READ FIRST CHUNK "BMHD" */

    // chunk-identifier (4 Bytes)
    CHECK_READ(libendian::read(chunk_identifier.data(), 4, fp));
    chunk_identifier[4] = '\0';
    // should be "BMHD" at this time
    if(strcmp(chunk_identifier.data(), "BMHD") != 0)
        return false;
    // length of data block
    CHECK_READ(libendian::be_read_ui(&length, fp));
    // width of picture (2 Bytes)
    CHECK_READ(libendian::be_read_us(&(bmpArray->w), fp));
    // heigth of picture (2 Bytes)
    CHECK_READ(libendian::be_read_us(&(bmpArray->h), fp));
    // skip unknown data (4x 1 Bytes)
    fseek(fp, 4, SEEK_CUR);
    // color depth of the picture (1x 2 Bytes)
    CHECK_READ(libendian::be_read_us(&color_depth, fp));
    // compression_flag (1x 1 Bytes)
    CHECK_READ(libendian::read(&compression_flag, 1, fp));
    fseek(fp, 9, SEEK_CUR);
    // skip unknown data (length - 20 x 1 Byte)
    // fseek(fp, length-20, SEEK_CUR);

    /* READ SECOND CHUNK "CMAP" */

    // chunk-identifier (4 Bytes)
    // search for the "CMAP" and skip other chunk-types
    while(!feof(fp))
    {
        CHECK_READ(libendian::read(chunk_identifier.data(), 4, fp));

        if(strcmp(chunk_identifier.data(), "CMAP") == 0)
            break;
        else
        {
            Uint32 chunkLen;
            CHECK_READ(libendian::be_read_ui(&chunkLen, fp));
            if(chunkLen & 1)
                chunkLen++;
            fseek(fp, chunkLen, SEEK_CUR);
        }
    }
    if(feof(fp))
        return false;
    // length of data block
    CHECK_READ(libendian::be_read_ui(&length, fp));
    // must be 768 (RGB = 3 Byte x 256 Colors)
    if(length != 3u * 256u)
        return false;
    // palette
    for(auto& color : colors)
    {
        CHECK_READ(libendian::read(&color.r, 1, fp));
        CHECK_READ(libendian::read(&color.g, 1, fp));
        CHECK_READ(libendian::read(&color.b, 1, fp));
    }

    /* READ THIRD CHUNK "BODY" */

    // chunk-identifier (4 Bytes)
    // search for the "BODY" and skip other chunk-types
    while(!feof(fp))
    {
        CHECK_READ(libendian::read(chunk_identifier.data(), 4, fp));

        if(strcmp(chunk_identifier.data(), "BODY") == 0)
            break;
        else
        {
            Uint32 chunkLen;
            CHECK_READ(libendian::be_read_ui(&chunkLen, fp));
            if(chunkLen & 1)
                chunkLen++;
            fseek(fp, chunkLen, SEEK_CUR);
        }
    }
    if(feof(fp))
        return false;
    // length of data block
    CHECK_READ(libendian::be_read_ui(&length, fp));

    // now we are ready to read the picture lines and fill the surface, so lets create one
    if(!(bmpArray->surface = makePalSurface(bmpArray->w, bmpArray->h, colors)))
    {
        std::cerr << "Failed to create surface: " << SDL_GetError() << std::endl;
        return false;
    }

    if(compression_flag == 0)
    {
        // picture uncompressed
        // main loop for reading picture lines
        for(int y = 0; y < bmpArray->h; y++)
        {
            // loop for reading pixels of the actual picture line
            for(int x = 0; x < bmpArray->w; x++)
            {
                // read color value (1 Byte)
                CHECK_READ(libendian::read(&color_value, 1, fp));
                // draw
                CSurface::DrawPixel_Color(bmpArray->surface.get(), x, y, (Uint32)color_value);
            }
        }

    } else if(compression_flag == 1)
    {
        // picture compressed

        // main loop for reading picture lines
        for(int y = 0; y < bmpArray->h; y++)
        {
            // loop for reading pixels of the actual picture line
            //(cause of a kind of RLE-Compression we cannot read the pixels sequentially)
            //'x' will be incremented WITHIN the loop
            for(int x = 0; x < bmpArray->w;)
            {
                // read compression type
                CHECK_READ(libendian::read(&ctype, 1, fp));

                if(ctype >= 0)
                {
                    // following 'ctype + 1' pixels are uncompressed
                    for(int k = 0; k < ctype + 1; k++, x++)
                    {
                        // read color value (1 Byte)
                        CHECK_READ(libendian::read(&color_value, 1, fp));
                        // draw
                        CSurface::DrawPixel_Color(bmpArray->surface.get(), x, y, (Uint32)color_value);
                    }
                } else if(ctype >= -127)
                {
                    // draw the following byte '-ctype + 1' times to the surface
                    CHECK_READ(libendian::read(&color_value, 1, fp));

                    for(int k = 0; k < -ctype + 1; k++, x++)
                        CSurface::DrawPixel_Color(bmpArray->surface.get(), x, y, (Uint32)color_value);
                } else // if (ctype == -128)
                {
                    // ignore
                }
            }
        }
    } else
        return false;

    // if this is a texture file, we need a secondary 32-bit surface for SGE and we set a color key at both surfaces
    if(filepath.filename() == "TEX5.LBM" || filepath.filename() == "TEX6.LBM" || filepath.filename() == "TEX7.LBM"
       || filepath.filename() == "TEXTUR_0.LBM" || filepath.filename() == "TEXTUR_3.LBM")
    {
        SDL_SetColorKey(bmpArray->surface.get(), SDL_TRUE, SDL_MapRGB(bmpArray->surface->format, 0, 0, 0));

        bmpArray++;
        if((bmpArray->surface = makeRGBSurface((bmpArray - 1)->w, (bmpArray - 1)->h)))
        {
            SDL_SetColorKey(bmpArray->surface.get(), SDL_TRUE, SDL_MapRGB(bmpArray->surface->format, 0, 0, 0));
            CSurface::Draw(bmpArray->surface, (bmpArray - 1)->surface);
        } else
            bmpArray--;
    }

    // we are finished, the surface is filled
    // increment bmpArray for the next picture
    bmpArray++;

    return true;
}

bool CFile::read_gou(FILE* fp)
{
    static int internalArrayCtr = 0; // maximum is two cause there are only 3 GOUx.DAT-Files

    if(internalArrayCtr > 2)
        return false;

    CHECK_READ(libendian::read(gouData[internalArrayCtr][0], sizeof(gouData[internalArrayCtr]), fp));
    internalArrayCtr++;

    return true;
}

bobMAP* CFile::read_wld(FILE* fp)
{
    auto myMap = std::make_unique<bobMAP>();
    std::array<char, 20> tmpNameAuthor;

    fseek(fp, 10, SEEK_SET);
    tmpNameAuthor.fill('\0');
    CHECK_READ(libendian::read(tmpNameAuthor, fp));
    myMap->setName(tmpNameAuthor.data());
    CHECK_READ(libendian::le_read_us(&myMap->width_old, fp));
    CHECK_READ(libendian::le_read_us(&myMap->height_old, fp));
    uint8_t mapType;
    CHECK_READ(libendian::read(&mapType, 1, fp));
    myMap->type = MapType(mapType);
    CHECK_READ(libendian::read(&myMap->player, 1, fp));
    tmpNameAuthor.fill('\0');
    CHECK_READ(libendian::read(tmpNameAuthor, fp));
    myMap->setAuthor(tmpNameAuthor.data());
    for(unsigned short& i : myMap->HQx)
        CHECK_READ(libendian::le_read_us(&i, fp));
    for(unsigned short& i : myMap->HQy)
        CHECK_READ(libendian::le_read_us(&i, fp));

    // go to big map header and read it
    fseek(fp, 92, SEEK_SET);
    for(auto& i : myMap->header)
    {
        CHECK_READ(libendian::read(&i.type, 1, fp));
        CHECK_READ(libendian::le_read_us(&i.x, fp));
        CHECK_READ(libendian::le_read_us(&i.y, fp));
        CHECK_READ(libendian::le_read_ui(&i.area, fp));
    }

    // go to real map height and width
    fseek(fp, 2348, SEEK_SET);
    CHECK_READ(libendian::le_read_us(&myMap->width, fp));
    CHECK_READ(libendian::le_read_us(&myMap->height, fp));

    myMap->vertex.resize(myMap->width * myMap->height);

    // go to altitude information (we skip the 16 bytes long map data header that each block has)
    fseek(fp, 16, SEEK_CUR);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
        {
            Uint8 heightFactor;
            CHECK_READ(libendian::read(&heightFactor, 1, fp));
            myMap->getVertex(i, j).h = heightFactor; //-V807
        }
    }
    myMap->initVertexCoords();

    // go to texture information for RightSideUp-Triangles
    fseek(fp, 16, SEEK_CUR);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            CHECK_READ(libendian::read(&myMap->getVertex(i, j).rsuTexture, 1, fp));
    }

    // go to texture information for UpSideDown-Triangles
    fseek(fp, 16, SEEK_CUR);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            CHECK_READ(libendian::read(&myMap->getVertex(i, j).usdTexture, 1, fp));
    }

    // go to road data
    fseek(fp, 16, SEEK_CUR);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            CHECK_READ(libendian::read(&myMap->getVertex(i, j).road, 1, fp));
    }

    // go to object type data
    fseek(fp, 16, SEEK_CUR);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            CHECK_READ(libendian::read(&myMap->getVertex(i, j).objectType, 1, fp));
    }

    // go to object info data
    fseek(fp, 16, SEEK_CUR);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            CHECK_READ(libendian::read(&myMap->getVertex(i, j).objectInfo, 1, fp));
    }

    // go to animal data
    fseek(fp, 16, SEEK_CUR);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            CHECK_READ(libendian::read(&myMap->getVertex(i, j).animal, 1, fp));
    }

    // go to unknown1 data
    fseek(fp, 16, SEEK_CUR);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            CHECK_READ(libendian::read(&myMap->getVertex(i, j).unknown1, 1, fp));
    }

    // go to build data
    fseek(fp, 16, SEEK_CUR);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            CHECK_READ(libendian::read(&myMap->getVertex(i, j).build, 1, fp));
    }

    // go to unknown2 data
    fseek(fp, 16, SEEK_CUR);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            CHECK_READ(libendian::read(&myMap->getVertex(i, j).unknown2, 1, fp));
    }

    // go to unknown3 data
    fseek(fp, 16, SEEK_CUR);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            CHECK_READ(libendian::read(&myMap->getVertex(i, j).unknown3, 1, fp));
    }

    // go to resource data
    fseek(fp, 16, SEEK_CUR);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            CHECK_READ(libendian::read(&myMap->getVertex(i, j).resource, 1, fp));
    }

    // go to shading data
    fseek(fp, 16, SEEK_CUR);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            CHECK_READ(libendian::read(&myMap->getVertex(i, j).shading, 1, fp));
    }

    // go to unknown5 data
    fseek(fp, 16, SEEK_CUR);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            CHECK_READ(libendian::read(&myMap->getVertex(i, j).unknown5, 1, fp));
    }

    return myMap.release();
}

bobMAP* CFile::read_swd(FILE* fp)
{
    return read_wld(fp);
}

bool CFile::save_file(const boost::filesystem::path& filepath, char filetype, void* data)
{
    if(filepath.empty() || !data)
        return false;

    s25util::file_handle fh(boost::nowide::fopen(filepath.string().c_str(), "wb"));
    if(!fh)
        return false;

    switch(filetype)
    {
        case LST: return save_lst(*fh, data);
        case BOB: return save_bob(*fh, data);
        case IDX: return save_idx(*fh, data);
        case BBM: return save_bbm(*fh, data);
        case LBM: return save_lbm(*fh, data);
        case WLD: return save_wld(*fh, data);
        case SWD: return save_swd(*fh, data);
        default: // no valid data type
            return false;
    }
}

bool CFile::save_lst(FILE*, void*)
{
    return false;
}

bool CFile::save_bob(FILE*, void*)
{
    return false;
}

bool CFile::save_idx(FILE*, void*)
{
    return false;
}

bool CFile::save_bbm(FILE*, void*)
{
    return false;
}

bool CFile::save_lbm(FILE*, void*)
{
    return false;
}

bool CFile::save_wld(FILE* fp, void* data)
{
    char zero = 0; // to fill bytes
    char temp = 0; // to fill bytes
    auto* myMap = (bobMAP*)data;
    char map_version[11] = "WORLD_V1.0";
    std::array<char, 16> map_data_header;

    // prepare map data header
    map_data_header[0] = 0x10;
    map_data_header[1] = 0x27;
    map_data_header[2] = 0x00;
    map_data_header[3] = 0x00;
    map_data_header[4] = 0x00;
    map_data_header[5] = 0x00;
    *((Uint16*)(&map_data_header[6])) = boost::endian::native_to_little(myMap->width);
    *((Uint16*)(&map_data_header[8])) = boost::endian::native_to_little(myMap->height);
    map_data_header[10] = 0x01;
    map_data_header[11] = 0x00;
    *((Uint32*)(&map_data_header[12])) = boost::endian::native_to_little(myMap->width * myMap->height);

    // begin writing data to file
    // first of all the map header
    // WORLD_V1.0
    libendian::write(map_version, 10, fp);
    auto makeNameArray = [](const std::string& name) {
        std::array<char, 20> ar;
        auto size = std::min(name.size(), ar.size());
        ar.fill(0);
        std::copy_n(name.begin(), size, ar.begin());
        return ar;
    };

    // name
    libendian::write(makeNameArray(myMap->getName()), fp);
    // old width
    libendian::le_write_us(myMap->width_old, fp);
    // old height
    libendian::le_write_us(myMap->height_old, fp);
    // type
    auto mapType = uint8_t(myMap->type);
    libendian::write(&mapType, 1, fp);
    // players
    libendian::write(&myMap->player, 1, fp);
    // author
    libendian::write(makeNameArray(myMap->getAuthor()), fp);
    // headquarters x
    for(unsigned short i : myMap->HQx)
        libendian::le_write_us(i, fp);
    // headquarters y
    for(unsigned short i : myMap->HQy)
        libendian::le_write_us(i, fp);
    // unknown data (8 Bytes)
    for(int i = 0; i < 8; i++)
        libendian::write(&zero, 1, fp);
    // big map header with area information
    for(auto& i : myMap->header)
    {
        libendian::write(&i.type, 1, fp);
        libendian::le_write_us(i.x, fp);
        libendian::le_write_us(i.y, fp);
        libendian::le_write_ui(i.area, fp);
    }
    // 0x11 0x27
    temp = 0x11;
    libendian::write(&temp, 1, fp);
    temp = 0x27;
    libendian::write(&temp, 1, fp);
    // unknown data (always null, 4 Bytes)
    for(int i = 0; i < 4; i++)
        libendian::write(&zero, 1, fp);
    // width
    libendian::le_write_us(myMap->width, fp);
    // height
    libendian::le_write_us(myMap->height, fp);

    // now begin writing the real map data

    // altitude information
    libendian::write(map_data_header, fp);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
        {
            temp = myMap->getVertex(i, j).z / 5 + 0x0A; //-V807
            libendian::write(&temp, 1, fp);
        }
    }

    // texture information for RightSideUp-Triangles
    libendian::write(map_data_header, fp);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            libendian::write(&myMap->getVertex(i, j).rsuTexture, 1, fp);
    }

    // go to texture information for UpSideDown-Triangles
    libendian::write(map_data_header, fp);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            libendian::write(&myMap->getVertex(i, j).usdTexture, 1, fp);
    }

    // go to road data
    libendian::write(map_data_header, fp);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            libendian::write(&myMap->getVertex(i, j).road, 1, fp);
    }

    // go to object type data
    libendian::write(map_data_header, fp);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            libendian::write(&myMap->getVertex(i, j).objectType, 1, fp);
    }

    // go to object info data
    libendian::write(map_data_header, fp);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            libendian::write(&myMap->getVertex(i, j).objectInfo, 1, fp);
    }

    // go to animal data
    libendian::write(map_data_header, fp);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            libendian::write(&myMap->getVertex(i, j).animal, 1, fp);
    }

    // go to unknown1 data
    libendian::write(map_data_header, fp);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            libendian::write(&myMap->getVertex(i, j).unknown1, 1, fp);
    }

    // go to build data
    libendian::write(map_data_header, fp);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            libendian::write(&myMap->getVertex(i, j).build, 1, fp);
    }

    // go to unknown2 data
    libendian::write(map_data_header, fp);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            libendian::write(&myMap->getVertex(i, j).unknown2, 1, fp);
    }

    // go to unknown3 data
    libendian::write(map_data_header, fp);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            libendian::write(&myMap->getVertex(i, j).unknown3, 1, fp);
    }

    // go to resource data
    libendian::write(map_data_header, fp);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            libendian::write(&myMap->getVertex(i, j).resource, 1, fp);
    }

    // go to shading data
    libendian::write(map_data_header, fp);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            libendian::write(&myMap->getVertex(i, j).shading, 1, fp);
    }

    // go to unknown5 data
    libendian::write(map_data_header, fp);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
            libendian::write(&myMap->getVertex(i, j).unknown5, 1, fp);
    }

    // at least write the map footer (ends in 0xFF)
    temp = char(0xFF);
    libendian::write(&temp, 1, fp);

    return true;
}

bool CFile::save_swd(FILE* fp, void* data)
{
    return save_wld(fp, data);
}

bool CFile::read_bob01(FILE*)
{
    return false;
}

bool CFile::read_bob02(FILE* fp)
{
    // length of data block
    Uint32 length;
    // offset of next entry in file
    Uint32 next_entry;
    // endmark - to test, if a data block has correctly read
    Uint8 endmark;

    // Pic-Data
    // start adresses of picture lines in the file (offsets)
    Uint16* starts;
    // starting point for start adresses (starting_point + starts[i] = beginning of i. picture line)
    Uint32 starting_point;
    // number of following colored pixels in data block
    Uint8 count_color;
    // number of transparent pixels
    Uint8 count_trans;
    // color value for read pixel
    Uint8 color_value;

    // coordinate for zeropoint x (2 Bytes)
    CHECK_READ(libendian::le_read_us(&(bmpArray->nx), fp));
    // coordinate for zeropoint y (2 Bytes)
    CHECK_READ(libendian::le_read_us(&(bmpArray->ny), fp));
    // skip unknown data (4x 1 Byte)
    fseek(fp, 4, SEEK_CUR);
    // width of picture (2 Bytes)
    CHECK_READ(libendian::le_read_us(&(bmpArray->w), fp));
    // height of picture (2 Bytes)
    CHECK_READ(libendian::le_read_us(&(bmpArray->h), fp));
    // skip unknown data (1x 2 Bytes)
    fseek(fp, 2, SEEK_CUR);
    // length of data block (1x 4 Bytes)
    CHECK_READ(libendian::le_read_ui(&length, fp));
    // fp points now ON the first start adress, so "actual position + length = first offset of next entry in the file"
    starting_point = ftell(fp);
    next_entry = starting_point + length;

    // if we only want to read a palette at the moment (loadPAL == 1) so skip this and set the filepointer to the next
    // entry
    if(loadPAL)
    {
        fseek(fp, (long int)next_entry, SEEK_SET);
        return true;
    }

    // array for start addresses of picture lines
    starts = new Uint16[bmpArray->h];

    // read start addresses
    for(int y = 0; y < bmpArray->h; y++)
        CHECK_READ(libendian::le_read_us(&starts[y], fp));

    // now we are ready to read the picture lines and fill the surface, so lets create one
    if((bmpArray->surface = makePalSurface(bmpArray->w, bmpArray->h, palActual->colors)) == nullptr)
        return false;
    SDL_SetColorKey(bmpArray->surface.get(), SDL_TRUE, SDL_MapRGB(bmpArray->surface->format, 0, 0, 0));
    // SDL_SetAlpha(bmpArray->surface, SDL_SRCALPHA, 128);

    // main loop for reading picture lines
    for(int y = 0; y < bmpArray->h; y++)
    {
        // set fp to the needed offset (where the picture line starts)
        fseek(fp, starting_point + (Uint32)starts[y], SEEK_SET);

        // loop for reading pixels of the actual picture line
        //(cause of a kind of RLE-Compression we cannot read the pixels sequentially)
        //'x' will be incremented WITHIN the loop
        for(int x = 0; x < bmpArray->w;)
        {
            // number of following colored pixels (1 Byte)
            CHECK_READ(libendian::read(&count_color, 1, fp));

            // loop for drawing the colored pixels to the surface
            for(int k = 0; k < count_color; k++, x++)
            {
                // read color value (1 Byte)
                CHECK_READ(libendian::read(&color_value, 1, fp));
                // draw
                CSurface::DrawPixel_Color(bmpArray->surface.get(), x, y, (Uint32)color_value);
            }

            // number of transparent pixels to draw now (1 Byte)
            CHECK_READ(libendian::read(&count_trans, 1, fp));

            // loop for drawing the transparent pixels to the surface
            for(int k = 0; k < count_trans; k++, x++)
            {
                CSurface::DrawPixel_RGBA(bmpArray->surface.get(), x, y, 0, 0, 0, 0);
            }
        }

        // the end of line should be 0xFF, otherwise an error has occurred (1 Byte)
        CHECK_READ(libendian::read(&endmark, 1, fp));
        if(endmark != 0xFF)
            return false;
    }

    // at the end of the block (after the last line) there should be another 0xFF, otherwise an error has occurred (1
    // Byte)
    CHECK_READ(libendian::read(&endmark, 1, fp));
    if(endmark != 0xFF)
        return false;

    // we are finished, the surface is filled
    // fp should now point to the first offset of the next entry, this is the last check if this is REALLY the RIGHT
    // position
    if(ftell(fp) != (long int)next_entry)
    {
        fseek(fp, (long int)next_entry, SEEK_SET);
        return false;
    }

    // increment bmpArray for the next picture
    bmpArray++;

    delete[] starts;

    return true;
}

bool CFile::read_bob03(FILE* fp)
{
    // temporary skip x- and y-spacing (2x 1 Byte) --> we will handle this later
    fseek(fp, 2, SEEK_CUR);

    // read bobtype04 115 times (for 115 ansi chars)
    for(int i = 1; i <= 115; i++)
    {
        // bobtype of the entry
        Uint16 bobtype;
        // following data blocks are bobtype04 for some ascii chars, bobtype is repeated at the beginning of each data
        // block
        CHECK_READ(libendian::le_read_us(&bobtype, fp));

        // bobtype should be 04. if not, it's possible that there are a lot of zeros till the next block begins
        if(bobtype != BOBTYPE04)
        {
            // read the zeros (2 Bytes for each zero)
            while(bobtype == 0)
                CHECK_READ(libendian::le_read_us(&bobtype, fp));

            // at the end of all the zeros --> if bobtype is STILL NOT 04, an error has occured
            if(bobtype != BOBTYPE04)
                return false;
        }

        // now read the picture for each player color
        const auto offset = ftell(fp);
        constexpr std::array<int, 7> player_colors{
          PLAYER_BLUE, PLAYER_RED, PLAYER_ORANGE, PLAYER_GREEN, PLAYER_MINTGREEN, PLAYER_YELLOW, PLAYER_RED_BRIGHT,
        };
        for(const int player_color : player_colors)
        {
            fseek(fp, offset, SEEK_SET);
            if(!read_bob04(fp, player_color))
                return false;
        }
    }

    return true;
}

bool CFile::read_bob04(FILE* fp, int player_color)
{
    // length of data block
    Uint32 length;
    // offset of next entry in file
    Uint32 next_entry;

    // Pic-Data
    //'shift' (1 Byte) --> to decide what pixels and how many we have to draw
    Uint8 shift;
    // start adresses of picture lines in the file (offsets)
    Uint16* starts;
    // starting point for start adresses (starting_point + starts[i] = beginning of i. picture line)
    Uint32 starting_point;
    // color value for read pixel
    Uint8 color_value;

    // coordinate for zeropoint x (2 Bytes)
    CHECK_READ(libendian::le_read_us(&(bmpArray->nx), fp));
    // coordinate for zeropoint y (2 Bytes)
    CHECK_READ(libendian::le_read_us(&(bmpArray->ny), fp));
    // skip unknown data (4x 1 Byte)
    fseek(fp, 4, SEEK_CUR);
    // width of picture (2 Bytes)
    CHECK_READ(libendian::le_read_us(&(bmpArray->w), fp));
    // heigth of picture (2 Bytes)
    CHECK_READ(libendian::le_read_us(&(bmpArray->h), fp));
    // skip unknown data (1x 2 Bytes)
    fseek(fp, 2, SEEK_CUR);
    // length of datablock (1x 4 Bytes)
    CHECK_READ(libendian::le_read_ui(&length, fp));
    // fp points now ON the first start adress, so "actual position + length = first offset of next entry in the file"
    starting_point = ftell(fp);
    next_entry = starting_point + length;

    // if we only want to read a palette at the moment (loadPAL == 1) so skip this and set the filepointer to the next
    // entry
    if(loadPAL)
    {
        fseek(fp, (long int)next_entry, SEEK_SET);
        return true;
    }

    // array for start addresses of picture lines
    starts = new Uint16[bmpArray->h];

    // read start addresses
    for(int y = 0; y < bmpArray->h; y++)
        CHECK_READ(libendian::le_read_us(&starts[y], fp));

    // now we are ready to read the picture lines and fill the surface, so lets create one
    if((bmpArray->surface = makePalSurface(bmpArray->w, bmpArray->h, palActual->colors)) == nullptr)
        return false;

    SDL_SetColorKey(bmpArray->surface.get(), SDL_TRUE, SDL_MapRGB(bmpArray->surface->format, 0, 0, 0));

    // main loop for reading picture lines
    for(int y = 0; y < bmpArray->h; y++)
    {
        // set fp to the needed offset (where the picture line starts)
        fseek(fp, starting_point + (Uint32)starts[y], SEEK_SET);

        // loop for reading pixels of the actual picture line
        //(cause of a kind of RLE-Compression we cannot read the pixels sequentially)
        //'x' will be incremented WITHIN the loop
        for(int x = 0; x < bmpArray->w;)
        {
            // read our 'shift' (1 Byte)
            CHECK_READ(libendian::read(&shift, 1, fp));

            if(shift < 0x41)
            {
                for(int i = 1; i <= shift; i++, x++)
                {
                    CSurface::DrawPixel_RGBA(bmpArray->surface.get(), x, y, 0, 0, 0, 0);
                }
            } else if(shift < 0x81)
            {
                CHECK_READ(libendian::read(&color_value, 1, fp));

                for(int i = 1; i <= shift - 0x40; i++, x++)
                {
                    CSurface::DrawPixel_Color(bmpArray->surface.get(), x, y, /*0x80 - 0x40*/ +(Uint32)color_value);
                }
            } else if(shift < 0xC1)
            {
                CHECK_READ(libendian::read(&color_value, 1, fp));

                for(int i = 1; i <= shift - 0x80; i++, x++)
                {
                    CSurface::DrawPixel_Color(bmpArray->surface.get(), x, y, player_color + (Uint32)color_value);
                }
            } else // if (shift > 0xC0)
            {
                CHECK_READ(libendian::read(&color_value, 1, fp));

                for(int i = 1; i <= shift - 0xC0; i++, x++)
                {
                    CSurface::DrawPixel_Color(bmpArray->surface.get(), x, y, (Uint32)color_value);
                }
            }
        }
    }

    // we are finished, the surface is filled
    // fp should now point to the first offset of the next entry, this is the last check if this is REALLY the RIGHT
    // position
    if(ftell(fp) != (long int)next_entry)
    {
        fseek(fp, (long int)next_entry, SEEK_SET);
        // return false;
    }

    // increment bmpArray for the next picture
    bmpArray++;

    delete[] starts;

    return true;
}

bool CFile::read_bob05(FILE* fp)
{
    // skip: unknown data (1x 2 Bytes)
    fseek(fp, 2, SEEK_CUR);

    for(auto& color : palArray->colors)
    {
        CHECK_READ(libendian::read(&(color.r), 1, fp));
        CHECK_READ(libendian::read(&(color.g), 1, fp));
        CHECK_READ(libendian::read(&(color.b), 1, fp));
    }

    palArray++;

    return true;
}

bool CFile::read_bob07(FILE* fp)
{
    // length of data block
    Uint32 length;
    // offset of next entry in file
    Uint32 next_entry;
    // endmark - to test, if a data block has correctly read
    Uint8 endmark;

    // Pic-Data
    // start adresses of picture lines in the file (offsets)
    Uint16* starts;
    // starting point for start adresses (starting_point + starts[i] = beginning of i. picture line)
    Uint32 starting_point;
    // number of half-transparent black pixels in data block
    Uint8 count_black;
    // number of transparent pixels
    Uint8 count_trans;

    // coordinate for zeropoint x (2 Bytes)
    CHECK_READ(libendian::le_read_us(&(shadowArray->nx), fp));
    // coordinate for zeropoint y (2 Bytes)
    CHECK_READ(libendian::le_read_us(&(shadowArray->ny), fp));
    // skip unknown data (4x 1 Byte)
    fseek(fp, 4, SEEK_CUR);
    // width of picture (2 Bytes)
    CHECK_READ(libendian::le_read_us(&(shadowArray->w), fp));
    // heigth of picture (2 Bytes)
    CHECK_READ(libendian::le_read_us(&(shadowArray->h), fp));
    // skip unknown data (1x 2 Bytes)
    fseek(fp, 2, SEEK_CUR);
    // length of datablock (1x 4 Bytes)
    CHECK_READ(libendian::le_read_ui(&length, fp));
    // fp points now ON the first start adress, so "actual position + length = first offset of next entry in the file"
    starting_point = ftell(fp);
    next_entry = starting_point + length;

    // if we only want to read a palette at the moment (loadPAL == 1) so skip this and set the filepointer to the next
    // entry
    if(loadPAL)
    {
        fseek(fp, (long int)next_entry, SEEK_SET);
        return true;
    }

    // array for start addresses of picture lines
    starts = new Uint16[shadowArray->h];

    // read start addresses
    for(int y = 0; y < shadowArray->h; y++)
        CHECK_READ(libendian::le_read_us(&starts[y], fp));

    // now we are ready to read the picture lines and fill the surface, so lets create one
    if((shadowArray->surface = makePalSurface(shadowArray->w, shadowArray->h, palActual->colors)) == nullptr)
        return false;
    // SDL_SetAlpha(shadowArray->surface, SDL_SRCALPHA, 128);

    // main loop for reading picture lines
    for(int y = 0; y < shadowArray->h; y++)
    {
        // set fp to the needed offset (where the picture line starts)
        fseek(fp, starting_point + (Uint32)starts[y], SEEK_SET);

        // loop for reading pixels of the actual picture line
        //(cause of a kind of RLE-Compression we cannot read the pixels sequentially)
        //'x' will be incremented WITHIN the loop
        for(int x = 0; x < shadowArray->w;)
        {
            // number of half-transparent black (alpha value = 0x40) pixels (1 Byte)
            CHECK_READ(libendian::read(&count_black, 1, fp));

            // loop for drawing the black pixels to the surface
            for(int k = 0; k < count_black; k++, x++)
            {
                // draw
                CSurface::DrawPixel_RGBA(shadowArray->surface.get(), x, y, 0, 0, 0, 0x40);
            }

            // number of transparent pixels to draw now (1 Byte)
            CHECK_READ(libendian::read(&count_trans, 1, fp));

            // loop for drawing the transparent pixels to the surface
            for(int k = 0; k < count_trans; k++, x++)
            {
                CSurface::DrawPixel_RGBA(shadowArray->surface.get(), x, y, 0, 0, 0, 0);
            }
        }

        // the end of line should be 0xFF, otherwise an error has occurred (1 Byte)
        CHECK_READ(libendian::read(&endmark, 1, fp));
        if(endmark != 0xFF)
            return false;
    }

    // at the end of the block (after the last line) there should be another 0xFF, otherwise an error has occurred (1
    // Byte)
    CHECK_READ(libendian::read(&endmark, 1, fp));
    if(endmark != 0xFF)
        return false;

    // we are finished, the surface is filled
    // fp should now point to the first offset of the next entry, this is the last check if this is REALLY the RIGHT
    // position
    if(ftell(fp) != (long int)next_entry)
    {
        fseek(fp, (long int)next_entry, SEEK_SET);
        return false;
    }

    /**FOLLOWING COMMENTS ARE ABSOLUTLY TEMPORARY, UNTIL I KNOW HOW TO HANDLE SHADOWS WITH ALPHA-BLENDING AND THEN
     *BLITTING
     ***---THIS CODE IS NOT USEFUL.**/
    // We have to blit picture and shadow together and because of transparency we need to set the colorkey
    // SDL_SetColorKey(shadowArray->surface, SDL_SRCCOLORKEY, SDL_MapRGBA(shadowArray->surface->format, 0, 0, 0, 0));
    // SDL_BlitSurface(shadowArray->surface, nullptr, (shadowArray-1)->surface, nullptr);
    // SDL_FreeSurface(shadowArray->surface);
    // increment shadowArray
    shadowArray++;

    delete[] starts;

    return true;
}

bool CFile::read_bob14(FILE* fp)
{
    // length of data block
    Uint32 length;
    // offset of next entry in file
    long int next_entry;

    // Pic-Data
    // offset for the first byte of the data block
    long int data_start;
    // color value for read pixel
    Uint8 color_value;

    // skip unknown data (1x 2 Bytes)
    fseek(fp, 2, SEEK_CUR);
    // length of datablock (1x 4 Bytes)
    CHECK_READ(libendian::le_read_ui(&length, fp));
    // start offset of data block
    data_start = ftell(fp);
    // jump to first offset after data block
    fseek(fp, length, SEEK_CUR);
    // coordinate for zero point x (2 Bytes)
    CHECK_READ(libendian::le_read_us(&(bmpArray->nx), fp));
    // coordinate for zero point y (2 Bytes)
    CHECK_READ(libendian::le_read_us(&(bmpArray->ny), fp));
    // width of picture (2 Bytes)
    CHECK_READ(libendian::le_read_us(&(bmpArray->w), fp));
    // heigth of picture (2 Bytes)
    CHECK_READ(libendian::le_read_us(&(bmpArray->h), fp));
    // skip unknown data (8x 1 Byte)
    fseek(fp, 8, SEEK_CUR);

    // fp points now ON the first adress of the next entry in the file
    next_entry = ftell(fp);

    // if we only want to read a palette at the moment (loadPAL == 1) so skip this and let fp on the adress of the next
    // entry
    if(loadPAL)
        return true;

    // now we are ready to read the picture lines and fill the surface, so lets create one
    if((bmpArray->surface = makePalSurface(bmpArray->w, bmpArray->h, palActual->colors)) == nullptr)
        return false;

    // set fp to back to the first offset of data block
    fseek(fp, data_start, SEEK_SET);

    // main loop for reading picture lines
    for(int y = 0; y < bmpArray->h; y++)
    {
        // loop for reading pixels of the actual picture line
        for(int x = 0; x < bmpArray->w; x++)
        {
            // read color value (1 Byte)
            CHECK_READ(libendian::read(&color_value, 1, fp));
            // draw
            CSurface::DrawPixel_Color(bmpArray->surface.get(), x, y, (Uint32)color_value);
        }
    }

    // we are finished, the surface is filled
    // fp should now point to nx again, so set it to the next entry in the file
    fseek(fp, (long int)next_entry, SEEK_SET);

    // increment bmpArray for the next picture
    bmpArray++;

    return true;
}

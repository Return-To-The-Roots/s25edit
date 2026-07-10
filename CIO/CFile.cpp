// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2024 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CFile.h"
#include "../globals.h"
#include "libendian/libendian.h"
#include "s25util/file_handle.h"
#include <boost/endian/conversion.hpp>
#include <boost/nowide/cstdio.hpp>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>

#define CHECK_READ(readCmd) \
    if(!(readCmd))          \
    throw std::runtime_error("Read failed")

void* CFile::open_file(const boost::filesystem::path& filepath, char filetype)
{
    if(filepath.empty())
        return nullptr;
    s25util::file_handle fh(boost::nowide::fopen(filepath.string().c_str(), "rb"));
    if(!fh)
        return nullptr;
    try
    {
        switch(filetype)
        {
            case WLD: return read_wld(*fh);
            case SWD: return read_swd(*fh);
            default: break;
        }
    } catch(const std::exception& e)
    {
        std::cerr << "Error reading " << filepath << ": " << e.what() << std::endl;
    }
    return nullptr;
}

bool CFile::save_file(const boost::filesystem::path& filepath, char filetype, void* data)
{
    if(filepath.empty() || !data)
        return false;
    s25util::file_handle fh(boost::nowide::fopen(filepath.string().c_str(), "wb"));
    if(!fh)
        return false;
    try
    {
        switch(filetype)
        {
            case WLD:
                if(save_wld(*fh, data))
                    return true;
                break;
            case SWD:
                if(save_swd(*fh, data))
                    return true;
                break;
            default: break;
        }
    } catch(const std::exception& e)
    {
        std::cerr << "Error saving " << filepath << ": " << e.what() << std::endl;
    }
    return false;
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

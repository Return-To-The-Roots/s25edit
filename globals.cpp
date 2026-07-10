// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "globals.h"
#include <libsiedler2/ArchivItem_Bitmap.h>
#include <libsiedler2/ArchivItem_Palette.h>
#include <libsiedler2/ArchivItem_PaletteAnimation.h>
#include <libsiedler2/ErrorCodes.h>
#include <libsiedler2/libsiedler2.h>
#include <cstring>
#include <iostream>

// single archive for all loaded assets
// typed archive storage
std::map<ArchiveID, libsiedler2::Archiv> global::typedArchives;

// currently active palette
const libsiedler2::ArchivItem_Palette* global::currentPalette = nullptr;

bool global::loadArchive(ArchiveID archive, const boost::filesystem::path& filepath,
                         const libsiedler2::ArchivItem_Palette* palette)
{
    libsiedler2::Archiv tmp;
    int ec = libsiedler2::Load(filepath, tmp, palette);
    if(ec)
    {
        std::cerr << "loadArchive(" << static_cast<int>(archive) << ") error for " << filepath << ": "
                  << libsiedler2::getErrorString(ec) << std::endl;
        return false;
    }
    typedArchives[archive] = std::move(tmp);
    return true;
}

bool global::loadBitmapArchive(ArchiveID archive, const boost::filesystem::path& filepath,
                               const libsiedler2::ArchivItem_Palette* palette)
{
    libsiedler2::Archiv tmp;
    int ec = libsiedler2::Load(filepath, tmp, palette);
    if(ec)
    {
        std::cerr << "loadBitmapArchive(" << static_cast<int>(archive) << ") error for " << filepath << ": "
                  << libsiedler2::getErrorString(ec) << std::endl;
        return false;
    }

    // Keep only bitmap-type items (skip nulls, shadows, palettes, fonts)
    libsiedler2::Archiv filtered;
    for(unsigned i = 0; i < tmp.size(); i++)
    {
        auto* item = tmp.get(i);
        if(!item)
            continue;
        auto bt = item->getBobType();
        if(bt == libsiedler2::BobType::Bitmap || bt == libsiedler2::BobType::BitmapRLE
           || bt == libsiedler2::BobType::Raw || bt == libsiedler2::BobType::BitmapPlayer)
        {
            filtered.push(std::unique_ptr<libsiedler2::ArchivItem>(tmp.release(i).release()));
        }
    }

    typedArchives[archive] = std::move(filtered);
    return true;
}

bool global::loadPalette(const boost::filesystem::path& filepath)
{
    libsiedler2::Archiv tmp;
    int ec = libsiedler2::Load(filepath, tmp, nullptr);
    if(ec)
    {
        auto datPath = filepath;
        datPath.replace_extension(".DAT");
        ec = libsiedler2::Load(datPath, tmp, nullptr);
    }
    if(ec || tmp.empty())
    {
        std::cerr << "loadPalette error for " << filepath << ": " << (ec ? libsiedler2::getErrorString(ec) : "empty")
                  << std::endl;
        return false;
    }
    currentPalette = dynamic_cast<const libsiedler2::ArchivItem_Palette*>(tmp.get(0));
    if(!currentPalette)
    {
        std::cerr << "loadPalette: first item is not a palette in " << filepath << std::endl;
        return false;
    }
    static libsiedler2::Archiv s_paletteStorage;
    s_paletteStorage = std::move(tmp);
    return true;
}

Extent global::getBitmapSize(ArchiveID archive, unsigned index)
{
    auto& archiv = typedArchives[archive];
    if(index >= archiv.size())
        return {0u, 0u};
    auto* bmp = dynamic_cast<const libsiedler2::baseArchivItem_Bitmap*>(archiv.get(index));
    if(!bmp)
        return {0u, 0u};
    return {bmp->getWidth(), bmp->getHeight()};
}

bool global::loadTileset(ArchiveID archive, int tsIdx, const boost::filesystem::path& filepath)
{
    // LBM files contain their own palette, so no external palette needed
    libsiedler2::Archiv tmp;
    int ec = libsiedler2::Load(filepath, tmp, nullptr);
    if(ec || tmp.empty())
    {
        std::cerr << "loadTileset(" << static_cast<int>(archive) << ") error for " << filepath << ": "
                  << (ec ? libsiedler2::getErrorString(ec) : "empty") << std::endl;
        return false;
    }

    // Store in typed archive (index 0 = main bitmap, 1+ = palette animations)
    auto& dest = typedArchives[archive];
    dest = std::move(tmp);

    // Extract palette animations into tilesetAnims[tsIdx]
    auto& ta = tilesetAnims[tsIdx];
    ta.archive = archive;
    ta.bmpIdx = 0; // index 0 within this archive = main bitmap
    ta.anims.clear();
    ta.storage.clear();
    for(unsigned ai = 1; ai < dest.size(); ai++)
    {
        auto* anim = dynamic_cast<const libsiedler2::ArchivItem_PaletteAnimation*>(dest.get(ai));
        if(!anim)
            continue;
        // We need mutable pointers for the animation system; clone them
        auto clone = std::unique_ptr<libsiedler2::ArchivItem>(dest.release(ai).release());
        ta.anims.push_back(static_cast<libsiedler2::ArchivItem_PaletteAnimation*>(clone.get()));
        ta.storage.push_back(std::move(clone));
    }

    return true;
}

// font archives loaded at startup
std::array<global::TilesetAnim, 3> global::tilesetAnims;
// the game object
CGame* global::s2;

boost::filesystem::path global::gameDataFilePath(".");
boost::filesystem::path global::userMapsPath;
WorldDescription global::worldDesc;

unsigned char triangleHeight = TRIANGLE_HEIGHT_DEFAULT;
unsigned char triangleWidth = TRIANGLE_WIDTH_DEFAULT;
unsigned char triangleIncrease = TRIANGLE_INCREASE_DEFAULT;

// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ArchiveID.h"
#include "Texture.h"
#include "gameData/WorldDescription.h"
#include <libsiedler2/Archiv.h>
#include <boost/filesystem/path.hpp>
#include <SDL.h>
#include <array>
#include <map>
#include <vector>

class CGame;

namespace libsiedler2 {
class ArchivItem_Palette;
class ArchivItem_PaletteAnimation;
class baseArchivItem_Bitmap;
struct ColorBGRA;
} // namespace libsiedler2

namespace global {

// ── Typed archive storage ─────────────────────────────────────────────
/// Per-ArchiveID archives, loaded and kept intact.
extern std::map<ArchiveID, libsiedler2::Archiv> typedArchives;

/// Load a file into typedArchives[archive] using libsiedler2::Load directly.
/// Returns true on success.
bool loadArchive(ArchiveID archive, const boost::filesystem::path& filepath,
                 const libsiedler2::ArchivItem_Palette* palette = nullptr);

/// Get the size of a bitmap from a typed archive.
Extent getBitmapSize(ArchiveID archive, unsigned index);

/// Load a palette BBM file into currentPalette. Returns true on success.
bool loadPalette(const boost::filesystem::path& filepath);

/// Load a file into typedArchives[archive], keeping only bitmap-type items
/// (Bitmap, BitmapRLE, Raw, BitmapPlayer), skipping nulls and other types.
/// Used for archives like MAP00.LST that contain non-bitmap items.
bool loadBitmapArchive(ArchiveID archive, const boost::filesystem::path& filepath,
                       const libsiedler2::ArchivItem_Palette* palette = nullptr);

/// Load a terrain tileset (TEX5/6/7.LBM) into typedArchives and extract
/// palette animations into tilesetAnims[tsIdx].  The old flat-archive
/// path is NOT used — call this instead of CFile::open_file.
bool loadTileset(ArchiveID archive, int tsIdx, const boost::filesystem::path& filepath);

// Palette animations per tileset (greenland=0, wasteland=1, winterland=2).
struct TilesetAnim
{
    ArchiveID archive = ArchiveID::EDITRES; // which archive contains the bitmap
    unsigned bmpIdx = 0;                    // index of the tileset bitmap within the archive
    std::vector<const libsiedler2::ArchivItem_PaletteAnimation*> anims;
    std::vector<std::unique_ptr<libsiedler2::ArchivItem>> storage; // keeps animations alive
};
extern std::array<TilesetAnim, 3> tilesetAnims;

/// Currently active palette (loaded from PAL*.BBM).
extern const libsiedler2::ArchivItem_Palette* currentPalette;

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

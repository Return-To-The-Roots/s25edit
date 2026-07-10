// Copyright (C) 2026 - 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

/// Identifies a loaded resource archive.
/// Each value corresponds to one file (or group of related files) that is loaded
/// and kept as a separate libsiedler2::Archiv, keyed by this type.
enum class ArchiveID : uint8_t
{
    // ── Editor / UI archives ──────────────────────────────────────────
    EDITBOB = 0, // DATA/EDITBOB.LST
    EDITIO,      // DATA/IO/EDITIO.IDX
    EDITRES,     // DATA/EDITRES.IDX  (or RESOURCE.IDX/DAT)

    // ── Loading screens ───────────────────────────────────────────────
    SETUP997,
    SETUP998,
    SETUP000,
    SETUP010,
    SETUP011,
    SETUP012,
    SETUP013,
    SETUP014,
    SETUP015,
    SETUP666,
    SETUP667,
    SETUP801,
    SETUP802,
    SETUP803,
    SETUP804,
    SETUP805,
    SETUP806,
    SETUP810,
    SETUP811,
    SETUP895,
    SETUP896,
    SETUP897,
    SETUP898,
    SETUP899,
    SETUP990,
    WORLD_LBM,
    WORLDMSK_LBM,

    // ── Mission BOBs ──────────────────────────────────────────────────
    MIS0BOBS,
    MIS1BOBS,
    MIS2BOBS,
    MIS3BOBS,
    MIS4BOBS,
    MIS5BOBS,

    // ── Terrain tilesets (landscape-specific, one per landscape) ─────
    TEX5, // GFX/TEXTURES/TEX5.LBM (greenland)
    TEX6, // GFX/TEXTURES/TEX6.LBM (wasteland)
    TEX7, // GFX/TEXTURES/TEX7.LBM (winterland)

    // ── Map object sprites (MAP00.LST) ───────────────────────────────
    MAP00, // DATA/MAP00.LST

    // ── Palettes (keyed separately) ───────────────────────────────────
    // (not in this enum — palettes have their own storage)
};

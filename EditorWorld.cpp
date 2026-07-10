// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EditorWorld.h"
#include "EditorWorldLoader.h"
#include "Loader.h"
#include "RttrForeachPt.h"
#include "defines.h"
#include "globals.h"
#include "nodeObjs/noAnimal.h"
#include "nodeObjs/noEnvObject.h"
#include "nodeObjs/noGranite.h"
#include "nodeObjs/noStaticObject.h"
#include "nodeObjs/noTree.h"
#include "libsiedler2/Archiv.h"
#include "libsiedler2/ArchivItem_Map_Header.h"
#include "libsiedler2/libsiedler2.h"
#include "libsiedler2/prototypen.h"
#include "gameTypes/AnimalTypes.h"
#include "gameTypes/FlagType.h"
#include "gameData/TerrainDesc.h"
#include <boost/filesystem.hpp>
#include <iostream>

namespace bfs = boost::filesystem;

/// Inlined shadow recalculation — mirrors World::RecalcShadow (protected)
static void recalcShadowAt(GameWorld& world, MapPoint pt)
{
    int altitude = world.GetNode(pt).altitude;
    int A = world.GetNode(world.GetNeighbour(pt, Direction::NorthEast)).altitude - altitude;
    int B = world.GetNode(world.GetNeighbour2(pt, 0)).altitude - altitude;
    int C = world.GetNode(world.GetNeighbour(pt, Direction::West)).altitude - altitude;
    int D = world.GetNode(world.GetNeighbour2(pt, 11)).altitude - altitude;

    int shadingS2 = 64 + 9 * A - 3 * B - 6 * C - 9 * D;
    if(shadingS2 > 128)
        shadingS2 = 128;
    else if(shadingS2 < 0)
        shadingS2 = 0;
    world.GetNodeWriteable(pt).shadow = static_cast<uint8_t>(shadingS2);
}

std::unique_ptr<EditorWorld> EditorWorld::loadFromSwd(const bfs::path& filepath)
{
    // ── 1. Load the SWD/WLD file ──
    libsiedler2::Archiv mapArchiv;
    if(int ec = libsiedler2::loader::LoadMAP(filepath, mapArchiv))
    {
        std::cerr << "Failed to load map: " << filepath << " (error " << ec << ")\n";
        return nullptr;
    }

    const auto& map = *static_cast<const libsiedler2::ArchivItem_Map*>(mapArchiv[0]);
    const auto& hdr = map.getHeader();

    const uint16_t w = hdr.getWidth();
    const uint16_t h = hdr.getHeight();
    const uint8_t gfxSet = hdr.getGfxSet();
    const uint8_t numPlayers = std::max<uint8_t>(hdr.getNumPlayers(), 1);

    // ── 2. Find the landscape description matching gfxSet ──
    DescIdx<LandscapeDesc> landscape;
    for(DescIdx<LandscapeDesc> i(0); i.value < global::worldDesc.landscapes.size(); i.value++)
    {
        if(global::worldDesc.landscapes.get(i).s2Id == gfxSet)
        {
            landscape = i;
            break;
        }
    }
    if(!landscape)
    {
        std::cerr << "No landscape found for gfxSet " << int(gfxSet) << " in map " << filepath << "\n";
        return nullptr;
    }

    // ── 3. Create base EditorWorld ──
    auto ew = std::make_unique<EditorWorld>(MapExtent(w, h), numPlayers);

    // ── 4. Override metadata ──
    ew->mapName_ = hdr.getName();
    ew->author_ = hdr.getAuthor();
    ew->filepath_ = filepath;

    // ── 5. Store HQ positions from the map header (max 7 players) ──
    for(unsigned p = 0; p < MAX_PLAYERS && p < 7u; p++)
    {
        uint16_t hx, hy;
        hdr.getPlayerHQ(p, hx, hy);
        if(hx != 0xFFFF && hy != 0xFFFF)
            ew->hqPositions_[p] = MapPoint(hx, hy);
    }

    // ── 6. Set the correct landscape type ──
    ew->world_.SetLandscapeType(landscape);

    // ── 6. Helper: look up a terrain DescIdx by s2Id + current landscape ──
    auto& desc = ew->world_.GetDescriptionWriteable();
    auto getTerrainFromS2 = [&](uint8_t s2Id) -> DescIdx<TerrainDesc> {
        for(DescIdx<TerrainDesc> i(0); i.value < desc.terrain.size(); i.value++)
        {
            const auto& t = desc.terrain.get(i);
            if(t.s2Id == s2Id && t.landscape == landscape)
                return i;
        }
        return DescIdx<TerrainDesc>();
    };

    // ── 7. Populate all nodes ──
    RTTR_FOREACH_PT(MapPoint, MapExtent(w, h))
    {
        auto& node = ew->world_.GetNodeWriteable(pt);

        // Clear roads
        std::fill(node.roads.begin(), node.roads.end(), PointRoad::None);

        // Altitude
        node.altitude = map.getMapDataAt(libsiedler2::MapLayer::Altitude, pt.x, pt.y);

        // Terrain (lower 6 bits; bit 6 = harbour, stored separately)
        uint8_t rawT1 = map.getMapDataAt(libsiedler2::MapLayer::Terrain1, pt.x, pt.y);
        uint8_t rawT2 = map.getMapDataAt(libsiedler2::MapLayer::Terrain2, pt.x, pt.y);

        node.t1 = getTerrainFromS2(rawT1 & 0x3F);
        node.t2 = getTerrainFromS2(rawT2 & 0x3F);

        // Resources
        uint8_t res = map.getMapDataAt(libsiedler2::MapLayer::Resources, pt.x, pt.y);
        if(res == 0x20 || res == 0x21)
            node.resources = Resource(ResourceType::Water, 7);
        else if(res > 0x40 && res < 0x48)
            node.resources = Resource(ResourceType::Coal, res - 0x40);
        else if(res > 0x48 && res < 0x50)
            node.resources = Resource(ResourceType::Iron, res - 0x48);
        else if(res > 0x50 && res < 0x58)
            node.resources = Resource(ResourceType::Gold, res - 0x50);
        else if(res > 0x58 && res < 0x60)
            node.resources = Resource(ResourceType::Granite, res - 0x58);
        else if(res > 0x80 && res < 0x90)
            node.resources = Resource(ResourceType::Fish, 4);
        else
            node.resources = Resource(ResourceType::Nothing, 0);

        // Roads (block 4)
        uint8_t roadData = map.getMapDataAt(libsiedler2::MapLayer::RoadsOld, pt.x, pt.y);
        // lower left: (roadData / 16) % 4  → SouthWest
        // lower right: (roadData % 16) / 4  → SouthEast
        // right: (roadData % 4)             → East
        node.roads[RoadDir::SouthWest] = static_cast<PointRoad>((roadData / 16) % 4);
        node.roads[RoadDir::SouthEast] = static_cast<PointRoad>((roadData % 16) / 4);
        node.roads[RoadDir::East]      = static_cast<PointRoad>(roadData % 4);

        // Harbour marker (bit 6 of terrain1) — stored in node.harborId for save reconstruction
        if((rawT1 & libsiedler2::HARBOR_MASK) != 0)
            node.harborId = HarborId(1);
        else
            node.harborId.reset();

        // FOW: all visible for the editor
        for(auto& fow : node.fow)
        {
            fow = FoWNode();
            fow.visibility = Visibility::Visible;
        }

        // Clear dynamic fields
        node.owner = 0;
        node.reserved = false;
        std::fill(node.boundary_stones.begin(), node.boundary_stones.end(), 0);
        node.seaId.reset();
        node.obj = nullptr;
        node.figures.clear();
        // Keep harbour marker if the terrain had the harbour flag
        // (set below after checking rawT1)
    }

    // ── 8. Place objects (trees, granite, decorations, HQ markers) ──
    {
        RTTR_FOREACH_PT(MapPoint, MapExtent(w, h))
        {
            using libsiedler2::MapLayer;
            uint8_t lc   = map.getMapDataAt(MapLayer::ObjectIndex, pt.x, pt.y);
            uint8_t type = map.getMapDataAt(MapLayer::ObjectType, pt.x, pt.y);
            noBase* obj  = nullptr;

            switch(type)
            {
                // ── Player HQ (override header position from layer data) ──
                case 0x80:
                {
                    if(lc < MAX_PLAYERS)
                        ew->hqPositions_[lc] = pt;
                    break;
                }

                // ── Trees (types 1-4) ──
                case 0xC4:
                {
                    if(lc >= 0x30 && lc <= 0x3D)       obj = new noTree(pt, 0, 3);
                    else if(lc >= 0x70 && lc <= 0x7D)  obj = new noTree(pt, 1, 3);
                    else if(lc >= 0xB0 && lc <= 0xBD)  obj = new noTree(pt, 2, 3);
                    else if(lc >= 0xF0 && lc <= 0xFD)  obj = new noTree(pt, 3, 3);
                    break;
                }

                // ── Trees (types 5-8) ──
                case 0xC5:
                {
                    if(lc >= 0x30 && lc <= 0x3D)       obj = new noTree(pt, 4, 3);
                    else if(lc >= 0x70 && lc <= 0x7D)  obj = new noTree(pt, 5, 3);
                    else if(lc >= 0xB0 && lc <= 0xBD)  obj = new noTree(pt, 6, 3);
                    else if(lc >= 0xF0 && lc <= 0xFD)  obj = new noTree(pt, 7, 3);
                    break;
                }

                // ── Tree type 9 ──
                case 0xC6:
                {
                    if(lc >= 0x30 && lc <= 0x3D)
                        obj = new noTree(pt, 8, 3);
                    break;
                }

                // ── Landscape objects ──
                case 0xC8:
                case 0xC9:
                {
                    if(lc == 0x0B)
                        obj = new noStaticObject(pt, 500 + lc);
                    else if(lc <= 0x0F)
                        obj = new noEnvObject(pt, 500 + lc);
                    else if(lc <= 0x14)
                        obj = new noEnvObject(pt, 542 + lc - 0x10);
                    else if(lc == 0x15)
                        obj = new noStaticObject(pt, 0, 0);
                    else if(lc == 0x16)
                        obj = new noStaticObject(pt, 560);
                    else if(lc == 0x17)
                        obj = new noStaticObject(pt, 561);
                    else if(lc <= 0x1E)
                        obj = new noStaticObject(pt, (lc - 0x18) * 2, 1);
                    else if(lc <= 0x20)
                        obj = new noStaticObject(pt, 20 + (lc - 0x1F) * 2, 1);
                    else if(lc == 0x21)
                        obj = new noStaticObject(pt, 30, 1);
                    else if(lc <= 0x2B)
                        obj = new noEnvObject(pt, 550 + lc - 0x22);
                    else if(lc <= 0x2E || lc == 0x30)
                        obj = new noStaticObject(pt, (lc - 0x2C) * 2, 2);
                    else if(lc == 0x2F)
                        obj = new noStaticObject(pt, (lc - 0x2C) * 2, 2, 2);
                    else if(lc == 0x31)
                        obj = new noStaticObject(pt, 0, 3);
                    else if(lc == 0x32)
                        obj = new noStaticObject(pt, 0, 4);
                    else if(lc == 0x33)
                        obj = new noStaticObject(pt, 0, 5);
                    else if(lc == 0x34)
                        obj = new noStaticObject(pt, 2, 5);
                    else if(lc == 0x35)
                        obj = new noStaticObject(pt, 4, 5);
                    break;
                }

                // ── Granite type 1 ──
                case 0xCC:
                {
                    if(lc >= 0x01 && lc <= 0x06)
                        obj = new noGranite(GraniteType::One, lc - 1);
                    break;
                }

                // ── Granite type 2 ──
                case 0xCD:
                {
                    if(lc >= 0x01 && lc <= 0x06)
                        obj = new noGranite(GraniteType::Two, lc - 1);
                    break;
                }
                // default: nothing
            }

            if(obj)
                ew->world_.SetNO(pt, obj);
        }
    }

    // ── 9. Place animals ──
    RTTR_FOREACH_PT(MapPoint, MapExtent(w, h))
    {
        using libsiedler2::MapLayer;
        uint8_t a = map.getMapDataAt(MapLayer::Animals, pt.x, pt.y);
        Species species;
        switch(a)
        {
            case 1:  species = Species::RabbitGrey; break;  // random rabbit
            case 2:  species = Species::Fox;   break;
            case 3:  species = Species::Stag;  break;
            case 4:  species = Species::Deer;  break;
            case 5:  species = Species::Duck;  break;
            case 6:  species = Species::Sheep; break;
            case 0:
            case 0xFF: continue;
            default:  continue;
        }
        ew->world_.AddFigure(pt, std::make_unique<noAnimal>(species, pt));
    }

    // ── 10. Extra animal info (from map's extraInfo list) ──
    // Note: extraInfo animals are positioned at explicit (x,y) coords that may
    // be outside the regular layer. We place them as additional figures.
    for(const auto& extra : map.extraInfo)
    {
        if(extra.x >= w || extra.y >= h)
            continue;
        MapPoint pt(extra.x, extra.y);
        Species species;
        switch(extra.id)
        {
            case 1:  species = Species::RabbitGrey; break;
            case 2:  species = Species::Fox;   break;
            case 3:  species = Species::Stag;  break;
            case 4:  species = Species::Deer;  break;
            case 5:  species = Species::Duck;  break;
            case 6:  species = Species::Sheep; break;
            default: continue;
        }
        ew->world_.AddFigure(pt, std::make_unique<noAnimal>(species, pt));
    }

    // ── 11. Shadows — recalculate from altitudes ──
    RTTR_FOREACH_PT(MapPoint, MapExtent(w, h))
        recalcShadowAt(ew->world_, pt);

    // ── 12. BQ recalculation ──
    ew->world_.InitAfterLoad();

    // ── 13. Regenerate the terrain renderer with the new data ──
    ew->viewer_->InitTerrainRenderer();

    return ew;
}

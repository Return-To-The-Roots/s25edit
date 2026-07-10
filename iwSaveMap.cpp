// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iwSaveMap.h"
#include "EditorWorld.h"
#include "RttrForeachPt.h"
#include "defines.h"
#include "globals.h"
#include "libsiedler2/Archiv.h"
#include "libsiedler2/ArchivItem_Map.h"
#include "libsiedler2/ArchivItem_Map_Header.h"
#include "libsiedler2/libsiedler2.h"
#include "libsiedler2/prototypen.h"
#include "gameData/TerrainDesc.h"
#include "gameTypes/MapNode.h"
#include "gameTypes/BuildingQuality.h"
#include "gameTypes/GO_Type.h"
#include "gameTypes/Resource.h"
#include "nodeObjs/noAnimal.h"
#include "nodeObjs/noEnvObject.h"
#include "nodeObjs/noGranite.h"
#include "nodeObjs/noStaticObject.h"
#include "nodeObjs/noTree.h"
#include "controls/ctrlButton.h"
#include "controls/ctrlEdit.h"
#include "controls/ctrlText.h"
#include "WindowManager.h"
#include "Loader.h"
#include <boost/filesystem.hpp>
#include <iostream>

namespace bfs = boost::filesystem;

iwSaveMap::iwSaveMap(EditorWorld& world)
    : IngameWindow(102, IngameWindow::posLastOrCenter, Extent(280, 230), "Save map",
                   LOADER.GetImageN("resource", 41)),
      world_(world)
{
    auto off = DrawPoint(contentOffset.x, contentOffset.y);
    int labelX = off.x + (GetSize().x - contentOffset.x - contentOffsetEnd.x) / 2 + contentOffset.x - 30;

    AddText(ID_lblFilename, DrawPoint(labelX, off.y + 2), "Filename", COLOR_YELLOW, FontStyle::CENTER, SmallFont);
    auto* editFile = AddEdit(ID_edtFilename, off + DrawPoint(10, 13), Extent(210, 22), TextureColor::Grey, NormalFont);
    {
        // Use the original filename stem if available, else fall back to map name
        auto fp = world_.getFilepath();
        if(!fp.empty())
            editFile->SetText(fp.stem().string());
        else
            editFile->SetText(world_.getMapName());
    }

    AddText(ID_lblMapname, DrawPoint(labelX, off.y + 38), "Mapname", COLOR_YELLOW, FontStyle::CENTER, SmallFont);
    auto* editMapname = AddEdit(ID_edtMapname, off + DrawPoint(10, 50), Extent(210, 22), TextureColor::Grey, NormalFont);
    editMapname->SetText(world_.getMapName());

    AddText(ID_lblAuthor, DrawPoint(labelX, off.y + 75), "Author", COLOR_YELLOW, FontStyle::CENTER, SmallFont);
    auto* editAuthor = AddEdit(ID_edtAuthor, off + DrawPoint(10, 87), Extent(210, 22), TextureColor::Grey, NormalFont);
    editAuthor->SetText(world_.getAuthor());

    AddTextButton(ID_btSave, off + DrawPoint(10, 125), Extent(100, 22), TextureColor::Green2, "Save", NormalFont);
    AddTextButton(ID_btAbort, off + DrawPoint(120, 125), Extent(100, 22), TextureColor::Red1, "Abort", NormalFont);
}

void iwSaveMap::Msg_ButtonClick(unsigned ctrl_id)
{
    switch(ctrl_id)
    {
        case ID_btSave:
        {
            auto& gameWorld = world_.getWorld();
            const auto mapSize = gameWorld.GetSize();

            // ── Get user input ──
            auto* edFilename = GetCtrl<ctrlEdit>(ID_edtFilename);
            auto* edMapname  = GetCtrl<ctrlEdit>(ID_edtMapname);
            auto* edAuthor   = GetCtrl<ctrlEdit>(ID_edtAuthor);
            if(!edFilename || !edMapname || !edAuthor)
                break;

            std::string filename = edFilename->GetText();
            if(filename.empty())
                filename = "MyMap";

            std::string mapName = edMapname->GetText();
            if(mapName.empty())
                mapName = filename;

            std::string author = edAuthor->GetText();
            if(author.empty())
                author = world_.getAuthor();

            bfs::path outPath = global::userMapsPath / (filename + ".SWD");

            // ── Build header ──
            auto header = std::make_unique<libsiedler2::ArchivItem_Map_Header>();
            header->setWidth(mapSize.x);
            header->setHeight(mapSize.y);
            auto landscape = gameWorld.GetLandscapeType();
            uint8_t gfxSet = 0;
            if(landscape)
                gfxSet = global::worldDesc.landscapes.get(landscape).s2Id;
            header->setGfxSet(gfxSet);
            header->setNumPlayers(static_cast<uint8_t>(gameWorld.GetNumPlayers()));
            header->setName(mapName);
            header->setAuthor(author);

            // ── Store HQ positions in header (max 7 players) ──
            {
                const auto& hqPos = world_.getHQPositions();
                for(unsigned p = 0; p < MAX_PLAYERS && p < 7u; p++)
                {
                    if(hqPos[p].isValid())
                        header->setPlayerHQ(p, hqPos[p].x, hqPos[p].y);
                }
            }

            // ── Build ArchivItem_Map ──
            auto mapItem = std::make_unique<libsiedler2::ArchivItem_Map>();
            mapItem->init(std::move(header));

            // Helper: find s2Id for a terrain descriptor
            auto& desc = gameWorld.GetDescription();
            auto s2IdOfTerrain = [&](DescIdx<TerrainDesc> t) -> uint8_t {
                if(!t) return 0;
                return desc.terrain.get(t).s2Id;
            };

            // Helper: check if a point is a harbor point
            auto isHarbor = [&](MapPoint pt) -> bool {
                return gameWorld.GetNode(pt).harborId.isValid();
            };

            // ── Fill each layer ──
            using libsiedler2::MapLayer;

            for(unsigned layerIdx = 0; layerIdx < libsiedler2::ArchivItem_Map::NUM_SWD_LAYERS; ++layerIdx)
            {
                auto layer = static_cast<MapLayer>(layerIdx);
                auto& layerData = mapItem->getLayer(layer);

                RTTR_FOREACH_PT(MapPoint, mapSize)
                {
                    unsigned i = pt.y * mapSize.x + pt.x;
                    const auto& node = gameWorld.GetNode(pt);

                    switch(layer)
                    {
                        case MapLayer::Altitude:
                            layerData[i] = node.altitude;
                            break;

                        case MapLayer::Terrain1:
                        {
                            uint8_t t = s2IdOfTerrain(node.t1) & 0x3F;
                            if(isHarbor(pt))
                                t |= libsiedler2::HARBOR_MASK;
                            layerData[i] = t;
                            break;
                        }

                        case MapLayer::Terrain2:
                            layerData[i] = s2IdOfTerrain(node.t2) & 0x3F;
                            break;

                        case MapLayer::RoadsOld:
                        {
                            uint8_t r = 0;
                            r |= static_cast<uint8_t>(node.roads[RoadDir::SouthWest]) * 16;
                            r |= static_cast<uint8_t>(node.roads[RoadDir::SouthEast]) * 4;
                            r |= static_cast<uint8_t>(node.roads[RoadDir::East]);
                            layerData[i] = r;
                            break;
                        }

                        case MapLayer::ObjectIndex:
                        {
                            // Check for HQ marker first
                            const auto& hqPos = world_.getHQPositions();
                            unsigned hqPlayer = MAX_PLAYERS;
                            for(unsigned p = 0; p < MAX_PLAYERS; p++)
                            {
                                if(hqPos[p] == pt)
                                {
                                    hqPlayer = p;
                                    break;
                                }
                            }
                            if(hqPlayer < MAX_PLAYERS)
                            {
                                layerData[i] = static_cast<uint8_t>(hqPlayer);
                                break;
                            }

                            noBase* obj = gameWorld.GetNO(pt);
                            uint8_t val = 0;
                            if(obj)
                            {
                                if(auto* tree = dynamic_cast<noTree*>(obj))
                                {
                                    unsigned t = tree->getTreeType();
                                    static const uint8_t baseIdx[9] = {0x30,0x70,0xB0,0xF0,
                                                                       0x30,0x70,0xB0,0xF0,
                                                                       0x30};
                                    val = (t < 9) ? baseIdx[t] : 0;
                                } else if(auto* granite = dynamic_cast<noGranite*>(obj))
                                {
                                    val = granite->GetSize() + 1;
                                } else if(auto* stat = dynamic_cast<noStaticObject*>(obj))
                                {
                                    val = static_cast<uint8_t>(stat->GetItemID() & 0xFF);
                                }
                            }
                            layerData[i] = val;
                            break;
                        }

                        case MapLayer::ObjectType:
                        {
                            // Check for HQ marker first
                            const auto& hqPos = world_.getHQPositions();
                            bool isHQ = false;
                            for(unsigned p = 0; p < MAX_PLAYERS; p++)
                            {
                                if(hqPos[p] == pt)
                                {
                                    isHQ = true;
                                    break;
                                }
                            }
                            if(isHQ)
                            {
                                layerData[i] = 0x80;
                                break;
                            }

                            noBase* obj = gameWorld.GetNO(pt);
                            uint8_t val = 0;
                            if(obj)
                            {
                                if(auto* tree = dynamic_cast<noTree*>(obj))
                                {
                                    unsigned t = tree->getTreeType();
                                    if(t <= 3)
                                        val = 0xC4;
                                    else if(t <= 7)
                                        val = 0xC5;
                                    else
                                        val = 0xC6;
                                } else if(dynamic_cast<noGranite*>(obj))
                                {
                                    val = 0xCC;
                                } else if(dynamic_cast<noStaticObject*>(obj))
                                {
                                    val = 0xC8;
                                }
                            }
                            layerData[i] = val;
                            break;
                        }

                        case MapLayer::Animals:
                        {
                            uint8_t val = 0;
                            for(const auto& fig : gameWorld.GetFigures(pt))
                            {
                                if(fig.GetGOT() != GO_Type::Animal)
                                    continue;
                                auto& animal = static_cast<const noAnimal&>(fig);
                                switch(animal.GetSpecies())
                                {
                                    case Species::RabbitGrey:
                                    case Species::RabbitWhite: val = 1; break;
                                    case Species::Fox:   val = 2; break;
                                    case Species::Stag:  val = 3; break;
                                    case Species::Deer:  val = 4; break;
                                    case Species::Duck:  val = 5; break;
                                    case Species::Sheep: val = 6; break;
                                    default: val = 0; break;
                                }
                                break;
                            }
                            layerData[i] = val;
                            break;
                        }

                        case MapLayer::Unknown7:
                            layerData[i] = 0;
                            break;

                        case MapLayer::BuildingQuality:
                        {
                            uint8_t bq = 0;
                            switch(node.bq)
                            {
                                case BuildingQuality::Nothing: bq = 0; break;
                                case BuildingQuality::Flag:   bq = 1; break;
                                case BuildingQuality::Hut:    bq = 2; break;
                                case BuildingQuality::House:  bq = 3; break;
                                case BuildingQuality::Castle: bq = 4; break;
                                case BuildingQuality::Mine:   bq = 5; break;
                                case BuildingQuality::Harbor: bq = 6; break;
                            }
                            layerData[i] = bq;
                            break;
                        }

                        case MapLayer::Unknown9:
                            layerData[i] = 0x07;
                            break;

                        case MapLayer::Unknown10:
                            layerData[i] = 0;
                            break;

                        case MapLayer::Resources:
                        {
                            uint8_t val = 0;
                            auto res = node.resources;
                            switch(res.getType())
                            {
                                case ResourceType::Nothing: val = 0; break;
                                case ResourceType::Water:
                                    val = 0x20 | (res.getAmount() ? 1 : 0);
                                    break;
                                case ResourceType::Coal:   val = 0x40 | res.getAmount(); break;
                                case ResourceType::Iron:   val = 0x48 | res.getAmount(); break;
                                case ResourceType::Gold:   val = 0x50 | res.getAmount(); break;
                                case ResourceType::Granite: val = 0x58 | res.getAmount(); break;
                                case ResourceType::Fish:
                                    val = 0x80 | (res.getAmount() > 0 ? 4 : 0);
                                    break;
                                default: val = 0; break;
                            }
                            layerData[i] = val;
                            break;
                        }

                        case MapLayer::Shadows:
                            layerData[i] = node.shadow;
                            break;

                        case MapLayer::Lakes:
                            layerData[i] = 0;
                            break;

                        default:
                            layerData[i] = 0;
                            break;
                    }
                }
            }

            // ── Extra animal info ──
            mapItem->extraInfo.clear();

            // ── Write file (wrap in Archiv, same pattern as LoadMAP produces) ──
            libsiedler2::Archiv outArchiv;
            outArchiv.push(std::move(mapItem));
            int ec = libsiedler2::loader::WriteMAP(outPath, outArchiv);
            if(ec == 0)
            {
                std::cout << "Map saved to " << outPath << "\n";
                world_.setFilepath(outPath);
            } else
            {
                std::cerr << "Failed to save map to " << outPath << " (error " << ec << ")\n";
            }

            Close();
            break;
        }
        case ID_btAbort:
            Close();
            break;
    }
}

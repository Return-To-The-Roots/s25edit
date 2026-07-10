// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "EditorWorldLoader.h"
#include "EventManager.h"
#include "GlobalGameSettings.h"
#include "PlayerInfo.h"
#include "RttrForeachPt.h"
#include "globals.h"
#include "lua/GameDataLoader.h"
#include "notifications/NodeNote.h"
#include "world/GameWorld.h"
#include "world/GameWorldViewer.h"
#include "gameData/MapConsts.h"
#include "libsiedler2/ArchivItem_Map.h"
#include <boost/filesystem/path.hpp>
#include <memory>

class EditorWorld
{
    EventManager em_;
    GlobalGameSettings ggs_;
    GameWorld world_;
    std::unique_ptr<GameWorldViewer> viewer_;
    std::string mapName_;
    std::string author_;
    boost::filesystem::path filepath_;

public:
    /// Load an SWD/WLD map file into a new EditorWorld
    static std::unique_ptr<EditorWorld> loadFromSwd(const boost::filesystem::path& filepath);

    EditorWorld(const MapExtent& size, unsigned numPlayers)
        : em_(0), world_(std::vector<PlayerInfo>(numPlayers), ggs_, em_),
          mapName_("Ohne Namen"), author_("Niemand")
    {
        world_.GetDescriptionWriteable() = global::worldDesc;
        loadTexturesForEditorWorld(global::gameDataFilePath.string());
        world_.Init(size);

        // Look up terrain by s2Id (matching old CMap::generateMap defaults)
        DescIdx<TerrainDesc> meadow, water;
        for(unsigned i = 0; i < global::worldDesc.terrain.size(); i++)
        {
            auto s2 = global::worldDesc.terrain.get(DescIdx<TerrainDesc>(i)).s2Id;
            if(s2 == 8)  meadow = DescIdx<TerrainDesc>(i);  // TRIANGLE_TEXTURE_MEADOW1
            if(s2 == 5)  water  = DescIdx<TerrainDesc>(i);  // TRIANGLE_TEXTURE_WATER
        }
        int border = 4;
        RTTR_FOREACH_PT(MapPoint, size)
        {
            auto& node = world_.GetNodeWriteable(pt);
            node.fow[0].visibility = Visibility::Visible;
            bool isBorder = (pt.x < border || pt.x >= size.x - border
                          || pt.y < border || pt.y >= size.y - border);
            node.t1 = isBorder ? water : meadow;
            node.t2 = isBorder ? water : meadow;
        }

        world_.InitAfterLoad();
        viewer_ = std::make_unique<GameWorldViewer>(0, world_);
        viewer_->InitTerrainRenderer();
    }

    GameWorld& getWorld() { return world_; }
    const GameWorld& getWorld() const { return world_; }
    GameWorldViewer& getViewer() { return *viewer_; }
    const GameWorldViewer& getViewer() const { return *viewer_; }

    const std::string& getMapName() const { return mapName_; }
    void setMapName(const std::string& name) { mapName_ = name; }
    const std::string& getAuthor() const { return author_; }
    void setAuthor(const std::string& author) { author_ = author; }
    const boost::filesystem::path& getFilepath() const { return filepath_; }
    void setFilepath(const boost::filesystem::path& fp) { filepath_ = fp; }

    /// HQ position per player (MapPoint::Invalid() = unused)
    using HQArray = std::array<MapPoint, MAX_PLAYERS>;
    const HQArray& getHQPositions() const { return hqPositions_; }
    void setHQPosition(unsigned player, MapPoint pt) { if(player < MAX_PLAYERS) hqPositions_[player] = pt; }
    void clearHQPosition(unsigned player) { if(player < MAX_PLAYERS) hqPositions_[player] = MapPoint::Invalid(); }

    /// Get index into editbob FLAG_* icons for a player (0-6 -> 11-17)
    static int getHQIconForPlayer(unsigned player) { return 11 + player; }

private:
    HQArray hqPositions_{};

    void notifyChanged(MapPoint pt)
    {
        world_.GetNotifications().publish(NodeNote(NodeNote::Altitude, pt));
    }

    void draw(const Position& firstPt, const Position& lastPt) const
    {
        viewer_->GetTerrainRenderer().Draw(firstPt, lastPt, *viewer_, nullptr);
    }
};

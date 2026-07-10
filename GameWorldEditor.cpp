// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "GameWorldEditor.h"
#include "EditorWorldLoader.h"
#include "Loader.h"
#include "defines.h"
#include "world/GameWorldViewer.h"
#include "world/GameWorld.h"
#include "TerrainRenderer.h"
#include "gameData/MapConsts.h"
#include "nodeObjs/noBase.h"
#include "nodeObjs/noTree.h"
#include "nodeObjs/noGranite.h"
#include "nodeObjs/noEnvObject.h"
#include "nodeObjs/noStaticObject.h"
#include "nodeObjs/noAnimal.h"
#include "gameData/AnimalConsts.h"
#include "libsiedler2/Archiv.h"
#include "ogl/glArchivItem_Bitmap.h"
#include "enum_cast.hpp"
#include <glad/glad.h>
#include <SDL.h>

void GameWorldEditor::MoveBy(const DrawPoint& delta)
{
    offset_ += delta;
    WrapOffset();
}

void GameWorldEditor::SetOffset(const DrawPoint& offset)
{
    offset_ = offset;
    WrapOffset();
}

void GameWorldEditor::WrapOffset()
{
    // Wrap offset modulo map size — same as GameWorldView::MoveTo
    DrawPoint size(world_.GetWidth() * TR_W, world_.GetHeight() * TR_H);
    if(size.x && size.y)
    {
        offset_.x %= size.x;
        offset_.y %= size.y;
        if(offset_.x < 0)
            offset_.x += size.x;
        if(offset_.y < 0)
            offset_.y += size.y;
    }
}

void GameWorldEditor::Draw(const Extent& screenSize, const HQArray& hqPositions)
{
    // Same tile range calculation as GameWorldView::CalcFxLx()
    int firstX = offset_.x / TR_W - 1;
    int firstY = offset_.y / TR_H - 1;
    int lastX = (offset_.x + screenSize.x) / TR_W + 1;
    int lastY = (offset_.y + screenSize.y + viewer_.getMaxNodeAltitude() * HEIGHT_FACTOR) / TR_H + 1;

    Position firstPt(firstX, firstY);
    Position lastPt(lastX, lastY);

    const auto mapSize = world_.GetSize();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screenSize.x, screenSize.y, 0, -100, 100);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Translate modelview to scroll position — same as GameWorldView
    glTranslatef(static_cast<GLfloat>(-offset_.x), static_cast<GLfloat>(-offset_.y), 0.0f);

    // Draw terrain
    viewer_.GetTerrainRenderer().Draw(firstPt, lastPt, viewer_, nullptr);

    // Draw objects (trees, animals, buildings, etc.) on top of terrain
    // Same visible range iteration as TerrainRenderer
    const auto& tr = viewer_.GetTerrainRenderer();
    for(int y = firstY; y <= lastY; y++)
    {
        for(int x = firstX; x <= lastX; x++)
        {
            Position rawPt(x, y);
            // Convert to map point with wrapping, get pixel offset for wrap
            Position wrapOffset;
            MapPoint pt = tr.ConvertCoords(rawPt, &wrapOffset);
            if(pt.x >= mapSize.x || pt.y >= mapSize.y)
                continue;

            noBase* obj = world_.GetNO(pt);
            if(!obj)
                continue;

            // World-space pixel position of this node
            Position nodePos = world_.GetNodePos(pt);
            // Apply wrap offset so wrapped copies draw at the correct screen position
            nodePos += wrapOffset;

            // Draw objects using LOADER's MAP00 archive with clean-index lookup
            // Tree types 0-8, where pineapple (type 5) has only 8 frames.
            static const int treeStart[9] = {
                MAPPIC_TREE_PINE,     // 0 pine
                MAPPIC_TREE_BIRCH,    // 1 birch
                MAPPIC_TREE_OAK,      // 2 oak
                MAPPIC_TREE_PALM1,    // 3 palm1
                MAPPIC_TREE_PALM2,    // 4 palm2
                MAPPIC_TREE_PINEAPPLE,// 5 pineapple (8 frames)
                MAPPIC_TREE_CYPRESS,  // 6 cypress
                MAPPIC_TREE_CHERRY,   // 7 cherry
                MAPPIC_TREE_FIR       // 8 fir
            };
            static constexpr int TREE_FRAMES = 8;  // animate 8 frames (standing animation)
            if(auto* tree = dynamic_cast<noTree*>(obj))
            {
                unsigned type = tree->getTreeType();
                if(type < 9)
                {
                    static Uint32 lastTick = 0;
                    static unsigned frame = 0;
                    Uint32 now = SDL_GetTicks();
                    if(now - lastTick > 80) { frame = (frame + 1) % TREE_FRAMES; lastTick = now; }
                    int cleanIdx = treeStart[type] + static_cast<int>(frame);
                    if(cleanIdx >= 0)
                    {
                        int archIdx = editorMapCleanToArchiveIdx(cleanIdx);
                        if(archIdx >= 0)
                        {
                            auto& arch = LOADER.GetArchive("map00");
                            auto* bmp = dynamic_cast<glArchivItem_Bitmap*>(arch.get(static_cast<unsigned>(archIdx)));
                            if(bmp)
                            {
                                bmp->DrawFull(DrawPoint(nodePos.x, nodePos.y));
                                // Draw shadow (image + 100 in map_?_z convention, but MAP00 has shadow at offset 7)
                                // The first 8 frames have shadows at +100 in map_?_z.
                                // For MAP00 we just draw the image without shadow for simplicity.
                            }
                        }
                    }
                }
            } else if(auto* granite = dynamic_cast<noGranite*>(obj))
            {
                int cleanIdx = MAPPIC_GRANITE_1_1 + rttr::enum_cast(granite->GetType()) * 6 + granite->GetSize();
                int archIdx = editorMapCleanToArchiveIdx(cleanIdx);
                if(archIdx >= 0)
                {
                    auto& arch = LOADER.GetArchive("map00");
                    auto* bmp = dynamic_cast<glArchivItem_Bitmap*>(arch.get(static_cast<unsigned>(archIdx)));
                    if(bmp)
                        bmp->DrawFull(DrawPoint(nodePos.x, nodePos.y));
                }
            } else if(auto* env = dynamic_cast<noStaticObject*>(obj))
            {
                // Map objects (file == 0xFFFF) use MAPPIC indices directly
                if(env->GetItemFile() == 0xFFFF)
                {
                    int cleanIdx = env->GetItemID();
                    int archIdx = editorMapCleanToArchiveIdx(cleanIdx);
                    if(archIdx >= 0)
                    {
                        auto& arch = LOADER.GetArchive("map00");
                        auto* bmp = dynamic_cast<glArchivItem_Bitmap*>(arch.get(static_cast<unsigned>(archIdx)));
                        if(bmp)
                            bmp->DrawFull(DrawPoint(nodePos.x, nodePos.y));
                    }
                } else
                {
                    // Non-map objects (mis*bobs) — skip for now
                }
            }
            // Draw animals (figures) at this node — just one static frame for now
            for(auto& fig : world_.GetFigures(pt))
            {
                if(fig.GetGOT() != GO_Type::Animal)
                    continue;
                auto& animal = static_cast<noAnimal&>(fig);
                Species sp = animal.GetSpecies();
                int walkingId = ANIMALCONSTS[sp].walking_id;
                int steps = ANIMALCONSTS[sp].animation_steps;
                // Use East direction (maps to offset 0) and first animation step
                int rawIdx = walkingId + steps * 0 + 0;
                auto& archZ = LOADER.GetArchive("map_0_z");
                if(rawIdx >= 0 && static_cast<unsigned>(rawIdx) < archZ.size())
                {
                    auto* bmp = dynamic_cast<glArchivItem_Bitmap*>(archZ.get(static_cast<unsigned>(rawIdx)));
                    if(bmp)
                        bmp->DrawFull(DrawPoint(nodePos.x, nodePos.y));
                }
            }
        }
    }

    // ── Draw HQ markers (colored flags from editbob) ──
    {
        for(unsigned player = 0; player < MAX_PLAYERS; player++)
        {
            const MapPoint& hqPt = hqPositions[player];
            if(!hqPt.isValid())
                continue;
            // Draw at the node's world position — OpenGL already has the -offset_ translation
            Position nodePos = world_.GetNodePos(hqPt);

            int flagIdx = 11 + static_cast<int>(player); // FLAG_BLUE_DARK..FLAG_ORANGE
            if(auto* flagImg = LOADER.GetImageN("editbob", flagIdx))
            {
                // DrawFull compensates for origin offset (nx/ny); offset slightly above the node
                flagImg->DrawFull(DrawPoint(nodePos.x - 10, nodePos.y - 20));
            }
        }
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

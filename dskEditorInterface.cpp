// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dskEditorInterface.h"
#include "EditorWorld.h"
#include "GameWorldEditor.h"
#include "Loader.h"
#include "WindowManager.h"
#include "driver/KeyEvent.h"
#include "driver/MouseCoords.h"
#include "drivers/VideoDriverWrapper.h"
#include "iwEditorMenu.h"
#include "iwEditorTexture.h"
#include "iwEditorTree.h"
#include "iwEditorResource.h"
#include "iwEditorLandscape.h"
#include "iwEditorAnimal.h"
#include "iwEditorPlayer.h"
#include "iwMinimap.h"
#include "nodeObjs/noEnvObject.h"
#include "nodeObjs/noGranite.h"
#include "nodeObjs/noTree.h"
#include "nodeObjs/noAnimal.h"
#include "gameTypes/AnimalTypes.h"
#include "gameData/AnimalConsts.h"
#include "enum_cast.hpp"
#include "iwEditorCreateWorld.h"
#include "iwEditorCursor.h"
#include "iwLoadMap.h"
#include "iwSaveMap.h"
#include "controls/ctrlButton.h"
#include "defines.h"
#include "world/MapGeometry.h"
#include "world/MapBase.h"
#include "world/World.h"
#include "TerrainRenderer.h"
#include "ogl/glArchivItem_Bitmap.h"
#include "gameData/WorldDescription.h"
#include "gameData/TerrainDesc.h"
#include "nodeObjs/noTree.h"
#include <glad/glad.h>

dskEditorInterface::dskEditorInterface(std::unique_ptr<EditorWorld> world)
    : Desktop(nullptr), world_(std::move(world))
{
    gwEditor_ = std::make_unique<GameWorldEditor>(world_->getViewer(), world_->getWorld());

    const auto screenSize = VIDEODRIVER.GetRenderSize();

    // Bottom menubar tool buttons
    const int cx = static_cast<int>(screenSize.x / 2);
    auto addTool = [&](unsigned id, int texIdx, int xOff) {
        auto* img = LOADER.GetImageN("editio", texIdx);
        if(!img) return;
        AddImageButton(id, DrawPoint(cx + xOff, screenSize.y - 36), Extent(37, 32),
                       TextureColor::Invisible, img, "")->SetBorder(false);
    };
    addTool(ID_btToolHeightRaise, MENUBAR_HEIGHT,    -236);
    addTool(ID_btToolTexture,     MENUBAR_TEXTURE,   -199);
    addTool(ID_btToolTree,        MENUBAR_TREE,      -162);
    addTool(ID_btToolResource,    MENUBAR_RESOURCE,  -125);
    addTool(ID_btToolLandscape,   MENUBAR_LANDSCAPE, -88);
    addTool(ID_btToolAnimal,      MENUBAR_ANIMAL,    -51);
    addTool(ID_btToolPlayer,      MENUBAR_PLAYER,    -14);
    addTool(ID_btToolBuildHelp,   MENUBAR_BUILDHELP, 92);
    addTool(ID_btToolMinimap,     MENUBAR_MINIMAP,   129);
    addTool(ID_btToolNewWorld,    MENUBAR_NEWWORLD,  166);
    addTool(ID_btEditorMenu,     MENUBAR_COMPUTER,  203);

    // Right menubar: Cursor (coord check in Msg_LeftDown), Load/Save buttons
    const int rx = static_cast<int>(screenSize.x);
    const int ry = static_cast<int>(screenSize.y) / 2;

    AddImageButton(ID_btRLoad, DrawPoint(rx - 36, ry + 163), Extent(32, 37),
                   TextureColor::Invisible,
                   LOADER.GetImageN("editio", MENUBAR_BUGKILL), "")->SetBorder(false);
    AddImageButton(ID_btRSave, DrawPoint(rx - 36, ry + 200), Extent(32, 37),
                   TextureColor::Invisible,
                   LOADER.GetImageN("editio", MENUBAR_BUGKILL), "")->SetBorder(false);
}

dskEditorInterface::~dskEditorInterface() = default;

// ── Coordinate helpers ────────────────────────────────────────────────

MapPoint dskEditorInterface::screenToMapPoint(Position screenPos) const
{
    const auto off = gwEditor_->GetOffset();
    const auto mapSize = world_->getWorld().GetSize();

    // Map pixel position (relative to map origin)
    int px = screenPos.x + off.x;
    int py = screenPos.y + off.y;

    // Wrap around map edges
    int mapPx = static_cast<int>(mapSize.x) * TR_W;
    int mapPy = static_cast<int>(mapSize.y) * TR_H;
    if(mapPx > 0) { px %= mapPx; if(px < 0) px += mapPx; }
    if(mapPy > 0) { py %= mapPy; if(py < 0) py += mapPy; }

    // Approximate Y first (row) — round to nearest row
    int y = (py + TR_H / 2) / TR_H;
    y = std::clamp(y, 0, static_cast<int>(mapSize.y) - 1);

    // Then X based on row parity using s25main convention:
    //   even rows: vertices at col * TR_W          (0, 56, 112…)
    //   odd  rows: vertices at col * TR_W + TR_W/2 (28, 84, 140…)
    //
    // For even rows we need rounding (boundary at col*TR_W + TR_W/2):
    //   col = (px + TR_W/2) / TR_W
    // For odd rows truncation suffices (boundary at (col+1)*TR_W):
    //   col = px / TR_W
    int x;
    if(y & 1)
        x = px / TR_W;                 // odd row — truncation
    else
        x = (px + TR_W / 2) / TR_W;    // even row — rounding
    x = std::clamp(x, 0, static_cast<int>(mapSize.x) - 1);

    return MapPoint(static_cast<unsigned short>(x), static_cast<unsigned short>(y));
}

Position dskEditorInterface::mapPointToScreen(MapPoint pt) const
{
    // Use the altitude-adjusted node position (s25client convention)
    uint8_t alt = world_->getWorld().GetNode(pt).altitude;
    Position nodePos = GetNodePos(pt, alt);

    const auto off = gwEditor_->GetOffset();
    nodePos -= off;

    // Toroidal wrap using modulo (cleaner than while loops)
    const int mapPx = static_cast<int>(world_->getWorld().GetWidth() * TR_W);
    const int mapPy = static_cast<int>(world_->getWorld().GetHeight() * TR_H);
    const int sx = static_cast<int>(VIDEODRIVER.GetRenderSize().x);

    if(mapPx > 0)
    {
        nodePos.x %= mapPx;
        if(nodePos.x < 0) nodePos.x += mapPx;
        // If the copy in [0, mapPx) is past the right screen edge,
        // use the copy on the left side instead
        if(nodePos.x > sx) nodePos.x -= mapPx;
    }
    if(mapPy > 0)
    {
        nodePos.y %= mapPy;
        if(nodePos.y < 0) nodePos.y += mapPy;
        if(nodePos.y > static_cast<int>(VIDEODRIVER.GetRenderSize().y))
            nodePos.y -= mapPy;
    }
    return nodePos;
}

// ── Cursor recalculation ──────────────────────────────────────────────

void dskEditorInterface::recalcCursor()
{
    cursorVerts_.clear();
    needRecalcCursor_ = false;

    const auto& world = world_->getWorld();
    const auto mapSize = world.GetSize();
    const int cx = static_cast<int>(cursorPos_.x);
    const int cy = static_cast<int>(cursorPos_.y);
    const bool cyEven = (cy % 2 == 0);
    const auto screenSize = VIDEODRIVER.GetRenderSize();

    for(int dy = -MAX_BRUSH; dy <= MAX_BRUSH; dy++)
    {
        bool dyEven = (std::abs(dy) % 2 == 0);
        int rowCount = dyEven ? (MAX_BRUSH * 2 + 1) : (MAX_BRUSH * 2);
        int dxStart = -MAX_BRUSH;
        int dxEnd = rowCount - MAX_BRUSH;

        for(int dx = dxStart; dx < dxEnd; dx++)
        {
            // Hexagon brush test matching old setupVerticesActivity
            int adx = std::abs(dx);
            int ady = std::abs(dy);
            bool active = (ady <= brushSize_);
            if(active)
            {
                int limit = brushSize_ - ady / 2;
                if(dyEven)
                    active = (adx <= limit);
                else
                {
                    if(dx < 0)
                        active = (adx <= limit);
                    else
                        active = (dx <= limit - 1);
                }
            }

            if(!active)
                continue;

            // MapPoint offset — matching old calculateVertices():
            // Odd offset rows get +1 in X when cursor center is on an odd row
            // (because the hex grid is staggered)
            int offX = dx;
            if(!dyEven && !cyEven)
                offX = dx + 1;

            int wx = cx + offX;
            int wy = cy + dy;
            if(wx < 0) wx += mapSize.x;
            else if(wx >= static_cast<int>(mapSize.x)) wx -= mapSize.x;
            if(wy < 0) wy += mapSize.y;
            else if(wy >= static_cast<int>(mapSize.y)) wy -= mapSize.y;

            auto pt = MapPoint(static_cast<unsigned short>(wx), static_cast<unsigned short>(wy));
            Position screenPos = mapPointToScreen(pt);

            // Cull off-screen
            if(screenPos.x < -50 || screenPos.x > static_cast<int>(screenSize.x + 50)
               || screenPos.y < -50 || screenPos.y > static_cast<int>(screenSize.y + 50))
                continue;

            CursorVertex cv;
            cv.pt = pt;
            cv.screenPos = screenPos;
            cv.active = true;
            cursorVerts_.push_back(cv);
        }
    }
}

// ── Effective mode ────────────────────────────────────────────────────

EditorToolMode dskEditorInterface::effectiveMode() const
{
    Uint32 mod = SDL_GetModState();
    bool shiftHeld = (mod & KMOD_SHIFT) != 0;
    bool altHeld   = (mod & KMOD_ALT) != 0;
    bool ctrlHeld  = (mod & KMOD_CTRL) != 0;

    EditorToolMode eff = mode_;
    if(ctrlHeld)
        return EditorToolMode::Cut;
    if(shiftHeld)
    {
        if(eff == EditorToolMode::HeightRaise)
            eff = EditorToolMode::HeightReduce;
        else if(eff == EditorToolMode::Resource)
            eff = EditorToolMode::HeightReduce;
        else if(eff == EditorToolMode::Flag)
            eff = EditorToolMode::Cut;
    }
    if(altHeld && eff == EditorToolMode::HeightRaise)
        eff = EditorToolMode::HeightPlane;
    return eff;
}

// ── Cursor drawing ────────────────────────────────────────────────────

void dskEditorInterface::drawCursor()
{
    if(needRecalcCursor_)
        recalcCursor();

    EditorToolMode drawMode = effectiveMode();

    int sym1 = -1, sym2 = -1;
    switch(drawMode)
    {
        case EditorToolMode::Cut:         sym1 = CURSOR_SYMBOL_SCISSORS; break;
        case EditorToolMode::Tree:        sym1 = CURSOR_SYMBOL_TREE; break;
        case EditorToolMode::HeightRaise:  sym1 = CURSOR_SYMBOL_ARROW_UP; break;
        case EditorToolMode::HeightReduce: sym1 = CURSOR_SYMBOL_ARROW_DOWN; break;
        case EditorToolMode::HeightPlane:
            sym1 = CURSOR_SYMBOL_ARROW_UP;
            sym2 = CURSOR_SYMBOL_ARROW_DOWN;
            break;
        case EditorToolMode::Texture:    sym1 = CURSOR_SYMBOL_TEXTURE; break;
        case EditorToolMode::Landscape:  sym1 = CURSOR_SYMBOL_LANDSCAPE; break;
        case EditorToolMode::Flag:       sym1 = CURSOR_SYMBOL_FLAG; break;
        case EditorToolMode::Resource:   sym1 = CURSOR_SYMBOL_PICKAXE_PLUS; break;
        case EditorToolMode::Animal:     sym1 = CURSOR_SYMBOL_ANIMAL; break;
    }

    for(const auto& cv : cursorVerts_)
    {
        if(!cv.active) continue;
        if(sym1 >= 0)
        {
            if(auto* img = LOADER.GetImageN("editbob", sym1))
                img->DrawFull(DrawPoint(cv.screenPos.x - 10, cv.screenPos.y - 10));
        }
        if(sym2 >= 0)
        {
            if(auto* img = LOADER.GetImageN("editbob", sym2))
                img->DrawFull(DrawPoint(cv.screenPos.x, cv.screenPos.y - 7));
        }
    }
}

// ── Height propagation ───────────────────────────────────────────────

/// After changing a vertex's altitude, recursively adjust neighbours when
/// the height difference exceeds MAX_HEIGHT_DIFF steps, preventing cliffs.
/// This mirrors the old CMap::modifyHeightRaise/modifyHeightReduce logic.
static constexpr int MAX_HEIGHT_DIFF = 5;

static void propagateRaise(MapPoint pt, GameWorld& world, int maxAltitude)
{
    auto& node = world.GetNodeWriteable(pt);
    if(node.altitude >= maxAltitude)
        return;
    world.ChangeAltitude(pt, node.altitude + 1);

    // Check all 6 neighbours — if they're too low, raise them too
    for(const auto& nb : world.GetNeighbours(pt))
    {
        auto& nbNode = world.GetNodeWriteable(nb);
        if(nbNode.altitude + MAX_HEIGHT_DIFF < node.altitude)
            propagateRaise(nb, world, maxAltitude);
    }
}

static void propagateLower(MapPoint pt, GameWorld& world, int minAltitude)
{
    auto& node = world.GetNodeWriteable(pt);
    if(node.altitude <= minAltitude)
        return;
    world.ChangeAltitude(pt, node.altitude - 1);

    for(const auto& nb : world.GetNeighbours(pt))
    {
        auto& nbNode = world.GetNodeWriteable(nb);
        if(node.altitude + MAX_HEIGHT_DIFF < nbNode.altitude)
            propagateLower(nb, world, minAltitude);
    }
}

// ── Tool application ──────────────────────────────────────────────────

void dskEditorInterface::applyTool()
{
    auto& world = world_->getWorld();

    EditorToolMode effMode = effectiveMode();

    if(needRecalcCursor_)
        recalcCursor();

    switch(effMode)
    {
        case EditorToolMode::HeightRaise:
        {
            int maxAlt = world.GetNode(cursorPos_).altitude;
            for(const auto& cv : cursorVerts_)
            {
                if(!cv.active) continue;
                auto alt = world.GetNode(cv.pt).altitude;
                if(alt > maxAlt) maxAlt = alt;
            }
            // Raise all brush vertices (cap at 0x3C) with propagation
            for(const auto& cv : cursorVerts_)
            {
                if(!cv.active) continue;
                auto& node = world.GetNodeWriteable(cv.pt);
                if(node.altitude < 0x3C)
                    propagateRaise(cv.pt, world, 0x3C);
            }
            break;
        }
        case EditorToolMode::HeightReduce:
        {
            for(const auto& cv : cursorVerts_)
            {
                if(!cv.active) continue;
                auto& node = world.GetNodeWriteable(cv.pt);
                if(node.altitude > 0)
                    propagateLower(cv.pt, world, 0);
            }
            break;
        }
        case EditorToolMode::HeightPlane:
        {
            int sum = 0, count = 0;
            for(const auto& cv : cursorVerts_)
            {
                if(!cv.active) continue;
                sum += world.GetNode(cv.pt).altitude;
                count++;
            }
            if(count > 0)
            {
                uint8_t avg = static_cast<uint8_t>(sum / count);
                for(const auto& cv : cursorVerts_)
                {
                    if(!cv.active) continue;
                    auto& node = world.GetNodeWriteable(cv.pt);
                    uint8_t alt = node.altitude;
                    if(alt < avg)
                        propagateRaise(cv.pt, world, avg);
                    else if(alt > avg)
                        propagateLower(cv.pt, world, avg);
                }
            }
            break;
        }
        case EditorToolMode::Texture:
        {
            // Map legacy s2Id (TRIANGLE_TEXTURE_* value) to DescIdx<TerrainDesc>
            auto terrainByS2Id = [&](int s2Id) -> DescIdx<TerrainDesc> {
                for(DescIdx<TerrainDesc> i(0); i.value < global::worldDesc.terrain.size(); i.value++)
                    if(global::worldDesc.terrain.get(i).s2Id == s2Id)
                        return i;
                return DescIdx<TerrainDesc>();
            };
            auto getTerrainOrMix = [&](int s2Id) -> DescIdx<TerrainDesc> {
                // MEADOW_MIXED is a sentinel: pick randomly from MEADOW1/2/3
                if(s2Id == TRIANGLE_TEXTURE_MEADOW_MIXED)
                {
                    int ids[] = {TRIANGLE_TEXTURE_MEADOW1, TRIANGLE_TEXTURE_MEADOW2, TRIANGLE_TEXTURE_MEADOW3};
                    return terrainByS2Id(ids[rand() % 3]);
                }
                return terrainByS2Id(s2Id);
            };
            for(const auto& cv : cursorVerts_)
            {
                if(!cv.active) continue;
                auto& node = world.GetNodeWriteable(cv.pt);
                auto terrain = getTerrainOrMix(modeContent_);
                if(!terrain) continue;
                node.t1 = terrain;
                node.t2 = terrain;
            }
            pendingTerrainRefresh_ = true;
            break;
        }
        case EditorToolMode::Tree:
        {
            for(const auto& cv : cursorVerts_)
            {
                if(!cv.active) continue;
                // Skip if there's already an object
                if(world.GetNode(cv.pt).obj != nullptr)
                    continue;
                int treeType = modeContent2_;
                if(treeType == -1) // mixed wood
                    treeType = rand() % 3; // pine, birch, oak
                else if(treeType == -2) // mixed palm
                    treeType = 3 + rand() % 5; // palm1, palm2, pineapple, cypress, cherry
                else if(treeType < 0 || treeType > 8)
                    treeType = 0;
                world.SetNO(cv.pt, new noTree(cv.pt, static_cast<unsigned char>(treeType), 3), true);
            }
            break;
        }
        case EditorToolMode::Landscape:
        {
            int mapPicId = modeContent2_;
            if(mapPicId <= 0)
                break;
            for(const auto& cv : cursorVerts_)
            {
                if(!cv.active) continue;
                if(world.GetNode(cv.pt).obj != nullptr)
                    continue;
                // Granite range in MAPPIC: 170-181
                if(mapPicId >= MAPPIC_GRANITE_1_1 && mapPicId <= MAPPIC_GRANITE_2_6)
                {
                    int graniteType = (mapPicId - MAPPIC_GRANITE_1_1) / 6;
                    int graniteSize = (mapPicId - MAPPIC_GRANITE_1_1) % 6;
                    world.SetNO(cv.pt, new noGranite(static_cast<GraniteType>(graniteType),
                                                      static_cast<unsigned char>(graniteSize)), true);
                } else
                {
                    world.SetNO(cv.pt, new noEnvObject(cv.pt, static_cast<unsigned short>(mapPicId), 0xFFFF), true);
                }
            }
            break;
        }
        case EditorToolMode::Cut:
        {
            for(const auto& cv : cursorVerts_)
            {
                if(!cv.active) continue;
                auto& node = world.GetNodeWriteable(cv.pt);
                // Destroy node object (trees, granite, landscape objects)
                if(node.obj != nullptr)
                    world.DestroyNO(cv.pt, false);
                // Remove figures (animals)
                if(!node.figures.empty())
                {
                    // Destroy each figure manually
                    for(auto it = node.figures.begin(); it != node.figures.end();)
                    {
                        auto* fig = it->release();
                        it = node.figures.erase(it);
                        fig->Destroy();
                        delete fig;
                    }
                }
            }
            break;
        }
        case EditorToolMode::Animal:
        {
            int species = modeContent2_;
            if(species < 0 || species > rttr::enum_cast(Species::Sheep))
                break;
            for(const auto& cv : cursorVerts_)
            {
                if(!cv.active) continue;
                // Animals are figures, not node objects. AddFigure adds to the figures list.
                // But only if there isn't already an animal there (skip occupied nodes)
                auto& node = world.GetNodeWriteable(cv.pt);
                if(!node.figures.empty())
                    continue;
                world.AddFigure(cv.pt,
                    std::make_unique<noAnimal>(static_cast<Species>(species), cv.pt));
            }
            break;
        }
        case EditorToolMode::Flag:
        {
            // Place HQ for current player at cursor, or remove if already there
            if(world_ && cursorPos_.isValid())
            {
                const auto& hqPos = world_->getHQPositions();
                // Check if there's already an HQ at this position
                bool alreadyHQ = false;
                for(unsigned p = 0; p < MAX_PLAYERS; p++)
                {
                    if(hqPos[p] == cursorPos_)
                    {
                        // Remove it
                        world_->clearHQPosition(p);
                        alreadyHQ = true;
                        break;
                    }
                }
                if(!alreadyHQ)
                    world_->setHQPosition(static_cast<unsigned>(currentPlayer_), cursorPos_);
            }
            break;
        }
        case EditorToolMode::Resource:
        default:
            break;
    }
}

// ── Event handlers ────────────────────────────────────────────────────

void dskEditorInterface::Msg_PaintBefore()
{
    Desktop::Msg_PaintBefore();

    if(!world_)
        return;

    const auto screenSize = VIDEODRIVER.GetRenderSize();
    const int w = static_cast<int>(screenSize.x);
    const int h = static_cast<int>(screenSize.y);

    // ── Refresh terrain renderer if world data was modified (e.g. texture painting) ──
    if(pendingTerrainRefresh_)
    {
        pendingTerrainRefresh_ = false;
        world_->getViewer().GetTerrainRenderer().GenerateOpenGL(world_->getViewer());
    }

    // ── Terrain via GameWorldEditor ──
    if(gwEditor_ && world_)
        gwEditor_->Draw(screenSize, world_->getHQPositions());

    // ── Cursor sprites (under UI chrome) ──
    drawCursor();

    // ── Editor frame ──
    auto* frameTex = LOADER.GetImageN("editres", MAINFRAME_640_480);
    if(!frameTex) return;

    if(w == 640 && h == 480)
        frameTex->DrawFull(DrawPoint(0, 0));
    else if(w == 800 && h == 600)
        LOADER.GetImageN("editres", MAINFRAME_800_600)->DrawFull(DrawPoint(0, 0));
    else if(w == 1024 && h == 768)
        LOADER.GetImageN("editres", MAINFRAME_1024_768)->DrawFull(DrawPoint(0, 0));
    else if(w == 1280 && h == 1024)
    {
        LOADER.GetImageN("editres", MAINFRAME_LEFT_1280_1024)->DrawFull(DrawPoint(0, 0));
        LOADER.GetImageN("editres", MAINFRAME_RIGHT_1280_1024)->DrawFull(DrawPoint(640, 0));
    } else
    {
        frameTex->Draw(Rect(0, 0, 150, 150), Rect(0, 0, 150, 150), COLOR_WHITE);
        frameTex->Draw(Rect(0, h - 150, 150, 150), Rect(0, 480 - 150, 150, 150), COLOR_WHITE);
        frameTex->Draw(Rect(w - 150, 0, 150, 150), Rect(640 - 150, 0, 150, 150), COLOR_WHITE);
        frameTex->Draw(Rect(w - 150, h - 150, 150, 150), Rect(640 - 150, 480 - 150, 150, 150), COLOR_WHITE);
        for(unsigned x = 150; x + 150 < static_cast<unsigned>(w); x += 150)
        {
            frameTex->Draw(Rect(static_cast<int>(x), 0, 150, 12), Rect(150, 0, 150, 12), COLOR_WHITE);
            frameTex->Draw(Rect(static_cast<int>(x), h - 12, 150, 12), Rect(150, 0, 150, 12), COLOR_WHITE);
        }
        for(unsigned y = 150; y + 150 < static_cast<unsigned>(h); y += 150)
        {
            frameTex->Draw(Rect(0, static_cast<int>(y), 12, 150), Rect(0, 150, 12, 150), COLOR_WHITE);
            frameTex->Draw(Rect(w - 12, static_cast<int>(y), 12, 150), Rect(0, 150, 12, 150), COLOR_WHITE);
        }
    }

    auto drawStat = [&](int idx, int x, int y) {
        if(auto* img = LOADER.GetImageN("editres", idx))
        {
            auto sz = img->GetSize();
            img->DrawPart(Rect(x, y, sz.x, sz.y), DrawPoint(0, 0));
        }
    };
    drawStat(STATUE_UP_LEFT,    12, 12);
    drawStat(STATUE_UP_RIGHT,   w - static_cast<int>(LOADER.GetImageN("editres", STATUE_UP_RIGHT)->GetSize().x) - 12, 12);
    drawStat(STATUE_DOWN_LEFT,  12, h - static_cast<int>(LOADER.GetImageN("editres", STATUE_DOWN_LEFT)->GetSize().y) - 12);
    drawStat(STATUE_DOWN_RIGHT, w - static_cast<int>(LOADER.GetImageN("editres", STATUE_DOWN_RIGHT)->GetSize().x) - 12,
             h - static_cast<int>(LOADER.GetImageN("editres", STATUE_DOWN_RIGHT)->GetSize().y) - 12);

    // ── Menubars ──
    const int cx = w / 2;
    auto* menubar = LOADER.GetImageN("editres", MENUBAR);
    if(!menubar) return;
    auto mbSz = menubar->GetSize();

    menubar->DrawPart(Rect((w - mbSz.x) / 2, h - mbSz.y, mbSz.x, mbSz.y), DrawPoint(0, 0));

    auto* slotBg = LOADER.GetImageN("editio", BUTTON_GREEN1_DARK);
    if(slotBg)
    {
        for(int xOff = -236; xOff <= -14; xOff += 37)
            slotBg->Draw(Rect(cx + xOff, h - 36, 37, 32), Rect(0, 0, 37, 32), COLOR_WHITE);
        for(int xOff = 92; xOff <= 203; xOff += 37)
            slotBg->Draw(Rect(cx + xOff, h - 36, 37, 32), Rect(0, 0, 37, 32), COLOR_WHITE);
    }

    // Right menubar (rotated)
    {
        const int rx = w;
        const int ry = h / 2;
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glTranslatef(static_cast<float>(rx - mbSz.y / 2 + 1), static_cast<float>(ry + 2), 0.f);
        glRotatef(270.f, 0.f, 0.f, 1.f);
        menubar->DrawFull(Rect(-static_cast<int>(mbSz.x) / 2, -static_cast<int>(mbSz.y) / 2, mbSz.x, mbSz.y));
        glPopMatrix();

        if(slotBg)
        {
            for(int yOff : {-239, -202, -165, -128, -22, 15, 52, 89, 126, 163, 200})
                slotBg->Draw(Rect(rx - 36, ry + yOff, 32, 37), Rect(0, 0, 32, 37), COLOR_WHITE);
        }

        // Draw arrow indicators — glArchivItem_Bitmap::Draw subtracts GetOrigin()
        // (the libsiedler2 anchor nx/ny), so we add it back to match the old
        // Texture::draw(Position) which did not adjust for origin.
        if(auto* texUp = LOADER.GetImageN("editbob", CURSOR_SYMBOL_ARROW_UP))
            texUp->DrawFull(DrawPoint(rx - 33 + texUp->getNx(), ry - 237 + texUp->getNy()));
        if(auto* texDn = LOADER.GetImageN("editbob", CURSOR_SYMBOL_ARROW_DOWN))
            texDn->DrawFull(DrawPoint(rx - 20 + texDn->getNx(), ry - 235 + texDn->getNy()));
        if(auto* texDn2 = LOADER.GetImageN("editbob", CURSOR_SYMBOL_ARROW_DOWN))
            texDn2->DrawFull(DrawPoint(rx - 33 + texDn2->getNx(), ry - 220 + texDn2->getNy()));
        if(auto* texUp2 = LOADER.GetImageN("editbob", CURSOR_SYMBOL_ARROW_UP))
            texUp2->DrawFull(DrawPoint(rx - 20 + texUp2->getNx(), ry - 220 + texUp2->getNy()));
    }
}

void dskEditorInterface::Draw_()
{
    Desktop::Draw_();
}

void dskEditorInterface::Msg_ButtonClick(unsigned ctrl_id)
{
    switch(ctrl_id)
    {
        case ID_btToolHeightRaise:
            mode_ = EditorToolMode::HeightRaise;
            needRecalcCursor_ = true;
            break;
        case ID_btToolTexture:
            mode_ = EditorToolMode::Texture;
            WINDOWMANAGER.Show(std::make_unique<iwEditorTexture>([this](int s2Id) noexcept {
                modeContent_ = s2Id;
            }));
            break;
        case ID_btToolTree:
            mode_ = EditorToolMode::Tree;
            WINDOWMANAGER.Show(std::make_unique<iwEditorTree>([this](int treeType) noexcept {
                modeContent2_ = treeType;
            }));
            break;
        case ID_btToolResource:
            mode_ = EditorToolMode::Resource;
            WINDOWMANAGER.Show(std::make_unique<iwEditorResource>());
            break;
        case ID_btToolLandscape:
            mode_ = EditorToolMode::Landscape;
            WINDOWMANAGER.Show(std::make_unique<iwEditorLandscape>([this](int mapPicId) noexcept {
                modeContent2_ = mapPicId;
            }));
            break;
        case ID_btToolAnimal:
            mode_ = EditorToolMode::Animal;
            WINDOWMANAGER.Show(std::make_unique<iwEditorAnimal>([this](int species) noexcept {
                modeContent2_ = species;
            }));
            break;
        case ID_btToolPlayer:
            mode_ = EditorToolMode::Flag;
            WINDOWMANAGER.Show(std::make_unique<iwEditorPlayer>(
                currentPlayer_,
                [this](int player) noexcept { currentPlayer_ = player; }));
            break;
        case ID_btToolBuildHelp:
            break;
        case ID_btToolMinimap:
            WINDOWMANAGER.Show(std::make_unique<iwMinimap>());
            break;
        case ID_btToolNewWorld:
            WINDOWMANAGER.Show(std::make_unique<iwEditorCreateWorld>());
            break;
        case ID_btEditorMenu:
            if(world_)
                WINDOWMANAGER.Show(std::make_unique<iwEditorMenu>(*world_));
            break;
        case ID_btRLoad:
            WINDOWMANAGER.Show(std::make_unique<iwLoadMap>());
            break;
        case ID_btRSave:
            if(world_)
                WINDOWMANAGER.Show(std::make_unique<iwSaveMap>(*world_));
            break;
    }
}

bool dskEditorInterface::Msg_LeftDown(const MouseCoords& mc)
{
    const auto screenSize = VIDEODRIVER.GetRenderSize();
    const int rx = static_cast<int>(screenSize.x);
    const int ry = static_cast<int>(screenSize.y) / 2;

    // Right-menubar: cursor menu slot (coord check matching CMap.cpp)
    if(mc.pos.x >= rx - 37 && mc.pos.x <= rx
       && mc.pos.y >= ry - 239 && mc.pos.y <= ry - 202)
    {
        WINDOWMANAGER.Show(std::make_unique<iwEditorCursor>());
        return true;
    }

    // Check if click is on the map area (not UI chrome)
    if(mc.pos.x >= 0 && mc.pos.x < static_cast<int>(screenSize.x) - 37
       && mc.pos.y >= 0 && mc.pos.y < static_cast<int>(screenSize.y) - 36)
    {
        cursorPos_ = screenToMapPoint(mc.pos);
        needRecalcCursor_ = true;
        isModifying_ = true;
        applyTool();
        return true;
    }
    return Desktop::Msg_LeftDown(mc);
}

bool dskEditorInterface::Msg_LeftUp(const MouseCoords& mc)
{
    isModifying_ = false;
    return Desktop::Msg_LeftUp(mc);
}

bool dskEditorInterface::Msg_MouseMove(const MouseCoords& mc)
{
    cursorPos_ = screenToMapPoint(mc.pos);
    needRecalcCursor_ = true;

    if(isModifying_)
        applyTool();

    return true;
}

bool dskEditorInterface::Msg_KeyDown(const KeyEvent& ke)
{
    // ── Arrow keys for scrolling ──
    if(ke.kt == KeyType::Left || ke.kt == KeyType::Right
       || ke.kt == KeyType::Up || ke.kt == KeyType::Down)
    {
        static constexpr int scrollSpeed = 20;
        if(!gwEditor_) return false;
        DrawPoint delta(0, 0);
        switch(ke.kt)
        {
            case KeyType::Left:  delta.x = -scrollSpeed; break;
            case KeyType::Right: delta.x = scrollSpeed; break;
            case KeyType::Up:    delta.y = -scrollSpeed; break;
            case KeyType::Down:  delta.y = scrollSpeed; break;
            default: break;
        }
        gwEditor_->MoveBy(delta);
        needRecalcCursor_ = true;
        return true;
    }

    // ── Brush size: +/- and number keys ──
    if(ke.kt == KeyType::Char)
    {
        char32_t c = ke.c;
        if(c == U'+' || c == U'=')
        {
            if(brushSize_ < MAX_BRUSH)
            {
                brushSize_++;
                needRecalcCursor_ = true;
            }
            return true;
        }
        if(c == U'-' || c == U'_')
        {
            if(brushSize_ > 0)
            {
                brushSize_--;
                needRecalcCursor_ = true;
            }
            return true;
        }
        if(c >= U'0' && c <= U'9')
        {
            int n = static_cast<int>(c - U'0');
            if(n <= MAX_BRUSH)
            {
                brushSize_ = n;
                needRecalcCursor_ = true;
            }
            return true;
        }
    }

    // ── Space: toggle build help (visual only for now) ──
    if(ke.kt == KeyType::Space)
    {
        // RenderBuildHelp toggle would go here once build-help rendering exists
        return true;
    }

    // ── F11: toggle borders ──
    if(ke.kt == KeyType::F11)
    {
        // RenderBorders toggle would go here once border rendering exists
        return true;
    }

    // ── F1: help menu ──
    if(ke.kt == KeyType::F1)
    {
        // callback::EditorHelpMenu would open a help window
        return true;
    }

    // ── Ctrl+Z / Ctrl+Y undo/redo (placeholder) ──
    Uint32 mod = SDL_GetModState();
    if((mod & KMOD_CTRL) && ke.kt == KeyType::Char && ke.c == U'z')
    {
        // Undo
        return true;
    }
    if((mod & KMOD_CTRL) && ke.kt == KeyType::Char && ke.c == U'y')
    {
        // Redo
        return true;
    }

    return false;
}

bool dskEditorInterface::Msg_WheelUp(const MouseCoords&) { return false; }
bool dskEditorInterface::Msg_WheelDown(const MouseCoords&) { return false; }

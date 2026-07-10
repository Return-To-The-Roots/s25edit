// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "desktops/Desktop.h"
#include "gameTypes/MapCoordinates.h"
#include <memory>
#include <vector>

class EditorWorld;
class GameWorldEditor;

struct CursorVertex
{
    MapPoint pt;
    Position screenPos;
    bool active;
};

/// Editor tool modes (matches old EDITOR_MODE_* semantics)
enum class EditorToolMode
{
    HeightRaise,
    HeightReduce,
    HeightPlane,
    Texture,
    Tree,
    Landscape,
    Resource,
    Animal,
    Flag,
    Cut
};

class dskEditorInterface : public Desktop
{
public:
    dskEditorInterface(std::unique_ptr<EditorWorld> world);
    ~dskEditorInterface() override;

    void Msg_ButtonClick(unsigned ctrl_id) override;
    void Msg_PaintBefore() override;
    bool Msg_LeftDown(const MouseCoords& mc) override;
    bool Msg_LeftUp(const MouseCoords& mc) override;
    bool Msg_MouseMove(const MouseCoords& mc) override;
    bool Msg_KeyDown(const KeyEvent& ke) override;
    bool Msg_WheelUp(const MouseCoords& mc) override;
    bool Msg_WheelDown(const MouseCoords& mc) override;

protected:
    void Draw_() override;

private:
    /// Convert screen pixel position to the nearest MapPoint
    MapPoint screenToMapPoint(Position screenPos) const;
    /// Get pixel position of a map point (screen-space, accounting for scroll offset)
    Position mapPointToScreen(MapPoint pt) const;
    /// Recalculate cursor vertices based on cursorPos_ and brushSize_
    void recalcCursor();
    /// Draw the cursor sprites at all active vertices
    void drawCursor();
    /// Apply the current tool to the vertices under the cursor
    void applyTool();
    /// Effective mode considering modifier keys (read from SDL_GetModState())
    EditorToolMode effectiveMode() const;

    std::unique_ptr<EditorWorld> world_;
    std::unique_ptr<GameWorldEditor> gwEditor_;

    // ── Editor state ──
    EditorToolMode mode_ = EditorToolMode::HeightRaise;
    int brushSize_ = 1;                 // 0..MAX_BRUSH
    MapPoint cursorPos_{0, 0};
    bool isModifying_ = false;
    int modeContent_ = 8;               // selected terrain s2Id (default TRIANGLE_TEXTURE_MEADOW1)
    int modeContent2_ = 0;              // secondary: tree type (0-8) or -1/-2 for mixed
    int currentPlayer_ = 0;             // active player for HQ/flag placement (0-6)
    bool pendingTerrainRefresh_ = false;

    // Cursor vertex field
    static constexpr int MAX_BRUSH = 10;
    std::vector<CursorVertex> cursorVerts_;
    bool needRecalcCursor_ = true;

    enum ControlIds
    {
        // Bottom menubar tools (matching CMap.cpp button order)
        ID_btToolHeightRaise,
        ID_btToolTexture,
        ID_btToolTree,
        ID_btToolResource,
        ID_btToolLandscape,
        ID_btToolAnimal,
        ID_btToolPlayer,
        ID_btToolBuildHelp,  // toggle, no window
        ID_btToolMinimap,
        ID_btToolNewWorld,
        ID_btEditorMenu,     // main menu
        ID_FIRST_TOOL = ID_btToolHeightRaise,
        ID_LAST_TOOL = ID_btEditorMenu,
        // Right menubar
        ID_btRLoad,
        ID_btRSave,
    };
};

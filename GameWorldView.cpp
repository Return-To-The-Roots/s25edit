// Copyright (C) 2009 - 2025 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "GameWorldView.h"
#include "CGame.h"
#include "CMap.h"
#include "CSurface.h"
#include "TerrainRenderer.h"
#include "defines.h"
#include "globals.h"

namespace EditorWorldView {

static void drawVertexOverlay(SDL_Surface* target, int screenX, int screenY, const MapNode& node)
{
    int objIdx = 0;
    switch(node.objectInfo)
    {
        // tree
        case 0xC4:
            if(node.objectType >= 0x30 && node.objectType <= 0x37)
            {
                if(node.objectType + TerrainRenderer::roundCount() > 0x37)
                    objIdx = MAPPIC_TREE_PINE + (node.objectType - 0x30) + (TerrainRenderer::roundCount() - 7);
                else
                    objIdx = MAPPIC_TREE_PINE + (node.objectType - 0x30) + TerrainRenderer::roundCount();

            } else if(node.objectType >= 0x70 && node.objectType <= 0x77)
            {
                if(node.objectType + TerrainRenderer::roundCount() > 0x77)
                    objIdx = MAPPIC_TREE_BIRCH + (node.objectType - 0x70) + (TerrainRenderer::roundCount() - 7);
                else
                    objIdx = MAPPIC_TREE_BIRCH + (node.objectType - 0x70) + TerrainRenderer::roundCount();
            } else if(node.objectType >= 0xB0 && node.objectType <= 0xB7)
            {
                if(node.objectType + TerrainRenderer::roundCount() > 0xB7)
                    objIdx = MAPPIC_TREE_OAK + (node.objectType - 0xB0) + (TerrainRenderer::roundCount() - 7);
                else
                    objIdx = MAPPIC_TREE_OAK + (node.objectType - 0xB0) + TerrainRenderer::roundCount();
            } else if(node.objectType >= 0xF0 && node.objectType <= 0xF7)
            {
                if(node.objectType + TerrainRenderer::roundCount() > 0xF7)
                    objIdx = MAPPIC_TREE_PALM1 + (node.objectType - 0xF0) + (TerrainRenderer::roundCount() - 7);
                else
                    objIdx = MAPPIC_TREE_PALM1 + (node.objectType - 0xF0) + TerrainRenderer::roundCount();
            }
            break;
        // tree
        case 0xC5:
            if(node.objectType >= 0x30 && node.objectType <= 0x37)
            {
                if(node.objectType + TerrainRenderer::roundCount() > 0x37)
                    objIdx = MAPPIC_TREE_PALM2 + (node.objectType - 0x30) + (TerrainRenderer::roundCount() - 7);
                else
                    objIdx = MAPPIC_TREE_PALM2 + (node.objectType - 0x30) + TerrainRenderer::roundCount();

            } else if(node.objectType >= 0x70 && node.objectType <= 0x77)
            {
                if(node.objectType + TerrainRenderer::roundCount() > 0x77)
                    objIdx = MAPPIC_TREE_PINEAPPLE + (node.objectType - 0x70) + (TerrainRenderer::roundCount() - 7);
                else
                    objIdx = MAPPIC_TREE_PINEAPPLE + (node.objectType - 0x70) + TerrainRenderer::roundCount();
            } else if(node.objectType >= 0xB0 && node.objectType <= 0xB7)
            {
                if(node.objectType + TerrainRenderer::roundCount() > 0xB7)
                    objIdx = MAPPIC_TREE_CYPRESS + (node.objectType - 0xB0) + (TerrainRenderer::roundCount() - 7);
                else
                    objIdx = MAPPIC_TREE_CYPRESS + (node.objectType - 0xB0) + TerrainRenderer::roundCount();
            } else if(node.objectType >= 0xF0 && node.objectType <= 0xF7)
            {
                if(node.objectType + TerrainRenderer::roundCount() > 0xF7)
                    objIdx = MAPPIC_TREE_CHERRY + (node.objectType - 0xF0) + (TerrainRenderer::roundCount() - 7);
                else
                    objIdx = MAPPIC_TREE_CHERRY + (node.objectType - 0xF0) + TerrainRenderer::roundCount();
            }
            break;
        // tree
        case 0xC6:
            if(node.objectType >= 0x30 && node.objectType <= 0x37)
            {
                if(node.objectType + TerrainRenderer::roundCount() > 0x37)
                    objIdx = MAPPIC_TREE_FIR + (node.objectType - 0x30) + (TerrainRenderer::roundCount() - 7);
                else
                    objIdx = MAPPIC_TREE_FIR + (node.objectType - 0x30) + TerrainRenderer::roundCount();
            }
            break;
        // landscape
        case 0xC8:
            switch(node.objectType)
            {
                case 0x00: objIdx = MAPPIC_MUSHROOM1; break;
                case 0x01: objIdx = MAPPIC_MUSHROOM2; break;
                case 0x02: objIdx = MAPPIC_STONE1; break;
                case 0x03: objIdx = MAPPIC_STONE2; break;
                case 0x04: objIdx = MAPPIC_STONE3; break;
                case 0x05: objIdx = MAPPIC_TREE_TRUNK_DEAD; break;
                case 0x06: objIdx = MAPPIC_TREE_DEAD; break;
                case 0x07: objIdx = MAPPIC_BONE1; break;
                case 0x08: objIdx = MAPPIC_BONE2; break;
                case 0x09: objIdx = MAPPIC_FLOWERS; break;
                case 0x10: objIdx = MAPPIC_BUSH2; break;
                case 0x11: objIdx = MAPPIC_BUSH3; break;
                case 0x12: objIdx = MAPPIC_BUSH4; break;

                case 0x0A: objIdx = MAPPIC_BUSH1; break;

                case 0x0C: objIdx = MAPPIC_CACTUS1; break;
                case 0x0D: objIdx = MAPPIC_CACTUS2; break;
                case 0x0E: objIdx = MAPPIC_SHRUB1; break;
                case 0x0F: objIdx = MAPPIC_SHRUB2; break;

                case 0x13: objIdx = MAPPIC_SHRUB3; break;
                case 0x14: objIdx = MAPPIC_SHRUB4; break;

                case 0x16: objIdx = MAPPIC_DOOR; break;

                case 0x18: objIdx = MIS1BOBS_STONE1; break;
                case 0x19: objIdx = MIS1BOBS_STONE2; break;
                case 0x1A: objIdx = MIS1BOBS_STONE3; break;
                case 0x1B: objIdx = MIS1BOBS_STONE4; break;
                case 0x1C: objIdx = MIS1BOBS_STONE5; break;
                case 0x1D: objIdx = MIS1BOBS_STONE6; break;
                case 0x1E: objIdx = MIS1BOBS_STONE7; break;

                case 0x22: objIdx = MAPPIC_MUSHROOM3; break;

                case 0x25: objIdx = MAPPIC_PEBBLE1; break;
                case 0x26: objIdx = MAPPIC_PEBBLE2; break;
                case 0x27: objIdx = MAPPIC_PEBBLE3; break;
                default: break;
            }
            break;
        // stone
        case 0xCC: objIdx = MAPPIC_GRANITE_1_1 + (node.objectType - 0x01); break;
        // stone
        case 0xCD: objIdx = MAPPIC_GRANITE_2_1 + (node.objectType - 0x01); break;
        // headquarter
        case 0x80: // node.objectType is the number of the player beginning with 0x00
                   // %7 cause in the original game there are only 7 players and 7 different flags
            objIdx = FLAG_BLUE_DARK + node.objectType % 7;
            break;

        default: break;
    }
    if(objIdx != 0)
        CSurface::Draw(target, global::bmpArray[objIdx].surface, screenX - global::bmpArray[objIdx].nx,
                       screenY - global::bmpArray[objIdx].ny);

    // resources
    if(node.resource >= 0x41 && node.resource <= 0x47)
    {
        for(char i = 0x41; i <= node.resource; i++)
            CSurface::Draw(target, global::bmpArray[PICTURE_RESOURCE_COAL].surface,
                           screenX - global::bmpArray[PICTURE_RESOURCE_COAL].nx,
                           screenY - global::bmpArray[PICTURE_RESOURCE_COAL].ny - (4 * (i - 0x40)));
    } else if(node.resource >= 0x49 && node.resource <= 0x4F)
    {
        for(char i = 0x49; i <= node.resource; i++)
            CSurface::Draw(target, global::bmpArray[PICTURE_RESOURCE_ORE].surface,
                           screenX - global::bmpArray[PICTURE_RESOURCE_ORE].nx,
                           screenY - global::bmpArray[PICTURE_RESOURCE_ORE].ny - (4 * (i - 0x48)));
    }
    if(node.resource >= 0x51 && node.resource <= 0x57)
    {
        for(char i = 0x51; i <= node.resource; i++)
            CSurface::Draw(target, global::bmpArray[PICTURE_RESOURCE_GOLD].surface,
                           screenX - global::bmpArray[PICTURE_RESOURCE_GOLD].nx,
                           screenY - global::bmpArray[PICTURE_RESOURCE_GOLD].ny - (4 * (i - 0x50)));
    }
    if(node.resource >= 0x59 && node.resource <= 0x5F)
    {
        for(char i = 0x59; i <= node.resource; i++)
            CSurface::Draw(target, global::bmpArray[PICTURE_RESOURCE_GRANITE].surface,
                           screenX - global::bmpArray[PICTURE_RESOURCE_GRANITE].nx,
                           screenY - global::bmpArray[PICTURE_RESOURCE_GRANITE].ny - (4 * (i - 0x58)));
    }

    // animals
    if(node.animal > 0x00 && node.animal <= 0x06)
    {
        CSurface::Draw(target, global::bmpArray[PICTURE_SMALL_BEAR + node.animal].surface,
                       screenX - global::bmpArray[PICTURE_SMALL_BEAR + node.animal].nx,
                       screenY - global::bmpArray[PICTURE_SMALL_BEAR + node.animal].ny);
    }

    // buildings (editor build-help preview)
    if(global::s2 && global::s2->getMapObj() && global::s2->getMapObj()->getRenderBuildHelp())
    {
        switch(node.build % 8)
        {
            case 0x01:
                CSurface::Draw(target, global::bmpArray[MAPPIC_FLAG].surface,
                               screenX - global::bmpArray[MAPPIC_FLAG].nx, screenY - global::bmpArray[MAPPIC_FLAG].ny);
                break;
            case 0x02:
                CSurface::Draw(target, global::bmpArray[MAPPIC_HOUSE_SMALL].surface,
                               screenX - global::bmpArray[MAPPIC_HOUSE_SMALL].nx,
                               screenY - global::bmpArray[MAPPIC_HOUSE_SMALL].ny);
                break;
            case 0x03:
                CSurface::Draw(target, global::bmpArray[MAPPIC_HOUSE_MIDDLE].surface,
                               screenX - global::bmpArray[MAPPIC_HOUSE_MIDDLE].nx,
                               screenY - global::bmpArray[MAPPIC_HOUSE_MIDDLE].ny);
                break;
            case 0x04:
                if(node.rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1_HARBOUR
                   || node.rsuTexture == TRIANGLE_TEXTURE_MEADOW1_HARBOUR
                   || node.rsuTexture == TRIANGLE_TEXTURE_MEADOW2_HARBOUR
                   || node.rsuTexture == TRIANGLE_TEXTURE_MEADOW3_HARBOUR
                   || node.rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2_HARBOUR
                   || node.rsuTexture == TRIANGLE_TEXTURE_FLOWER_HARBOUR
                   || node.rsuTexture == TRIANGLE_TEXTURE_MINING_MEADOW_HARBOUR)
                    CSurface::Draw(target, global::bmpArray[MAPPIC_HOUSE_HARBOUR].surface,
                                   screenX - global::bmpArray[MAPPIC_HOUSE_HARBOUR].nx,
                                   screenY - global::bmpArray[MAPPIC_HOUSE_HARBOUR].ny);
                else
                    CSurface::Draw(target, global::bmpArray[MAPPIC_HOUSE_BIG].surface,
                                   screenX - global::bmpArray[MAPPIC_HOUSE_BIG].nx,
                                   screenY - global::bmpArray[MAPPIC_HOUSE_BIG].ny);
                break;
            case 0x05:
                CSurface::Draw(target, global::bmpArray[MAPPIC_MINE].surface,
                               screenX - global::bmpArray[MAPPIC_MINE].nx, screenY - global::bmpArray[MAPPIC_MINE].ny);
                break;
            default: break;
        }
    }
}

void Draw(SDL_Surface* target, const DisplayRectangle& viewRect, const bobMAP& map)
{
    const int width = map.width;
    const int height = map.height;
    const int width_pixel = map.width_pixel;
    const int height_pixel = map.height_pixel;

    // Floor division for possibly negative view coordinates.
    auto floorDiv = [](int a, int b) { return (a >= 0) ? a / b : (a - b + 1) / b; };

    // Map-coordinate range that can possibly be visible, with the same margin
    // the old triangle loop used.
    const int firstX = floorDiv(viewRect.left, triangleWidth) - 1;
    const int lastX = floorDiv(viewRect.right, triangleWidth) + 1;
    const int firstY = floorDiv(viewRect.top, triangleHeight) - 2;
    const int lastY = floorDiv(viewRect.bottom, triangleHeight) + 2;

    // Allow sprites that extend a bit outside the view rectangle.
    const int marginX = triangleWidth * 4;
    const int marginY = triangleHeight * 4;
    const int surfW = viewRect.getSize().x;
    const int surfH = viewRect.getSize().y;

    for(int y = firstY; y <= lastY; ++y)
    {
        for(int x = firstX; x <= lastX; ++x)
        {
            // Wrap to canonical map coordinates.
            int wx = x % width;
            if(wx < 0)
                wx += width;
            int wy = y % height;
            if(wy < 0)
                wy += height;

            // Pixel offset for this wrapped copy of the map.
            const int offsetX = (x - wx) / width * width_pixel;
            const int offsetY = (y - wy) / height * height_pixel;

            const MapNode& node = map.getVertex(wx, wy);
            const int screenX = node.x + offsetX - viewRect.left;
            const int screenY = node.y + offsetY - viewRect.top;

            if(screenX < -marginX || screenX > surfW + marginX || screenY < -marginY || screenY > surfH + marginY)
                continue;

            drawVertexOverlay(target, screenX, screenY, node);
        }
    }
}

} // namespace EditorWorldView

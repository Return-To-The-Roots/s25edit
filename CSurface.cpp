// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CSurface.h"
#include "CGame.h"
#include "CMap.h"
#include "Rect.h"
#include "Texture.h"
#include "globals.h"
#include "gameData/EdgeDesc.h"
#include "gameData/TerrainDesc.h"
#include <libsiedler2/ArchivItem_Bitmap.h>
#include <libsiedler2/ArchivItem_Bitmap_Raw.h>
#include <libsiedler2/ArchivItem_PaletteAnimation.h>
#include <libsiedler2/PixelBufferBGRA.h>
#include <libsiedler2/PixelBufferPaletted.h>
#include <glad/glad.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <map>

static uint8_t intensityToColor(Sint32 val)
{
    if(val <= 0)
        return 0;
    constexpr Sint32 maxVal = 2 * 65536;
    if(val >= maxVal)
        return 255;
    return static_cast<uint8_t>((val * 255 + maxVal / 2) / maxVal);
}

// For border rendering we use GL_MODULATE (no RGB_SCALE), so
// [0, 65536] maps linearly to [0, 255].
static uint8_t intensityToModulate(Sint32 val)
{
    if(val <= 0)
        return 0;
    constexpr Sint32 maxVal = 65536;
    if(val >= maxVal)
        return 255;
    return static_cast<uint8_t>((val * 255 + maxVal / 2) / maxVal);
}

namespace {
const TerrainDesc* getTerrainDesc(const bobMAP& map, Uint8 rawTextureId)
{
    const Uint8 s2Id = rawTextureId & ~0x40;
    if(s2Id < map.s2IdToTerrain.size())
    {
        const auto idx = map.s2IdToTerrain[s2Id];
        if(idx)
            return &global::worldDesc.get(idx);
    }
    return nullptr;
}

static void DrawFadedTexturedTrigon(const Point16& p1, const Point16& p2, const Point16& p3, const Rect& rect,
                                    Sint32 I1, Sint32 I2, float texW, float texH)
{
    uint8_t c1 = intensityToModulate(I1);
    uint8_t c2 = intensityToModulate(I2);
    uint8_t ct = c2;
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBegin(GL_TRIANGLES);
    glTexCoord2f(float(rect.left) / texW, float(rect.top) / texH);
    glColor4ub(c1, c1, c1, 255);
    glVertex2f(float(p1.x), float(p1.y));
    glTexCoord2f(float(rect.right) / texW, float(rect.top) / texH);
    glColor4ub(c2, c2, c2, 255);
    glVertex2f(float(p2.x), float(p2.y));
    glTexCoord2f(float((rect.left + rect.right) / 2) / texW, float(rect.bottom) / texH);
    glColor4ub(ct, ct, ct, 255);
    glVertex2f(float(p3.x), float(p3.y));
    glEnd();
}

} // namespace

bool CSurface::drawTextures = false;

void CSurface::DrawTriangleField(const DisplayRectangle& displayRect, const bobMAP& myMap)
{
    Uint16 width = myMap.width;
    Uint16 height = myMap.height;
    auto type = myMap.type;
    MapNode tempP1, tempP2, tempP3;

    // min size to avoid underflows
    if(width < 8 || height < 8)
        return;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    // draw triangle field
    // NOTE: WE DO THIS TWICE, AT FIRST ONLY TRIANGLE-TEXTURES, AT SECOND THE TEXTURE-BORDERS AND OBJECTS
    for(int i = 0; i < 2; i++)
    {
        drawTextures = (i == 0);

        for(int k = 0; k < 4; k++)
        {
            // IMPORTANT: integer values like +8 or -1 are for tolerance to beware of high triangles are not shown

            int row_start = std::max(0, displayRect.top / triangleHeight - 1);
            int row_end = std::min<int>(height, displayRect.bottom / triangleHeight + 2);
            int col_start = std::max(0, displayRect.left / triangleWidth - 1);
            int col_end = std::min<int>(width, displayRect.right / triangleWidth + 2);
            bool view_outside_edges;

            if(k > 0)
            {
                // now call DrawTriangle for all triangles outside the map edges
                view_outside_edges = false;

                if(k == 1 || k == 3)
                {
                    // at first call DrawTriangle for all triangles up or down outside
                    if(displayRect.top < 0)
                    {
                        row_start = std::max(0, height - 1 - (-displayRect.top / triangleHeight) - 1);
                        row_end = height - 1;
                        view_outside_edges = true;
                    } else if(displayRect.bottom > myMap.height_pixel)
                    {
                        row_start = 0;
                        row_end = (displayRect.bottom - myMap.height_pixel) / triangleHeight + 8;
                        view_outside_edges = true;
                    } else if(displayRect.top <= 2 * triangleHeight)
                    {
                        // this is for draw triangles that are reduced under the lower map edge (have bigger y-coords as
                        // myMap.height_pixel)
                        row_start = height - 3;
                        row_end = height - 1;
                        view_outside_edges = true;
                    } else if(displayRect.bottom >= (myMap.height_pixel - 8 * triangleHeight))
                    {
                        // this is for draw triangles that are raised over the upper map edge (have negative y-coords)
                        row_start = 0;
                        row_end = 8;
                        view_outside_edges = true;
                    }
                }

                if(k == 2 || k == 3)
                {
                    // now call DrawTriangle for all triangles left or right outside
                    if(displayRect.left <= 0)
                    {
                        col_start = std::max(0, width - 1 - (-displayRect.left / triangleWidth) - 1);
                        col_end = width - 1;
                        view_outside_edges = true;
                    } else if(displayRect.left < triangleWidth)
                    {
                        col_start = width - 2;
                        col_end = width - 1;
                        view_outside_edges = true;
                    } else if(displayRect.right > myMap.width_pixel)
                    {
                        col_start = 0;
                        col_end = (displayRect.right - myMap.width_pixel) / triangleWidth + 1;
                        view_outside_edges = true;
                    }
                }

                // if displayRect is not outside the map edges, there is nothing to do
                if(!view_outside_edges)
                    continue;
            }

            assert(col_start >= 0);
            assert(row_start >= 0);
            assert(col_start <= col_end);
            assert(row_start <= row_end);

            for(unsigned y = row_start; y < height - 1u && y <= static_cast<unsigned>(row_end); y++)
            {
                if(y % 2 == 0)
                {
                    // first RightSideUp
                    tempP2 = myMap.getVertex(width - 1, y + 1);
                    tempP2.x = 0;
                    DrawTriangle(displayRect, myMap, type, myMap.getVertex(0, y), tempP2, myMap.getVertex(0, y + 1));
                    for(unsigned x = std::max(col_start, 1); x < width && x <= static_cast<unsigned>(col_end); x++)
                    {
                        // RightSideUp
                        DrawTriangle(displayRect, myMap, type, myMap.getVertex(x, y), myMap.getVertex(x - 1, y + 1),
                                     myMap.getVertex(x, y + 1));
                        // UpSideDown
                        DrawTriangle(displayRect, myMap, type, myMap.getVertex(x - 1, y + 1), myMap.getVertex(x - 1, y),
                                     myMap.getVertex(x, y));
                    }
                    // last UpSideDown
                    tempP3 = myMap.getVertex(0, y);
                    tempP3.x = myMap.getVertex(width - 1, y).x + triangleWidth;
                    DrawTriangle(displayRect, myMap, type, myMap.getVertex(width - 1, y + 1),
                                 myMap.getVertex(width - 1, y), tempP3);
                } else
                {
                    for(unsigned x = col_start; x < width - 1u && x <= static_cast<unsigned>(col_end); x++)
                    {
                        // RightSideUp
                        DrawTriangle(displayRect, myMap, type, myMap.getVertex(x, y), myMap.getVertex(x, y + 1),
                                     myMap.getVertex(x + 1, y + 1));
                        // UpSideDown
                        DrawTriangle(displayRect, myMap, type, myMap.getVertex(x + 1, y + 1), myMap.getVertex(x, y),
                                     myMap.getVertex(x + 1, y));
                    }
                    // last RightSideUp
                    tempP3 = myMap.getVertex(0, y + 1);
                    tempP3.x = myMap.getVertex(width - 1, y + 1).x + triangleWidth;
                    DrawTriangle(displayRect, myMap, type, myMap.getVertex(width - 1, y),
                                 myMap.getVertex(width - 1, y + 1), tempP3);
                    // last UpSideDown
                    tempP1 = myMap.getVertex(0, y + 1);
                    tempP1.x = myMap.getVertex(width - 1, y + 1).x + triangleWidth;
                    tempP3 = myMap.getVertex(0, y);
                    tempP3.x = myMap.getVertex(width - 1, y).x + triangleWidth;
                    DrawTriangle(displayRect, myMap, type, tempP1, myMap.getVertex(width - 1, y), tempP3);
                }
            }

            // draw last line
            for(unsigned x = col_start; x < width - 1u && x <= static_cast<unsigned>(col_end); x++)
            {
                // RightSideUp
                tempP2 = myMap.getVertex(x, 0);
                tempP2.y = height * triangleHeight + myMap.getVertex(x, 0).y;
                tempP3 = myMap.getVertex(x + 1, 0);
                tempP3.y = height * triangleHeight + myMap.getVertex(x + 1, 0).y;
                DrawTriangle(displayRect, myMap, type, myMap.getVertex(x, height - 1), tempP2, tempP3);
                // UpSideDown
                tempP1 = myMap.getVertex(x + 1, 0);
                tempP1.y = height * triangleHeight + myMap.getVertex(x + 1, 0).y;
                DrawTriangle(displayRect, myMap, type, tempP1, myMap.getVertex(x, height - 1),
                             myMap.getVertex(x + 1, height - 1));
            }
        }

        // last RightSideUp
        tempP2 = myMap.getVertex(width - 1, 0);
        tempP2.y += height * triangleHeight;
        tempP3 = myMap.getVertex(0, 0);
        tempP3.x = myMap.getVertex(width - 1, 0).x + triangleWidth;
        tempP3.y += height * triangleHeight;
        DrawTriangle(displayRect, myMap, type, myMap.getVertex(width - 1, height - 1), tempP2, tempP3);
        // last UpSideDown
        tempP1 = myMap.getVertex(0, 0);
        tempP1.x = myMap.getVertex(width - 1, 0).x + triangleWidth;
        tempP1.y += height * triangleHeight;
        tempP3 = myMap.getVertex(0, height - 1);
        tempP3.x = myMap.getVertex(width - 1, height - 1).x + triangleWidth;
        DrawTriangle(displayRect, myMap, type, tempP1, myMap.getVertex(width - 1, height - 1), tempP3);
    }
}

namespace {
enum class BorderPreference
{
    None,
    LeftTop,
    RightBottom
};
BorderPreference CalcBorders(const bobMAP& map, Uint8 s2Id1, Uint8 s2Id2, Rect& borderRect)
{
    // we have to decide which border to blit, "left or right" or "top or bottom"
    s2Id1 &= ~(0x40 | 0x80);
    s2Id2 &= ~(0x40 | 0x80);

    assert(s2Id1 < map.s2IdToTerrain.size());
    assert(s2Id2 < map.s2IdToTerrain.size());

    DescIdx<TerrainDesc> idxTop = map.s2IdToTerrain[s2Id1];
    DescIdx<TerrainDesc> idxBottom = map.s2IdToTerrain[s2Id2];
    if(idxTop == idxBottom)
        return BorderPreference::None;
    const TerrainDesc& t1 = global::worldDesc.get(idxTop);
    const TerrainDesc& t2 = global::worldDesc.get(idxBottom);
    if(t1.edgePriority > t2.edgePriority)
    {
        if(!t1.edgeType)
            return BorderPreference::None;
        borderRect = global::worldDesc.get(t1.edgeType).posInTexture;
        return BorderPreference::LeftTop;
    } else if(t1.edgePriority < t2.edgePriority)
    {
        if(!t2.edgeType)
            return BorderPreference::None;
        borderRect = global::worldDesc.get(t2.edgeType).posInTexture;
        return BorderPreference::RightBottom;
    }
    return BorderPreference::None;
}

template<typename T>
constexpr bool isInRange(T val, T min, T max)
{
    return val >= min && val <= max;
}

/// Return true if triangle is drawn
bool GetAdjustedPoints(const DisplayRectangle& displayRect, const bobMAP& myMap, Point32& p1, Point32& p2, Point32& p3)
{
    if((!isInRange(p1.x, displayRect.left, displayRect.right) && !isInRange(p2.x, displayRect.left, displayRect.right)
        && !isInRange(p3.x, displayRect.left, displayRect.right))
       || (!isInRange(p1.y, displayRect.top, displayRect.bottom)
           && !isInRange(p2.y, displayRect.top, displayRect.bottom)
           && !isInRange(p3.y, displayRect.top, displayRect.bottom)))
    {
        bool triangle_shown = false;

        if(displayRect.left <= 0)
        {
            int outside_left = displayRect.left;
            int outside_right = 0;
            if(isInRange(p1.x - myMap.width_pixel, outside_left, outside_right)
               || isInRange(p2.x - myMap.width_pixel, outside_left, outside_right)
               || isInRange(p3.x - myMap.width_pixel, outside_left, outside_right))
            {
                p1.x -= myMap.width_pixel;
                p2.x -= myMap.width_pixel;
                p3.x -= myMap.width_pixel;
                triangle_shown = true;
            }
        } else if(displayRect.left < triangleWidth)
        {
            int outside_left = displayRect.left;
            int outside_right = displayRect.left + triangleWidth;
            if(isInRange(p1.x - myMap.width_pixel, outside_left, outside_right)
               || isInRange(p2.x - myMap.width_pixel, outside_left, outside_right)
               || isInRange(p3.x - myMap.width_pixel, outside_left, outside_right))
            {
                p1.x -= myMap.width_pixel;
                p2.x -= myMap.width_pixel;
                p3.x -= myMap.width_pixel;
                triangle_shown = true;
            }
        } else if(displayRect.right > myMap.width_pixel)
        {
            int outside_left = myMap.width_pixel;
            int outside_right = displayRect.right;
            if(isInRange(p1.x + myMap.width_pixel, outside_left, outside_right)
               || isInRange(p2.x + myMap.width_pixel, outside_left, outside_right)
               || isInRange(p3.x + myMap.width_pixel, outside_left, outside_right))
            {
                p1.x += myMap.width_pixel;
                p2.x += myMap.width_pixel;
                p3.x += myMap.width_pixel;
                triangle_shown = true;
            }
        }

        if(displayRect.top < 0)
        {
            int outside_top = displayRect.top;
            int outside_bottom = 0;
            if(isInRange(p1.y - myMap.height_pixel, outside_top, outside_bottom)
               || isInRange(p2.y - myMap.height_pixel, outside_top, outside_bottom)
               || isInRange(p3.y - myMap.height_pixel, outside_top, outside_bottom))
            {
                p1.y -= myMap.height_pixel;
                p2.y -= myMap.height_pixel;
                p3.y -= myMap.height_pixel;
                triangle_shown = true;
            }
        } else if(displayRect.bottom > myMap.height_pixel)
        {
            int outside_top = myMap.height_pixel;
            int outside_bottom = displayRect.bottom;
            if(isInRange(p1.y + myMap.height_pixel, outside_top, outside_bottom)
               || isInRange(p2.y + myMap.height_pixel, outside_top, outside_bottom)
               || isInRange(p3.y + myMap.height_pixel, outside_top, outside_bottom))
            {
                p1.y += myMap.height_pixel;
                p2.y += myMap.height_pixel;
                p3.y += myMap.height_pixel;
                triangle_shown = true;
            }
        }

        // now test if triangle has negative y-coords cause it's raised over the upper map edge
        if(p1.y < 0 || p2.y < 0 || p3.y < 0)
        {
            if(isInRange(p1.y + myMap.height_pixel, displayRect.top, displayRect.bottom)
               || isInRange(p2.y + myMap.height_pixel, displayRect.top, displayRect.bottom)
               || isInRange(p3.y + myMap.height_pixel, displayRect.top, displayRect.bottom))
            {
                p1.y += myMap.height_pixel;
                p2.y += myMap.height_pixel;
                p3.y += myMap.height_pixel;
                triangle_shown = true;
            }
        }

        // now test if triangle has bigger y-coords as myMap.height_pixel cause it's reduced under the lower map edge
        if(p1.y > myMap.height_pixel || p2.y > myMap.height_pixel || p3.y > myMap.height_pixel)
        {
            if(isInRange(p1.y - myMap.height_pixel, displayRect.top, displayRect.bottom)
               || isInRange(p2.y - myMap.height_pixel, displayRect.top, displayRect.bottom)
               || isInRange(p3.y - myMap.height_pixel, displayRect.top, displayRect.bottom))
            {
                p1.y -= myMap.height_pixel;
                p2.y -= myMap.height_pixel;
                p3.y -= myMap.height_pixel;
                triangle_shown = true;
            }
        }

        if(!triangle_shown)
            return false;
    }
    return true;
}
} // namespace

// palette animation helpers

/// Holds pre-rendered frames for one palette-animated terrain.
struct AnimFrames
{
    std::vector<Texture> frames;
};

static std::map<std::pair<const TerrainDesc*, int>, AnimFrames> s_animFrameCache;

static const AnimFrames* getAnimFrames(const TerrainDesc& td, MapType mapType)
{
    // Determine tileset index from map type
    auto& ta = global::tilesetAnims[static_cast<int>(mapType)];
    if(ta.anims.empty())
        return nullptr;
    // palAnimIdx is the index in the tileset Archiv (0 = bitmap, 1+ = animations)
    // ta.anims[0] = Archiv[1], ta.anims[1] = Archiv[2], etc.
    if(td.palAnimIdx < 1)
        return nullptr;
    size_t animIdx = static_cast<size_t>(td.palAnimIdx) - 1;
    if(animIdx >= ta.anims.size())
        return nullptr;
    const auto* anim = ta.anims[animIdx];
    if(!anim || !anim->isActive)
        return nullptr;

    unsigned numFrames = anim->lastClr - anim->firstClr + 1;
    if(numFrames < 2)
        return nullptr;
    if(numFrames > 32)
        numFrames = 32;

    auto key = std::make_pair(&td, static_cast<int>(mapType));
    auto it = s_animFrameCache.find(key);
    if(it != s_animFrameCache.end())
        return &it->second;

    // Get the tileset bitmap from the typed archive
    const libsiedler2::baseArchivItem_Bitmap* tsBmp =
      dynamic_cast<const libsiedler2::baseArchivItem_Bitmap*>(global::typedArchives[ta.archive].get(ta.bmpIdx));
    if(!tsBmp)
        return nullptr;
    const auto* srcPal = tsBmp->getPalette();
    if(!srcPal)
        return nullptr;

    // Extract the terrain sub-rect from the tileset
    auto r = td.posInTexture;
    Extent texSize = r.getSize();
    if(texSize.x == 0 || texSize.y == 0)
        return nullptr;

    // Create paletted pixel buffer with the sub-rect
    libsiedler2::PixelBufferPaletted buffer(texSize.x, texSize.y);
    if(tsBmp->print(buffer, nullptr, 0, 0, r.left, r.top, texSize.x, texSize.y))
        return nullptr;

    // Build the per-frame animation object (direction hardcoded to false,
    // matching s25client's ExtractAnimatedTexture which ignores the CRNG flags)
    libsiedler2::ArchivItem_PaletteAnimation frameStep;
    frameStep.isActive = true;
    frameStep.moveUp = false;
    frameStep.firstClr = anim->firstClr;
    frameStep.lastClr = anim->lastClr;

    auto& result = s_animFrameCache[key];
    result.frames.reserve(numFrames);

    std::unique_ptr<libsiedler2::ArchivItem_Palette> curPal;
    for(unsigned fi = 0; fi < numFrames; fi++)
    {
        auto pal = (fi == 0) ? std::make_unique<libsiedler2::ArchivItem_Palette>(*srcPal) : frameStep.apply(*curPal);
        if(!pal)
            continue;

        // Render frame to BGRA
        libsiedler2::PixelBufferBGRA bgraBuf(texSize.x, texSize.y);
        for(unsigned y = 0; y < texSize.y; y++)
        {
            for(unsigned x = 0; x < texSize.x; x++)
            {
                auto idx = buffer.get(x, y);
                auto c = pal->get(idx);
                uint32_t* dst = reinterpret_cast<uint32_t*>(bgraBuf.getPixelPtr()) + y * texSize.x + x;
                *dst = (0xFFu << 24) | (uint32_t(c.r) << 16) | (uint32_t(c.g) << 8) | uint32_t(c.b);
            }
        }

        Texture tex;
        libsiedler2::ArchivItem_Bitmap_Raw tmpBmp;
        tmpBmp.create(texSize.x, texSize.y, bgraBuf.getPixelPtr(), texSize.x, texSize.y,
                      libsiedler2::TextureFormat::BGRA, nullptr);
        tex.load(tmpBmp);
        result.frames.push_back(std::move(tex));

        curPal = std::move(pal);
    }

    return result.frames.empty() ? nullptr : &result;
}

/// Choose the current animation frame based on elapsed wall-clock time.
/// Matches s25client: each frame is shown for 630*5/16 = ~197 ms.
static unsigned getAnimFrameIdx(const AnimFrames& af)
{
    unsigned numFrames = af.frames.size();
    if(numFrames <= 1)
        return 0;
    // Match s25client's GetGlobalAnimation(numFrames, 5*numFrames, 16, 0):
    // unit = 630ms * 5 * numFrames / 16  →  197ms per frame
    constexpr unsigned unitPerFrame = 630 * 5 / 16; // ≈ 197 ms
    uint32_t now = SDL_GetTicks();
    return (now / unitPerFrame) % numFrames;
}

void CSurface::GetTerrainTextureCoords(MapType mapType, TriangleTerrainType texture, bool isRSU, Point16& upper,
                                       Point16& left, Point16& right, Point16& upper2, Point16& left2, Point16& right2)
{
    switch(texture)
    {
            // in case of USD-Triangle "upper.x" and "upper.y" means "lowerX" and "lowerY"
        case TRIANGLE_TEXTURE_STEPPE_MEADOW1:
            upper = Point16(17, 96);
            left = Point16(0, 126);
            right = Point16(35, 126);
            break;
        case TRIANGLE_TEXTURE_MINING1:
            upper = Point16(17, 48);
            left = Point16(0, 78);
            right = Point16(35, 78);
            break;
        case TRIANGLE_TEXTURE_SNOW:
            if(isRSU)
            {
                upper = Point16(17, 0);
                left = Point16(0, 30);
                right = Point16(35, 30);
            } else
            {
                upper = Point16(17, 28);
                left = Point16(0, 0);
                right = Point16(37, 0);
            }
            if(mapType == MAP_WINTERLAND)
            {
                if(isRSU)
                {
                    upper2 = Point16(231, 61);
                    left2 = Point16(207, 62);
                    right2 = Point16(223, 78);
                } else
                {
                    upper2 = Point16(224, 79);
                    left2 = Point16(232, 62);
                    right2 = Point16(245, 76);
                }
            }
            break;
        case TRIANGLE_TEXTURE_SWAMP:
            upper = Point16(113, 0);
            left = Point16(96, 30);
            right = Point16(131, 30);
            if(mapType == MAP_WINTERLAND)
            {
                if(isRSU)
                {
                    upper2 = Point16(231, 61);
                    left2 = Point16(207, 62);
                    right2 = Point16(223, 78);
                } else
                {
                    upper2 = Point16(224, 79);
                    left2 = Point16(232, 62);
                    right2 = Point16(245, 76);
                }
            }
            break;
        case TRIANGLE_TEXTURE_STEPPE:
        case TRIANGLE_TEXTURE_STEPPE_:
        case TRIANGLE_TEXTURE_STEPPE__:
        case TRIANGLE_TEXTURE_STEPPE___:
            upper = Point16(65, 0);
            left = Point16(48, 30);
            right = Point16(83, 30);
            break;
        case TRIANGLE_TEXTURE_WATER:
        case TRIANGLE_TEXTURE_WATER_:
        case TRIANGLE_TEXTURE_WATER__:
            if(isRSU)
            {
                upper = Point16(231, 61);
                left = Point16(207, 62);
                right = Point16(223, 78);
            } else
            {
                upper = Point16(224, 79);
                left = Point16(232, 62);
                right = Point16(245, 76);
            }
            break;
        case TRIANGLE_TEXTURE_MEADOW1:
            upper = Point16(65, 96);
            left = Point16(48, 126);
            right = Point16(83, 126);
            break;
        case TRIANGLE_TEXTURE_MEADOW2:
            upper = Point16(113, 96);
            left = Point16(96, 126);
            right = Point16(131, 126);
            break;
        case TRIANGLE_TEXTURE_MEADOW3:
            upper = Point16(161, 96);
            left = Point16(144, 126);
            right = Point16(179, 126);
            break;
        case TRIANGLE_TEXTURE_MINING2:
            upper = Point16(65, 48);
            left = Point16(48, 78);
            right = Point16(83, 78);
            break;
        case TRIANGLE_TEXTURE_MINING3:
            upper = Point16(113, 48);
            left = Point16(96, 78);
            right = Point16(131, 78);
            break;
        case TRIANGLE_TEXTURE_MINING4:
            upper = Point16(161, 48);
            left = Point16(144, 78);
            right = Point16(179, 78);
            break;
        case TRIANGLE_TEXTURE_STEPPE_MEADOW2:
            upper = Point16(17, 144);
            left = Point16(0, 174);
            right = Point16(35, 174);
            break;
        case TRIANGLE_TEXTURE_FLOWER:
            upper = Point16(161, 0);
            left = Point16(144, 30);
            right = Point16(179, 30);
            break;
        case TRIANGLE_TEXTURE_LAVA:
            if(isRSU)
            {
                upper = Point16(231, 117);
                left = Point16(207, 118);
                right = Point16(223, 134);
            } else
            {
                upper = Point16(224, 135);
                left = Point16(232, 118);
                right = Point16(245, 132);
            }
            break;
        case TRIANGLE_TEXTURE_MINING_MEADOW:
            upper = Point16(65, 144);
            left = Point16(48, 174);
            right = Point16(83, 174);
            break;
        default: // TRIANGLE_TEXTURE_FLOWER
            upper = Point16(161, 0);
            left = Point16(144, 30);
            right = Point16(179, 30);
            break;
    }
}

void CSurface::DrawTriangle(const DisplayRectangle& displayRect, const bobMAP& myMap, MapType type, const MapNode& P1,
                            const MapNode& P2, const MapNode& P3)
{
    Point32 p1(P1.x, P1.y);
    Point32 p2(P2.x, P2.y);
    Point32 p3(P3.x, P3.y);

    // prevent drawing triangles that are not shown
    if(!GetAdjustedPoints(displayRect, myMap, p1, p2, p3))
        return;

    static int roundCount = 0;
    static Uint32 roundTimeObjects = SDL_GetTicks();
    if(SDL_GetTicks() - roundTimeObjects > 30)
    {
        roundTimeObjects = SDL_GetTicks();
        if(roundCount >= 7)
            roundCount = 0;
        else
            roundCount++;
    }

    // Determine tileset texture
    ArchiveID tsArchive;
    switch(type)
    {
        case MAP_GREENLAND:
        default: tsArchive = ArchiveID::TEX5; break;
        case MAP_WASTELAND: tsArchive = ArchiveID::TEX6; break;
        case MAP_WINTERLAND: tsArchive = ArchiveID::TEX7; break;
    }
    bool const isRSU = p1.y < p2.y;
    auto& tilesetTex = Texture::getTexture(tsArchive, 0);
    if(!tilesetTex.isValid())
        return;
    glBindTexture(GL_TEXTURE_2D, tilesetTex.getHandle());

    const auto* tsBmp =
      dynamic_cast<const libsiedler2::baseArchivItem_Bitmap*>(global::typedArchives[tsArchive].get(0));
    const float texW = tsBmp ? static_cast<float>(tsBmp->getWidth()) : 0.0f;
    const float texH = tsBmp ? static_cast<float>(tsBmp->getHeight()) : 0.0f;
    if(drawTextures)
    {
        Point16 upper, left, right, upper2, left2, right2;
        auto const texture =
          TriangleTerrainType((isRSU ? P1.rsuTexture : P2.usdTexture) & ~0x40); // Mask out harbor bit
        GetTerrainTextureCoords(type, texture, isRSU, upper, left, right, upper2, left2, right2);

        const float uvs[3][2] = {{static_cast<float>(upper.x) / texW, static_cast<float>(upper.y) / texH},
                                 {static_cast<float>(left.x) / texW, static_cast<float>(left.y) / texH},
                                 {static_cast<float>(right.x) / texW, static_cast<float>(right.y) / texH}};

        const float vertCoord[3][2] = {
          {float(p1.x), float(p1.y)}, {float(p2.x), float(p2.y)}, {float(p3.x), float(p3.y)}};
        const MapNode* verts[3] = {&P1, &P2, &P3};

        // --- palette animation: any terrain with palAnimIdx >= 0 ---
        if(const auto* terrainDesc = getTerrainDesc(myMap, texture); terrainDesc && terrainDesc->palAnimIdx >= 0)
        {
            const auto* af = getAnimFrames(*terrainDesc, type);
            if(af && !af->frames.empty())
            {
                unsigned frameIdx = getAnimFrameIdx(*af);
                glBindTexture(GL_TEXTURE_2D, af->frames[frameIdx].getHandle());

                // Use the GDL triangle UVs (the animated texture covers only the terrain sub-rect)
                auto tri = isRSU ? terrainDesc->GetRSUTriangle() : terrainDesc->GetUSDTriangle();
                auto r = terrainDesc->posInTexture;
                float ox = static_cast<float>(r.left);
                float oy = static_cast<float>(r.top);
                float sx = static_cast<float>(r.getSize().x);
                float sy = static_cast<float>(r.getSize().y);
                float nu[3][2] = {{(tri.tip.x - ox) / sx, (tri.tip.y - oy) / sy},
                                  {(tri.left.x - ox) / sx, (tri.left.y - oy) / sy},
                                  {(tri.right.x - ox) / sx, (tri.right.y - oy) / sy}};

                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
                glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 2.0f);
                glBegin(GL_TRIANGLES);
                for(int k = 0; k < 3; k++)
                {
                    glTexCoord2f(nu[k][0], nu[k][1]);
                    glColor4ub(intensityToColor(verts[k]->i), intensityToColor(verts[k]->i),
                               intensityToColor(verts[k]->i), 255);
                    glVertex2f(vertCoord[k][0], vertCoord[k][1]);
                }
                glEnd();
                return;
            }
        }

        // Most terrains: draw from the tileset with standard shading
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
        glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 2.0f);
        glBegin(GL_TRIANGLES);
        for(int k = 0; k < 3; k++)
        {
            glTexCoord2f(uvs[k][0], uvs[k][1]);
            glColor4ub(intensityToColor(verts[k]->i), intensityToColor(verts[k]->i), intensityToColor(verts[k]->i),
                       255);
            glVertex2f(vertCoord[k][0], vertCoord[k][1]);
        }
        glEnd();
        return;
    }

    // blit borders
    /// PRIORITY FROM HIGH TO LOW: SNOW, MINING_MEADOW, STEPPE, STEPPE_MEADOW2, MINING, MEADOW, FLOWER, STEPPE_MEADOW1,
    /// SWAMP, WATER, LAVA
    if(global::s2->getMapObj()->getRenderBorders())
    {
        // RSU-Triangle
        if(isRSU)
        {
            // left upper / right lower edge - therefore get the usd-texture from left to compare
            Uint16 col = (P1.VertexX - 1 < 0 ? myMap.width - 1 : P1.VertexX - 1);
            MapNode tempP = myMap.getVertex(col, P1.VertexY);

            Rect BorderRect;
            auto borderSide = CalcBorders(myMap, tempP.usdTexture, P1.rsuTexture, BorderRect);
            if(borderSide != BorderPreference::None)
            {
                Point16 tmpP1{p1}, tmpP2{p2};
                Point32 thirdPt;
                if(borderSide == BorderPreference::LeftTop)
                    thirdPt = p3;
                else
                {
                    tmpP1 += Point16(1, 0);
                    tmpP2 += Point16(1, 0);
                    thirdPt = Point32(tempP.x, tempP.y);
                    // Shift it close to p1
                    auto diff = thirdPt - p1;
                    if(diff.x < -myMap.width_pixel / 2)
                        thirdPt.x += myMap.width_pixel;
                    else if(diff.x > myMap.width_pixel / 2)
                        thirdPt.x -= myMap.width_pixel;
                    if(diff.y < -myMap.height_pixel / 2)
                        thirdPt.y += myMap.height_pixel;
                    else if(diff.y > myMap.height_pixel / 2)
                        thirdPt.y -= myMap.height_pixel;
                }
                Point16 tipPt{(p1 + p2 + thirdPt) / 3};

                DrawFadedTexturedTrigon(tmpP1, tmpP2, tipPt, BorderRect, P1.i, P2.i, texW, texH);
            }
        }
        // USD-Triangle
        else
        {
            Rect BorderRect;
            // left lower / right upper
            auto borderSide = CalcBorders(myMap, P2.rsuTexture, P2.usdTexture, BorderRect);

            if(borderSide != BorderPreference::None)
            {
                Uint16 col = (P1.VertexX - 1 < 0 ? myMap.width - 1 : P1.VertexX - 1);
                MapNode tempP = myMap.getVertex(col, P1.VertexY);

                Point16 tmpP1{p1}, tmpP2{p2};
                Point32 thirdPt;
                if(borderSide == BorderPreference::LeftTop)
                {
                    thirdPt = p3;
                    tmpP1 -= Point16(1, 0);
                    tmpP2 -= Point16(1, 0);
                } else
                {
                    thirdPt = Point32(tempP.x, tempP.y);
                    // Shift it close to p1
                    auto diff = thirdPt - p1;
                    if(diff.x < -myMap.width_pixel / 2)
                        thirdPt.x += myMap.width_pixel;
                    else if(diff.x > myMap.width_pixel / 2)
                        thirdPt.x -= myMap.width_pixel;
                    if(diff.y < -myMap.height_pixel / 2)
                        thirdPt.y += myMap.height_pixel;
                    else if(diff.y > myMap.height_pixel / 2)
                        thirdPt.y -= myMap.height_pixel;
                }

                Point16 tipPt{(p1 + p2 + thirdPt) / 3};

                DrawFadedTexturedTrigon(tmpP1, tmpP2, tipPt, BorderRect, P1.i, P2.i, texW, texH);
            }

            // top / bottom - therefore get the rsu-texture one line above to compare
            Uint16 row = (P2.VertexY - 1 < 0 ? myMap.height - 1 : P2.VertexY - 1);
            Uint16 col = (P2.VertexY % 2 == 0 ? P2.VertexX : (P2.VertexX + 1 > myMap.width - 1 ? 0 : P2.VertexX + 1));
            MapNode tempP = myMap.getVertex(col, row);

            borderSide = CalcBorders(myMap, tempP.rsuTexture, P2.usdTexture, BorderRect);
            if(borderSide != BorderPreference::None)
            {
                Point32 thirdPt;
                if(borderSide == BorderPreference::LeftTop)
                    thirdPt = p1;
                else
                {
                    thirdPt = Point32(tempP.x, tempP.y);
                    // Shift it close to p2
                    auto diff = thirdPt - p2;
                    if(diff.x < -myMap.width_pixel / 2)
                        thirdPt.x += myMap.width_pixel;
                    else if(diff.x > myMap.width_pixel / 2)
                        thirdPt.x -= myMap.width_pixel;
                    if(diff.y < -myMap.height_pixel / 2)
                        thirdPt.y += myMap.height_pixel;
                    else if(diff.y > myMap.height_pixel / 2)
                        thirdPt.y -= myMap.height_pixel;
                }
                Point16 tipPt{(p2 + p3 + thirdPt) / 3};

                DrawFadedTexturedTrigon(Point16(p2), Point16(p3), tipPt, BorderRect, P2.i, P3.i, texW, texH);
            }
        }
    }

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // blit picture to vertex (trees, animals, buildings and so on) --> BUT ONLY AT node1 ON RIGHTSIDEUP-TRIANGLES

    // blit objects
    if(!isRSU)
    {
        int objIdx = 0;
        switch(P2.objectInfo)
        {
            // tree
            case 0xC4:
                if(P2.objectType >= 0x30 && P2.objectType <= 0x37)
                {
                    if(P2.objectType + roundCount > 0x37)
                        objIdx = MAPPIC_TREE_PINE + (P2.objectType - 0x30) + (roundCount - 7);
                    else
                        objIdx = MAPPIC_TREE_PINE + (P2.objectType - 0x30) + roundCount;

                } else if(P2.objectType >= 0x70 && P2.objectType <= 0x77)
                {
                    if(P2.objectType + roundCount > 0x77)
                        objIdx = MAPPIC_TREE_BIRCH + (P2.objectType - 0x70) + (roundCount - 7);
                    else
                        objIdx = MAPPIC_TREE_BIRCH + (P2.objectType - 0x70) + roundCount;
                } else if(P2.objectType >= 0xB0 && P2.objectType <= 0xB7)
                {
                    if(P2.objectType + roundCount > 0xB7)
                        objIdx = MAPPIC_TREE_OAK + (P2.objectType - 0xB0) + (roundCount - 7);
                    else
                        objIdx = MAPPIC_TREE_OAK + (P2.objectType - 0xB0) + roundCount;
                } else if(P2.objectType >= 0xF0 && P2.objectType <= 0xF7)
                {
                    if(P2.objectType + roundCount > 0xF7)
                        objIdx = MAPPIC_TREE_PALM1 + (P2.objectType - 0xF0) + (roundCount - 7);
                    else
                        objIdx = MAPPIC_TREE_PALM1 + (P2.objectType - 0xF0) + roundCount;
                }
                break;
            // tree
            case 0xC5:
                if(P2.objectType >= 0x30 && P2.objectType <= 0x37)
                {
                    if(P2.objectType + roundCount > 0x37)
                        objIdx = MAPPIC_TREE_PALM2 + (P2.objectType - 0x30) + (roundCount - 7);
                    else
                        objIdx = MAPPIC_TREE_PALM2 + (P2.objectType - 0x30) + roundCount;

                } else if(P2.objectType >= 0x70 && P2.objectType <= 0x77)
                {
                    if(P2.objectType + roundCount > 0x77)
                        objIdx = MAPPIC_TREE_PINEAPPLE + (P2.objectType - 0x70) + (roundCount - 7);
                    else
                        objIdx = MAPPIC_TREE_PINEAPPLE + (P2.objectType - 0x70) + roundCount;
                } else if(P2.objectType >= 0xB0 && P2.objectType <= 0xB7)
                {
                    if(P2.objectType + roundCount > 0xB7)
                        objIdx = MAPPIC_TREE_CYPRESS + (P2.objectType - 0xB0) + (roundCount - 7);
                    else
                        objIdx = MAPPIC_TREE_CYPRESS + (P2.objectType - 0xB0) + roundCount;
                } else if(P2.objectType >= 0xF0 && P2.objectType <= 0xF7)
                {
                    if(P2.objectType + roundCount > 0xF7)
                        objIdx = MAPPIC_TREE_CHERRY + (P2.objectType - 0xF0) + (roundCount - 7);
                    else
                        objIdx = MAPPIC_TREE_CHERRY + (P2.objectType - 0xF0) + roundCount;
                }
                break;
            // tree
            case 0xC6:
                if(P2.objectType >= 0x30 && P2.objectType <= 0x37)
                {
                    if(P2.objectType + roundCount > 0x37)
                        objIdx = MAPPIC_TREE_FIR + (P2.objectType - 0x30) + (roundCount - 7);
                    else
                        objIdx = MAPPIC_TREE_FIR + (P2.objectType - 0x30) + roundCount;
                }
                break;
            // landscape
            case 0xC8:
                switch(P2.objectType)
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

                    case 0x18:
                        Texture::getTexture(ArchiveID::MIS1BOBS, MIS1BOBS_STONE1).drawSprite(p2);
                        objIdx = 0;
                        break;
                    case 0x19:
                        Texture::getTexture(ArchiveID::MIS1BOBS, MIS1BOBS_STONE2).drawSprite(p2);
                        objIdx = 0;
                        break;
                    case 0x1A:
                        Texture::getTexture(ArchiveID::MIS1BOBS, MIS1BOBS_STONE3).drawSprite(p2);
                        objIdx = 0;
                        break;
                    case 0x1B:
                        Texture::getTexture(ArchiveID::MIS1BOBS, MIS1BOBS_STONE4).drawSprite(p2);
                        objIdx = 0;
                        break;
                    case 0x1C:
                        Texture::getTexture(ArchiveID::MIS1BOBS, MIS1BOBS_STONE5).drawSprite(p2);
                        objIdx = 0;
                        break;
                    case 0x1D:
                        Texture::getTexture(ArchiveID::MIS1BOBS, MIS1BOBS_STONE6).drawSprite(p2);
                        objIdx = 0;
                        break;
                    case 0x1E:
                        Texture::getTexture(ArchiveID::MIS1BOBS, MIS1BOBS_STONE7).drawSprite(p2);
                        objIdx = 0;
                        break;

                    case 0x22: objIdx = MAPPIC_MUSHROOM3; break;

                    case 0x25: objIdx = MAPPIC_PEBBLE1; break;
                    case 0x26: objIdx = MAPPIC_PEBBLE2; break;
                    case 0x27: objIdx = MAPPIC_PEBBLE3; break;
                    default: break;
                }
                break;
            // stone
            case 0xCC: objIdx = MAPPIC_GRANITE_1_1 + (P2.objectType - 0x01); break;
            // stone
            case 0xCD: objIdx = MAPPIC_GRANITE_2_1 + (P2.objectType - 0x01); break;
            // headquarter
            case 0x80: // node2.objectType is the number of the player beginning with 0x00
                       //%7 cause in the original game there are only 7 players and 7 different flags
                Texture::getTexture(ArchiveID::EDITBOB, FLAG_BLUE_DARK + P2.objectType % 7).drawSprite(p2);
                break; // don't set objIdx — drawn directly from typed archive
            default: break;
        }
        if(objIdx != 0)
            Texture::getTexture(ArchiveID::MAP00, objIdx).drawSprite(p2);
    }

    // blit resources
    if(!isRSU)
    {
        if(P2.resource >= 0x41 && P2.resource <= 0x47)
        {
            for(char i = 0x41; i <= P2.resource; i++)
                Texture::getTexture(ArchiveID::EDITIO, PICTURE_RESOURCE_COAL)
                  .drawSprite(Position(p2.x, p2.y - 4 * (i - 0x40)));
        } else if(P2.resource >= 0x49 && P2.resource <= 0x4F)
        {
            for(char i = 0x49; i <= P2.resource; i++)
                Texture::getTexture(ArchiveID::EDITIO, PICTURE_RESOURCE_ORE)
                  .drawSprite(Position(p2.x, p2.y - 4 * (i - 0x48)));
        }
        if(P2.resource >= 0x51 && P2.resource <= 0x57)
        {
            for(char i = 0x51; i <= P2.resource; i++)
                Texture::getTexture(ArchiveID::EDITIO, PICTURE_RESOURCE_GOLD)
                  .drawSprite(Position(p2.x, p2.y - 4 * (i - 0x50)));
        }
        if(P2.resource >= 0x59 && P2.resource <= 0x5F)
        {
            for(char i = 0x59; i <= P2.resource; i++)
                Texture::getTexture(ArchiveID::EDITIO, PICTURE_RESOURCE_GRANITE)
                  .drawSprite(Position(p2.x, p2.y - 4 * (i - 0x58)));
        }
        // blit animals
        if(P2.animal > 0x00 && P2.animal <= 0x06)
            Texture::getTexture(ArchiveID::EDITBOB, PICTURE_SMALL_BEAR + P2.animal).drawSprite(p2);
    }

    // blit buildings
    if(global::s2->getMapObj()->getRenderBuildHelp())
    {
        if(!isRSU)
        {
            switch(P2.build % 8)
            {
                case 0x01: Texture::getTexture(ArchiveID::MAP00, MAPPIC_FLAG).drawSprite(p2); break;
                case 0x02: Texture::getTexture(ArchiveID::MAP00, MAPPIC_HOUSE_SMALL).drawSprite(p2); break;
                case 0x03: Texture::getTexture(ArchiveID::MAP00, MAPPIC_HOUSE_MIDDLE).drawSprite(p2); break;
                case 0x04:
                    if(P2.rsuTexture & 0x40)
                        Texture::getTexture(ArchiveID::MAP00, MAPPIC_HOUSE_HARBOUR).drawSprite(p2);
                    else
                        Texture::getTexture(ArchiveID::MAP00, MAPPIC_HOUSE_BIG).drawSprite(p2);
                    break;
                case 0x05: Texture::getTexture(ArchiveID::MAP00, MAPPIC_MINE).drawSprite(p2); break;
                default: break;
            }
        }
    }
}

void CSurface::get_nodeVectors(bobMAP& myMap)
{
    // prepare triangle field
    int height = myMap.height;
    int width = myMap.width;
    IntVector tempP2, tempP3;

    // get flat vectors
    for(int j = 0; j < height - 1; j++)
    {
        if(j % 2 == 0)
        {
            // vector of first triangle
            tempP2.x = 0;
            tempP2.y = myMap.getVertex(width - 1, j + 1).y;
            tempP2.z = myMap.getVertex(width - 1, j + 1).z;
            myMap.getVertex(0, j).flatVector = get_flatVector(myMap.getVertex(0, j), tempP2, myMap.getVertex(0, j + 1));

            for(int i = 1; i < width; i++)
                myMap.getVertex(i, j).flatVector =
                  get_flatVector(myMap.getVertex(i, j), myMap.getVertex(i - 1, j + 1), myMap.getVertex(i, j + 1));
        } else
        {
            for(int i = 0; i < width - 1; i++)
                myMap.getVertex(i, j).flatVector =
                  get_flatVector(myMap.getVertex(i, j), myMap.getVertex(i, j + 1), myMap.getVertex(i + 1, j + 1));

            // vector of last triangle
            tempP3.x = myMap.getVertex(width - 1, j + 1).x + triangleWidth;
            tempP3.y = myMap.getVertex(0, j + 1).y;
            tempP3.z = myMap.getVertex(0, j + 1).z;
            myMap.getVertex(width - 1, j).flatVector =
              get_flatVector(myMap.getVertex(width - 1, j), myMap.getVertex(width - 1, j + 1), tempP3);
        }
    }
    // flat vectors of last line
    for(int i = 0; i < width - 1; i++)
    {
        tempP2 = myMap.getVertex(i, 0);
        tempP2.y += height * triangleHeight;
        tempP3 = myMap.getVertex(i + 1, 0);
        tempP3.y += height * triangleHeight;
        myMap.getVertex(i, height - 1).flatVector = get_flatVector(myMap.getVertex(i, height - 1), tempP2, tempP3);
    }
    // vector of last Triangle
    tempP2 = myMap.getVertex(width - 1, 0);
    tempP2.y += height * triangleHeight;
    tempP3.x = myMap.getVertex(width - 1, 0).x + triangleWidth;
    tempP3.y = height * triangleHeight + myMap.getVertex(0, 0).y;
    tempP3.z = myMap.getVertex(0, 0).z;
    myMap.getVertex(width - 1, height - 1).flatVector =
      get_flatVector(myMap.getVertex(width - 1, height - 1), tempP2, tempP3);

    // now get the vector at each node and save it to myMap.getVertex(j*width+i, 0).normVector
    for(int j = 0; j < height; j++)
    {
        if(j % 2 == 0)
        {
            for(int i = 0; i < width; i++)
            {
                MapNode& curVertex = myMap.getVertex(i, j);
                int iM1 = (i == 0 ? width - 1 : i - 1);
                if(j == 0) // first line
                    curVertex.normVector =
                      get_nodeVector(myMap.getVertex(iM1, height - 1).flatVector,
                                     myMap.getVertex(i, height - 1).flatVector, curVertex.flatVector);
                else
                    curVertex.normVector = get_nodeVector(myMap.getVertex(iM1, j - 1).flatVector,
                                                          myMap.getVertex(i, j - 1).flatVector, curVertex.flatVector);
                curVertex.i = get_LightIntensity(curVertex.normVector);
            }
        } else
        {
            for(int i = 0; i < width; i++)
            {
                MapNode& curVertex = myMap.getVertex(i, j);
                int iP1 = (i + 1 == width ? 0 : i + 1);

                curVertex.normVector = get_nodeVector(myMap.getVertex(i, j - 1).flatVector,
                                                      myMap.getVertex(iP1, j - 1).flatVector, curVertex.flatVector);
                curVertex.i = get_LightIntensity(curVertex.normVector);
            }
        }
    }
}

Sint32 CSurface::get_LightIntensity(const vector& node)
{
    // we calculate the light intensity right now
    float I, Ip = 1.1f, kd = 1, light_const = 1.0f;
    vector L = {-10, 5, 0.5f};
    L = get_normVector(L);
    I = Ip * kd * (node.x * L.x + node.y * L.y + node.z * L.z) + light_const;
    return (Sint32)(I * pow(2, 16));
}

vector CSurface::get_nodeVector(const vector& v1, const vector& v2, const vector& v3)
{
    vector node;
    // dividing through 3 is not necessary cause normal vector would be the same
    node.x = v1.x + v2.x + v3.x;
    node.y = v1.y + v2.y + v3.y;
    node.z = v1.z + v2.z + v3.z;
    node = get_normVector(node);
    return node;
}

vector CSurface::get_normVector(const vector& v)
{
    vector normal;
    auto length = static_cast<float>(sqrt(pow(v.x, 2) + pow(v.y, 2) + pow(v.z, 2)));
    // in case vector length equals 0 (should not happen)
    if(std::abs(length) < 1e-20f)
    {
        normal.x = 0;
        normal.y = 0;
        normal.z = 1;
    } else
    {
        normal = v;
        normal.x /= length;
        normal.y /= length;
        normal.z /= length;
    }

    return normal;
}

vector CSurface::get_flatVector(const IntVector& P1, const IntVector& P2, const IntVector& P3)
{
    // vector components
    float vax, vay, vaz, vbx, vby, vbz;
    // cross product
    vector cross;

    vax = static_cast<float>(P1.x - P2.x);
    vay = static_cast<float>(P1.y - P2.y);
    vaz = static_cast<float>(P1.z - P2.z);
    vbx = static_cast<float>(P3.x - P2.x);
    vby = static_cast<float>(P3.y - P2.y);
    vbz = static_cast<float>(P3.z - P2.z);

    cross.x = (vay * vbz - vaz * vby);
    cross.y = (vaz * vbx - vax * vbz);
    cross.z = (vax * vby - vay * vbx);
    // normalize
    cross = get_normVector(cross);

    return cross;
}

void CSurface::update_shading(bobMAP& myMap, Position pos)
{
    // vertex count for the points
    int X, Y;

    bool even = false;
    if(pos.y % 2 == 0)
        even = true;

    update_flatVectors(myMap, pos);
    update_nodeVector(myMap, pos);

    // now update all nodeVectors around pos
    // update first vertex left upside
    X = pos.x - (even ? 1 : 0);
    if(X < 0)
        X += myMap.width;
    Y = pos.y - 1;
    if(Y < 0)
        Y += myMap.height;
    update_nodeVector(myMap, Position(X, Y));
    // update second vertex right upside
    X = pos.x + (even ? 0 : 1);
    if(X >= myMap.width)
        X -= myMap.width;
    Y = pos.y - 1;
    if(Y < 0)
        Y += myMap.height;
    update_nodeVector(myMap, Position(X, Y));
    // update third point bottom left
    X = pos.x - 1;
    if(X < 0)
        X += myMap.width;
    Y = pos.y;
    update_nodeVector(myMap, Position(X, Y));
    // update fourth point bottom right
    X = pos.x + 1;
    if(X >= myMap.width)
        X -= myMap.width;
    Y = pos.y;
    update_nodeVector(myMap, Position(X, Y));
    // update fifth point down left
    X = pos.x - (even ? 1 : 0);
    if(X < 0)
        X += myMap.width;
    Y = pos.y + 1;
    if(Y >= myMap.height)
        Y -= myMap.height;
    update_nodeVector(myMap, Position(X, Y));
    // update sixth point down right
    X = pos.x + (even ? 0 : 1);
    if(X >= myMap.width)
        X -= myMap.width;
    Y = pos.y + 1;
    if(Y >= myMap.height)
        Y -= myMap.height;
    update_nodeVector(myMap, Position(X, Y));
}

void CSurface::update_flatVectors(bobMAP& myMap, Position pos)
{
    // point structures for the triangles, Pmiddle is the point in the middle of the hexagon we will update
    MapNode *P1, *P2, *P3, *Pmiddle;
    // vertex count for the points
    int P1x, P1y, P2x, P2y, P3x, P3y;

    bool even = false;
    if(pos.y % 2 == 0)
        even = true;

    Pmiddle = &myMap.getVertex(pos.x, pos.y);

    // update first triangle left upside
    P1x = pos.x - (even ? 1 : 0);
    if(P1x < 0)
        P1x += myMap.width;
    P1y = pos.y - 1;
    if(P1y < 0)
        P1y += myMap.height;
    P1 = &myMap.getVertex(P1x, P1y);
    P2x = pos.x - 1;
    if(P2x < 0)
        P2x += myMap.width;
    P2y = pos.y;
    P2 = &myMap.getVertex(P2x, P2y);
    P3 = Pmiddle;
    P1->flatVector = get_flatVector(*P1, *P2, *P3);

    // update second triangle right upside
    P1x = pos.x + (even ? 0 : 1);
    if(P1x >= myMap.width)
        P1x -= myMap.width;
    P1y = pos.y - 1;
    if(P1y < 0)
        P1y += myMap.height;
    P1 = &myMap.getVertex(P1x, P1y);
    P2 = Pmiddle;
    P3x = pos.x + 1;
    if(P3x >= myMap.width)
        P3x -= myMap.width;
    P3y = pos.y;
    P3 = &myMap.getVertex(P3x, P3y);
    P1->flatVector = get_flatVector(*P1, *P2, *P3);

    // update third triangle down middle
    P1 = Pmiddle;
    P2x = pos.x - (even ? 1 : 0);
    if(P2x < 0)
        P2x += myMap.width;
    P2y = pos.y + 1;
    if(P2y >= myMap.height)
        P2y -= myMap.height;
    P2 = &myMap.getVertex(P2x, P2y);
    P3x = pos.x + (even ? 0 : 1);
    if(P3x >= myMap.width)
        P3x -= myMap.width;
    P3y = pos.y + 1;
    if(P3y >= myMap.height)
        P3y -= myMap.height;
    P3 = &myMap.getVertex(P3x, P3y);
    P1->flatVector = get_flatVector(*P1, *P2, *P3);
}

void CSurface::update_nodeVector(bobMAP& myMap, Position pos)
{
    int j = pos.y;
    int i = pos.x;
    int width = myMap.width;
    int height = myMap.height;

    if(j % 2 == 0)
    {
        MapNode& curVertex = myMap.getVertex(i, j);
        int iM1 = (i == 0 ? width - 1 : i - 1);
        if(j == 0) // first line
            curVertex.normVector = get_nodeVector(myMap.getVertex(iM1, height - 1).flatVector,
                                                  myMap.getVertex(i, height - 1).flatVector, curVertex.flatVector);
        else
            curVertex.normVector = get_nodeVector(myMap.getVertex(iM1, j - 1).flatVector,
                                                  myMap.getVertex(i, j - 1).flatVector, curVertex.flatVector);
        curVertex.i = get_LightIntensity(curVertex.normVector);
    } else
    {
        MapNode& curVertex = myMap.getVertex(i, j);
        int iP1 = (i + 1 == width ? 0 : i + 1);

        curVertex.normVector = get_nodeVector(myMap.getVertex(i, j - 1).flatVector,
                                              myMap.getVertex(iP1, j - 1).flatVector, curVertex.flatVector);
        curVertex.i = get_LightIntensity(curVertex.normVector);
    }
}

float CSurface::absf(float a)
{
    if(a >= 0)
        return a;
    else
        return a * (-1);
}

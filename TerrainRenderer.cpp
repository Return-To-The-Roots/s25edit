// Copyright (C) 2025 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Adapted from s25client: libs/s25main/TerrainRenderer.cpp
//   Original copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)

#include "TerrainRenderer.h"
#include "CSurfaceGL.h"
#include "Rect.h"
#include "globals.h"
#include "ogl/VBO.h"
#include "ogl/constants.h"
#include "gameData/EdgeDesc.h"
#include "gameData/TerrainDesc.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <emmintrin.h>

// ===========================================================================
// Types  (s25client: Triangle / ColorTriangle)
// ===========================================================================

using TriPos = std::array<std::array<float, 2>, 3>;
using TriTex = std::array<std::array<float, 2>, 3>;
struct VertColor
{
    uint8_t r, g, b, a;
};
using TriCol = std::array<VertColor, 3>;

// ===========================================================================
// Texture LUT  —  hardcoded atlas pixel coords  (original editor code)
// These have margin for UV shifting, unlike GetRSUTriangle which sits at
// the sub-rect edges.  Copied from CSurface::GetTerrainTextureCoords.
// ===========================================================================

struct TexBase
{
    int ux, uy, lx, ly, rx, ry;
};
static TexBase s_texLUT[64][2];

static void initTexLUT()
{
    for(int t = 0; t < 64; t++)
    {
        for(int r = 0; r < 2; r++)
        {
            auto& e = s_texLUT[t][r];
            bool rsu = (r == 1);
            switch(TriangleTerrainType(t))
            {
                case TRIANGLE_TEXTURE_STEPPE_MEADOW1: e = {17, 96, 0, 126, 35, 126}; break;
                case TRIANGLE_TEXTURE_MINING1: e = {17, 48, 0, 78, 35, 78}; break;
                case TRIANGLE_TEXTURE_SNOW:
                    if(rsu)
                        e = {17, 0, 0, 30, 35, 30};
                    else
                        e = {17, 28, 0, 0, 37, 0};
                    break;
                case TRIANGLE_TEXTURE_SWAMP: e = {113, 0, 96, 30, 131, 30}; break;
                case TRIANGLE_TEXTURE_STEPPE:
                case TRIANGLE_TEXTURE_STEPPE_:
                case TRIANGLE_TEXTURE_STEPPE__:
                case TRIANGLE_TEXTURE_STEPPE___: e = {65, 0, 48, 30, 83, 30}; break;
                case TRIANGLE_TEXTURE_WATER:
                case TRIANGLE_TEXTURE_WATER_:
                case TRIANGLE_TEXTURE_WATER__:
                    if(rsu)
                        e = {231, 61, 207, 62, 223, 78};
                    else
                        e = {224, 79, 232, 62, 245, 76};
                    break;
                case TRIANGLE_TEXTURE_MEADOW1: e = {65, 96, 48, 126, 83, 126}; break;
                case TRIANGLE_TEXTURE_MEADOW2: e = {113, 96, 96, 126, 131, 126}; break;
                case TRIANGLE_TEXTURE_MEADOW3: e = {161, 96, 144, 126, 179, 126}; break;
                case TRIANGLE_TEXTURE_MINING2: e = {65, 48, 48, 78, 83, 78}; break;
                case TRIANGLE_TEXTURE_MINING3: e = {113, 48, 96, 78, 131, 78}; break;
                case TRIANGLE_TEXTURE_MINING4: e = {161, 48, 144, 78, 179, 78}; break;
                case TRIANGLE_TEXTURE_STEPPE_MEADOW2: e = {17, 144, 0, 174, 35, 174}; break;
                case TRIANGLE_TEXTURE_FLOWER: e = {161, 0, 144, 30, 179, 30}; break;
                case TRIANGLE_TEXTURE_LAVA:
                    if(rsu)
                        e = {231, 117, 207, 118, 223, 134};
                    else
                        e = {224, 135, 232, 118, 245, 132};
                    break;
                default: e = {161, 0, 144, 30, 179, 30}; break;
            }
        }
    }
}

// ===========================================================================
// Shading
// ===========================================================================

static uint8_t intensityToColor(Sint32 val)
{
    if(val <= 0)
        return 0;
    constexpr Sint32 maxVal = 2 * 65536;
    if(val >= maxVal)
        return 255;
    return static_cast<uint8_t>((val * 255 + maxVal / 2) / maxVal);
}

static uint8_t borderIntensity(Sint32 v)
{
    if(v <= 0)
        return 0;
    if(v >= 65536)
        return 255;
    return static_cast<uint8_t>((v * 255 + 32768) / 65536);
}

// ===========================================================================
// Module-level state
// ===========================================================================

static int s_W = 0, s_H = 0, s_Wpx = 0, s_Hpx = 0;
static float s_texW = 1, s_texH = 1;
static SDL_Surface* s_tileset = nullptr;

// Per-triangle cached data (2 per vertex)
struct CachedTri
{
    float t0u, t0v, t1u, t1v, t2u, t2v;
    uint8_t c0, c1, c2;
    bool isWaterLava;
};
static std::vector<CachedTri> s_cached;

// Water animation base UVs
static std::vector<std::array<float, 6>> s_waterUV;
static int s_lastWaterAnim = -1;

// Combined VBO: terrain then borders
static ogl::VBO<TriPos> s_posVBO;
static ogl::VBO<TriTex> s_texVBO;
static ogl::VBO<TriCol> s_colVBO;
static size_t s_terrainTris = 0;
static size_t s_borderTris = 0;
static bool s_valid = false;

// Animation
static int s_texMove = 0;
static int s_round = 0;
static Uint32 s_lastRound = 0, s_lastTex = 0;

// ===========================================================================
// Helpers
// ===========================================================================

static int wX(int x)
{
    return ((x % s_W) + s_W) % s_W;
}
static int wY(int y)
{
    return ((y % s_H) + s_H) % s_H;
}

// Euclidean floor division for positive divisors (map pixel dimensions).
static int floorDiv(int a, int b)
{
    if(a >= 0)
        return a / b;
    return (a - b + 1) / b;
}

static int vIdx(int x, int y)
{
    return wY(y) * s_W + wX(x);
}
static size_t tIdx(int x, int y, int sub)
{
    return size_t(vIdx(x, y)) * 2 + sub;
}

static Point32 wrapNear2(const MapNode& n, const Point32& ref)
{
    Point32 p(n.x, n.y);
    if(p.x - ref.x < -s_Wpx / 2)
        p.x += s_Wpx;
    else if(p.x - ref.x > s_Wpx / 2)
        p.x -= s_Wpx;
    if(p.y - ref.y < -s_Hpx / 2)
        p.y += s_Hpx;
    else if(p.y - ref.y > s_Hpx / 2)
        p.y -= s_Hpx;
    return p;
}

static MapNode getMapNodeWrapped(const bobMAP& map, int x, int y)
{
    return map.getVertex(wX(x), wY(y));
}

static Point32 getPosWrapped(const bobMAP& map, int x, int y, const Point32& ref)
{
    return wrapNear2(getMapNodeWrapped(map, x, y), ref);
}

static uint8_t getBorderColor(const bobMAP& map, int x, int y)
{
    return borderIntensity(getMapNodeWrapped(map, x, y).i);
}

static Point32 rsuCentroid(const bobMAP& map, int x, int y, const Point32& ref)
{
    Point32 pt = getPosWrapped(map, x, y, ref);
    Point32 sw = getPosWrapped(map, x - !(y & 1), y + 1, ref);
    Point32 se = getPosWrapped(map, x + (y & 1), y + 1, ref);
    return Point32((pt.x + sw.x + se.x) / 3, (pt.y + sw.y + se.y) / 3);
}

static Point32 usdCentroid(const bobMAP& map, int x, int y, const Point32& ref)
{
    Point32 pt = getPosWrapped(map, x, y, ref);
    Point32 se = getPosWrapped(map, x + (y & 1), y + 1, ref);
    Point32 e = getPosWrapped(map, x + 1, y, ref);
    return Point32((pt.x + se.x + e.x) / 3, (pt.y + se.y + e.y) / 3);
}

static TriangleTerrainType normaliseTexture(uint8_t raw)
{
    raw &= ~0x40;
    switch(raw)
    {
        case TRIANGLE_TEXTURE_WATER_:
        case TRIANGLE_TEXTURE_WATER__: return TRIANGLE_TEXTURE_WATER;
        case TRIANGLE_TEXTURE_STEPPE__:
        case TRIANGLE_TEXTURE_STEPPE___: return TRIANGLE_TEXTURE_STEPPE;
        default: return TriangleTerrainType(raw);
    }
}

// ===========================================================================
// Edge-type detection  —  s25client: GetEdgeType
// ===========================================================================

static DescIdx<EdgeDesc> GetEdgeType(const TerrainDesc& t1, const TerrainDesc& t2)
{
    if(!t1.edgeType || t1.edgePriority <= t2.edgePriority)
        return {};
    return t1.edgeType;
}

static const TerrainDesc& getTerrain(const bobMAP& map, uint8_t raw)
{
    raw &= ~(0x40 | 0x80);
    return global::worldDesc.get(map.s2IdToTerrain[raw]);
}

// ===========================================================================
// Per-triangle cache computation  (working LUT-based approach)
// ===========================================================================

static void computeCached(const bobMAP& m, int x, int y, int sub)
{
    size_t idx = tIdx(x, y, sub);
    CachedTri& ct = s_cached[idx];
    bool rsu = (sub == 0);

    int texX = x, texY = y;
    if(!rsu && y % 2 == 0)
        texX = x - 1;

    const MapNode& n = m.getVertex(wX(texX), wY(texY));
    uint8_t raw = rsu ? n.rsuTexture : n.usdTexture;
    int ti = int(normaliseTexture(raw));
    if(ti < 0 || ti >= 64)
        ti = 0;
    const TexBase& b = s_texLUT[ti][rsu ? 1 : 0];

    ct.t0u = float(b.ux) / s_texW;
    ct.t0v = float(b.uy) / s_texH;
    ct.t1u = float(b.lx) / s_texW;
    ct.t1v = float(b.ly) / s_texH;
    ct.t2u = float(b.rx) / s_texW;
    ct.t2v = float(b.ry) / s_texH;
    ct.isWaterLava = (ti == TRIANGLE_TEXTURE_WATER || ti == TRIANGLE_TEXTURE_LAVA);

    if(ct.isWaterLava)
    {
        auto& w = s_waterUV[idx];
        w[0] = ct.t0u;
        w[1] = ct.t0v;
        w[2] = ct.t1u;
        w[3] = ct.t1v;
        w[4] = ct.t2u;
        w[5] = ct.t2v;
    }

    int vx[3], vy[3];
    if(rsu)
    {
        if(y % 2 == 0)
        {
            vx[0] = x;
            vy[0] = y;
            vx[1] = x - 1;
            vy[1] = y + 1;
            vx[2] = x;
            vy[2] = y + 1;
        } else
        {
            vx[0] = x;
            vy[0] = y;
            vx[1] = x;
            vy[1] = y + 1;
            vx[2] = x + 1;
            vy[2] = y + 1;
        }
    } else
    {
        if(y % 2 == 0)
        {
            vx[0] = x - 1;
            vy[0] = y + 1;
            vx[1] = x - 1;
            vy[1] = y;
            vx[2] = x;
            vy[2] = y;
        } else
        {
            vx[0] = x + 1;
            vy[0] = y + 1;
            vx[1] = x;
            vy[1] = y;
            vx[2] = x + 1;
            vy[2] = y;
        }
    }

    Sint32 i[3];
    if(ct.isWaterLava)
    {
        i[0] = i[1] = i[2] = 65536;
    } else
    {
        for(int k = 0; k < 3; k++)
            i[k] = m.getVertex(wX(vx[k]), wY(vy[k])).i;
    }

    ct.c0 = intensityToColor(i[0]);
    ct.c1 = intensityToColor(i[1]);
    ct.c2 = intensityToColor(i[2]);
}

// ===========================================================================
// Build terrain VBO arrays from cached data
// ===========================================================================

static void buildTerrainVBOs(const bobMAP& m, std::vector<TriPos>& pos, std::vector<TriTex>& tex,
                             std::vector<TriCol>& col)
{
    pos.clear();
    tex.clear();
    col.clear();
    pos.reserve(size_t(s_W) * size_t(s_H) * 2);
    tex.reserve(pos.capacity());
    col.reserve(pos.capacity());

    for(int y = 0; y < s_H; y++)
    {
        for(int x = 0; x < s_W; x++)
        {
            const bool isXSeam = ((y % 2 == 0) && x == 0) || ((y % 2 != 0) && x == s_W - 1);

            for(int sub = 0; sub < 2; sub++)
            {
                const size_t idx = tIdx(x, y, sub);
                const CachedTri& ct = s_cached[idx];
                const bool rsu = (sub == 0);

                int vx[3], vy[3];
                if(rsu)
                {
                    if(y % 2 == 0)
                    {
                        vx[0] = x;
                        vy[0] = y;
                        vx[1] = x - 1;
                        vy[1] = y + 1;
                        vx[2] = x;
                        vy[2] = y + 1;
                    } else
                    {
                        vx[0] = x;
                        vy[0] = y;
                        vx[1] = x;
                        vy[1] = y + 1;
                        vx[2] = x + 1;
                        vy[2] = y + 1;
                    }
                } else
                {
                    if(y % 2 == 0)
                    {
                        vx[0] = x - 1;
                        vy[0] = y + 1;
                        vx[1] = x - 1;
                        vy[1] = y;
                        vx[2] = x;
                        vy[2] = y;
                    } else
                    {
                        vx[0] = x + 1;
                        vy[0] = y + 1;
                        vx[1] = x;
                        vy[1] = y;
                        vx[2] = x + 1;
                        vy[2] = y;
                    }
                }

                auto getX = [&](int p) {
                    float xf = float(m.getVertex(wX(vx[p]), wY(vy[p])).x);
                    if(isXSeam)
                    {
                        if(y % 2 == 0)
                        {
                            if((rsu && p == 1) || (!rsu && (p == 0 || p == 1)))
                                if(xf > float(s_Wpx) / 2)
                                    xf -= float(s_Wpx);
                        } else
                        {
                            if((rsu && p == 2) || (!rsu && (p == 0 || p == 2)))
                                if(xf < float(s_Wpx) / 2)
                                    xf += float(s_Wpx);
                        }
                    }
                    return xf;
                };
                auto getY = [&](int p) {
                    float yf = float(m.getVertex(wX(vx[p]), wY(vy[p])).y);
                    if(y == s_H - 1 && vy[p] == s_H)
                        yf += float(s_Hpx);
                    return yf;
                };

                TriPos tp;
                tp[0] = {{getX(0), getY(0)}};
                tp[1] = {{getX(1), getY(1)}};
                tp[2] = {{getX(2), getY(2)}};
                pos.push_back(tp);

                tex.push_back({{{{ct.t0u, ct.t0v}}, {{ct.t1u, ct.t1v}}, {{ct.t2u, ct.t2v}}}});

                TriCol tc;
                tc[0] = {ct.c0, ct.c0, ct.c0, 255};
                tc[1] = {ct.c1, ct.c1, ct.c1, 255};
                tc[2] = {ct.c2, ct.c2, ct.c2, 255};
                col.push_back(tc);
            }
        }
    }
}

// ===========================================================================
// Border triangle computation
// ===========================================================================

static void emitBorderTri(std::vector<TriPos>& pos, std::vector<TriTex>& tex, std::vector<TriCol>& col,
                          const Point32& v0, const Point32& v1, const Point32& v2, const SDL_Rect& src, uint8_t c0,
                          uint8_t c1, uint8_t c2, bool swapTip)
{
    float uLeft = float(src.x) / s_texW;
    float vTop = float(src.y) / s_texH;
    float uRight = float(src.x + src.w) / s_texW;
    float uMid = float(src.x + src.w / 2) / s_texW;
    float vBot = float(src.y + src.h) / s_texH;

    TriPos tp = {{{{float(v0.x), float(v0.y)}}, {{float(v1.x), float(v1.y)}}, {{float(v2.x), float(v2.y)}}}};
    pos.push_back(tp);

    // s25client: for i==1 the tip/centroid (v0) uses the bottom-centre texcoord.
    if(swapTip)
        tex.push_back({{{{uMid, vBot}}, {{uRight, vTop}}, {{uLeft, vTop}}}});
    else
        tex.push_back({{{{uLeft, vTop}}, {{uRight, vTop}}, {{uMid, vBot}}}});

    TriCol tc;
    tc[0] = {c0, c0, c0, 255};
    tc[1] = {c1, c1, c1, 255};
    tc[2] = {c2, c2, c2, 255};
    col.push_back(tc);
}

static SDL_Rect edgeRect(DescIdx<EdgeDesc> idx)
{
    const EdgeDesc& ed = global::worldDesc.get(idx);
    const Rect& r = ed.posInTexture;
    return SDL_Rect{Sint16(r.getOrigin().x), Sint16(r.getOrigin().y), Uint16(r.getSize().x), Uint16(r.getSize().y)};
}

/// Generate border triangles aligned with s25client::TerrainRenderer.
/// See GEOMETRY.md for the mapping between editor and s25client geometry.
static void buildBorderVBOs(const bobMAP& map, std::vector<TriPos>& pos, std::vector<TriTex>& tex,
                            std::vector<TriCol>& col)
{
    for(int y = 0; y < s_H; y++)
    {
        for(int x = 0; x < s_W; x++)
        {
            // Reference vertex and its s25client neighbours.
            const MapNode& ptNode = map.getVertex(x, y);
            Point32 ptRef(ptNode.x, ptNode.y);

            int ex = x + 1, ey = y;
            int sex = x + (y & 1), sey = y + 1;
            int swx = x - !(y & 1), swy = y + 1;

            Point32 pt = getPosWrapped(map, x, y, ptRef);
            Point32 e = getPosWrapped(map, ex, ey, ptRef);
            Point32 se = getPosWrapped(map, sex, sey, ptRef);
            Point32 sw = getPosWrapped(map, swx, swy, ptRef);

            // Terrain samples for edge-type decisions (s25client indexing).
            // Note: vertices are wrapped because ex/x+1 and swy/y+1 can be one
            // past the map edge.
            const TerrainDesc& t1 = getTerrain(map, map.getVertex(x, y).rsuTexture);
            const TerrainDesc& t2 = getTerrain(map, map.getVertex(x, y).usdTexture);
            const TerrainDesc& t3 = getTerrain(map, getMapNodeWrapped(map, ex, ey).rsuTexture);
            const TerrainDesc& t4 = getTerrain(map, getMapNodeWrapped(map, swx, swy).usdTexture);

            // Vertex colours and centroids used by s25client border triangles.
            uint8_t c_pt = getBorderColor(map, x, y);
            uint8_t c_e = getBorderColor(map, ex, ey);
            uint8_t c_se = getBorderColor(map, sex, sey);
            uint8_t c_sw = getBorderColor(map, swx, swy);

            Point32 c_rsu = rsuCentroid(map, x, y, ptRef);
            Point32 c_usd = usdCentroid(map, x, y, ptRef);
            Point32 c_rsuE = rsuCentroid(map, ex, ey, ptRef);
            Point32 c_usdSW = usdCentroid(map, swx, swy, ptRef);

            uint8_t c_rsuCentroid = (c_sw + c_pt + c_se) / 3;
            uint8_t c_usdCentroid = (c_e + c_pt + c_se) / 3;

            uint8_t c_e_sw = getBorderColor(map, ex - !(ey & 1), ey + 1);
            uint8_t c_e_se = getBorderColor(map, ex + (ey & 1), ey + 1);
            uint8_t c_rsuECentroid = (c_e + c_e_sw + c_e_se) / 3;

            uint8_t c_sw_e = getBorderColor(map, swx + 1, swy);
            uint8_t c_sw_se = getBorderColor(map, swx + (swy & 1), swy + 1);
            uint8_t c_usdSWCentroid = (c_sw + c_sw_se + c_sw_e) / 3;

            // left_right[0]: USD over RSU
            if(DescIdx<EdgeDesc> edge = GetEdgeType(t2, t1))
                emitBorderTri(pos, tex, col, pt, se, c_rsu, edgeRect(edge), c_pt, c_se, c_rsuCentroid, false);

            // left_right[1]: RSU over USD
            if(DescIdx<EdgeDesc> edge = GetEdgeType(t1, t2))
                emitBorderTri(pos, tex, col, c_usd, se, pt, edgeRect(edge), c_usdCentroid, c_se, c_pt, true);

            // right_left[0]: RSU(E) over USD
            if(DescIdx<EdgeDesc> edge = GetEdgeType(t3, t2))
                emitBorderTri(pos, tex, col, se, e, c_usd, edgeRect(edge), c_se, c_e, c_usdCentroid, false);

            // right_left[1]: USD over RSU(E)
            if(DescIdx<EdgeDesc> edge = GetEdgeType(t2, t3))
                emitBorderTri(pos, tex, col, c_rsuE, e, se, edgeRect(edge), c_rsuECentroid, c_e, c_se, true);

            // top_down[0]: USD(SW) over RSU
            if(DescIdx<EdgeDesc> edge = GetEdgeType(t4, t1))
                emitBorderTri(pos, tex, col, sw, se, c_rsu, edgeRect(edge), c_sw, c_se, c_rsuCentroid, false);

            // top_down[1]: RSU over USD(SW)
            if(DescIdx<EdgeDesc> edge = GetEdgeType(t1, t4))
                emitBorderTri(pos, tex, col, c_usdSW, se, sw, edgeRect(edge), c_usdSWCentroid, c_se, c_sw, true);
        }
    }
}

// ===========================================================================
// TerrainRenderer namespace
// ===========================================================================

namespace TerrainRenderer {

void Init(const Extent& /*size*/)
{
    initTexLUT();
    s_valid = false;
}

void setCamera(int x, int y, int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(x, x + w, y + h, y, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void setScreenProjection(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// -----------------------------------------------------------------------
// GenerateOpenGL  —  s25client: TerrainRenderer::GenerateOpenGL
// -----------------------------------------------------------------------
void GenerateOpenGL(const bobMAP& map, SDL_Surface* tileset)
{
    s_W = map.width;
    s_H = map.height;
    s_Wpx = map.width_pixel;
    s_Hpx = map.height_pixel;
    const EditorGLTexture& tex = CSurfaceGL::getOrCreateTexture(tileset);
    s_texW = float(tex.texWidth());
    s_texH = float(tex.texHeight());
    s_tileset = tileset;

    size_t nV = size_t(s_W) * size_t(s_H);
    s_cached.assign(nV * 2, CachedTri{});
    s_waterUV.assign(nV * 2, std::array<float, 6>{});
    s_lastWaterAnim = -1;

    for(int y = 0; y < s_H; y++)
        for(int x = 0; x < s_W; x++)
        {
            computeCached(map, x, y, 0);
            computeCached(map, x, y, 1);
        }

    std::vector<TriPos> pos;
    std::vector<TriTex> txc;
    std::vector<TriCol> col;
    buildTerrainVBOs(map, pos, txc, col);
    s_terrainTris = pos.size();

    buildBorderVBOs(map, pos, txc, col);
    s_borderTris = pos.size() - s_terrainTris;

    s_posVBO = ogl::VBO<TriPos>(ogl::Target::Array);
    s_texVBO = ogl::VBO<TriTex>(ogl::Target::Array);
    s_colVBO = ogl::VBO<TriCol>(ogl::Target::Array);
    s_posVBO.fill(pos, ogl::Usage::Static);
    // Texcoords are updated every animation frame for water/lava.
    s_texVBO.fill(txc, ogl::Usage::Dynamic);
    s_colVBO.fill(col, ogl::Usage::Static);

    s_cached.shrink_to_fit();
    s_waterUV.shrink_to_fit();

    s_valid = true;
}

void AltitudeChanged(int /*x*/, int /*y*/, const bobMAP& /*map*/)
{
    // Full regeneration is simpler in the editor; CMap::Draw will call
    // GenerateOpenGL on the next frame when isTerrainValid() is false.
    s_valid = false;
}

// -----------------------------------------------------------------------
// Draw  —  s25client: TerrainRenderer::Draw
// -----------------------------------------------------------------------
void Draw(const DisplayRectangle& displayRect)
{
    if(!s_valid || !s_tileset)
        return;
    const EditorGLTexture& tex = CSurfaceGL::getOrCreateTexture(s_tileset);
    if(!tex.valid())
        return;

    updateAnimation();

    // Water/lava animation: shift texcoords (LUT values have margin for this)
    if(s_texMove != s_lastWaterAnim)
    {
        s_lastWaterAnim = s_texMove;
        float du = float(s_texMove) / s_texW, dv = float(s_texMove) / s_texH;
        std::vector<TriTex> at(s_cached.size());
        for(size_t i = 0; i < s_cached.size(); i++)
        {
            if(s_cached[i].isWaterLava)
            {
                auto& w = s_waterUV[i];
                at[i][0] = {{w[0] - du, w[1] + dv}};
                at[i][1] = {{w[2] - du, w[3] + dv}};
                at[i][2] = {{w[4] - du, w[5] + dv}};
            } else
            {
                at[i][0] = {{s_cached[i].t0u, s_cached[i].t0v}};
                at[i][1] = {{s_cached[i].t1u, s_cached[i].t1v}};
                at[i][2] = {{s_cached[i].t2u, s_cached[i].t2v}};
            }
        }
        // Update only terrain portion of texcoord VBO (borders follow after)
        if(!at.empty() && at.size() <= s_terrainTris)
            s_texVBO.update(at, 0);
    }

    glBindTexture(GL_TEXTURE_2D, tex.texture());

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    s_posVBO.bind();
    glVertexPointer(2, GL_FLOAT, sizeof(std::array<float, 2>), nullptr);
    s_texVBO.bind();
    glTexCoordPointer(2, GL_FLOAT, sizeof(std::array<float, 2>), nullptr);
    s_colVBO.bind();
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(VertColor), nullptr);

    // Wrap passes: render every copy of the map whose bounding box intersects
    // the visible display rectangle.  When the map is zoomed out (or on a 4K
    // display) the view can cover many wraps, so generate the full Cartesian
    // product of required offsets instead of a single neighbour pass.
    struct Off
    {
        int dx, dy;
    };
    std::vector<Off> passes;

    // The canonical VBO contains seam triangles that extend beyond the map's
    // pixel bounds (up to one triangle width/height).  Include neighbouring
    // wrap copies when the visible area intersects those extended bounds, so
    // triangles at the left/top edge are not clipped away when the seam sits
    // just outside the screen.
    const int minDx = floorDiv(displayRect.left - triangleWidth, s_Wpx);
    const int maxDx = floorDiv(displayRect.right - 1 + triangleWidth, s_Wpx);
    const int minDy = floorDiv(displayRect.top - triangleHeight, s_Hpx);
    const int maxDy = floorDiv(displayRect.bottom - 1 + triangleHeight, s_Hpx);

    passes.reserve(static_cast<size_t>(maxDx - minDx + 1) * static_cast<size_t>(maxDy - minDy + 1));
    for(int dy = minDy; dy <= maxDy; ++dy)
        for(int dx = minDx; dx <= maxDx; ++dx)
            passes.push_back({dx * s_Wpx, dy * s_Hpx});

    // --- Terrain ---
    glDisable(GL_BLEND);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 2.0f);

    glPushMatrix();
    for(auto& pass : passes)
    {
        glPushMatrix();
        glTranslatef(GLfloat(pass.dx), GLfloat(pass.dy), 0);
        glDrawArrays(GL_TRIANGLES, 0, GLsizei(s_terrainTris * 3));
        glPopMatrix();
    }
    glPopMatrix();

    // --- Borders ---
    if(s_borderTris > 0)
    {
        glEnable(GL_BLEND);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        glPushMatrix();
        for(auto& pass : passes)
        {
            glPushMatrix();
            glTranslatef(GLfloat(pass.dx), GLfloat(pass.dy), 0);
            glDrawArrays(GL_TRIANGLES, GLsizei(s_terrainTris * 3), GLsizei(s_borderTris * 3));
            glPopMatrix();
        }
        glPopMatrix();
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

bool isTerrainValid()
{
    return s_valid;
}
void invalidateTerrain()
{
    s_valid = false;
}
float texWidth()
{
    return s_texW;
}
float texHeight()
{
    return s_texH;
}
int mapWidthPx()
{
    return s_Wpx;
}
int mapHeightPx()
{
    return s_Hpx;
}

void updateAnimation()
{
    Uint32 now = SDL_GetTicks();
    if(now - s_lastRound > 30)
    {
        s_lastRound = now;
        s_round = (s_round >= 7) ? 0 : s_round + 1;
    }
    if(now - s_lastTex > 170)
    {
        s_lastTex = now;
        s_texMove++;
        if(s_texMove > 14)
            s_texMove = 0;
    }
}

int textureOffset()
{
    return s_texMove;
}
int roundCount()
{
    return s_round;
}

} // namespace TerrainRenderer

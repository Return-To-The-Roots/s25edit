// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Point.h"
#include "Rect.h"
#include "gameData/DescIdx.h"
#include <SDL.h>
#include <array>
#include <string>
#include <vector>

// structure for display, cause SDL_Rect's datatypes are too small
using DisplayRectangle = RectBase<Sint32>;
using Point16 = Point<Sint16>;
using Point32 = Point<Sint32>;
using Extent16 = Point<Uint16>;

struct TerrainDesc;

// define the mode to compile (if all is uncommented, the game will compile in normal mode
// in admin mode, there are some key combos to open debugger, resource viewer and so on
// #define _ADMINMODE

// callback parameters
enum
{
    // NOTE: we don't have a global WINDOW_QUIT_MESSAGE, cause if there were more than one window created by a callback
    // function
    //      we wouldn't know which window was closed by the user. so we need a specified quit-message for each window.

    // first call of a callback function
    INITIALIZING_CALL = -1,
    // a callback that is registered at the gameloop will be called from the gameloop with this value
    CALL_FROM_GAMELOOP = -2,
    // if user goes to main menu, all menubar callbacks will be called with MAP_QUIT
    MAP_QUIT = -3,
    // parameter for closing the debugger window
    DEBUGGER_QUIT = -4,
    // this will happen every time the user clicks anywhere on the window
    WINDOW_CLICKED_CALL = -5,
    // this window quit message is ONLY usable to call a callback function explicit with this value
    WINDOW_QUIT_MESSAGE = -6,
    // this will happen every time the window is resized
    WINDOW_RESIZED_CALL = -7
};

// BOBTYPES
enum
{
    BOBTYPE01 = 1,
    BOBTYPE02 = 2,
    BOBTYPE03 = 3,
    BOBTYPE04 = 4,
    BOBTYPE05 = 5,
    BOBTYPE07 = 7,
    BOBTYPE14 = 14
};

// FILE ENDINGS
enum
{
    LST = 0,
    BOB,
    IDX,
    BBM,
    LBM,
    WLD,
    SWD
};

// bobBMP, bobPAL, bobSHADOW replaced by libsiedler2 types
// bobBMP  -> libsiedler2::ArchivItem_Bitmap
// bobPAL  -> libsiedler2::ArchivItem_Palette
// bobSHADOW -> libsiedler2::ArchivItem_Bitmap_Shadow
// Use the helper functions declared in globals.h for access

// Datatypes for the Map
// vector structure
struct vector
{
    float x, y, z;
};
struct IntVector
{
    Sint32 x, y, z;
};
// structure for the 250 9Byte-Items from die 2250Bytes long map header
struct MapHeaderItem
{
    Uint8 type; // land or water (snow, swamp and lava are not counted)
    Uint16 x;
    Uint16 y;
    Uint32 area; // number of vertices this area has
};
// point structure
struct MapNode
{
    Uint16 VertexX; /* number of the vertex on x-axis */
    Uint16 VertexY; /* number of the vertex on y-axis */
    Sint32 x;
    Sint32 y; /* calculated with section 1 */
    int z;    /* calculated with section 1 */
    Uint8 h;  /* section 1 */
    Sint32 i; /* calculated light values for new shading by SGE (a 16 bit integer shifted left 16 times --> fixed point
                 math for speed) */
    vector flatVector;
    vector normVector;
    Uint8 rsuTexture; /* section 2 */
    Uint8 usdTexture; /* section 3 */
    Uint8 road;       /* section 4 */
    Uint8 objectType; /* section 5 */
    Uint8 objectInfo; /* section 6 */
    Uint8 animal;     /* section 7 */
    Uint8 unknown1;   /* section 8 */
    Uint8 build;      /* section 9 */
    Uint8 unknown2;   /* section 10 */
    Uint8 unknown3;   /* section 11 */
    Uint8 resource;   /* section 12 */
    Uint8 shading;    /* section 13 */
    Uint8 unknown5;   /* section 14 */

    operator IntVector() const
    {
        IntVector result;
        result.x = x;
        result.y = y;
        result.z = z;
        return result;
    }
};
// map types
enum MapType
{
    MAP_GREENLAND = 0x00,
    MAP_WASTELAND = 0x01,
    MAP_WINTERLAND = 0x02
};
// map strutcture
struct bobMAP
{
    Uint16 height;
    Uint16 height_old;
    Uint16 height_pixel;
    Uint16 width;
    Uint16 width_old;
    Uint16 width_pixel;
    MapType type;
    Uint8 player;
    // these are the original values
    std::array<Uint16, 7> HQx;
    std::array<Uint16, 7> HQy;
    // 250 items from the big map header
    std::array<MapHeaderItem, 250> header;
    std::vector<MapNode> vertex;
    MapNode& getVertex(unsigned x, unsigned y) { return vertex[y * width + x]; }
    MapNode& getVertex(Point32 pos) { return vertex[pos.y * width + pos.x]; }
    const MapNode& getVertex(unsigned x, unsigned y) const { return vertex[y * width + x]; }
    const MapNode& getVertex(Point32 pos) const { return vertex[pos.y * width + pos.x]; }
    std::vector<DescIdx<TerrainDesc>> s2IdToTerrain;
    // Initializes or updates the vertex indices and coordinates
    void initVertexCoords();
    /// Updates x,y,z positions (e.g. after height change)
    void updateVertexCoords();

    const std::string& getName() const { return name; }
    const std::string& getAuthor() const { return author; }
    void setName(const std::string& newName);
    void setAuthor(const std::string& newAuthor);

private:
    std::string name;
    std::string author;
};
// structure to save vertex coordinates
struct cursorPoint : public Point32
{
    Position blit;
    bool active;
    bool fill_rsu;
    bool fill_usd;
};

// IMPORTANT: for enumerating the contents of loaded files put the constants in the right order here.
//           if the order of file loading changes, so change the constants in the same way!
//           (these are the array-indices for an array of BobtypeBMP-Structures)

// enumeration for BobtypePAL (palettes)
enum
{
    RESOURCE_PALETTE = 0,
    IO_PALETTE
};

// font alignment (after all used by CFont and other objects using CFont)
enum class FontAlign
{
    Left = 0,
    Middle,
    Right
};

/// Font color in order they are loaded (see usage of NUM_FONT_COLORS)
enum class FontColor
{
    Blue,
    Red,
    Orange,
    Green,
    MintGreen,
    Yellow,
    BrightRed,
};
constexpr unsigned NUM_FONT_COLORS = static_cast<unsigned>(FontColor::BrightRed) + 1u;

/// Font sizes with values equal to height in pixels
enum class FontSize
{
    Small = 9,
    Medium = 11,
    Large = 14
};
/// Height of 1 line for the given font including vertical spacing
inline unsigned getLineHeight(FontSize size)
{
    const auto fontSize = static_cast<unsigned>(size);
    switch(size)
    {
        default:
        case FontSize::Small: return fontSize + 1;
        case FontSize::Medium: return fontSize + 3;
        case FontSize::Large: return fontSize + 4;
    }
}

// player colors, necessary for the read_bob03- and read_bob04-function
enum
{
    PLAYER_BLUE = 0x80,
    PLAYER_RED = 0x88,
    PLAYER_ORANGE = 0x04,
    PLAYER_GREEN = 0x85,
    PLAYER_MINTGREEN = 0x94,
    PLAYER_YELLOW = 0x01,
    PLAYER_RED_BRIGHT = 0x10
};

// BEGIN: /GFX/PICS/SETUP997.LBM
constexpr int SPLASHSCREEN_LOADING_S2SCREEN = 0;
// END: /GFX/PICS/SETUP997.LBM

// BEGIN: /GFX/PICS/SETUP000.LBM
constexpr int SPLASHSCREEN_MAINMENU_BROWN = 0;
// END: /GFX/PICS/SETUP000.LBM

// BEGIN: /GFX/PICS/SETUP010.LBM
constexpr int SPLASHSCREEN_MAINMENU = 0;
// END: /GFX/PICS/SETUP010.LBM

// BEGIN: /GFX/PICS/SETUP011.LBM
constexpr int SPLASHSCREEN_SUBMENU1 = 0;
// END: /GFX/PICS/SETUP011.LBM

// BEGIN: /GFX/PICS/SETUP012.LBM
constexpr int SPLASHSCREEN_SUBMENU2 = 0;
// END: /GFX/PICS/SETUP012.LBM

// BEGIN: /GFX/PICS/SETUP013.LBM
constexpr int SPLASHSCREEN_SUBMENU3 = 0;
// END: /GFX/PICS/SETUP013.LBM

// BEGIN: /GFX/PICS/SETUP014.LBM
constexpr int SPLASHSCREEN_SUBMENU4 = 0;
// END: /GFX/PICS/SETUP014.LBM

// BEGIN: /GFX/PICS/SETUP015.LBM
constexpr int SPLASHSCREEN_SUBMENU5 = 0;
// END: /GFX/PICS/SETUP015.LBM

// BEGIN: /GFX/PICS/SETUP666.LBM
constexpr int SPLASHSCREEN_UNIVERSE1 = 0;
// END: /GFX/PICS/SETUP666.LBM

// BEGIN: /GFX/PICS/SETUP667.LBM
constexpr int SPLASHSCREEN_SUN1 = 0;
// END: /GFX/PICS/SETUP667.LBM

// BEGIN: /GFX/PICS/SETUP801.LBM
constexpr int SPLASHSCREEN_SETUP801 = 0;
// END: /GFX/PICS/SETUP801.LBM

// BEGIN: /GFX/PICS/SETUP802.LBM
constexpr int SPLASHSCREEN_LOADING_STANDARD = 0;
// END: /GFX/PICS/SETUP802.LBM

// BEGIN: /GFX/PICS/SETUP803.LBM
constexpr int SPLASHSCREEN_LOADING_GREENLAND1 = 0;
// END: /GFX/PICS/SETUP803.LBM

// BEGIN: /GFX/PICS/SETUP804.LBM
constexpr int SPLASHSCREEN_LOADING_WASTELAND = 0;
// END: /GFX/PICS/SETUP804.LBM

// BEGIN: /GFX/PICS/SETUP805.LBM
constexpr int SPLASHSCREEN_LOADING_GREENLAND2 = 0;
// END: /GFX/PICS/SETUP805.LBM

// BEGIN: /GFX/PICS/SETUP806.LBM
constexpr int SPLASHSCREEN_LOADING_GREENLAND3 = 0;
// END: /GFX/PICS/SETUP806.LBM

// BEGIN: /GFX/PICS/SETUP810.LBM
constexpr int SPLASHSCREEN_LOADING_WINTER1 = 0;
// END: /GFX/PICS/SETUP810.LBM

// BEGIN: /GFX/PICS/SETUP811.LBM
constexpr int SPLASHSCREEN_LOADING_WINTER2 = 0;
// END: /GFX/PICS/SETUP811.LBM

// BEGIN: /GFX/PICS/SETUP895.LBM
constexpr int SPLASHSCREEN_LOADING_SETUP895 = 0;
// END: /GFX/PICS/SETUP895.LBM

// BEGIN: /GFX/PICS/SETUP896.LBM
constexpr int SPLASHSCREEN_LOADING_ROMANCAMPAIGN1 = 0;
// END: /GFX/PICS/SETUP896.LBM

// BEGIN: /GFX/PICS/SETUP897.LBM
constexpr int SPLASHSCREEN_LOADING_ROMANCAMPAIGN2 = 0;
// END: /GFX/PICS/SETUP897.LBM

// BEGIN: /GFX/PICS/SETUP898.LBM
constexpr int SPLASHSCREEN_LOADING_ROMANCAMPAIGN3 = 0;
// END: /GFX/PICS/SETUP898.LBM

// BEGIN: /GFX/PICS/SETUP899.LBM
constexpr int SPLASHSCREEN_LOADING_ROMANCAMPAIGN_GREY = 0;
// END: /GFX/PICS/SETUP899.LBM

// BEGIN: /GFX/PICS/SETUP990.LBM
constexpr int SPLASHSCREEN_SETUP990 = 0;
// END: /GFX/PICS/SETUP990.LBM

// BEGIN: /GFX/PICS/WORLD.LBM
constexpr int SPLASHSCREEN_WORLDCAMPAIGN = 0;
// END: /GFX/PICS/WORLD.LBM

// BEGIN: /GFX/PICS/WORLDMSK.LBM
constexpr int SPLASHSCREEN_WORLDCAMPAIGN_SECTIONS = 0;
// END: /GFX/PICS/WORLDMSK.LBM

// BEGIN: /DATA/RESOURCE.IDX (AND /DATA/RESOURCE.DAT) OR /DATA/EDITRES.IDX (AND /DATA/EDITRES.DAT)
// (bitmap section only — fonts are in the standalone FontIndices enum below)
// BEGIN: FONT

/// IMPORTANT:   BECAUSE OF MULTIPLE COLORS FOR EACH CHARACTER THIS FONT-ENUMERATION IS NO LONGER CONSISTENT.
///             ONLY THE START-VALUES (FONT9_SPACE, FONT11_SPACE, FONT14_SPACE) HAVE THE RIGHT INDEX!

// ── Font glyph indices (documentation of S2 font file format) ─────────
// Fonts live in typedArchives[ArchiveID::EDITRES] and are read directly
// by CFont through findFont().  These constants are for reference only.
enum
{
    // fontsize 11
    FONT11_SPACE,                                       // spacebar
    FONT11_EXCLAMATION_POINT,                           // !
    FONT11_DOUBLE_QUOTES,                               // "
    FONT11_SHARP,                                       // #
    FONT11_DOLLAR,                                      // $
    FONT11_PERCENT,                                     // %
    FONT11_AMPERSAND,                                   // &
    FONT11_SINGLE_QUOTES,                               // '
    FONT11_ROUND_BRACKET_OPEN,                          // (
    FONT11_ROUND_BRACKET_CLOSE,                         // )
    FONT11_STAR,                                        // *
    FONT11_PLUS,                                        // +
    FONT11_COMMA,                                       // ,
    FONT11_MINUS,                                       // -
    FONT11_DOT,                                         // .
    FONT11_SLASH,                                       // /
    FONT11_0,                                           // 0
    FONT11_1,                                           // 1
    FONT11_2,                                           // 2
    FONT11_3,                                           // 3
    FONT11_4,                                           // 4
    FONT11_5,                                           // 5
    FONT11_6,                                           // 6
    FONT11_7,                                           // 7
    FONT11_8,                                           // 8
    FONT11_9,                                           // 9
    FONT11_COLON,                                       // :
    FONT11_SEMICOLON,                                   // ;
    FONT11_ARROW_BRACKET_OPEN,                          // <
    FONT11_EQUAL,                                       // =
    FONT11_ARROW_BRACKET_CLOSE,                         // >
    FONT11_INTERROGATION_POINT,                         // ?
    FONT11_AT,                                          // @
    FONT11_A,                                           // A
    FONT11_B,                                           // B
    FONT11_C,                                           // C
    FONT11_D,                                           // D
    FONT11_E,                                           // E
    FONT11_F,                                           // F
    FONT11_G,                                           // G
    FONT11_H,                                           // H
    FONT11_I,                                           // I
    FONT11_J,                                           // J
    FONT11_K,                                           // K
    FONT11_L,                                           // L
    FONT11_M,                                           // M
    FONT11_N,                                           // N
    FONT11_O,                                           // O
    FONT11_P,                                           // P
    FONT11_Q,                                           // Q
    FONT11_R,                                           // R
    FONT11_S,                                           // S
    FONT11_T,                                           // T
    FONT11_U,                                           // U
    FONT11_V,                                           // V
    FONT11_W,                                           // W
    FONT11_X,                                           // X
    FONT11_Y,                                           // Y
    FONT11_Z,                                           // Z
    FONT11_BACKSLASH,                                   //
    FONT11_UNDERSCORE,                                  // _
    FONT11_a,                                           // a
    FONT11_b,                                           // b
    FONT11_c,                                           // c
    FONT11_d,                                           // d
    FONT11_e,                                           // e
    FONT11_f,                                           // f
    FONT11_g,                                           // g
    FONT11_h,                                           // h
    FONT11_i,                                           // i
    FONT11_j,                                           // j
    FONT11_k,                                           // k
    FONT11_l,                                           // l
    FONT11_m,                                           // m
    FONT11_n,                                           // n
    FONT11_o,                                           // o
    FONT11_p,                                           // p
    FONT11_q,                                           // q
    FONT11_r,                                           // r
    FONT11_s,                                           // s
    FONT11_t,                                           // t
    FONT11_u,                                           // u
    FONT11_v,                                           // v
    FONT11_w,                                           // w
    FONT11_x,                                           // x
    FONT11_y,                                           // y
    FONT11_z,                                           // z
    FONT11_ANSI_199,                                    // Ç
    FONT11_ANSI_252,                                    // ü
    FONT11_ANSI_233,                                    // é
    FONT11_ANSI_226,                                    // â
    FONT11_ANSI_228,                                    // ä
    FONT11_ANSI_224,                                    // à
    FONT11_ANSI_231,                                    // ç
    FONT11_ANSI_234,                                    // ê
    FONT11_ANSI_235,                                    // ë
    FONT11_ANSI_232,                                    // è
    FONT11_ANSI_239,                                    // ï
    FONT11_ANSI_238,                                    // î
    FONT11_ANSI_236,                                    // ì
    FONT11_ANSI_196,                                    // Ä
    FONT11_ANSI_244,                                    // ô
    FONT11_ANSI_246,                                    // ö
    FONT11_ANSI_242,                                    // ò
    FONT11_ANSI_251,                                    // û
    FONT11_ANSI_249,                                    // ù
    FONT11_ANSI_214,                                    // Ö
    FONT11_ANSI_220,                                    // Ü
    FONT11_ANSI_225,                                    // á
    FONT11_ANSI_237,                                    // í
    FONT11_ANSI_243,                                    // ó
    FONT11_ANSI_250,                                    // ú
    FONT11_ANSI_241,                                    // ñ
    FONT11_ANSI_223,                                    // ß
    FONT11_ANSI_169,                                    // ©
                                                        // fontsize 9
    FONT9_SPACE = FONT11_SPACE + NUM_FONT_COLORS * 115, // spacebar
    FONT9_EXCLAMATION_POINT,                            // !
    FONT9_DOUBLE_QUOTES,                                // "
    FONT9_SHARP,                                        // #
    FONT9_DOLLAR,                                       // $
    FONT9_PERCENT,                                      // %
    FONT9_AMPERSAND,                                    // &
    FONT9_SINGLE_QUOTES,                                // '
    FONT9_ROUND_BRACKET_OPEN,                           // (
    FONT9_ROUND_BRACKET_CLOSE,                          // )
    FONT9_STAR,                                         // *
    FONT9_PLUS,                                         // +
    FONT9_COMMA,                                        // ,
    FONT9_MINUS,                                        // -
    FONT9_DOT,                                          // .
    FONT9_SLASH,                                        // /
    FONT9_0,                                            // 0
    FONT9_1,                                            // 1
    FONT9_2,                                            // 2
    FONT9_3,                                            // 3
    FONT9_4,                                            // 4
    FONT9_5,                                            // 5
    FONT9_6,                                            // 6
    FONT9_7,                                            // 7
    FONT9_8,                                            // 8
    FONT9_9,                                            // 9
    FONT9_COLON,                                        // :
    FONT9_SEMICOLON,                                    // ;
    FONT9_ARROW_BRACKET_OPEN,                           // <
    FONT9_EQUAL,                                        // =
    FONT9_ARROW_BRACKET_CLOSE,                          // >
    FONT9_INTERROGATION_POINT,                          // ?
    FONT9_AT,                                           // @
    FONT9_A,                                            // A
    FONT9_B,                                            // B
    FONT9_C,                                            // C
    FONT9_D,                                            // D
    FONT9_E,                                            // E
    FONT9_F,                                            // F
    FONT9_G,                                            // G
    FONT9_H,                                            // H
    FONT9_I,                                            // I
    FONT9_J,                                            // J
    FONT9_K,                                            // K
    FONT9_L,                                            // L
    FONT9_M,                                            // M
    FONT9_N,                                            // N
    FONT9_O,                                            // O
    FONT9_P,                                            // P
    FONT9_Q,                                            // Q
    FONT9_R,                                            // R
    FONT9_S,                                            // S
    FONT9_T,                                            // T
    FONT9_U,                                            // U
    FONT9_V,                                            // V
    FONT9_W,                                            // W
    FONT9_X,                                            // X
    FONT9_Y,                                            // Y
    FONT9_Z,                                            // Z
    FONT9_BACKSLASH,                                    //
    FONT9_UNDERSCORE,                                   // _
    FONT9_a,                                            // a
    FONT9_b,                                            // b
    FONT9_c,                                            // c
    FONT9_d,                                            // d
    FONT9_e,                                            // e
    FONT9_f,                                            // f
    FONT9_g,                                            // g
    FONT9_h,                                            // h
    FONT9_i,                                            // i
    FONT9_j,                                            // j
    FONT9_k,                                            // k
    FONT9_l,                                            // l
    FONT9_m,                                            // m
    FONT9_n,                                            // n
    FONT9_o,                                            // o
    FONT9_p,                                            // p
    FONT9_q,                                            // q
    FONT9_r,                                            // r
    FONT9_s,                                            // s
    FONT9_t,                                            // t
    FONT9_u,                                            // u
    FONT9_v,                                            // v
    FONT9_w,                                            // w
    FONT9_x,                                            // x
    FONT9_y,                                            // y
    FONT9_z,                                            // z
    FONT9_ANSI_199,                                     // Ç
    FONT9_ANSI_252,                                     // ü
    FONT9_ANSI_233,                                     // é
    FONT9_ANSI_226,                                     // â
    FONT9_ANSI_228,                                     // ä
    FONT9_ANSI_224,                                     // à
    FONT9_ANSI_231,                                     // ç
    FONT9_ANSI_234,                                     // ê
    FONT9_ANSI_235,                                     // ë
    FONT9_ANSI_232,                                     // è
    FONT9_ANSI_239,                                     // ï
    FONT9_ANSI_238,                                     // î
    FONT9_ANSI_236,                                     // ì
    FONT9_ANSI_196,                                     // Ä
    FONT9_ANSI_244,                                     // ô
    FONT9_ANSI_246,                                     // ö
    FONT9_ANSI_242,                                     // ò
    FONT9_ANSI_251,                                     // û
    FONT9_ANSI_249,                                     // ù
    FONT9_ANSI_214,                                     // Ö
    FONT9_ANSI_220,                                     // Ü
    FONT9_ANSI_225,                                     // á
    FONT9_ANSI_237,                                     // í
    FONT9_ANSI_243,                                     // ó
    FONT9_ANSI_250,                                     // ú
    FONT9_ANSI_241,                                     // ñ
    FONT9_ANSI_223,                                     // ß
    FONT9_ANSI_169,                                     // ©
                                                        // fontsize 14
    FONT14_SPACE = FONT9_SPACE + NUM_FONT_COLORS * 115, // spacebar
    FONT14_EXCLAMATION_POINT,                           // !
    FONT14_DOUBLE_QUOTES,                               // "
    FONT14_SHARP,                                       // #
    FONT14_DOLLAR,                                      // $
    FONT14_PERCENT,                                     // %
    FONT14_AMPERSAND,                                   // &
    FONT14_SINGLE_QUOTES,                               // '
    FONT14_ROUND_BRACKET_OPEN,                          // (
    FONT14_ROUND_BRACKET_CLOSE,                         // )
    FONT14_STAR,                                        // *
    FONT14_PLUS,                                        // +
    FONT14_COMMA,                                       // ,
    FONT14_MINUS,                                       // -
    FONT14_DOT,                                         // .
    FONT14_SLASH,                                       // /
    FONT14_0,                                           // 0
    FONT14_1,                                           // 1
    FONT14_2,                                           // 2
    FONT14_3,                                           // 3
    FONT14_4,                                           // 4
    FONT14_5,                                           // 5
    FONT14_6,                                           // 6
    FONT14_7,                                           // 7
    FONT14_8,                                           // 8
    FONT14_9,                                           // 9
    FONT14_COLON,                                       // :
    FONT14_SEMICOLON,                                   // ;
    FONT14_ARROW_BRACKET_OPEN,                          // <
    FONT14_EQUAL,                                       // =
    FONT14_ARROW_BRACKET_CLOSE,                         // >
    FONT14_INTERROGATION_POINT,                         // ?
    FONT14_AT,                                          // @
    FONT14_A,                                           // A
    FONT14_B,                                           // B
    FONT14_C,                                           // C
    FONT14_D,                                           // D
    FONT14_E,                                           // E
    FONT14_F,                                           // F
    FONT14_G,                                           // G
    FONT14_H,                                           // H
    FONT14_I,                                           // I
    FONT14_J,                                           // J
    FONT14_K,                                           // K
    FONT14_L,                                           // L
    FONT14_M,                                           // M
    FONT14_N,                                           // N
    FONT14_O,                                           // O
    FONT14_P,                                           // P
    FONT14_Q,                                           // Q
    FONT14_R,                                           // R
    FONT14_S,                                           // S
    FONT14_T,                                           // T
    FONT14_U,                                           // U
    FONT14_V,                                           // V
    FONT14_W,                                           // W
    FONT14_X,                                           // X
    FONT14_Y,                                           // Y
    FONT14_Z,                                           // Z
    FONT14_BACKSLASH,                                   //
    FONT14_UNDERSCORE,                                  // _
    FONT14_a,                                           // a
    FONT14_b,                                           // b
    FONT14_c,                                           // c
    FONT14_d,                                           // d
    FONT14_e,                                           // e
    FONT14_f,                                           // f
    FONT14_g,                                           // g
    FONT14_h,                                           // h
    FONT14_i,                                           // i
    FONT14_j,                                           // j
    FONT14_k,                                           // k
    FONT14_l,                                           // l
    FONT14_m,                                           // m
    FONT14_n,                                           // n
    FONT14_o,                                           // o
    FONT14_p,                                           // p
    FONT14_q,                                           // q
    FONT14_r,                                           // r
    FONT14_s,                                           // s
    FONT14_t,                                           // t
    FONT14_u,                                           // u
    FONT14_v,                                           // v
    FONT14_w,                                           // w
    FONT14_x,                                           // x
    FONT14_y,                                           // y
    FONT14_z,                                           // z
    FONT14_ANSI_199,                                    // Ç
    FONT14_ANSI_252,                                    // ü
    FONT14_ANSI_233,                                    // é
    FONT14_ANSI_226,                                    // â
    FONT14_ANSI_228,                                    // ä
    FONT14_ANSI_224,                                    // à
    FONT14_ANSI_231,                                    // ç
    FONT14_ANSI_234,                                    // ê
    FONT14_ANSI_235,                                    // ë
    FONT14_ANSI_232,                                    // è
    FONT14_ANSI_239,                                    // ï
    FONT14_ANSI_238,                                    // î
    FONT14_ANSI_236,                                    // ì
    FONT14_ANSI_196,                                    // Ä
    FONT14_ANSI_244,                                    // ô
    FONT14_ANSI_246,                                    // ö
    FONT14_ANSI_242,                                    // ò
    FONT14_ANSI_251,                                    // û
    FONT14_ANSI_249,                                    // ù
    FONT14_ANSI_214,                                    // Ö
    FONT14_ANSI_220,                                    // Ü
    FONT14_ANSI_225,                                    // á
    FONT14_ANSI_237,                                    // í
    FONT14_ANSI_243,                                    // ó
    FONT14_ANSI_250,                                    // ú
    FONT14_ANSI_241,                                    // ñ
    FONT14_ANSI_223,                                    // ß
    FONT14_ANSI_169,                                    // ©
                                                        // END: FONT
};

// now the main resources will follow (frames, cursor, ...)
// resolution behind means not resolution of the pic but window resolution the pic belongs to
constexpr int MAINFRAME_640_480 = 4;
constexpr int SPLITFRAME_LEFT_640_480 = 5;
constexpr int SPLITFRAME_RIGHT_640_480 = 6;
constexpr int MAINFRAME_800_600 = 7;
constexpr int SPLITFRAME_LEFT_800_600 = 8;
constexpr int SPLITFRAME_RIGHT_800_600 = 9;
constexpr int MAINFRAME_1024_768 = 10;
constexpr int SPLITFRAME_LEFT_1024_768 = 11;
constexpr int SPLITFRAME_RIGHT_1024_768 = 12;
constexpr int MAINFRAME_LEFT_1280_1024 = 13;
constexpr int MAINFRAME_RIGHT_1280_1024 = 14;
constexpr int SPLITFRAME_LEFT_1280_1024 = 15;
constexpr int SPLITFRAME_RIGHT_1280_1024 = 16;
constexpr int STATUE_UP_LEFT = 17;
constexpr int STATUE_UP_RIGHT = 18;
constexpr int STATUE_DOWN_LEFT = 19;
constexpr int STATUE_DOWN_RIGHT = 20;
constexpr int SPLITFRAME_ADDITIONAL_LEFT_640_480 = 21;
constexpr int SPLITFRAME_ADDITIONAL_RIGHT_640_480 = 22;
constexpr int SPLITFRAME_ADDITIONAL_LEFT_800_600 = 23;
constexpr int SPLITFRAME_ADDITIONAL_RIGHT_800_600 = 24;
constexpr int SPLITFRAME_ADDITIONAL_LEFT_1024_768 = 25;
constexpr int SPLITFRAME_ADDITIONAL_RIGHT_1024_768 = 26;
constexpr int SPLITFRAME_ADDITIONAL_LEFT_1280_1024 = 27;
constexpr int SPLITFRAME_ADDITIONAL_RIGHT_1280_1024 = 28;
constexpr int MENUBAR = 29;
constexpr int CURSOR = 30;
constexpr int CURSOR_CLICKED = 31;
constexpr int CROSS = 32;
constexpr int MOON = 33;
constexpr int CIRCLE_HIGH_GREY = 34;
constexpr int CIRCLE_FLAT_GREY = 35;
constexpr int WINDOW_LEFT_UPPER_CORNER = 36;
constexpr int WINDOW_RIGHT_UPPER_CORNER = 37;
constexpr int WINDOW_LEFT_FRAME = 38;
constexpr int WINDOW_RIGHT_FRAME = 39;
constexpr int WINDOW_LOWER_FRAME = 40;
constexpr int WINDOW_BACKGROUND = 41;
constexpr int WINDOW_UPPER_FRAME = 42;
constexpr int WINDOW_UPPER_FRAME_MARKED = 43;
constexpr int WINDOW_UPPER_FRAME_CLICKED = 44;
constexpr int WINDOW_CORNER_RECTANGLE = 45;
constexpr int WINDOW_BUTTON_RESIZE = 46;
constexpr int WINDOW_BUTTON_CLOSE = 47;
constexpr int WINDOW_BUTTON_MINIMIZE = 48;
constexpr int WINDOW_CORNER_RECTANGLE_2 = 49;
constexpr int WINDOW_BUTTON_RESIZE_CLICKED = 50;
constexpr int WINDOW_BUTTON_CLOSE_CLICKED = 51;
constexpr int WINDOW_BUTTON_MINIMIZE_CLICKED = 52;
constexpr int WINDOW_CORNER_RECTANGLE_3 = 53;
constexpr int WINDOW_BUTTON_RESIZE_MARKED = 54;
constexpr int WINDOW_BUTTON_CLOSE_MARKED = 55;
constexpr int WINDOW_BUTTON_MINIMIZE_MARKED = 56;
// END: /DATA/RESOURCE.IDX (AND /DATA/RESOURCE.DAT) OR /DATA/EDITRES.IDX (AND /DATA/EDITRES.DAT)

// BEGIN: /DATA/IO/EDITIO.IDX (AND /DATA/IO/EDITIO.DAT)
constexpr int BUTTON_GREY_BRIGHT = 0;
constexpr int BUTTON_GREY_DARK = 1;
constexpr int BUTTON_RED1_BRIGHT = 2;
constexpr int BUTTON_RED1_DARK = 3;
constexpr int BUTTON_GREEN1_BRIGHT = 4;
constexpr int BUTTON_GREEN1_DARK = 5;
constexpr int BUTTON_GREEN2_BRIGHT = 6;
constexpr int BUTTON_GREEN2_DARK = 7;
constexpr int BUTTON_RED2_BRIGHT = 8;
constexpr int BUTTON_RED2_DARK = 9;
constexpr int BUTTON_STONE_BRIGHT = 10;
constexpr int BUTTON_STONE_DARK = 11;
constexpr int BUTTON_GREY_BACKGROUND = 12;
constexpr int BUTTON_RED1_BACKGROUND = 13;
constexpr int BUTTON_GREEN1_BACKGROUND = 14;
constexpr int BUTTON_GREEN2_BACKGROUND = 15;
constexpr int BUTTON_RED2_BACKGROUND = 16;
constexpr int BUTTON_STONE_BACKGROUND = 17;
constexpr int MENUBAR_BUILDHELP = 18;
constexpr int PICTURE_SHOW_POLITICAL_EDGES = 19;
constexpr int CIRCLE_BANG = 20;
constexpr int MENUBAR_BUGKILL = 21;
constexpr int PICTURE_SMALL_ARROW_UP = 22;
constexpr int PICTURE_SMALL_ARROW_DOWN = 23;
constexpr int PICTURE_SMALL_CIRCLE = 24;
constexpr int PICTURE_TROWEL = 25;
constexpr int MENUBAR_COMPUTER = 26;
constexpr int MENUBAR_LOUPE = 27;
constexpr int PICTURE_SMALL_TICK = 28;
constexpr int PICTURE_SMALL_CROSS = 29;
constexpr int MENUBAR_MINIMAP = 30;
constexpr int PICTURE_SMALL_ARROW_LEFT = 31;
constexpr int PICTURE_SMALL_ARROW_RIGHT = 32;
constexpr int PICTURE_LETTER_I = 33;
constexpr int PICTURE_GREENLAND_TEXTURE_SNOW = 34;
constexpr int PICTURE_GREENLAND_TEXTURE_STEPPE = 35;
constexpr int PICTURE_GREENLAND_TEXTURE_SWAMP = 36;
constexpr int PICTURE_GREENLAND_TEXTURE_FLOWER = 37;
constexpr int PICTURE_GREENLAND_TEXTURE_MINING1 = 38;
constexpr int PICTURE_GREENLAND_TEXTURE_MINING2 = 39;
constexpr int PICTURE_GREENLAND_TEXTURE_MINING3 = 40;
constexpr int PICTURE_GREENLAND_TEXTURE_MINING4 = 41;
constexpr int PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW1 = 42;
constexpr int PICTURE_GREENLAND_TEXTURE_MEADOW1 = 43;
constexpr int PICTURE_GREENLAND_TEXTURE_MEADOW2 = 44;
constexpr int PICTURE_GREENLAND_TEXTURE_MEADOW3 = 45;
constexpr int PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW2 = 46;
constexpr int PICTURE_GREENLAND_TEXTURE_MINING_MEADOW = 47;
constexpr int PICTURE_GREENLAND_TEXTURE_WATER = 48;
constexpr int PICTURE_GREENLAND_TEXTURE_LAVA = 49;
constexpr int PICTURE_GREENLAND_TEXTURE_MEADOW_MIXED = 50;
constexpr int PICTURE_WASTELAND_TEXTURE_SNOW = 51;
constexpr int PICTURE_WASTELAND_TEXTURE_STEPPE = 52;
constexpr int PICTURE_WASTELAND_TEXTURE_SWAMP = 53;
constexpr int PICTURE_WASTELAND_TEXTURE_FLOWER = 54;
constexpr int PICTURE_WASTELAND_TEXTURE_MINING1 = 55;
constexpr int PICTURE_WASTELAND_TEXTURE_MINING2 = 56;
constexpr int PICTURE_WASTELAND_TEXTURE_MINING3 = 57;
constexpr int PICTURE_WASTELAND_TEXTURE_MINING4 = 58;
constexpr int PICTURE_WASTELAND_TEXTURE_STEPPE_MEADOW1 = 59;
constexpr int PICTURE_WASTELAND_TEXTURE_MEADOW1 = 60;
constexpr int PICTURE_WASTELAND_TEXTURE_MEADOW2 = 61;
constexpr int PICTURE_WASTELAND_TEXTURE_MEADOW3 = 62;
constexpr int PICTURE_WASTELAND_TEXTURE_STEPPE_MEADOW2 = 63;
constexpr int PICTURE_WASTELAND_TEXTURE_MINING_MEADOW = 64;
constexpr int PICTURE_WASTELAND_TEXTURE_WATER = 65;
constexpr int PICTURE_WASTELAND_TEXTURE_LAVA = 66;
constexpr int PICTURE_WINTERLAND_TEXTURE_SNOW = 67;
constexpr int PICTURE_WINTERLAND_TEXTURE_STEPPE = 68;
constexpr int PICTURE_WINTERLAND_TEXTURE_SWAMP = 69;
constexpr int PICTURE_WINTERLAND_TEXTURE_FLOWER = 70;
constexpr int PICTURE_WINTERLAND_TEXTURE_MINING1 = 71;
constexpr int PICTURE_WINTERLAND_TEXTURE_MINING2 = 72;
constexpr int PICTURE_WINTERLAND_TEXTURE_MINING3 = 73;
constexpr int PICTURE_WINTERLAND_TEXTURE_MINING4 = 74;
constexpr int PICTURE_WINTERLAND_TEXTURE_STEPPE_MEADOW1 = 75;
constexpr int PICTURE_WINTERLAND_TEXTURE_MEADOW1 = 76;
constexpr int PICTURE_WINTERLAND_TEXTURE_MEADOW2 = 77;
constexpr int PICTURE_WINTERLAND_TEXTURE_MEADOW3 = 78;
constexpr int PICTURE_WINTERLAND_TEXTURE_STEPPE_MEADOW2 = 79;
constexpr int PICTURE_WINTERLAND_TEXTURE_MINING_MEADOW = 80;
constexpr int PICTURE_WINTERLAND_TEXTURE_WATER = 81;
constexpr int PICTURE_WINTERLAND_TEXTURE_LAVA = 82;
constexpr int PICTURE_WINTERLAND_TEXTURE_MEADOW_MIXED = 83;
constexpr int PICTURE_TREE_CYPRESS = 84;
constexpr int PICTURE_TREE_PINE = 85;
constexpr int PICTURE_TREE_PALM2 = 86;
constexpr int PICTURE_TREE_PINEAPPLE = 87;
constexpr int PICTURE_TREE_FIR = 88;
constexpr int PICTURE_TREE_OAK = 89;
constexpr int PICTURE_TREE_BIRCH = 90;
constexpr int PICTURE_TREE_CHERRY = 91;
constexpr int PICTURE_TREE_PALM1 = 92;
constexpr int PICTURE_TREE_FLAPHAT = 93;
constexpr int PICTURE_TREE_SPIDER = 94;
constexpr int PICTURE_TREE_WOOD_MIXED = 95;
constexpr int PICTURE_TREE_PALM_MIXED = 96;
constexpr int PICTURE_SQUARE_CIRCLE1 = 97;
constexpr int PICTURE_SQUARE_CIRCLE2 = 98;
constexpr int PICTURE_SQUARE_CIRCLE3 = 99;
constexpr int PICTURE_SQUARE_CIRCLE4 = 100;
constexpr int PICTURE_RESOURCE_GOLD = 101;
constexpr int PICTURE_RESOURCE_ORE = 102;
constexpr int PICTURE_RESOURCE_COAL = 103;
constexpr int PICTURE_RESOURCE_GRANITE = 104;
constexpr int MENUBAR_TREE = 113;
constexpr int MENUBAR_RESOURCE = 114;
constexpr int MENUBAR_TEXTURE = 115;
constexpr int MENUBAR_HEIGHT = 116;
constexpr int MENUBAR_PLAYER = 117;
constexpr int MENUBAR_LANDSCAPE = 118;
constexpr int MENUBAR_ANIMAL = 119;
constexpr int MENUBAR_NEWWORLD = 120;
constexpr int PICTURE_EYE_CROSS = 121;
constexpr int MENUBAR_COMPUTER_DOUBLE_ENTRY = 122;
constexpr int PICTURE_LANDSCAPE_GRANITE = 126;
constexpr int PICTURE_LANDSCAPE_TREE_DEAD = 127;
constexpr int PICTURE_LANDSCAPE_STONE = 128;
constexpr int PICTURE_LANDSCAPE_CACTUS = 129;
constexpr int PICTURE_LANDSCAPE_PEBBLE = 130;
constexpr int PICTURE_LANDSCAPE_BUSH = 131;
constexpr int PICTURE_LANDSCAPE_SHRUB = 132;
constexpr int PICTURE_LANDSCAPE_BONE = 133;
constexpr int PICTURE_LANDSCAPE_MUSHROOM = 134;
constexpr int PICTURE_LANDSCAPE_STALAGMITE = 135;
constexpr int PICTURE_LANDSCAPE_GRANITE_WINTER = 136;
constexpr int PICTURE_LANDSCAPE_TREE_DEAD_WINTER = 137;
constexpr int PICTURE_LANDSCAPE_STONE_WINTER = 138;
constexpr int PICTURE_LANDSCAPE_PEBBLE_WINTER = 139;
constexpr int PICTURE_LANDSCAPE_BONE_WINTER = 140;
constexpr int PICTURE_LANDSCAPE_MUSHROOM_WINTER = 141;
constexpr int PICTURE_ANIMAL_BEAR = 142;
constexpr int PICTURE_ANIMAL_RABBIT = 143;
constexpr int PICTURE_ANIMAL_FOX = 144;
constexpr int PICTURE_ANIMAL_STAG = 145;
constexpr int PICTURE_ANIMAL_ROE = 146;
constexpr int PICTURE_ANIMAL_DUCK = 147;
constexpr int PICTURE_ANIMAL_SHEEP = 148;
// END: /DATA/IO/EDITIO.IDX (AND /DATA/IO/EDITIO.DAT)

// BEGIN: /DATA/EDITBOB.LST
constexpr int CURSOR_SYMBOL_SCISSORS = 1;
constexpr int CURSOR_SYMBOL_TREE = 2;
constexpr int CURSOR_SYMBOL_ARROW_UP = 3;
constexpr int CURSOR_SYMBOL_ARROW_DOWN = 4;
constexpr int CURSOR_SYMBOL_TEXTURE = 5;
constexpr int CURSOR_SYMBOL_LANDSCAPE = 6;
constexpr int CURSOR_SYMBOL_FLAG = 7;
constexpr int CURSOR_SYMBOL_PICKAXE_MINUS = 8;
constexpr int CURSOR_SYMBOL_PICKAXE_PLUS = 9;
constexpr int CURSOR_SYMBOL_ANIMAL = 10;
constexpr int FLAG_BLUE_DARK = 11;
constexpr int FLAG_YELLOW = 12;
constexpr int FLAG_RED = 13;
constexpr int FLAG_BLUE_BRIGHT = 14;
constexpr int FLAG_GREEN_DARK = 15;
constexpr int FLAG_GREEN_BRIGHT = 16;
constexpr int FLAG_ORANGE = 17;
constexpr int PICTURE_SMALL_BEAR = 18;
constexpr int PICTURE_SMALL_RABBIT = 19;
constexpr int PICTURE_SMALL_FOX = 20;
constexpr int PICTURE_SMALL_STAG = 21;
constexpr int PICTURE_SMALL_DEER = 22;
constexpr int PICTURE_SMALL_DUCK = 23;
constexpr int PICTURE_SMALL_SHEEP = 24;
// END: /DATA/EDITBOB.LST

// BEGIN: /GFX/TEXTURES/TEX5.LBM
constexpr int TILESET_GREENLAND = 0;
// END: /GFX/TEXTURES/TEX5.LBM

// BEGIN: /GFX/TEXTURES/TEX6.LBM
constexpr int TILESET_WASTELAND = 0;
// END: /GFX/TEXTURES/TEX6.LBM

// BEGIN: /GFX/TEXTURES/TEX7.LBM
constexpr int TILESET_WINTERLAND = 0;
// END: /GFX/TEXTURES/TEX7.LBM

// BEGIN: /DATA/MIS*BOBS.LST   * = 0,1,2,3,4,5

// MIS0BOBS.LST: bitmap at 0 (ship), shadow at 1, nulls 2-5, bitmap at 6 (tent)
constexpr int MIS0BOBS_SHIP = 0;
constexpr int MIS0BOBS_TENT = 6;

// MIS1BOBS.LST: 7 stones (bitmaps at 0,2,4,6,8,10,12), 2 trees (20,22), skeleton (30)
constexpr int MIS1BOBS_STONE1 = 0;
constexpr int MIS1BOBS_STONE2 = 2;
constexpr int MIS1BOBS_STONE3 = 4;
constexpr int MIS1BOBS_STONE4 = 6;
constexpr int MIS1BOBS_STONE5 = 8;
constexpr int MIS1BOBS_STONE6 = 10;
constexpr int MIS1BOBS_STONE7 = 12;
constexpr int MIS1BOBS_TREE1 = 20;
constexpr int MIS1BOBS_TREE2 = 22;
constexpr int MIS1BOBS_SKELETON = 30;

// MIS2BOBS.LST: tent (0), guardhouse (2), guardtower (4), fortress (6), puppy (8)
constexpr int MIS2BOBS_TENT = 0;
constexpr int MIS2BOBS_GUARDHOUSE = 2;
constexpr int MIS2BOBS_GUARDTOWER = 4;
constexpr int MIS2BOBS_FORTRESS = 6;
constexpr int MIS2BOBS_PUPPY = 8;

// MIS3BOBS.LST: viking (0)
constexpr int MIS3BOBS_VIKING = 0;

// MIS4BOBS.LST: scrolls (0)
constexpr int MIS4BOBS_SCROLLS = 0;

// MIS5BOBS.LST: skeleton1 (0), skeleton2 (2), cave (4), viking (6)
constexpr int MIS5BOBS_SKELETON1 = 0;
constexpr int MIS5BOBS_SKELETON2 = 2;
constexpr int MIS5BOBS_CAVE = 4;
constexpr int MIS5BOBS_VIKING = 6;
// END: /DATA/MIS*BOBS.LST

// BEGIN: /DATA/MAP00.LST (ONLY IF A MAP IS ACTIVE)
constexpr int MAPPIC_ARROWCROSS_YELLOW = 0;
constexpr int MAPPIC_CIRCLE_YELLOW = 1;
constexpr int MAPPIC_ARROWCROSS_RED = 2;
constexpr int MAPPIC_ARROWCROSS_ORANGE = 3;
constexpr int MAPPIC_ARROWCROSS_RED_FLAG = 4;
constexpr int MAPPIC_ARROWCROSS_RED_MINE = 5;
constexpr int MAPPIC_ARROWCROSS_RED_HOUSE_SMALL = 6;
constexpr int MAPPIC_ARROWCROSS_RED_HOUSE_MIDDLE = 7;
constexpr int MAPPIC_ARROWCROSS_RED_HOUSE_BIG = 8;
constexpr int MAPPIC_ARROWCROSS_RED_HOUSE_HARBOUR = 9;
constexpr int MAPPIC_PAPER_RED_CROSS = 10;
constexpr int MAPPIC_FLAG = 11;
constexpr int MAPPIC_HOUSE_SMALL = 12;
constexpr int MAPPIC_HOUSE_MIDDLE = 13;
constexpr int MAPPIC_HOUSE_BIG = 14;
constexpr int MAPPIC_MINE = 15;
constexpr int MAPPIC_HOUSE_HARBOUR = 16;
constexpr int MAPPIC_TREE_PINE = 26;
constexpr int MAPPIC_TREE_BIRCH = 41;
constexpr int MAPPIC_TREE_OAK = 56;
constexpr int MAPPIC_TREE_PALM1 = 71;
constexpr int MAPPIC_TREE_PALM2 = 86;
constexpr int MAPPIC_TREE_PINEAPPLE = 101;
constexpr int MAPPIC_TREE_CYPRESS = 109;
constexpr int MAPPIC_TREE_CHERRY = 124;
constexpr int MAPPIC_TREE_FIR = 139;
constexpr int MAPPIC_MUSHROOM1 = 154;
constexpr int MAPPIC_MUSHROOM2 = 155;
constexpr int MAPPIC_STONE1 = 156;
constexpr int MAPPIC_STONE2 = 157;
constexpr int MAPPIC_STONE3 = 158;
constexpr int MAPPIC_TREE_TRUNK_DEAD = 159;
constexpr int MAPPIC_TREE_DEAD = 160;
constexpr int MAPPIC_BONE1 = 161;
constexpr int MAPPIC_BONE2 = 162;
constexpr int MAPPIC_FLOWERS = 163;
constexpr int MAPPIC_BUSH1 = 164;
constexpr int MAPPIC_ROCK4 = 165;
constexpr int MAPPIC_CACTUS1 = 166;
constexpr int MAPPIC_CACTUS2 = 167;
constexpr int MAPPIC_SHRUB1 = 168;
constexpr int MAPPIC_SHRUB2 = 169;
constexpr int MAPPIC_GRANITE_1_1 = 170;
constexpr int MAPPIC_GRANITE_1_2 = 171;
constexpr int MAPPIC_GRANITE_1_3 = 172;
constexpr int MAPPIC_GRANITE_1_4 = 173;
constexpr int MAPPIC_GRANITE_1_5 = 174;
constexpr int MAPPIC_GRANITE_1_6 = 175;
constexpr int MAPPIC_GRANITE_2_1 = 176;
constexpr int MAPPIC_GRANITE_2_2 = 177;
constexpr int MAPPIC_GRANITE_2_3 = 178;
constexpr int MAPPIC_GRANITE_2_4 = 179;
constexpr int MAPPIC_GRANITE_2_5 = 180;
constexpr int MAPPIC_GRANITE_2_6 = 181;
constexpr int MAPPIC_UNKNOWN_PICTURE = 182;
constexpr int MAPPIC_FIELD_1_1 = 183;
constexpr int MAPPIC_FIELD_1_2 = 184;
constexpr int MAPPIC_FIELD_1_3 = 185;
constexpr int MAPPIC_FIELD_1_4 = 186;
constexpr int MAPPIC_FIELD_1_5 = 187;
constexpr int MAPPIC_FIELD_2_1 = 188;
constexpr int MAPPIC_FIELD_2_2 = 189;
constexpr int MAPPIC_FIELD_2_3 = 190;
constexpr int MAPPIC_FIELD_2_4 = 191;
constexpr int MAPPIC_FIELD_2_5 = 192;
constexpr int MAPPIC_BUSH2 = 193;
constexpr int MAPPIC_BUSH3 = 194;
constexpr int MAPPIC_BUSH4 = 195;
constexpr int MAPPIC_SHRUB3 = 196;
constexpr int MAPPIC_SHRUB4 = 197;
constexpr int MAPPIC_BONE3 = 198;
constexpr int MAPPIC_BONE4 = 199;
constexpr int MAPPIC_MUSHROOM3 = 200;
constexpr int MAPPIC_STONE4 = 201;
constexpr int MAPPIC_STONE5 = 202;
constexpr int MAPPIC_PEBBLE1 = 203;
constexpr int MAPPIC_PEBBLE2 = 204;
constexpr int MAPPIC_PEBBLE3 = 205;
constexpr int MAPPIC_SHRUB5 = 206;
constexpr int MAPPIC_SHRUB6 = 207;
constexpr int MAPPIC_SHRUB7 = 208;
constexpr int MAPPIC_SNOWMAN = 209;
constexpr int MAPPIC_DOOR = 210;
// Upper bound for cache invalidation (past last named entry, not exact)
constexpr int MAPPIC_LAST_ENTRY = 211;
// END: /DATA/MAP00.LST

// ── Shadow index documentation (documents file format, unused in code) ──
constexpr auto MAXBOBBMP = 5000;
constexpr auto MAXBOBSHADOW = 5000;
enum
{
    MIS0BOBS_SHIP_SHADOW = MAXBOBBMP,
    MIS0BOBS_TENT_SHADOW,
    MIS1BOBS_STONE1_SHADOW,
    MIS1BOBS_STONE2_SHADOW,
    MIS1BOBS_STONE3_SHADOW,
    MIS1BOBS_STONE4_SHADOW,
    MIS1BOBS_STONE5_SHADOW,
    MIS1BOBS_STONE6_SHADOW,
    MIS1BOBS_STONE7_SHADOW,
    MIS1BOBS_TREE1_SHADOW,
    MIS1BOBS_TREE2_SHADOW,
    MIS1BOBS_SKELETON_SHADOW,
    MIS2BOBS_TENT_SHADOW,
    MIS2BOBS_GUARDHOUSE_SHADOW,
    MIS2BOBS_GUARDTOWER_SHADOW,
    MIS2BOBS_FORTRESS_SHADOW,
    MIS2BOBS_PUPPY_SHADOW,
    MIS3BOBS_VIKING_SHADOW,
    MIS4BOBS_SCROLLS_SHADOW,
    MIS5BOBS_SKELETON1_SHADOW,
    MIS5BOBS_SKELETON2_SHADOW,
    MIS5BOBS_CAVE_SHADOW,
    MIS5BOBS_VIKING_SHADOW
};

// Button-Colors (after all used by CButton and other Objects using CButton)
enum
{
    BUTTON_GREY = BUTTON_GREY_BRIGHT,
    BUTTON_RED1 = BUTTON_RED1_BRIGHT,
    BUTTON_GREEN1 = BUTTON_GREEN1_BRIGHT,
    BUTTON_GREEN2 = BUTTON_GREEN2_BRIGHT,
    BUTTON_RED2 = BUTTON_RED2_BRIGHT,
    BUTTON_STONE = BUTTON_STONE_BRIGHT
};

// some necessary data for CWindow
// background color
enum
{
    WINDOW_NOTHING = -1,
    WINDOW_GREEN1 = WINDOW_BACKGROUND,
    WINDOW_GREEN2 = BUTTON_GREEN1_DARK,
    WINDOW_GREEN3 = BUTTON_GREEN1_BRIGHT,
    WINDOW_GREEN4 = BUTTON_GREEN1_BACKGROUND,
    WINDOW_GREEN5 = BUTTON_GREEN2_DARK,
    WINDOW_GREEN6 = BUTTON_GREEN2_BRIGHT,
    WINDOW_GREEN7 = BUTTON_GREEN2_BACKGROUND,
    WINDOW_GREY1 = BUTTON_GREY_DARK,
    WINDOW_GREY2 = BUTTON_GREY_BRIGHT,
    WINDOW_GREY3 = BUTTON_GREY_BACKGROUND,
    WINDOW_RED1 = BUTTON_RED1_DARK,
    WINDOW_RED2 = BUTTON_RED1_BRIGHT,
    WINDOW_RED3 = BUTTON_RED1_BACKGROUND,
    WINDOW_RED4 = BUTTON_RED2_DARK,
    WINDOW_RED5 = BUTTON_RED2_BRIGHT,
    WINDOW_RED6 = BUTTON_RED2_BACKGROUND,
    WINDOW_STONE1 = BUTTON_STONE_DARK,
    WINDOW_STONE2 = BUTTON_STONE_BRIGHT,
    WINDOW_STONE3 = BUTTON_STONE_BACKGROUND
};
// flags
enum
{
    WINDOW_MOVE = 1,
    WINDOW_CLOSE = 2,
    WINDOW_MINIMIZE = 4,
    WINDOW_RESIZE = 8
};

// modes for editor (what will happen if user clicks somewhere on the map in editor mode)
enum
{
    EDITOR_MODE_CUT = 0,
    EDITOR_MODE_TREE,
    EDITOR_MODE_HEIGHT_RAISE,
    EDITOR_MODE_HEIGHT_REDUCE,
    EDITOR_MODE_HEIGHT_PLANE,
    EDITOR_MODE_HEIGHT_MAKE_BIG_HOUSE,
    EDITOR_MODE_TEXTURE,
    EDITOR_MODE_TEXTURE_MAKE_HARBOUR,
    EDITOR_MODE_LANDSCAPE,
    EDITOR_MODE_FLAG,
    EDITOR_MODE_FLAG_DELETE,
    EDITOR_MODE_RESOURCE_RAISE,
    EDITOR_MODE_RESOURCE_REDUCE,
    EDITOR_MODE_ANIMAL
};

// maximum range for the cursor in editor mode
constexpr auto MAX_CHANGE_SECTION = 10;

// maximum players for a map
constexpr auto MAXPLAYERS = 16;
// maximum map size
constexpr auto MAXMAPWIDTH = 1024;
constexpr auto MAXMAPHEIGHT = 1024;

enum TriangleTerrainType
{
    TRIANGLE_TEXTURE_STEPPE_MEADOW1 = 0x00,
    TRIANGLE_TEXTURE_STEPPE_MEADOW1_HARBOUR = 0x40,
    TRIANGLE_TEXTURE_MINING1 = 0x01,
    TRIANGLE_TEXTURE_SNOW = 0x02,
    TRIANGLE_TEXTURE_SWAMP = 0x03,
    TRIANGLE_TEXTURE_STEPPE = 0x04,
    TRIANGLE_TEXTURE_WATER = 0x05,
    TRIANGLE_TEXTURE_WATER_ = 0x06,
    TRIANGLE_TEXTURE_STEPPE_ = 0x07,
    TRIANGLE_TEXTURE_MEADOW1 = 0x08,
    TRIANGLE_TEXTURE_MEADOW1_HARBOUR = 0x48,
    TRIANGLE_TEXTURE_MEADOW2 = 0x09,
    TRIANGLE_TEXTURE_MEADOW2_HARBOUR = 0x49,
    TRIANGLE_TEXTURE_MEADOW3 = 0x0A,
    TRIANGLE_TEXTURE_MEADOW3_HARBOUR = 0x4A,
    TRIANGLE_TEXTURE_MINING2 = 0x0B,
    TRIANGLE_TEXTURE_MINING3 = 0x0C,
    TRIANGLE_TEXTURE_MINING4 = 0x0D,
    TRIANGLE_TEXTURE_STEPPE_MEADOW2 = 0x0E,
    TRIANGLE_TEXTURE_STEPPE_MEADOW2_HARBOUR = 0x4E,
    TRIANGLE_TEXTURE_FLOWER = 0x0F,
    TRIANGLE_TEXTURE_FLOWER_HARBOUR = 0x4F,
    TRIANGLE_TEXTURE_LAVA = 0x10,
    TRIANGLE_TEXTURE_COLOR = 0x11,
    TRIANGLE_TEXTURE_MINING_MEADOW = 0x12,
    TRIANGLE_TEXTURE_MINING_MEADOW_HARBOUR = 0x52,
    TRIANGLE_TEXTURE_WATER__ = 0x13,
    TRIANGLE_TEXTURE_STEPPE__ = 0x80,
    TRIANGLE_TEXTURE_STEPPE___ = 0x84,
    TRIANGLE_TEXTURE_MEADOW_MIXED =
      0xBF, // this will not be written to map-files, it is only a indicator for mixed meadow in editor mode
    TRIANGLE_TEXTURE_MEADOW_MIXED_HARBOUR =
      0xFF, // this will not be written to map-files, it is only a indicator for mixed meadow in editor mode
};

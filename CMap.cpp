// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CMap.h"
#include "CGame.h"
#include "CIO/CFile.h"
#include "CIO/CFont.h"
#include "CSurface.h"
#include "callbacks.h"
#include "globals.h"
#include "gameData/LandscapeDesc.h"
#include "gameData/TerrainDesc.h"
#include <cassert>
#include <iostream>
#include <string>

void bobMAP::setName(const std::string& newName)
{
    name = newName;
    if(name.size() > 20)
        name.resize(20u);
}
void bobMAP::setAuthor(const std::string& newAuthor)
{
    author = newAuthor;
    if(author.size() > 20)
        author.resize(20);
}

void bobMAP::initVertexCoords()
{
    for(unsigned j = 0; j < height; j++)
    {
        for(unsigned i = 0; i < width; i++)
        {
            MapNode& curVertex = getVertex(i, j);
            curVertex.VertexX = i;
            curVertex.VertexY = j;
        }
    }
    updateVertexCoords();
}

void bobMAP::updateVertexCoords()
{
    width_pixel = width * TRIANGLE_WIDTH;
    height_pixel = height * TRIANGLE_HEIGHT;

    Sint32 b = 0;
    for(unsigned j = 0; j < height; j++)
    {
        Sint32 a;
        if(j % 2u == 0u)
            a = TRIANGLE_WIDTH / 2u;
        else
            a = TRIANGLE_WIDTH;

        for(unsigned i = 0; i < width; i++)
        {
            MapNode& curVertex = getVertex(i, j);
            curVertex.x = a;
            curVertex.y = b - TRIANGLE_INCREASE * (curVertex.h - 0x0A);
            curVertex.z = TRIANGLE_INCREASE * (curVertex.h - 0x0A);
            a += TRIANGLE_WIDTH;
        }
        b += TRIANGLE_HEIGHT;
    }
}

CMap::CMap(const boost::filesystem::path& filepath)
{
    constructMap(filepath);
}

CMap::~CMap()
{
    destructMap();
}

void CMap::constructMap(const boost::filesystem::path& filepath, int width, int height, MapType type,
                        TriangleTerrainType texture, int border, int border_texture)
{
    map = nullptr;
    Surf_Map.reset();
    Surf_RightMenubar.reset();
    displayRect.left = 0;
    displayRect.top = 0;
    displayRect.setSize(global::s2->GameResolution);

    if(!filepath.empty())
    {
        map.reset((bobMAP*)CFile::open_file(filepath, WLD));
        if(map)
            filepath_ = filepath;
    }

    if(!map)
    {
        filepath_.clear();
        map = generateMap(width, height, type, texture, border, border_texture);
    }

    // load the right MAP0x.LST for all pictures
    loadMapPics();

    CSurface::get_nodeVectors(*map);

    // for safety recalculate build and shadow data and test if fishes and water is correct
    for(int i = 0; i < map->height; i++)
    {
        for(int j = 0; j < map->width; j++)
        {
            modifyBuild(j, i);
            modifyShading(j, i);
            modifyResource(j, i);
        }
    }

    Surf_Map.reset();
    active = true;
    Vertex_ = {10, 10};
    RenderBuildHelp = false;
    RenderBorders = true;
    BitsPerPixel = 32;
    MouseBlit = correctMouseBlit(Vertex_);
    ChangeSection_ = 1;
    lastChangeSection = ChangeSection_;
    ChangeSectionHexagonMode = true;
    VertexFillRSU = true;
    VertexFillUSD = true;
    VertexFillRandom = false;
    VertexActivityRandom = false;
    // calculate the maximum number of vertices for cursor
    VertexCounter = 0;
    for(int i = -MAX_CHANGE_SECTION; i <= MAX_CHANGE_SECTION; i++)
    {
        if(abs(i) % 2 == 0)
            for(int j = -MAX_CHANGE_SECTION; j <= MAX_CHANGE_SECTION; j++)
                VertexCounter++;
        else
            for(int j = -MAX_CHANGE_SECTION; j <= MAX_CHANGE_SECTION - 1; j++)
                VertexCounter++;
    }
    Vertices.resize(VertexCounter);
    calculateVertices();
    setupVerticesActivity();
    mode = EDITOR_MODE_HEIGHT_RAISE;
    lastMode = EDITOR_MODE_HEIGHT_RAISE;
    modeContent = 0x00;
    modeContent2 = 0x00;
    modify = false;
    MaxRaiseHeight = 0x3C;
    MinReduceHeight = 0x00;
    saveCurrentVertices = false;
    undoBuffer.clear();
    redoBuffer.clear();

    // we count the players, cause the original editor writes number of players to header no matter if they are set or
    // not
    int CountPlayers = 0;
    // now for internal reasons save all players to a new array, also players with number greater than 7
    // initalize the internal array
    for(int i = 0; i < MAXPLAYERS; i++)
    {
        PlayerHQx[i] = 0xFFFF;
        PlayerHQy[i] = 0xFFFF;
    }
    // find out player positions
    for(int y = 0; y < map->height; y++)
    {
        for(int x = 0; x < map->width; x++)
        {
            MapNode& curVertex = map->getVertex(x, y);
            if(curVertex.objectInfo == 0x80)
            {
                CountPlayers++;
                // objectType is the number of the player
                if(curVertex.objectType < MAXPLAYERS)
                {
                    PlayerHQx[curVertex.objectType] = x;
                    PlayerHQy[curVertex.objectType] = y;

                    // for compatibility with original settlers 2 save the first 7 player positions to the map header
                    // NOTE: this is already done by map loading, but to prevent inconsistence we do it again this way
                    if(curVertex.objectType < 0x07)
                    {
                        map->HQx[curVertex.objectType] = x;
                        map->HQy[curVertex.objectType] = y;
                    }
                }
            }
        }
    }
    map->player = CountPlayers;

    HorizontalMovementLocked = false;
    VerticalMovementLocked = false;

    DescIdx<LandscapeDesc> lt(0);
    for(DescIdx<LandscapeDesc> i(0); i.value < global::worldDesc.landscapes.size(); i.value++)
    {
        if(global::worldDesc.get(i).s2Id == map->type)
        {
            lt = i;
            break;
        }
    }
    for(DescIdx<TerrainDesc> i(0); i.value < global::worldDesc.terrain.size(); i.value++)
    {
        const TerrainDesc& t = global::worldDesc.get(i);
        if(t.landscape == lt)
        {
            if(map->s2IdToTerrain.size() <= t.s2Id)
                map->s2IdToTerrain.resize(t.s2Id + 1);
            map->s2IdToTerrain[t.s2Id] = i;
        }
    }
}
void CMap::destructMap()
{
    // free all surfaces that MAP0x.LST needed
    unloadMapPics();
    undoBuffer.clear();
    redoBuffer.clear();
    Surf_Map.reset();
    Surf_RightMenubar.reset();
    // free vertex array
    Vertices.clear();
    // free map structure memory
    map.reset();
    filepath_.clear();
}

std::unique_ptr<bobMAP> CMap::generateMap(int width, int height, MapType type, TriangleTerrainType texture, int border,
                                          int border_texture)
{
    auto myMap = std::make_unique<bobMAP>();

    myMap->setName("Ohne Namen");
    myMap->width = width;
    myMap->width_old = width;
    myMap->height = height;
    myMap->height_old = height;
    myMap->type = type;
    myMap->player = 0;
    myMap->setAuthor("Niemand");
    for(int i = 0; i < 7; i++)
    {
        myMap->HQx[i] = 0xFFFF;
        myMap->HQy[i] = 0xFFFF;
    }

    myMap->vertex.resize(myMap->width * myMap->height);

    for(int j = 0; j < myMap->height; j++)
    {
        for(int i = 0; i < myMap->width; i++)
        {
            MapNode& curVertex = myMap->getVertex(i, j);
            curVertex.h = 0x0A;

            if((j < border || myMap->height - j <= border) || (i < border || myMap->width - i <= border))
            {
                curVertex.rsuTexture = border_texture;
                curVertex.usdTexture = border_texture;
            } else
            {
                curVertex.rsuTexture = texture;
                curVertex.usdTexture = texture;
            }

            // initialize all other blocks -- outcommented blocks are recalculated at map load
            curVertex.road = 0x00;
            curVertex.objectType = 0x00;
            curVertex.objectInfo = 0x00;
            curVertex.animal = 0x00;
            curVertex.unknown1 = 0x00;
            // curVertex.build = 0x00;
            curVertex.unknown2 = 0x07;
            curVertex.unknown3 = 0x00;
            // curVertex.resource = 0x00;
            // curVertex.shading = 0x00;
            curVertex.unknown5 = 0x00;
        }
    }
    myMap->initVertexCoords();

    return myMap;
}

void CMap::rotateMap()
{
    // we allocate memory for the new triangle field but with x equals the height and y equals the width
    std::vector<MapNode> new_vertex(map->vertex.size());

    undoBuffer.clear();
    redoBuffer.clear();

    // copy old to new while permuting x and y
    for(int y = 0; y < map->height; y++)
    {
        for(int x = 0; x < map->width; x++)
        {
            new_vertex[x * map->height + (map->height - 1 - y)] = map->getVertex(x, y);
        }
    }

    // release old map and point to new
    std::swap(map->vertex, new_vertex);

    // permute width and height
    Uint16 tmp_height = map->height;
    Uint16 tmp_height_old = map->height_old;
    Uint16 tmp_height_pixel = map->height_pixel;
    Uint16 tmp_width = map->width;
    Uint16 tmp_width_old = map->width_old;
    Uint16 tmp_width_pixel = map->width_pixel;

    map->height = tmp_width;
    map->height_old = tmp_width_old;
    map->height_pixel = tmp_width_pixel;
    map->width = tmp_height;
    map->width_old = tmp_height_old;
    map->width_pixel = tmp_height_pixel;

    // permute player positions
    // at first the internal array
    swap(PlayerHQx, PlayerHQy);
    // and now the map array
    swap(map->HQx, map->HQy);

    // recalculate some values
    map->initVertexCoords();
    for(int y = 0; y < map->height; y++)
    {
        for(int x = 0; x < map->width; x++)
        {
            modifyBuild(x, y);
            modifyShading(x, y);
            modifyResource(x, y);
            CSurface::update_shading(*map, x, y);
        }
    }

    // reset mouse and view position to prevent failures
    Vertex_ = {12, 12};
    MouseBlit = correctMouseBlit(Vertex_);
    calculateVertices();
    displayRect.left = 0;
    displayRect.top = 0;
}

void CMap::MirrorMapOnXAxis()
{
    for(int y = 1; y < map->height / 2; y++)
    {
        for(int x = 0; x < map->width; x++)
        {
            map->getVertex(x, map->height - y) = map->getVertex(x, y);
        }
    }

    // recalculate some values
    map->initVertexCoords();
    for(int y = 0; y < map->height; y++)
    {
        for(int x = 0; x < map->width; x++)
        {
            modifyBuild(x, y);
            modifyShading(x, y);
            modifyResource(x, y);
            CSurface::update_shading(*map, x, y);
        }
    }
}

void CMap::MirrorMapOnYAxis()
{
    for(int y = 0; y < map->height; y++)
    {
        for(int x = 0; x < map->width / 2; x++)
        {
            if(y % 2 != 0)
            {
                if(x != map->width / 2 - 1)
                    map->getVertex(map->width - 2 - x, y) = map->getVertex(x, y);
            } else
                map->getVertex(map->width - 1 - x, y) = map->getVertex(x, y);
        }
    }

    // recalculate some values
    map->initVertexCoords();
    for(int y = 0; y < map->height; y++)
    {
        for(int x = 0; x < map->width; x++)
        {
            modifyBuild(x, y);
            modifyShading(x, y);
            modifyResource(x, y);
            CSurface::update_shading(*map, x, y);
        }
    }
}

void CMap::loadMapPics()
{
    std::string picFile, palFile;
    switch(map->type)
    {
        case 0:
            picFile = "DATA/MAP00.LST";
            palFile = "GFX/PALETTE/PAL5.BBM";
            break;
        case 1:
            picFile = "DATA/MAP01.LST";
            palFile = "GFX/PALETTE/PAL6.BBM";
            break;
        case 2:
            picFile = "DATA/MAP02.LST";
            palFile = "GFX/PALETTE/PAL7.BBM";
            break;
        default:
            picFile = "DATA/MAP00.LST";
            palFile = "GFX/PALETTE/PAL5.BBM";
            break;
    }
    CFile::set_palArray(&global::palArray[PAL_MAPxx]);
    // load only the palette at this time from MAP0x.LST
    std::cout << "\nLoading palette from file: " << picFile << "...";
    if(!CFile::open_file(global::gameDataFilePath / picFile, LST, true))
    {
        std::cout << "failure";
    }
    // set the right palette
    CFile::set_palActual(CFile::get_palArray() - 1);
    std::cout << "\nLoading file: " << picFile << "...";
    if(!CFile::open_file(global::gameDataFilePath / picFile, LST))
    {
        std::cout << "failure";
    }
    // set back palette
    // CFile::set_palActual(CFile::get_palArray());
    // std::cout << "\nLoading file: DATA/MBOB/ROM_BOBS.LST...";
    // if ( !CFile::open_file(global::gameDataFilePath + "DATA/MBOB/ROM_BOBS.LST", LST) )
    //{
    //    std::cout << "failure";
    //}
    // set back palette
    CFile::set_palActual(CFile::get_palArray());
    CFile::set_palArray(&global::palArray[PAL_xBBM]);
    // load palette file for the map (for precalculated shading)
    std::cout << "\nLoading palette from file: " << palFile << "...";
    if(!CFile::open_file(global::gameDataFilePath / palFile, BBM, true))
    {
        std::cout << "failure";
    }
}

void CMap::unloadMapPics()
{
    for(int i = MAPPIC_ARROWCROSS_YELLOW; i <= MAPPIC_LAST_ENTRY; i++)
    {
        global::bmpArray[i].surface.reset();
    }
    // set back bmpArray-pointer, cause MAP0x.LST is no longer needed
    CFile::set_bmpArray(&global::bmpArray[MAPPIC_ARROWCROSS_YELLOW]);
    // set back palArray-pointer, cause PALx.BBM is no longer needed
    CFile::set_palActual(&global::palArray[PAL_IO]);
    CFile::set_palArray(&global::palArray[PAL_IO + 1]);
}

void CMap::moveMap(Position offset)
{
    displayRect.setOrigin(displayRect.getOrigin() + offset);
    // reset coords of displayRects when end of map is reached
    if(displayRect.left >= map->width_pixel)
        displayRect.move(Position(-map->width_pixel, 0));
    else if(displayRect.left <= -static_cast<int>(displayRect.getSize().x))
        displayRect.setOrigin(Position(map->width_pixel - displayRect.getSize().x, displayRect.top));

    if(displayRect.top >= map->height_pixel)
        displayRect.move(Position(0, -map->height_pixel));
    else if(displayRect.top <= -static_cast<int>(displayRect.getSize().y))
        displayRect.setOrigin(Position(displayRect.left, map->height_pixel - displayRect.getSize().y));
}

void CMap::setMouseData(const SDL_MouseMotionEvent& motion)
{
    // following code important for blitting the right field of the map
    // Are we scrolling?
    if(startScrollPos)
    {
        assert(motion.state & SDL_BUTTON(3));
        Position offset = Position(motion.x, motion.y) - *startScrollPos;
        if(HorizontalMovementLocked)
            offset.x = 0;
        if(VerticalMovementLocked)
            offset.y = 0;
        moveMap(offset);

        // this whole "warping-thing" is to prevent cursor-moving WITHIN the window while user moves over the map
        SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
        SDL_WarpMouseInWindow(nullptr, startScrollPos->x, startScrollPos->y);
        SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);
    }

    storeVerticesFromMouse(motion.x, motion.y, motion.state);
}

void CMap::onLeftMouseDown(const Point32& pos)
{ // find out if user clicked on one of the game menu pictures
    // we start with lower menubar
    const Point32 displaySize(displayRect.getSize());
    if(pos.x >= (displaySize.x / 2 - 236) && pos.x <= (displaySize.x / 2 - 199) && pos.y >= (displaySize.y - 35)
       && pos.y <= (displaySize.y - 3))
    {
        // the height-mode picture was clicked
        mode = EDITOR_MODE_HEIGHT_RAISE;
    } else if(pos.x >= (displaySize.x / 2 - 199) && pos.x <= (displaySize.x / 2 - 162) && pos.y >= (displaySize.y - 35)
              && pos.y <= (displaySize.y - 3))
    {
        // the texture-mode picture was clicked
        mode = EDITOR_MODE_TEXTURE;
        callback::EditorTextureMenu(INITIALIZING_CALL);
    } else if(pos.x >= (displaySize.x / 2 - 162) && pos.x <= (displaySize.x / 2 - 125) && pos.y >= (displaySize.y - 35)
              && pos.y <= (displaySize.y - 3))
    {
        // the tree-mode picture was clicked
        mode = EDITOR_MODE_TREE;
        callback::EditorTreeMenu(INITIALIZING_CALL);
    } else if(pos.x >= (displaySize.x / 2 - 125) && pos.x <= (displaySize.x / 2 - 88) && pos.y >= (displaySize.y - 35)
              && pos.y <= (displaySize.y - 3))
    {
        // the resource-mode picture was clicked
        mode = EDITOR_MODE_RESOURCE_RAISE;
        callback::EditorResourceMenu(INITIALIZING_CALL);
    } else if(pos.x >= (displaySize.x / 2 - 88) && pos.x <= (displaySize.x / 2 - 51) && pos.y >= (displaySize.y - 35)
              && pos.y <= (displaySize.y - 3))
    {
        // the landscape-mode picture was clicked
        mode = EDITOR_MODE_LANDSCAPE;
        callback::EditorLandscapeMenu(INITIALIZING_CALL);
    } else if(pos.x >= (displaySize.x / 2 - 51) && pos.x <= (displaySize.x / 2 - 14) && pos.y >= (displaySize.y - 35)
              && pos.y <= (displaySize.y - 3))
    {
        // the animal-mode picture was clicked
        mode = EDITOR_MODE_ANIMAL;
        callback::EditorAnimalMenu(INITIALIZING_CALL);
    } else if(pos.x >= (displaySize.x / 2 - 14) && pos.x <= (displaySize.x / 2 + 23) && pos.y >= (displaySize.y - 35)
              && pos.y <= (displaySize.y - 3))
    {
        // the player-mode picture was clicked
        mode = EDITOR_MODE_FLAG;
        ChangeSection_ = 0;
        setupVerticesActivity();
        callback::EditorPlayerMenu(INITIALIZING_CALL);
    } else if(pos.x >= (displaySize.x / 2 + 96) && pos.x <= (displaySize.x / 2 + 133) && pos.y >= (displaySize.y - 35)
              && pos.y <= (displaySize.y - 3))
    {
        // the build-help picture was clicked
        RenderBuildHelp = !RenderBuildHelp;
    } else if(pos.x >= (displaySize.x / 2 + 131) && pos.x <= (displaySize.x / 2 + 168) && pos.y >= (displaySize.y - 35)
              && pos.y <= (displaySize.y - 3))
    {
        // the minimap picture was clicked
        callback::MinimapMenu(INITIALIZING_CALL);
    } else if(pos.x >= (displaySize.x / 2 + 166) && pos.x <= (displaySize.x / 2 + 203) && pos.y >= (displaySize.y - 35)
              && pos.y <= (displaySize.y - 3))
    {
        // the create-world picture was clicked
        callback::EditorCreateMenu(INITIALIZING_CALL);
    } else if(pos.x >= (displaySize.x / 2 + 203) && pos.x <= (displaySize.x / 2 + 240) && pos.y >= (displaySize.y - 35)
              && pos.y <= (displaySize.y - 3))
    {
        // the editor-main-menu picture was clicked
        callback::EditorMainMenu(INITIALIZING_CALL);
    }
    // now we check the right menubar
    else if(pos.x >= (displaySize.x - 37) && pos.x <= (displaySize.x) && pos.y >= (displaySize.y / 2 + 162)
            && pos.y <= (displaySize.y / 2 + 199))
    {
        // the bugkill picture was clicked for quickload
        callback::PleaseWait(INITIALIZING_CALL);
        // we have to close the windows and initialize them again to prevent failures
        callback::EditorCursorMenu(MAP_QUIT);
        callback::EditorTextureMenu(MAP_QUIT);
        callback::EditorTreeMenu(MAP_QUIT);
        callback::EditorLandscapeMenu(MAP_QUIT);
        callback::MinimapMenu(MAP_QUIT);
        callback::EditorResourceMenu(MAP_QUIT);
        callback::EditorAnimalMenu(MAP_QUIT);
        callback::EditorPlayerMenu(MAP_QUIT);

        destructMap();
        constructMap(global::userMapsPath / "quicksave.swd");
        callback::PleaseWait(WINDOW_QUIT_MESSAGE);
    } else if(pos.x >= (displaySize.x - 37) && pos.x <= (displaySize.x) && pos.y >= (displaySize.y / 2 + 200)
              && pos.y <= (displaySize.y / 2 + 237))
    {
        // the bugkill picture was clicked for quicksave
        callback::PleaseWait(INITIALIZING_CALL);
        if(!CFile::save_file(global::userMapsPath / "quicksave.swd", SWD, getMap()))
        {
            callback::ShowStatus(INITIALIZING_CALL);
            callback::ShowStatus(2);
        }
        callback::PleaseWait(WINDOW_QUIT_MESSAGE);
    } else if(pos.x >= (displaySize.x - 37) && pos.x <= (displaySize.x) && pos.y >= (displaySize.y / 2 - 239)
              && pos.y <= (displaySize.y / 2 - 202))
    {
        // the cursor picture was clicked
        callback::EditorCursorMenu(INITIALIZING_CALL);
    } else
    {
        // no picture was clicked
        // touch vertex data
        modify = true;
        saveCurrentVertices = true;
    }
}

void CMap::setMouseData(const SDL_MouseButtonEvent& button)
{
    if(button.state == SDL_PRESSED)
    {
        if(button.button == SDL_BUTTON_LEFT)
            onLeftMouseDown({button.x, button.y});
        else if(button.button == SDL_BUTTON_RIGHT)
            startScrollPos = Position(button.x, button.y);
    } else if(button.state == SDL_RELEASED)
    {
        // stop touching vertex data
        if(button.button == SDL_BUTTON_LEFT)
            modify = false;
        else if(button.button == SDL_BUTTON_RIGHT)
            startScrollPos = std::nullopt;
    }
}

namespace {
SavedVertex saveVertex(Position pt, const bobMAP& map)
{
    SavedVertex res;
    res.pos = pt;
    for(int i = pt.x - SavedVertex::NODES_PER_DIR, k = 0; i <= pt.x + SavedVertex::NODES_PER_DIR; i++, k++)
    {
        for(int j = pt.y - SavedVertex::NODES_PER_DIR, l = 0; j <= pt.y + SavedVertex::NODES_PER_DIR; j++, l++)
        {
            // Correct i and j to take wrap around map borders into account
            int m = i;
            if(m < 0)
                m += map.width;
            else if(m >= map.width)
                m -= map.width;
            int n = j;
            if(n < 0)
                n += map.height;
            else if(n >= map.height)
                n -= map.height;
            res.PointsArroundVertex[l][k] = map.getVertex(m, n);
        }
    }
    return res;
}

void restoreVertex(const SavedVertex& vertex, bobMAP& map)
{
    for(int i = vertex.pos.x - SavedVertex::NODES_PER_DIR, k = 0; i <= vertex.pos.x + SavedVertex::NODES_PER_DIR;
        i++, k++)
    {
        for(int j = vertex.pos.y - SavedVertex::NODES_PER_DIR, l = 0; j <= vertex.pos.y + SavedVertex::NODES_PER_DIR;
            j++, l++)
        {
            int m = i;
            if(m < 0)
                m += map.width;
            else if(m >= map.width)
                m -= map.width;
            int n = j;
            if(n < 0)
                n += map.height;
            else if(n >= map.height)
                n -= map.height;
            map.getVertex(m, n) = vertex.PointsArroundVertex[l][k];
        }
    }
}
} // namespace

void CMap::setKeyboardData(const SDL_KeyboardEvent& key)
{
    if(key.type == SDL_KEYDOWN)
    {
        switch(key.keysym.sym)
        {
            case SDLK_LSHIFT:
                if(mode == EDITOR_MODE_HEIGHT_RAISE)
                    mode = EDITOR_MODE_HEIGHT_REDUCE;
                else if(mode == EDITOR_MODE_RESOURCE_RAISE)
                    mode = EDITOR_MODE_RESOURCE_REDUCE;
                else if(mode == EDITOR_MODE_FLAG)
                    mode = EDITOR_MODE_FLAG_DELETE;
                break;
            case SDLK_LALT:
                if(mode == EDITOR_MODE_HEIGHT_RAISE)
                    mode = EDITOR_MODE_HEIGHT_PLANE;
                break;
            case SDLK_INSERT:
                if(mode == EDITOR_MODE_HEIGHT_RAISE || mode == EDITOR_MODE_HEIGHT_REDUCE)
                {
                    if(MaxRaiseHeight > 0x00)
                        MaxRaiseHeight--;
                }
                break;
            case SDLK_HOME:
                if(mode == EDITOR_MODE_HEIGHT_RAISE || mode == EDITOR_MODE_HEIGHT_REDUCE)
                {
                    MaxRaiseHeight = 0x3C;
                }
                break;
            case SDLK_PAGEUP:
                if(mode == EDITOR_MODE_HEIGHT_RAISE || mode == EDITOR_MODE_HEIGHT_REDUCE)
                {
                    if(MaxRaiseHeight < 0x3C)
                        MaxRaiseHeight++;
                }
                break;
            case SDLK_DELETE:
                if(mode == EDITOR_MODE_HEIGHT_RAISE || mode == EDITOR_MODE_HEIGHT_REDUCE)
                {
                    if(MinReduceHeight > 0x00)
                        MinReduceHeight--;
                }
                break;
            case SDLK_END:
                if(mode == EDITOR_MODE_HEIGHT_RAISE || mode == EDITOR_MODE_HEIGHT_REDUCE)
                {
                    MinReduceHeight = 0x00;
                }
                break;
            case SDLK_PAGEDOWN:
                if(mode == EDITOR_MODE_HEIGHT_RAISE || mode == EDITOR_MODE_HEIGHT_REDUCE)
                {
                    if(MinReduceHeight < 0x3C)
                        MinReduceHeight++;
                }
                break;
            case SDLK_LCTRL:
                lastMode = mode;
                mode = EDITOR_MODE_CUT;
                break;
            case SDLK_b:
                if(mode != EDITOR_MODE_HEIGHT_MAKE_BIG_HOUSE && mode != EDITOR_MODE_TEXTURE_MAKE_HARBOUR)
                {
                    lastMode = mode;
                    lastChangeSection = ChangeSection_;
                    ChangeSection_ = 0;
                    setupVerticesActivity();
                    mode = EDITOR_MODE_HEIGHT_MAKE_BIG_HOUSE;
                }
                break;
            case SDLK_h:
                if(mode != EDITOR_MODE_HEIGHT_MAKE_BIG_HOUSE && mode != EDITOR_MODE_TEXTURE_MAKE_HARBOUR)
                {
                    lastMode = mode;
                    lastChangeSection = ChangeSection_;
                    ChangeSection_ = 0;
                    setupVerticesActivity();
                    mode = EDITOR_MODE_TEXTURE_MAKE_HARBOUR;
                }
                break;
            case SDLK_r:
                callback::PleaseWait(INITIALIZING_CALL);
                rotateMap();
                rotateMap();
                callback::PleaseWait(WINDOW_QUIT_MESSAGE);
                break;
            case SDLK_x:
                callback::PleaseWait(INITIALIZING_CALL);
                MirrorMapOnXAxis();
                callback::PleaseWait(WINDOW_QUIT_MESSAGE);
                break;
            case SDLK_y:
                callback::PleaseWait(INITIALIZING_CALL);
                MirrorMapOnYAxis();
                callback::PleaseWait(WINDOW_QUIT_MESSAGE);
                break;
            case SDLK_KP_PLUS:
                if(ChangeSection_ < MAX_CHANGE_SECTION)
                {
                    ChangeSection_++;
                    setupVerticesActivity();
                }
                break;
            case SDLK_KP_MINUS:
                if(ChangeSection_ > 0)
                {
                    ChangeSection_--;
                    setupVerticesActivity();
                }
                break;
            case SDLK_1:
            case SDLK_KP_1:
                ChangeSection_ = 0;
                setupVerticesActivity();
                break;
            case SDLK_2:
            case SDLK_KP_2:
                ChangeSection_ = 1;
                setupVerticesActivity();
                break;
            case SDLK_3:
            case SDLK_KP_3:
                ChangeSection_ = 2;
                setupVerticesActivity();
                break;
            case SDLK_4:
            case SDLK_KP_4:
                ChangeSection_ = 3;
                setupVerticesActivity();
                break;
            case SDLK_5:
            case SDLK_KP_5:
                ChangeSection_ = 4;
                setupVerticesActivity();
                break;
            case SDLK_6:
            case SDLK_KP_6:
                ChangeSection_ = 5;
                setupVerticesActivity();
                break;
            case SDLK_7:
            case SDLK_KP_7:
                ChangeSection_ = 6;
                setupVerticesActivity();
                break;
            case SDLK_8:
            case SDLK_KP_8:
                ChangeSection_ = 7;
                setupVerticesActivity();
                break;
            case SDLK_9:
            case SDLK_KP_9:
                ChangeSection_ = 8;
                setupVerticesActivity();
                break;
            case SDLK_SPACE: RenderBuildHelp = !RenderBuildHelp; break;
            case SDLK_F11: RenderBorders = !RenderBorders; break;
            case SDLK_q:
                if(!saveCurrentVertices)
                {
                    if(SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT))
                    {
                        if(!redoBuffer.empty())
                        {
                            undoBuffer.push_back(saveVertex(redoBuffer.back().pos, *map));
                            restoreVertex(redoBuffer.back(), *map);
                            redoBuffer.pop_back();
                        }
                    } else if(!undoBuffer.empty())
                    {
                        redoBuffer.push_back(saveVertex(undoBuffer.back().pos, *map));
                        restoreVertex(undoBuffer.back(), *map);
                        undoBuffer.pop_back();
                    }
                }
                break;
            case SDLK_UP:
            case SDLK_DOWN:
            case SDLK_LEFT:
            case SDLK_RIGHT:
            {
                Position offset{key.keysym.sym == SDLK_LEFT ? -100 : (key.keysym.sym == SDLK_RIGHT ? 100 : 0),
                                key.keysym.sym == SDLK_UP ? -100 : (key.keysym.sym == SDLK_DOWN ? 100 : 0)};
                moveMap(offset);
            }
            break;
            case SDLK_F1: // help menu
                callback::EditorHelpMenu(INITIALIZING_CALL);
                break;
            case SDLK_g: // convert map to greenland
                callback::PleaseWait(INITIALIZING_CALL);

                // we have to close the windows and initialize them again to prevent failures
                callback::EditorCursorMenu(MAP_QUIT);
                callback::EditorTextureMenu(MAP_QUIT);
                callback::EditorTreeMenu(MAP_QUIT);
                callback::EditorLandscapeMenu(MAP_QUIT);
                callback::MinimapMenu(MAP_QUIT);
                callback::EditorResourceMenu(MAP_QUIT);
                callback::EditorAnimalMenu(MAP_QUIT);
                callback::EditorPlayerMenu(MAP_QUIT);

                map->type = MAP_GREENLAND;
                unloadMapPics();
                loadMapPics();

                callback::PleaseWait(WINDOW_QUIT_MESSAGE);
                break;
            case SDLK_o: // convert map to wasteland
                callback::PleaseWait(INITIALIZING_CALL);

                // we have to close the windows and initialize them again to prevent failures
                callback::EditorCursorMenu(MAP_QUIT);
                callback::EditorTextureMenu(MAP_QUIT);
                callback::EditorTreeMenu(MAP_QUIT);
                callback::EditorLandscapeMenu(MAP_QUIT);
                callback::MinimapMenu(MAP_QUIT);
                callback::EditorResourceMenu(MAP_QUIT);
                callback::EditorAnimalMenu(MAP_QUIT);
                callback::EditorPlayerMenu(MAP_QUIT);

                map->type = MAP_WASTELAND;
                unloadMapPics();
                loadMapPics();

                break;
            case SDLK_w: // convert map to winterland
                callback::PleaseWait(INITIALIZING_CALL);

                // we have to close the windows and initialize them again to prevent failures
                callback::EditorCursorMenu(MAP_QUIT);
                callback::EditorTextureMenu(MAP_QUIT);
                callback::EditorTreeMenu(MAP_QUIT);
                callback::EditorLandscapeMenu(MAP_QUIT);
                callback::MinimapMenu(MAP_QUIT);
                callback::EditorResourceMenu(MAP_QUIT);
                callback::EditorAnimalMenu(MAP_QUIT);
                callback::EditorPlayerMenu(MAP_QUIT);

                map->type = MAP_WINTERLAND;
                unloadMapPics();
                loadMapPics();

                callback::PleaseWait(WINDOW_QUIT_MESSAGE);
                break;
            case SDLK_p:
                if(BitsPerPixel == 8)
                    setBitsPerPixel(32);
                else
                    setBitsPerPixel(8);
                break;
            case SDLK_F9: // lock horizontal movement
                HorizontalMovementLocked = !HorizontalMovementLocked;

                break;
            case SDLK_F10: // lock vertical movement
                           // VerticalMovementLocked = !VerticalMovementLocked;
            default: break;
        }
    } else if(key.type == SDL_KEYUP)
    {
        switch(key.keysym.sym)
        {
            case SDLK_LSHIFT: // user probably released EDITOR_MODE_HEIGHT_REDUCE
                if(mode == EDITOR_MODE_HEIGHT_REDUCE)
                    mode = EDITOR_MODE_HEIGHT_RAISE;
                else if(mode == EDITOR_MODE_RESOURCE_REDUCE)
                    mode = EDITOR_MODE_RESOURCE_RAISE;
                else if(mode == EDITOR_MODE_FLAG_DELETE)
                    mode = EDITOR_MODE_FLAG;
                break;
            case SDLK_LALT: // user probably released EDITOR_MODE_HEIGHT_PLANE
                if(mode == EDITOR_MODE_HEIGHT_PLANE)
                    mode = EDITOR_MODE_HEIGHT_RAISE;
                break;
            case SDLK_LCTRL: // user probably released EDITOR_MODE_CUT
                mode = lastMode;
                break;
            case SDLK_b: // user probably released EDITOR_MODE_HEIGHT_MAKE_BIG_HOUSE
            case SDLK_h: // user probably released EDITOR_MODE_TEXTURE_MAKE_HARBOUR
                mode = lastMode;
                ChangeSection_ = lastChangeSection;
                setupVerticesActivity();
                break;
            default: break;
        }
    }
}

void CMap::storeVerticesFromMouse(Uint16 MouseX, Uint16 MouseY, Uint8 /*MouseState*/)
{
    // if user raises or reduces the height of a vertex, don't let the cursor jump to another vertex
    // if ( (MouseState == SDL_PRESSED) && (mode == EDITOR_MODE_HEIGHT_RAISE || mode == EDITOR_MODE_HEIGHT_REDUCE) )
    // return;

    int X = 0, Xeven = 0, Xodd = 0;
    int Y = 0, MousePosY = 0;

    // get X
    // following out commented lines are the correct ones, but for tolerance (to prevent to early jumps of the cursor)
    // we subtract "TRIANGLE_WIDTH/2"  Xeven = (MouseX + displayRect.left) / TRIANGLE_WIDTH;
    Xeven = (MouseX + displayRect.left - TRIANGLE_WIDTH / 2) / TRIANGLE_WIDTH;
    if(Xeven < 0)
        Xeven += (map->width);
    else if(Xeven > map->width - 1)
        Xeven -= (map->width - 1);
    // Add rows are already shifted by TRIANGLE_WIDTH / 2
    Xodd = (MouseX + displayRect.left) / TRIANGLE_WIDTH;
    // Xodd = (MouseX + displayRect.left) / TRIANGLE_WIDTH;
    if(Xodd < 0)
        Xodd += (map->width - 1);
    else if(Xodd > map->width - 1)
        Xodd -= (map->width);

    MousePosY = MouseY + displayRect.top;
    // correct mouse position Y if displayRect is outside map edges
    if(MousePosY < 0)
        MousePosY += map->height_pixel;
    else if(MousePosY > map->height_pixel)
        MousePosY = MouseY - (map->height_pixel - displayRect.top);

    // get Y
    for(int j = 0; j < map->height; j++)
    {
        if(j % 2 == 0)
        {
            // subtract "TRIANGLE_HEIGHT/2" is for tolerance, we did the same for X
            if((MousePosY - TRIANGLE_HEIGHT / 2) > map->getVertex(Xeven, j).y)
                Y++;
            else
            {
                X = Xodd;
                break;
            }
        } else
        {
            if((MousePosY - TRIANGLE_HEIGHT / 2) > map->getVertex(Xodd, j).y)
                Y++;
            else
            {
                X = Xeven;
                break;
            }
        }
    }
    if(Y >= map->height)
    {
        Y -= map->height;
        X = Y % 2 == 0 ? Xeven : Xodd;
    }

    Vertex_ = {X, Y};

    MouseBlit = correctMouseBlit(Vertex_);

    calculateVertices();
}

Position CMap::correctMouseBlit(Position vertexPos) const
{
    const auto& vertex = map->getVertex(vertexPos);
    Position newBlit(vertex.x, vertex.y);
    if(newBlit.x < displayRect.left)
        newBlit.x += map->width_pixel;
    else if(newBlit.x > displayRect.right)
        newBlit.x -= map->width_pixel;
    if(newBlit.y < displayRect.top)
        newBlit.y += map->height_pixel;
    else if(newBlit.y > displayRect.bottom)
        newBlit.y -= map->height_pixel;
    newBlit -= displayRect.getOrigin();

    return newBlit;
}

void CMap::render()
{
    // check if game resolution has been changed
    if(displayRect.getSize() != global::s2->GameResolution)
    {
        displayRect.setSize(global::s2->GameResolution);
        Surf_Map.reset();
    }

    // if we need a new surface
    if(!Surf_Map)
    {
        if(BitsPerPixel == 8)
            Surf_Map =
              makePalSurface(displayRect.getSize().x, displayRect.getSize().y, global::palArray[PAL_xBBM].colors);
        else
            Surf_Map = makeRGBSurface(displayRect.getSize().x, displayRect.getSize().y);
    }
    // else
    // clear the surface before drawing new (in normal case not needed)
    // SDL_FillRect( Surf_Map, nullptr, SDL_MapRGB(Surf_Map->format,0,0,0) );

    // touch vertex data if user modifies it
    if(modify)
        modifyVertex();

    if(!map->vertex.empty())
        CSurface::DrawTriangleField(Surf_Map.get(), displayRect, *map);

    // draw pictures to cursor position
    int symbol_index, symbol_index2 = -1;
    switch(mode)
    {
        case EDITOR_MODE_CUT: symbol_index = CURSOR_SYMBOL_SCISSORS; break;
        case EDITOR_MODE_TREE: symbol_index = CURSOR_SYMBOL_TREE; break;
        case EDITOR_MODE_HEIGHT_RAISE: symbol_index = CURSOR_SYMBOL_ARROW_UP; break;
        case EDITOR_MODE_HEIGHT_REDUCE: symbol_index = CURSOR_SYMBOL_ARROW_DOWN; break;
        case EDITOR_MODE_HEIGHT_PLANE:
            symbol_index = CURSOR_SYMBOL_ARROW_UP;
            symbol_index2 = CURSOR_SYMBOL_ARROW_DOWN;
            break;
        case EDITOR_MODE_HEIGHT_MAKE_BIG_HOUSE: symbol_index = MAPPIC_ARROWCROSS_RED_HOUSE_BIG; break;
        case EDITOR_MODE_TEXTURE: symbol_index = CURSOR_SYMBOL_TEXTURE; break;
        case EDITOR_MODE_TEXTURE_MAKE_HARBOUR: symbol_index = MAPPIC_ARROWCROSS_RED_HOUSE_HARBOUR; break;
        case EDITOR_MODE_LANDSCAPE: symbol_index = CURSOR_SYMBOL_LANDSCAPE; break;
        case EDITOR_MODE_FLAG:
        case EDITOR_MODE_FLAG_DELETE: symbol_index = CURSOR_SYMBOL_FLAG; break;
        case EDITOR_MODE_RESOURCE_REDUCE: symbol_index = CURSOR_SYMBOL_PICKAXE_MINUS; break;
        case EDITOR_MODE_RESOURCE_RAISE: symbol_index = CURSOR_SYMBOL_PICKAXE_PLUS; break;
        case EDITOR_MODE_ANIMAL: symbol_index = CURSOR_SYMBOL_ANIMAL; break;
        default: symbol_index = CURSOR_SYMBOL_ARROW_UP; break;
    }
    for(int i = 0; i < VertexCounter; i++)
    {
        if(Vertices[i].active)
        {
            CSurface::Draw(Surf_Map, global::bmpArray[symbol_index].surface, Vertices[i].blit - Position::all(10));
            if(symbol_index2 >= 0)
                CSurface::Draw(Surf_Map, global::bmpArray[symbol_index2].surface, Vertices[i].blit - Position(0, 7));
        }
    }

    // draw the frame
    if(displayRect.getSize() == Extent(640, 480))
        CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_640_480].surface);
    else if(displayRect.getSize() == Extent(800, 600))
        CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_800_600].surface);
    else if(displayRect.getSize() == Extent(1024, 768))
        CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_1024_768].surface);
    else if(displayRect.getSize() == Extent(1280, 1024))
    {
        CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_LEFT_1280_1024].surface);
        CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_RIGHT_1280_1024].surface, Position(640, 0));
    } else
    {
        // draw the corners
        CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_640_480].surface, 0, 0, 0, 0, 150, 150);
        CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_640_480].surface, 0, displayRect.getSize().y - 150, 0,
                       480 - 150, 150, 150);
        CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_640_480].surface, displayRect.getSize().x - 150, 0,
                       640 - 150, 0, 150, 150);
        CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_640_480].surface, displayRect.getSize().x - 150,
                       displayRect.getSize().y - 150, 640 - 150, 480 - 150, 150, 150);
        // draw the edges
        unsigned x = 150, y = 150;
        while(x + 150 < displayRect.getSize().x)
        {
            CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_640_480].surface, x, 0, 150, 0, 150, 12);
            CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_640_480].surface, x, displayRect.getSize().y - 12, 150,
                           0, 150, 12);
            x += 150;
        }
        while(y + 150 < displayRect.getSize().y)
        {
            CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_640_480].surface, 0, y, 0, 150, 12, 150);
            CSurface::Draw(Surf_Map, global::bmpArray[MAINFRAME_640_480].surface, displayRect.getSize().x - 12, y, 0,
                           150, 12, 150);
            y += 150;
        }
    }

    // draw the statues at the frame
    CSurface::Draw(Surf_Map, global::bmpArray[STATUE_UP_LEFT].surface, Position(12, 12));
    CSurface::Draw(Surf_Map, global::bmpArray[STATUE_UP_RIGHT].surface,
                   Position(displayRect.getSize().x - global::bmpArray[STATUE_UP_RIGHT].w - 12, 12));
    CSurface::Draw(Surf_Map, global::bmpArray[STATUE_DOWN_LEFT].surface,
                   Position(12, displayRect.getSize().y - global::bmpArray[STATUE_DOWN_LEFT].h - 12));
    CSurface::Draw(Surf_Map, global::bmpArray[STATUE_DOWN_RIGHT].surface,
                   displayRect.getSize()
                     - Position(global::bmpArray[STATUE_DOWN_RIGHT].w, global::bmpArray[STATUE_DOWN_RIGHT].h)
                     - Position::all(12));

    // lower menubar
    const Position menubarPos = Position(displayRect.getSize().x / 2, displayRect.getSize().y);
    // draw lower menubar
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR].surface,
                   menubarPos - Extent(global::bmpArray[MENUBAR].w / 2, global::bmpArray[MENUBAR].h));

    // draw pictures to lower menubar
    // backgrounds
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, menubarPos.x - 236, menubarPos.y - 36, 0, 0,
                   37, 32);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, menubarPos.x - 199, menubarPos.y - 36, 0, 0,
                   37, 32);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, menubarPos.x - 162, menubarPos.y - 36, 0, 0,
                   37, 32);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, menubarPos.x - 125, menubarPos.y - 36, 0, 0,
                   37, 32);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, menubarPos.x - 88, menubarPos.y - 36, 0, 0,
                   37, 32);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, menubarPos.x - 51, menubarPos.y - 36, 0, 0,
                   37, 32);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, menubarPos.x - 14, menubarPos.y - 36, 0, 0,
                   37, 32);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, menubarPos.x + 92, menubarPos.y - 36, 0, 0,
                   37, 32);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, menubarPos.x + 129, menubarPos.y - 36, 0, 0,
                   37, 32);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, menubarPos.x + 166, menubarPos.y - 36, 0, 0,
                   37, 32);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, menubarPos.x + 203, menubarPos.y - 36, 0, 0,
                   37, 32);
    // pictures
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_HEIGHT].surface, menubarPos - Extent(232, 35));
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_TEXTURE].surface, menubarPos - Extent(195, 35));
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_TREE].surface, menubarPos - Extent(158, 37));
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_RESOURCE].surface, menubarPos - Extent(121, 32));
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_LANDSCAPE].surface, menubarPos - Extent(84, 37));
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_ANIMAL].surface, menubarPos - Extent(48, 36));
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_PLAYER].surface, menubarPos - Extent(10, 34));

    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_BUILDHELP].surface, menubarPos + Position(96, -35));
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_MINIMAP].surface, menubarPos + Position(131, -37));
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_NEWWORLD].surface, menubarPos + Position(166, -37));
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_COMPUTER].surface, menubarPos + Position(207, -35));

    // right menubar
    // do we need a surface?
    if(!Surf_RightMenubar)
    {
        // we permute width and height, cause we want to rotate the menubar 90 degrees
        if((Surf_RightMenubar = makePalSurface(global::bmpArray[MENUBAR].h, global::bmpArray[MENUBAR].w,
                                               global::palArray[PAL_RESOURCE].colors))
           != nullptr)
        {
            SDL_SetColorKey(Surf_RightMenubar.get(), SDL_TRUE, SDL_MapRGB(Surf_RightMenubar->format, 0, 0, 0));
            CSurface::Draw(Surf_RightMenubar, global::bmpArray[MENUBAR].surface, 0, 0, 270);
        }
    }
    // draw right menubar (remember permutation of width and height)
    const Position rightMenubarPos = Position(displayRect.getSize().x, displayRect.getSize().y / 2);
    CSurface::Draw(Surf_Map, Surf_RightMenubar,
                   rightMenubarPos - Extent(global::bmpArray[MENUBAR].h, global::bmpArray[MENUBAR].w / 2));

    // draw pictures to right menubar
    // backgrounds
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, rightMenubarPos.x - 36,
                   rightMenubarPos.y - 239, 0, 0, 32, 37);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, rightMenubarPos.x - 36,
                   rightMenubarPos.y - 202, 0, 0, 32, 37);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, rightMenubarPos.x - 36,
                   rightMenubarPos.y - 165, 0, 0, 32, 37);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, rightMenubarPos.x - 36,
                   rightMenubarPos.y - 128, 0, 0, 32, 37);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, rightMenubarPos.x - 36,
                   rightMenubarPos.y - 22, 0, 0, 32, 37);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, rightMenubarPos.x - 36,
                   rightMenubarPos.y + 15, 0, 0, 32, 37);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, rightMenubarPos.x - 36,
                   rightMenubarPos.y + 52, 0, 0, 32, 37);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, rightMenubarPos.x - 36,
                   rightMenubarPos.y + 89, 0, 0, 32, 37);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, rightMenubarPos.x - 36,
                   rightMenubarPos.y + 126, 0, 0, 32, 37);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, rightMenubarPos.x - 36,
                   rightMenubarPos.y + 163, 0, 0, 32, 37);
    CSurface::Draw(Surf_Map, global::bmpArray[BUTTON_GREEN1_DARK].surface, rightMenubarPos.x - 36,
                   rightMenubarPos.y + 200, 0, 0, 32, 37);
    // pictures
    // four cursor menu pictures
    CSurface::Draw(Surf_Map, global::bmpArray[CURSOR_SYMBOL_ARROW_UP].surface, rightMenubarPos - Extent(33, 237));
    CSurface::Draw(Surf_Map, global::bmpArray[CURSOR_SYMBOL_ARROW_DOWN].surface, rightMenubarPos - Extent(20, 235));
    CSurface::Draw(Surf_Map, global::bmpArray[CURSOR_SYMBOL_ARROW_DOWN].surface, rightMenubarPos - Extent(33, 220));
    CSurface::Draw(Surf_Map, global::bmpArray[CURSOR_SYMBOL_ARROW_UP].surface, rightMenubarPos - Extent(20, 220));
    // bugkill picture for quickload with text
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_BUGKILL].surface, rightMenubarPos + Position(-37, 162));
    CFont::writeText(Surf_Map, "Load", rightMenubarPos.x - 35, rightMenubarPos.y + 193);
    // bugkill picture for quicksave with text
    CSurface::Draw(Surf_Map, global::bmpArray[MENUBAR_BUGKILL].surface, rightMenubarPos + Position(-37, 200));
    CFont::writeText(Surf_Map, "Save", rightMenubarPos.x - 35, rightMenubarPos.y + 231);
}

static void getTriangleColor(TriangleTerrainType terrainType, MapType mapType, Sint16& r, Sint16& g, Sint16& b)
{
    switch(terrainType)
    {
        case TRIANGLE_TEXTURE_STEPPE_MEADOW1:
            r = (mapType == MAP_GREENLAND ? 100 : (mapType == MAP_WASTELAND ? 68 : 160));
            g = (mapType == MAP_GREENLAND ? 144 : (mapType == MAP_WASTELAND ? 72 : 172));
            b = (mapType == MAP_GREENLAND ? 20 : (mapType == MAP_WASTELAND ? 80 : 204));
            break;
        case TRIANGLE_TEXTURE_MINING1:
            r = (mapType == MAP_GREENLAND ? 156 : (mapType == MAP_WASTELAND ? 112 : 84));
            g = (mapType == MAP_GREENLAND ? 128 : (mapType == MAP_WASTELAND ? 108 : 88));
            b = (mapType == MAP_GREENLAND ? 88 : (mapType == MAP_WASTELAND ? 84 : 108));
            break;
        case TRIANGLE_TEXTURE_SNOW:
            r = (mapType == MAP_GREENLAND ? 180 : (mapType == MAP_WASTELAND ? 132 : 0));
            g = (mapType == MAP_GREENLAND ? 192 : (mapType == MAP_WASTELAND ? 0 : 48));
            b = (mapType == MAP_GREENLAND ? 200 : (mapType == MAP_WASTELAND ? 0 : 104));
            break;
        case TRIANGLE_TEXTURE_SWAMP:
            r = (mapType == MAP_GREENLAND ? 100 : 0);
            g = (mapType == MAP_GREENLAND ? 144 : (mapType == MAP_WASTELAND ? 24 : 40));
            b = (mapType == MAP_GREENLAND ? 20 : (mapType == MAP_WASTELAND ? 32 : 108));
            break;
        case TRIANGLE_TEXTURE_STEPPE:
            r = (mapType == MAP_GREENLAND ? 192 : (mapType == MAP_WASTELAND ? 156 : 0));
            g = (mapType == MAP_GREENLAND ? 156 : (mapType == MAP_WASTELAND ? 124 : 112));
            b = (mapType == MAP_GREENLAND ? 124 : (mapType == MAP_WASTELAND ? 100 : 176));
            break;
        case TRIANGLE_TEXTURE_WATER:
            r = (mapType == MAP_GREENLAND ? 16 : (mapType == MAP_WASTELAND ? 68 : 0));
            g = (mapType == MAP_GREENLAND ? 56 : (mapType == MAP_WASTELAND ? 68 : 48));
            b = (mapType == MAP_GREENLAND ? 164 : (mapType == MAP_WASTELAND ? 44 : 104));
            break;
        case TRIANGLE_TEXTURE_MEADOW1:
            r = (mapType == MAP_GREENLAND ? 72 : (mapType == MAP_WASTELAND ? 92 : 176));
            g = (mapType == MAP_GREENLAND ? 120 : (mapType == MAP_WASTELAND ? 88 : 164));
            b = (mapType == MAP_GREENLAND ? 12 : (mapType == MAP_WASTELAND ? 64 : 148));
            break;
        case TRIANGLE_TEXTURE_MEADOW2:
            r = (mapType == MAP_GREENLAND ? 100 : (mapType == MAP_WASTELAND ? 100 : 180));
            g = (mapType == MAP_GREENLAND ? 144 : (mapType == MAP_WASTELAND ? 96 : 184));
            b = (mapType == MAP_GREENLAND ? 20 : (mapType == MAP_WASTELAND ? 72 : 180));
            break;
        case TRIANGLE_TEXTURE_MEADOW3:
            r = (mapType == MAP_GREENLAND ? 64 : (mapType == MAP_WASTELAND ? 100 : 160));
            g = (mapType == MAP_GREENLAND ? 112 : (mapType == MAP_WASTELAND ? 96 : 172));
            b = (mapType == MAP_GREENLAND ? 8 : (mapType == MAP_WASTELAND ? 72 : 204));
            break;
        case TRIANGLE_TEXTURE_MINING2:
            r = (mapType == MAP_GREENLAND ? 156 : (mapType == MAP_WASTELAND ? 112 : 96));
            g = (mapType == MAP_GREENLAND ? 128 : (mapType == MAP_WASTELAND ? 100 : 96));
            b = (mapType == MAP_GREENLAND ? 88 : (mapType == MAP_WASTELAND ? 84 : 124));
            break;
        case TRIANGLE_TEXTURE_MINING3:
            r = (mapType == MAP_GREENLAND ? 156 : 104);
            g = (mapType == MAP_GREENLAND ? 128 : (mapType == MAP_WASTELAND ? 76 : 108));
            b = (mapType == MAP_GREENLAND ? 88 : (mapType == MAP_WASTELAND ? 36 : 140));
            break;
        case TRIANGLE_TEXTURE_MINING4:
            r = (mapType == MAP_GREENLAND ? 140 : 104);
            g = (mapType == MAP_GREENLAND ? 112 : (mapType == MAP_WASTELAND ? 76 : 108));
            b = (mapType == MAP_GREENLAND ? 72 : (mapType == MAP_WASTELAND ? 36 : 140));
            break;
        case TRIANGLE_TEXTURE_STEPPE_MEADOW2:
            r = (mapType == MAP_GREENLAND ? 136 : (mapType == MAP_WASTELAND ? 112 : 100));
            g = (mapType == MAP_GREENLAND ? 176 : (mapType == MAP_WASTELAND ? 108 : 144));
            b = (mapType == MAP_GREENLAND ? 40 : (mapType == MAP_WASTELAND ? 84 : 20));
            break;
        case TRIANGLE_TEXTURE_FLOWER:
            r = (mapType == MAP_GREENLAND ? 72 : (mapType == MAP_WASTELAND ? 68 : 124));
            g = (mapType == MAP_GREENLAND ? 120 : (mapType == MAP_WASTELAND ? 72 : 132));
            b = (mapType == MAP_GREENLAND ? 12 : (mapType == MAP_WASTELAND ? 80 : 172));
            break;
        case TRIANGLE_TEXTURE_LAVA:
            r = (mapType == MAP_GREENLAND ? 192 : (mapType == MAP_WASTELAND ? 128 : 144));
            g = (mapType == MAP_GREENLAND ? 32 : (mapType == MAP_WASTELAND ? 20 : 44));
            b = (mapType == MAP_GREENLAND ? 32 : (mapType == MAP_WASTELAND ? 0 : 4));
            break;
        case TRIANGLE_TEXTURE_MINING_MEADOW:
            r = (mapType == MAP_GREENLAND ? 156 : (mapType == MAP_WASTELAND ? 0 : 148));
            g = (mapType == MAP_GREENLAND ? 128 : (mapType == MAP_WASTELAND ? 24 : 160));
            b = (mapType == MAP_GREENLAND ? 88 : (mapType == MAP_WASTELAND ? 32 : 192));
            break;
        default: // color grey
            r = 128;
            g = 128;
            b = 128;
            break;
    }
}

void CMap::drawMinimap(SDL_Surface* Window)
{
    // this variables are needed to reduce the size of minimap-windows of big maps
    int num_x = (map->width > 256 ? map->width / 256 : 1);
    int num_y = (map->height > 256 ? map->height / 256 : 1);

    // make sure the minimap has the same proportions as the "real" map, so scale the same rate
    num_x = (num_x > num_y ? num_x : num_y);
    num_y = (num_x > num_y ? num_x : num_y);

    // if (Window->w < map->width || Window->h < map->height)
    // return;

    for(int y = 0; y < map->height; y++)
    {
        if(y % num_y != 0)
            continue;

        for(int x = 0; x < map->width; x++)
        {
            if(x % num_x != 0)
                continue;

            Sint16 r, g, b;
            getTriangleColor(TriangleTerrainType(map->getVertex(x, y).rsuTexture), map->type, r, g, b);

            Uint32* row = (Uint32*)Window->pixels + (y / num_y + 20) * Window->pitch / 4; //-V206
            //+6 because of the left window frame
            Uint32* pixel = row + x / num_x + 6;

            Sint32 vertexLighting = map->getVertex(x, y).i;
            r = ((r * vertexLighting) >> 16);
            g = ((g * vertexLighting) >> 16);
            b = ((b * vertexLighting) >> 16);
            const auto r8 = (Uint8)(r > 255 ? 255 : (r < 0 ? 0 : r));
            const auto g8 = (Uint8)(g > 255 ? 255 : (g < 0 ? 0 : g));
            const auto b8 = (Uint8)(b > 255 ? 255 : (b < 0 ? 0 : b));
            *pixel = ((r8 << Window->format->Rshift) + (g8 << Window->format->Gshift) + (b8 << Window->format->Bshift));
        }
    }

    // draw the player flags
    for(int i = 0; i < MAXPLAYERS; i++)
    {
        if(PlayerHQx[i] != 0xFFFF && PlayerHQy[i] != 0xFFFF)
        {
            // draw flag
            //%7 cause in the original game there are only 7 players and 7 different flags
            CSurface::Draw(Window, global::bmpArray[FLAG_BLUE_DARK + i % 7].surface,
                           6 + PlayerHQx[i] / num_x - global::bmpArray[FLAG_BLUE_DARK + i % 7].nx,
                           20 + PlayerHQy[i] / num_y - global::bmpArray[FLAG_BLUE_DARK + i % 7].ny);
            // write player number
            CFont::writeText(Window, std::to_string(i + 1), 6 + PlayerHQx[i] / num_x, 20 + PlayerHQy[i] / num_y,
                             FontSize::Small, FontColor::MintGreen);
        }
    }

    // draw the arrow --> 6px is width of left window frame and 20px is the height of the upper window frame
    CSurface::Draw(Window, global::bmpArray[MAPPIC_ARROWCROSS_ORANGE].surface,
                   6 + (displayRect.left + displayRect.getSize().x / 2) / TRIANGLE_WIDTH / num_x
                     - global::bmpArray[MAPPIC_ARROWCROSS_ORANGE].nx,
                   20 + (displayRect.top + displayRect.getSize().y / 2) / TRIANGLE_HEIGHT / num_y
                     - global::bmpArray[MAPPIC_ARROWCROSS_ORANGE].ny);
}

void CMap::modifyVertex()
{
    static Uint32 TimeOfLastModification = SDL_GetTicks();

    if((SDL_GetTicks() - TimeOfLastModification) < 5)
        return;
    else
        TimeOfLastModification = SDL_GetTicks();

    // save vertices for "undo"
    if(saveCurrentVertices)
    {
        // Cannot redo anymore
        redoBuffer.clear();
        undoBuffer.push_back(saveVertex(Vertex_, *map));
        saveCurrentVertices = false;
    }

    if(mode == EDITOR_MODE_HEIGHT_RAISE)
    {
        for(int i = 0; i < VertexCounter; i++)
            if(Vertices[i].active)
                modifyHeightRaise(Vertices[i].x, Vertices[i].y);
    } else if(mode == EDITOR_MODE_HEIGHT_REDUCE)
    {
        for(int i = 0; i < VertexCounter; i++)
            if(Vertices[i].active)
                modifyHeightReduce(Vertices[i].x, Vertices[i].y);
    } else if(mode == EDITOR_MODE_HEIGHT_PLANE)
    {
        // calculate height average over all vertices
        int h_sum = 0;
        int h_count = 0;
        Uint8 h_avg = 0x00;

        for(int i = 0; i < VertexCounter; i++)
        {
            if(Vertices[i].active)
            {
                h_sum += map->getVertex(Vertices[i]).h;
                h_count++;
            }
        }

        if(h_count > 0)
        {
            h_avg = h_sum / h_count;

            for(int i = 0; i < VertexCounter; i++)
                if(Vertices[i].active)
                    modifyHeightPlane(Vertices[i].x, Vertices[i].y, h_avg);
        }
    } else if(mode == EDITOR_MODE_HEIGHT_MAKE_BIG_HOUSE)
    {
        modifyHeightMakeBigHouse(Vertex_.x, Vertex_.y);
    } else if(mode == EDITOR_MODE_TEXTURE_MAKE_HARBOUR)
    {
        modifyHeightMakeBigHouse(Vertex_.x, Vertex_.y);
        modifyTextureMakeHarbour(Vertex_.x, Vertex_.y);
    }
    // at this time we need a modeContent to set
    else if(mode == EDITOR_MODE_CUT)
    {
        for(int i = 0; i < VertexCounter; i++)
        {
            if(Vertices[i].active)
            {
                modifyObject(Vertices[i].x, Vertices[i].y);
                modifyAnimal(Vertices[i].x, Vertices[i].y);
            }
        }
    } else if(mode == EDITOR_MODE_TEXTURE)
    {
        for(int i = 0; i < VertexCounter; i++)
            if(Vertices[i].active)
                modifyTexture(Vertices[i].x, Vertices[i].y, Vertices[i].fill_rsu, Vertices[i].fill_usd);
    } else if(mode == EDITOR_MODE_TREE)
    {
        for(int i = 0; i < VertexCounter; i++)
            if(Vertices[i].active)
                modifyObject(Vertices[i].x, Vertices[i].y);
    } else if(mode == EDITOR_MODE_LANDSCAPE)
    {
        for(int i = 0; i < VertexCounter; i++)
            if(Vertices[i].active)
                modifyObject(Vertices[i].x, Vertices[i].y);
    } else if(mode == EDITOR_MODE_RESOURCE_RAISE || mode == EDITOR_MODE_RESOURCE_REDUCE)
    {
        for(int i = 0; i < VertexCounter; i++)
            if(Vertices[i].active)
                modifyResource(Vertices[i].x, Vertices[i].y);
    } else if(mode == EDITOR_MODE_ANIMAL)
    {
        for(int i = 0; i < VertexCounter; i++)
            if(Vertices[i].active)
                modifyAnimal(Vertices[i].x, Vertices[i].y);
    } else if(mode == EDITOR_MODE_FLAG || mode == EDITOR_MODE_FLAG_DELETE)
    {
        modifyPlayer(Vertex_.x, Vertex_.y);
    }
}

void CMap::modifyHeightRaise(int VertexX, int VertexY)
{
    // vertex count for the points
    int X, Y;
    MapNode* tempP = &map->getVertex(VertexX, VertexY);
    // this is to setup the building depending on the vertices around
    std::array<Point32, 19> tempVertices;
    calculateVerticesAround(tempVertices, VertexX, VertexY);

    bool even = false;
    if(VertexY % 2 == 0)
        even = true;

    // DO IT
    if(tempP->z >= TRIANGLE_INCREASE * (MaxRaiseHeight - 0x0A)) // user specified maximum reached
        return;

    if(tempP->z >= TRIANGLE_INCREASE * (0x3C - 0x0A)) // maximum reached (0x3C is max)
        return;

    tempP->y -= TRIANGLE_INCREASE;
    tempP->z += TRIANGLE_INCREASE;
    tempP->h += 0x01;
    CSurface::update_shading(*map, VertexX, VertexY);

    // after (5*TRIANGLE_INCREASE) pixel all vertices around will be raised too
    // update first vertex left upside
    X = VertexX - (even ? 1 : 0);
    if(X < 0)
        X += map->width;
    Y = VertexY - 1;
    if(Y < 0)
        Y += map->height;
    // only modify if the other point is lower than the middle point of the hexagon (-5 cause point was raised a few
    // lines before)
    if(map->getVertex(X, Y).z < tempP->z - (5 * TRIANGLE_INCREASE)) //-V807
        modifyHeightRaise(X, Y);
    // update second vertex right upside
    X = VertexX + (even ? 0 : 1);
    if(X >= map->width)
        X -= map->width;
    Y = VertexY - 1;
    if(Y < 0)
        Y += map->height;
    // only modify if the other point is lower than the middle point of the hexagon (-5 cause point was raised a few
    // lines before)
    if(map->getVertex(X, Y).z < tempP->z - (5 * TRIANGLE_INCREASE))
        modifyHeightRaise(X, Y);
    // update third point bottom left
    X = VertexX - 1;
    if(X < 0)
        X += map->width;
    Y = VertexY;
    // only modify if the other point is lower than the middle point of the hexagon (-5 cause point was raised a few
    // lines before)
    if(map->getVertex(X, Y).z < tempP->z - (5 * TRIANGLE_INCREASE))
        modifyHeightRaise(X, Y);
    // update fourth point bottom right
    X = VertexX + 1;
    if(X >= map->width)
        X -= map->width;
    Y = VertexY;
    // only modify if the other point is lower than the middle point of the hexagon (-5 cause point was raised a few
    // lines before)
    if(map->getVertex(X, Y).z < tempP->z - (5 * TRIANGLE_INCREASE))
        modifyHeightRaise(X, Y);
    // update fifth point down left
    X = VertexX - (even ? 1 : 0);
    if(X < 0)
        X += map->width;
    Y = VertexY + 1;
    if(Y >= map->height)
        Y -= map->height;
    // only modify if the other point is lower than the middle point of the hexagon (-5 cause point was raised a few
    // lines before)
    if(map->getVertex(X, Y).z < tempP->z - (5 * TRIANGLE_INCREASE))
        modifyHeightRaise(X, Y);
    // update sixth point down right
    X = VertexX + (even ? 0 : 1);
    if(X >= map->width)
        X -= map->width;
    Y = VertexY + 1;
    if(Y >= map->height)
        Y -= map->height;
    // only modify if the other point is lower than the middle point of the hexagon (-5 cause point was raised a few
    // lines before)
    if(map->getVertex(X, Y).z < tempP->z - (5 * TRIANGLE_INCREASE))
        modifyHeightRaise(X, Y);

    // at least setup the possible building and shading at the vertex and 2 sections around
    for(int i = 0; i < 19; i++)
    {
        modifyBuild(tempVertices[i].x, tempVertices[i].y);
        modifyShading(tempVertices[i].x, tempVertices[i].y);
    }
}

void CMap::modifyHeightReduce(int VertexX, int VertexY)
{
    // vertex count for the points
    int X, Y;
    MapNode* tempP = &map->getVertex(VertexX, VertexY);
    // this is to setup the building depending on the vertices around
    std::array<Point32, 19> tempVertices;
    calculateVerticesAround(tempVertices, VertexX, VertexY);

    bool even = false;
    if(VertexY % 2 == 0)
        even = true;

    // DO IT
    if(tempP->z <= TRIANGLE_INCREASE * (MinReduceHeight - 0x0A)) // user specified minimum reached
        return;

    if(tempP->z <= TRIANGLE_INCREASE * (0x00 - 0x0A)) // minimum reached (0x00 is min)
        return;

    tempP->y += TRIANGLE_INCREASE;
    tempP->z -= TRIANGLE_INCREASE;
    tempP->h -= 0x01;
    CSurface::update_shading(*map, VertexX, VertexY);
    // after (5*TRIANGLE_INCREASE) pixel all vertices around will be reduced too
    // update first vertex left upside
    X = VertexX - (even ? 1 : 0);
    if(X < 0)
        X += map->width;
    Y = VertexY - 1;
    if(Y < 0)
        Y += map->height;
    // only modify if the other point is higher than the middle point of the hexagon (+5 cause point was reduced a few
    // lines before)
    if(map->getVertex(X, Y).z > tempP->z + (5 * TRIANGLE_INCREASE)) //-V807
        modifyHeightReduce(X, Y);
    // update second vertex right upside
    X = VertexX + (even ? 0 : 1);
    if(X >= map->width)
        X -= map->width;
    Y = VertexY - 1;
    if(Y < 0)
        Y += map->height;
    // only modify if the other point is higher than the middle point of the hexagon (+5 cause point was reduced a few
    // lines before)
    if(map->getVertex(X, Y).z > tempP->z + (5 * TRIANGLE_INCREASE))
        modifyHeightReduce(X, Y);
    // update third point bottom left
    X = VertexX - 1;
    if(X < 0)
        X += map->width;
    Y = VertexY;
    // only modify if the other point is higher than the middle point of the hexagon (+5 cause point was reduced a few
    // lines before)
    if(map->getVertex(X, Y).z > tempP->z + (5 * TRIANGLE_INCREASE))
        modifyHeightReduce(X, Y);
    // update fourth point bottom right
    X = VertexX + 1;
    if(X >= map->width)
        X -= map->width;
    Y = VertexY;
    // only modify if the other point is higher than the middle point of the hexagon (+5 cause point was reduced a few
    // lines before)
    if(map->getVertex(X, Y).z > tempP->z + (5 * TRIANGLE_INCREASE))
        modifyHeightReduce(X, Y);
    // update fifth point down left
    X = VertexX - (even ? 1 : 0);
    if(X < 0)
        X += map->width;
    Y = VertexY + 1;
    if(Y >= map->height)
        Y -= map->height;
    // only modify if the other point is higher than the middle point of the hexagon (+5 cause point was reduced a few
    // lines before)
    if(map->getVertex(X, Y).z > tempP->z + (5 * TRIANGLE_INCREASE))
        modifyHeightReduce(X, Y);
    // update sixth point down right
    X = VertexX + (even ? 0 : 1);
    if(X >= map->width)
        X -= map->width;
    Y = VertexY + 1;
    if(Y >= map->height)
        Y -= map->height;
    // only modify if the other point is higher than the middle point of the hexagon (+5 cause point was reduced a few
    // lines before)
    if(map->getVertex(X, Y).z > tempP->z + (5 * TRIANGLE_INCREASE))
        modifyHeightReduce(X, Y);

    // at least setup the possible building and shading at the vertex and 2 sections around
    for(int i = 0; i < 19; i++)
    {
        modifyBuild(tempVertices[i].x, tempVertices[i].y);
        modifyShading(tempVertices[i].x, tempVertices[i].y);
    }
}

void CMap::modifyHeightPlane(int VertexX, int VertexY, Uint8 h)
{
    // we could do "while" but "if" looks better during planing (optical effect)
    if(map->getVertex(VertexX, VertexY).h < h)
        modifyHeightRaise(VertexX, VertexY);
    // we could do "while" but "if" looks better during planing (optical effect)
    if(map->getVertex(VertexX, VertexY).h > h)
        modifyHeightReduce(VertexX, VertexY);
}

void CMap::modifyHeightMakeBigHouse(int VertexX, int VertexY)
{
    // at first save all vertices we need to calculate the new building
    std::array<Point32, 19> tempVertices;
    calculateVerticesAround(tempVertices, VertexX, VertexY);

    MapNode& middleVertex = map->getVertex(VertexX, VertexY);
    Uint8 height = middleVertex.h;

    // calculate the building using the height of the vertices

    // test the whole section
    for(int i = 0; i < 6; i++)
    {
        MapNode& vertex = map->getVertex(tempVertices[i]);
        for(int j = height - vertex.h; j >= 0x04; --j)
            modifyHeightRaise(tempVertices[i].x, tempVertices[i].y);

        for(int j = vertex.h - height; j >= 0x04; --j)
            modifyHeightReduce(tempVertices[i].x, tempVertices[i].y);
    }

    // test vertex lower right
    MapNode& vertex = map->getVertex(tempVertices[6]);
    for(int j = height - vertex.h; j >= 0x04; --j)
        modifyHeightRaise(tempVertices[6].x, tempVertices[6].y);

    for(int j = vertex.h - height; j >= 0x02; --j)
        modifyHeightReduce(tempVertices[6].x, tempVertices[6].y);

    // now test the second section around the vertex

    // test the whole section
    for(int i = 7; i < 19; i++)
    {
        MapNode& vertex = map->getVertex(tempVertices[i]);
        for(int j = height - vertex.h; j >= 0x03; --j)
            modifyHeightRaise(tempVertices[i].x, tempVertices[i].y);

        for(int j = vertex.h - height; j >= 0x03; --j)
            modifyHeightReduce(tempVertices[i].x, tempVertices[i].y);
    }

    // remove harbour if there is one
    if(middleVertex.rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1_HARBOUR
       || middleVertex.rsuTexture == TRIANGLE_TEXTURE_MEADOW1_HARBOUR
       || middleVertex.rsuTexture == TRIANGLE_TEXTURE_MEADOW2_HARBOUR
       || middleVertex.rsuTexture == TRIANGLE_TEXTURE_MEADOW3_HARBOUR
       || middleVertex.rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2_HARBOUR
       || middleVertex.rsuTexture == TRIANGLE_TEXTURE_FLOWER_HARBOUR
       || middleVertex.rsuTexture == TRIANGLE_TEXTURE_MINING_MEADOW_HARBOUR)
    {
        middleVertex.rsuTexture &= ~0x40;
    }
}

void CMap::modifyShading(int VertexX, int VertexY)
{
    // temporary to keep the lines short
    int X, Y;
    // this is to setup the shading depending on the vertices around (2 sections from the cursor)
    std::array<Point32, 19> tempVertices;
    calculateVerticesAround(tempVertices, VertexX, VertexY);
    MapNode& middleVertex = map->getVertex(VertexX, VertexY);

    // shading stakes
    int A, B, C, D, Result;

    // shading stake of point right upside (first section)
    X = tempVertices[2].x;
    Y = tempVertices[2].y;
    A = 9 * (map->getVertex(X, Y).h - middleVertex.h); //-V807
    // shading stake of point left (first section)
    X = tempVertices[3].x;
    Y = tempVertices[3].y;
    B = -6 * (map->getVertex(X, Y).h - middleVertex.h);
    // shading stake of point left (second section)
    X = tempVertices[12].x;
    Y = tempVertices[12].y;
    C = -3 * (map->getVertex(X, Y).h - middleVertex.h);
    // shading stake of point bottom/middle left (second section)
    X = tempVertices[14].x;
    Y = tempVertices[14].y;
    D = -9 * (map->getVertex(X, Y).h - middleVertex.h);

    Result = 0x40 + A + B + C + D;
    if(Result > 0x80)
        Result = 0x80;
    else if(Result < 0x00)
        Result = 0x00;

    middleVertex.shading = Result;
}

void CMap::modifyTexture(int VertexX, int VertexY, bool rsu, bool usd)
{
    if(modeContent == TRIANGLE_TEXTURE_MEADOW_MIXED || modeContent == TRIANGLE_TEXTURE_MEADOW_MIXED_HARBOUR)
    {
        int newContent = rand() % 3;
        if(newContent == 0)
        {
            if(modeContent == TRIANGLE_TEXTURE_MEADOW_MIXED)
                newContent = TRIANGLE_TEXTURE_MEADOW1;
            else
                newContent = TRIANGLE_TEXTURE_MEADOW1_HARBOUR;
        } else if(newContent == 1)
        {
            if(modeContent == TRIANGLE_TEXTURE_MEADOW_MIXED)
                newContent = TRIANGLE_TEXTURE_MEADOW2;
            else
                newContent = TRIANGLE_TEXTURE_MEADOW2_HARBOUR;
        } else
        {
            if(modeContent == TRIANGLE_TEXTURE_MEADOW_MIXED)
                newContent = TRIANGLE_TEXTURE_MEADOW3;
            else
                newContent = TRIANGLE_TEXTURE_MEADOW3_HARBOUR;
        }
        if(rsu)
            map->getVertex(VertexX, VertexY).rsuTexture = newContent;
        if(usd)
            map->getVertex(VertexX, VertexY).usdTexture = newContent;
    } else
    {
        if(rsu)
            map->getVertex(VertexX, VertexY).rsuTexture = modeContent;
        if(usd)
            map->getVertex(VertexX, VertexY).usdTexture = modeContent;
    }

    // at least setup the possible building and the resources at the vertex and 1 section/2 sections around
    std::array<Point32, 19> tempVertices;
    calculateVerticesAround(tempVertices, VertexX, VertexY);
    for(int i = 0; i < 19; i++)
    {
        if(i < 7)
            modifyBuild(tempVertices[i].x, tempVertices[i].y);
        modifyResource(tempVertices[i].x, tempVertices[i].y);
    }
}

void CMap::modifyTextureMakeHarbour(int VertexX, int VertexY)
{
    MapNode& vertex = map->getVertex(VertexX, VertexY);
    if(vertex.rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1 || vertex.rsuTexture == TRIANGLE_TEXTURE_MEADOW1
       || vertex.rsuTexture == TRIANGLE_TEXTURE_MEADOW2 || vertex.rsuTexture == TRIANGLE_TEXTURE_MEADOW3
       || vertex.rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2 || vertex.rsuTexture == TRIANGLE_TEXTURE_FLOWER
       || vertex.rsuTexture == TRIANGLE_TEXTURE_MINING_MEADOW)
    {
        vertex.rsuTexture |= 0x40;
    }
}

void CMap::modifyObject(int x, int y)
{
    MapNode& curVertex = map->getVertex(x, y);
    if(mode == EDITOR_MODE_CUT)
    {
        // prevent cutting a player position
        if(curVertex.objectInfo != 0x80)
        {
            curVertex.objectType = 0x00;
            curVertex.objectInfo = 0x00;
        }
    } else if(mode == EDITOR_MODE_TREE)
    {
        // if there is another object at the vertex, return
        if(curVertex.objectInfo != 0x00)
            return;
        if(modeContent == 0xFF)
        {
            // mixed wood
            if(modeContent2 == 0xC4)
            {
                int newContent = rand() % 3;
                if(newContent == 0)
                    newContent = 0x30;
                else if(newContent == 1)
                    newContent = 0x70;
                else
                    newContent = 0xB0;
                // we set different start pictures for the tree, cause the trees should move different, so we add a
                // random value that walks from 0 to 7
                curVertex.objectType = newContent + rand() % 8;
                curVertex.objectInfo = modeContent2;
            }
            // mixed palm
            else // if (modeContent2 == 0xC5)
            {
                int newContent = rand() % 2;
                int newContent2;
                if(newContent == 0)
                {
                    newContent = 0x30;
                    newContent2 = 0xC5;
                } else
                {
                    newContent = 0xF0;
                    newContent2 = 0xC4;
                }
                // we set different start pictures for the tree, cause the trees should move different, so we add a
                // random value that walks from 0 to 7
                curVertex.objectType = newContent + rand() % 8;
                curVertex.objectInfo = newContent2;
            }
        } else
        {
            // we set different start pictures for the tree, cause the trees should move different, so we add a random
            // value that walks from 0 to 7
            curVertex.objectType = modeContent + rand() % 8;
            curVertex.objectInfo = modeContent2;
        }
    } else if(mode == EDITOR_MODE_LANDSCAPE)
    {
        // if there is another object at the vertex, return
        if(curVertex.objectInfo != 0x00)
            return;

        if(modeContent == 0x01)
        {
            int newContent = modeContent + rand() % 6;
            int newContent2 = 0xCC + rand() % 2;

            curVertex.objectType = newContent;
            curVertex.objectInfo = newContent2;

            // now set up the buildings around the granite
            modifyBuild(x, y);
        } else if(modeContent == 0x05)
        {
            int newContent = modeContent + rand() % 2;

            curVertex.objectType = newContent;
            curVertex.objectInfo = modeContent2;
        } else if(modeContent == 0x02)
        {
            int newContent = modeContent + rand() % 3;

            curVertex.objectType = newContent;
            curVertex.objectInfo = modeContent2;
        } else if(modeContent == 0x0C)
        {
            int newContent = modeContent + rand() % 2;

            curVertex.objectType = newContent;
            curVertex.objectInfo = modeContent2;
        } else if(modeContent == 0x25)
        {
            int newContent = modeContent + rand() % 3;

            curVertex.objectType = newContent;
            curVertex.objectInfo = modeContent2;
        } else if(modeContent == 0x10)
        {
            int newContent = rand() % 4;
            if(newContent == 0)
                newContent = 0x10;
            else if(newContent == 1)
                newContent = 0x11;
            else if(newContent == 2)
                newContent = 0x12;
            else
                newContent = 0x0A;

            curVertex.objectType = newContent;
            curVertex.objectInfo = modeContent2;
        } else if(modeContent == 0x0E)
        {
            int newContent = rand() % 4;
            if(newContent == 0)
                newContent = 0x0E;
            else if(newContent == 1)
                newContent = 0x0F;
            else if(newContent == 2)
                newContent = 0x13;
            else
                newContent = 0x14;

            curVertex.objectType = newContent;
            curVertex.objectInfo = modeContent2;
        } else if(modeContent == 0x07)
        {
            int newContent = modeContent + rand() % 2;

            curVertex.objectType = newContent;
            curVertex.objectInfo = modeContent2;
        } else if(modeContent == 0x00)
        {
            int newContent = rand() % 3;
            if(newContent == 0)
                newContent = 0x00;
            else if(newContent == 1)
                newContent = 0x01;
            else
                newContent = 0x22;

            curVertex.objectType = newContent;
            curVertex.objectInfo = modeContent2;
        } else if(modeContent == 0x18)
        {
            int newContent = modeContent + rand() % 7;

            curVertex.objectType = newContent;
            curVertex.objectInfo = modeContent2;
        } else if(modeContent == 0x09)
        {
            curVertex.objectType = modeContent;
            curVertex.objectInfo = modeContent2;
        }
    }
    // at least setup the possible building at the vertex and 1 section around
    std::array<Point32, 7> tempVertices;
    calculateVerticesAround(tempVertices, x, y);
    for(int i = 0; i < 7; i++)
        modifyBuild(tempVertices[i].x, tempVertices[i].y);
}

void CMap::modifyAnimal(int VertexX, int VertexY)
{
    if(mode == EDITOR_MODE_CUT)
    {
        map->getVertex(VertexX, VertexY).animal = 0x00;
    } else if(mode == EDITOR_MODE_ANIMAL)
    {
        // if there is another object at the vertex, return
        if(map->getVertex(VertexX, VertexY).animal != 0x00)
            return;

        if(modeContent > 0x00 && modeContent <= 0x06)
            map->getVertex(VertexX, VertexY).animal = modeContent;
    }
}

void CMap::modifyBuild(int x, int y)
{
    // at first save all vertices we need to calculate the new building
    std::array<Point32, 19> tempVertices;
    calculateVerticesAround(tempVertices, x, y);

    /// evtl. keine festen werte sondern addition und subtraktion wegen originalkompatibilitaet (bei baeumen bspw. keine
    /// 0x00 sondern 0x68)

    Uint8 building;
    MapNode& curVertex = map->getVertex(x, y);
    const Uint8 height = curVertex.h;
    std::array<const MapNode*, 7> mapVertices;
    for(unsigned i = 0; i < mapVertices.size(); i++)
        mapVertices[i] = &map->getVertex(tempVertices[i]);

    // calculate the building using the height of the vertices
    // this building is a mine
    if(curVertex.rsuTexture == TRIANGLE_TEXTURE_MINING1 || curVertex.rsuTexture == TRIANGLE_TEXTURE_MINING2
       || curVertex.rsuTexture == TRIANGLE_TEXTURE_MINING3 || curVertex.rsuTexture == TRIANGLE_TEXTURE_MINING4)
    {
        building = 0x05;
        // test vertex lower right
        const auto tmpHeight = mapVertices[6]->h;
        if(tmpHeight - height >= 0x04)
            building = 0x01;
    }
    // not a mine
    else
    {
        building = 0x04;
        // test the whole section
        for(int i = 0; i < 6; i++)
        {
            auto tmpHeight = mapVertices[i]->h;
            if(height - tmpHeight >= 0x04 || tmpHeight - height >= 0x04)
                building = 0x01;
        }

        // test vertex lower right
        auto tmpHeight = mapVertices[6]->h;
        if(height - tmpHeight >= 0x04 || tmpHeight - height >= 0x02)
            building = 0x01;

        // now test the second section around the vertex
        if(building > 0x02)
        {
            // test the whole section
            for(int i = 7; i < 19; i++)
            {
                tmpHeight = map->getVertex(tempVertices[i]).h;
                if(height - tmpHeight >= 0x03 || tmpHeight - height >= 0x03)
                    building = 0x02;
            }
        }
    }

    // test if there is an object AROUND the vertex (trees or granite)
    if(building > 0x01)
    {
        for(int i = 1; i < 7; i++)
        {
            const MapNode& vertexI = *mapVertices[i];
            if(vertexI.objectInfo == 0xC4    // tree
               || vertexI.objectInfo == 0xC5 // tree
               || vertexI.objectInfo == 0xC6 // tree
            )
            {
                // if lower right
                if(i == 6)
                {
                    building = 0x01;
                    break;
                } else
                    building = 0x02;
            } else if(vertexI.objectInfo == 0xCC    // granite
                      || vertexI.objectInfo == 0xCD // granite
            )
            {
                building = 0x01;
                break;
            }
        }
    }

    // test if there is an object AT the vertex (trees or granite)
    if(building > 0x00)
    {
        if(curVertex.objectInfo == 0xC4    // tree
           || curVertex.objectInfo == 0xC5 // tree
           || curVertex.objectInfo == 0xC6 // tree
           || curVertex.objectInfo == 0xCC // granite
           || curVertex.objectInfo == 0xCD // granite
        )
        {
            building = 0x00;
        }
    }

    // test if there is snow or lava at the vertex or around the vertex and touching the vertex (first section)
    if(building > 0x00)
    {
        if(mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_SNOW || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_SNOW
           || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_LAVA || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_LAVA
           || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_SNOW || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_SNOW
           || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_LAVA || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_LAVA
           || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_SNOW || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_LAVA
           || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_SNOW
           || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_LAVA)
        {
            building = 0x00;
        }
    }

    // test if there is snow or lava on the right side (RSU), in lower left (USD) or in lower right (first section)
    if(building > 0x01)
    {
        if(mapVertices[4]->rsuTexture == TRIANGLE_TEXTURE_SNOW || mapVertices[4]->rsuTexture == TRIANGLE_TEXTURE_LAVA
           || mapVertices[5]->usdTexture == TRIANGLE_TEXTURE_SNOW || mapVertices[5]->usdTexture == TRIANGLE_TEXTURE_LAVA
           || mapVertices[6]->rsuTexture == TRIANGLE_TEXTURE_SNOW || mapVertices[6]->usdTexture == TRIANGLE_TEXTURE_SNOW
           || mapVertices[6]->rsuTexture == TRIANGLE_TEXTURE_LAVA
           || mapVertices[6]->usdTexture == TRIANGLE_TEXTURE_LAVA)
        {
            building = 0x01;
        }
    }

    // test if vertex is surrounded by water or swamp
    if(building > 0x00)
    {
        if((mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_WATER
            || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_SWAMP)
           && (mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_WATER
               || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_SWAMP)
           && (mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_WATER
               || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_SWAMP)
           && (mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_WATER
               || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_SWAMP)
           && (mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_WATER
               || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_SWAMP)
           && (mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_WATER
               || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_SWAMP))
        {
            building = 0x00;
        } else if((mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_WATER
                   || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_SWAMP)
                  || (mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_WATER
                      || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_SWAMP)
                  || (mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_WATER
                      || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_SWAMP)
                  || (mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_WATER
                      || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_SWAMP)
                  || (mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_WATER
                      || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_SWAMP)
                  || (mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_WATER
                      || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_SWAMP))
        {
            building = 0x01;
        }
    }

    // test if there is steppe at the vertex or touching the vertex
    if(building > 0x01)
    {
        if(mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_STEPPE
           || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_STEPPE
           || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_STEPPE
           || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_STEPPE
           || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_STEPPE
           || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_STEPPE)
        {
            building = 0x01;
        }
    }

    // test if vertex is surrounded by mining-textures
    if(building > 0x01)
    {
        if((mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_MINING1
            || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_MINING2
            || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_MINING3
            || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_MINING4)
           && (mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_MINING1
               || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_MINING2
               || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_MINING3
               || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_MINING4)
           && (mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_MINING1
               || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_MINING2
               || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_MINING3
               || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_MINING4)
           && (mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_MINING1
               || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_MINING2
               || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_MINING3
               || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_MINING4)
           && (mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_MINING1
               || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_MINING2
               || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_MINING3
               || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_MINING4)
           && (mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_MINING1
               || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_MINING2
               || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_MINING3
               || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_MINING4))
        {
            building = 0x05;
        } else if((mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_MINING1
                   || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_MINING2
                   || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_MINING3
                   || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_MINING4)
                  || (mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_MINING1
                      || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_MINING2
                      || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_MINING3
                      || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_MINING4)
                  || (mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_MINING1
                      || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_MINING2
                      || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_MINING3
                      || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_MINING4)
                  || (mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_MINING1
                      || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_MINING2
                      || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_MINING3
                      || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_MINING4)
                  || (mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_MINING1
                      || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_MINING2
                      || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_MINING3
                      || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_MINING4)
                  || (mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_MINING1
                      || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_MINING2
                      || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_MINING3
                      || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_MINING4))
        {
            building = 0x01;
        }
    }

    // test for headquarters around the point
    // NOTE: In EDITORMODE don't test AT the point, cause in Original game we need a big house AT the point, otherwise
    // the game wouldn't set a player there
    if(building > 0x00)
    {
        for(int i = 1; i < 7; i++)
        {
            if(map->getVertex(tempVertices[i]).objectInfo == 0x80)
                building = 0x00;
        }
    }

    // test for headquarters around (second section)
    if(building > 0x01)
    {
        for(int i = 7; i < 19; i++)
        {
            if(map->getVertex(tempVertices[i]).objectInfo == 0x80)
            {
                if(i == 15 || i == 17 || i == 18)
                    building = 0x01;
                else
                {
                    // make middle house, but only if it's not a mine
                    if(building > 0x03 && building < 0x05)
                        building = 0x03;
                }
            }
        }
    }

    // Some additional information for "ingame"-building-calculation:
    // There is no difference between small, middle and big houses. If you set a small house on a vertex, the
    // buildings around will change like this where a middle or a big house.
    // Only a flag has another algorithm.
    //--Flagge einfuegen!!!

    curVertex.build = building;
}

void CMap::modifyResource(int x, int y)
{
    // at first save all vertices we need to check
    std::array<Point32, 19> tempVertices;
    calculateVerticesAround(tempVertices, x, y);
    MapNode& curVertex = map->getVertex(x, y);
    std::array<const MapNode*, 7> mapVertices;
    for(unsigned i = 0; i < mapVertices.size(); i++)
        mapVertices[i] = &map->getVertex(tempVertices[i]);

    // SPECIAL CASE: test if we should set water only
    // test if vertex is surrounded by meadow and meadow-like textures
    if((mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1
        || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1_HARBOUR
        || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_MEADOW1
        || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_MEADOW1_HARBOUR
        || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_MEADOW2
        || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_MEADOW2_HARBOUR
        || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_MEADOW3
        || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_MEADOW3_HARBOUR
        || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2
        || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2_HARBOUR
        || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_FLOWER
        || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_FLOWER_HARBOUR
        || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_MINING_MEADOW
        || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_MINING_MEADOW_HARBOUR)
       && (mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1
           || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1_HARBOUR
           || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_MEADOW1
           || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_MEADOW1_HARBOUR
           || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_MEADOW2
           || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_MEADOW2_HARBOUR
           || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_MEADOW3
           || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_MEADOW3_HARBOUR
           || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2
           || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2_HARBOUR
           || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_FLOWER
           || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_FLOWER_HARBOUR
           || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_MINING_MEADOW
           || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_MINING_MEADOW_HARBOUR)
       && (mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1
           || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1_HARBOUR
           || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_MEADOW1
           || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_MEADOW1_HARBOUR
           || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_MEADOW2
           || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_MEADOW2_HARBOUR
           || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_MEADOW3
           || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_MEADOW3_HARBOUR
           || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2
           || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2_HARBOUR
           || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_FLOWER
           || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_FLOWER_HARBOUR
           || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_MINING_MEADOW
           || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_MINING_MEADOW_HARBOUR)
       && (mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1
           || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1_HARBOUR
           || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_MEADOW1
           || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_MEADOW1_HARBOUR
           || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_MEADOW2
           || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_MEADOW2_HARBOUR
           || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_MEADOW3
           || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_MEADOW3_HARBOUR
           || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2
           || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2_HARBOUR
           || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_FLOWER
           || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_FLOWER_HARBOUR
           || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_MINING_MEADOW
           || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_MINING_MEADOW_HARBOUR)
       && (mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1
           || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1_HARBOUR
           || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_MEADOW1
           || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_MEADOW1_HARBOUR
           || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_MEADOW2
           || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_MEADOW2_HARBOUR
           || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_MEADOW3
           || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_MEADOW3_HARBOUR
           || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2
           || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2_HARBOUR
           || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_FLOWER
           || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_FLOWER_HARBOUR
           || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_MINING_MEADOW
           || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_MINING_MEADOW_HARBOUR)
       && (mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1
           || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW1_HARBOUR
           || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_MEADOW1
           || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_MEADOW1_HARBOUR
           || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_MEADOW2
           || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_MEADOW2_HARBOUR
           || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_MEADOW3
           || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_MEADOW3_HARBOUR
           || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2
           || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_STEPPE_MEADOW2_HARBOUR
           || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_FLOWER
           || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_FLOWER_HARBOUR
           || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_MINING_MEADOW
           || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_MINING_MEADOW_HARBOUR))
    {
        curVertex.resource = 0x21;
    }
    // SPECIAL CASE: test if we should set fishes only
    // test if vertex is surrounded by water (first section) and at least one non-water texture in the second section
    else if((mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_WATER)
            && (mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_WATER)
            && (mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_WATER)
            && (mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_WATER)
            && (mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_WATER)
            && (mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_WATER)
            && (mapVertices[2]->usdTexture != TRIANGLE_TEXTURE_WATER
                || mapVertices[3]->rsuTexture != TRIANGLE_TEXTURE_WATER
                || mapVertices[4]->rsuTexture != TRIANGLE_TEXTURE_WATER
                || mapVertices[4]->usdTexture != TRIANGLE_TEXTURE_WATER
                || mapVertices[5]->rsuTexture != TRIANGLE_TEXTURE_WATER
                || mapVertices[5]->usdTexture != TRIANGLE_TEXTURE_WATER
                || mapVertices[6]->rsuTexture != TRIANGLE_TEXTURE_WATER
                || mapVertices[6]->usdTexture != TRIANGLE_TEXTURE_WATER
                || map->getVertex(tempVertices[7]).rsuTexture != TRIANGLE_TEXTURE_WATER
                || map->getVertex(tempVertices[7]).usdTexture != TRIANGLE_TEXTURE_WATER
                || map->getVertex(tempVertices[8]).rsuTexture != TRIANGLE_TEXTURE_WATER
                || map->getVertex(tempVertices[8]).usdTexture != TRIANGLE_TEXTURE_WATER
                || map->getVertex(tempVertices[9]).rsuTexture != TRIANGLE_TEXTURE_WATER
                || map->getVertex(tempVertices[10]).rsuTexture != TRIANGLE_TEXTURE_WATER
                || map->getVertex(tempVertices[10]).usdTexture != TRIANGLE_TEXTURE_WATER
                || map->getVertex(tempVertices[11]).rsuTexture != TRIANGLE_TEXTURE_WATER
                || map->getVertex(tempVertices[12]).usdTexture != TRIANGLE_TEXTURE_WATER
                || map->getVertex(tempVertices[14]).usdTexture != TRIANGLE_TEXTURE_WATER))
    {
        curVertex.resource = 0x87;
    }
    // test if vertex is surrounded by mining textures
    else if((mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_MINING1
             || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_MINING2
             || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_MINING3
             || mapVertices[0]->rsuTexture == TRIANGLE_TEXTURE_MINING4)
            && (mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_MINING1
                || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_MINING2
                || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_MINING3
                || mapVertices[0]->usdTexture == TRIANGLE_TEXTURE_MINING4)
            && (mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_MINING1
                || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_MINING2
                || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_MINING3
                || mapVertices[1]->rsuTexture == TRIANGLE_TEXTURE_MINING4)
            && (mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_MINING1
                || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_MINING2
                || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_MINING3
                || mapVertices[1]->usdTexture == TRIANGLE_TEXTURE_MINING4)
            && (mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_MINING1
                || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_MINING2
                || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_MINING3
                || mapVertices[2]->rsuTexture == TRIANGLE_TEXTURE_MINING4)
            && (mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_MINING1
                || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_MINING2
                || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_MINING3
                || mapVertices[3]->usdTexture == TRIANGLE_TEXTURE_MINING4))
    {
        // check which resource to set
        if(mode == EDITOR_MODE_RESOURCE_RAISE)
        {
            // if there is no or another resource at the moment
            if(curVertex.resource == 0x40 || curVertex.resource < modeContent || curVertex.resource > modeContent + 6)
            {
                curVertex.resource = modeContent;
            } else if(curVertex.resource >= modeContent && curVertex.resource <= modeContent + 6)
            {
                // maximum not reached?
                if(curVertex.resource != modeContent + 6)
                    curVertex.resource++;
            }
        } else if(mode == EDITOR_MODE_RESOURCE_REDUCE)
        {
            // minimum not reached?
            if(curVertex.resource != 0x40)
            {
                curVertex.resource--;
                // minimum now reached? if so, set it to 0x40
                if(curVertex.resource == 0x48 || curVertex.resource == 0x50 || curVertex.resource == 0x58
                   // in case of coal we already have a 0x40, so don't check this
                )
                    curVertex.resource = 0x40;
            }
        } else if(curVertex.resource == 0x00)
            curVertex.resource = 0x40;
    } else
        curVertex.resource = 0x00;
}

void CMap::modifyPlayer(int VertexX, int VertexY)
{
    // if we have repositioned a player, we need the old position to recalculate the buildings there
    bool PlayerRePositioned = false;
    int oldPositionX = 0;
    int oldPositionY = 0;
    MapNode& vertex = map->getVertex(VertexX, VertexY);

    // set player position
    if(mode == EDITOR_MODE_FLAG)
    {
        // only allowed on big houses (0x04) --> but in cheat mode within the game also small houses (0x02) are allowed
        if(vertex.build % 8 == 0x04 && vertex.objectInfo != 0x80)
        {
            vertex.objectType = modeContent;
            vertex.objectInfo = 0x80;

            // for compatibility with original settlers 2 we write the headquarters positions to the map header (for the
            // first 7 players)
            if(modeContent >= 0 && modeContent < 7)
            {
                map->HQx[modeContent] = VertexX;
                map->HQy[modeContent] = VertexY;
            }

            // save old position if exists
            if(PlayerHQx[modeContent] != 0xFFFF && PlayerHQy[modeContent] != 0xFFFF)
            {
                oldPositionX = PlayerHQx[modeContent];
                oldPositionY = PlayerHQy[modeContent];
                map->getVertex(oldPositionX, oldPositionY).objectType = 0x00;
                map->getVertex(oldPositionX, oldPositionY).objectInfo = 0x00;
                PlayerRePositioned = true;
            }
            PlayerHQx[modeContent] = VertexX;
            PlayerHQy[modeContent] = VertexY;

            // setup number of players in map header
            if(!PlayerRePositioned)
                map->player++;
        }
    }
    // delete player position
    else if(mode == EDITOR_MODE_FLAG_DELETE)
    {
        if(vertex.objectInfo == 0x80)
        {
            // at first delete the player position using the number of the player as saved in objectType
            if(vertex.objectType < MAXPLAYERS)
            {
                PlayerHQx[vertex.objectType] = 0xFFFF;
                PlayerHQy[vertex.objectType] = 0xFFFF;

                // for compatibility with original settlers 2 we write the headquarters positions to the map header (for
                // the first 7 players)
                if(vertex.objectType < 7)
                {
                    map->HQx[vertex.objectType] = 0xFFFF;
                    map->HQy[vertex.objectType] = 0xFFFF;
                }
            }

            vertex.objectType = 0x00;
            vertex.objectInfo = 0x00;

            // setup number of players in map header
            map->player--;
        }
    }

    // at least setup the possible building at the vertex and 2 sections around
    std::array<Point32, 19> tempVertices;
    calculateVerticesAround(tempVertices, VertexX, VertexY);
    for(int i = 0; i < 19; i++)
        modifyBuild(tempVertices[i].x, tempVertices[i].y);

    if(PlayerRePositioned)
    {
        calculateVerticesAround(tempVertices, oldPositionX, oldPositionY);
        for(int i = 0; i < 19; i++)
            modifyBuild(tempVertices[i].x, tempVertices[i].y);
    }
}

void CMap::calculateVertices()
{
    const bool even = Vertex_.y % 2 == 0;

    int index = 0;
    for(int i = -MAX_CHANGE_SECTION; i <= MAX_CHANGE_SECTION; i++)
    {
        if(abs(i) % 2 == 0)
        {
            for(int j = -MAX_CHANGE_SECTION; j <= MAX_CHANGE_SECTION; j++, index++)
            {
                static_cast<Position&>(Vertices[index]) = Vertex_ + Position(j, i);
                if(Vertices[index].x < 0)
                    Vertices[index].x += map->width;
                else if(Vertices[index].x >= map->width)
                    Vertices[index].x -= map->width;
                if(Vertices[index].y < 0)
                    Vertices[index].y += map->height;
                else if(Vertices[index].y >= map->height)
                    Vertices[index].y -= map->height;
                Vertices[index].blit = correctMouseBlit(Vertices[index]);
            }
        } else
        {
            for(int j = -MAX_CHANGE_SECTION; j <= MAX_CHANGE_SECTION - 1; j++, index++)
            {
                static_cast<Position&>(Vertices[index]) = Vertex_ + Position(even ? j : j + 1, i);
                if(Vertices[index].x < 0)
                    Vertices[index].x += map->width;
                else if(Vertices[index].x >= map->width)
                    Vertices[index].x -= map->width;
                if(Vertices[index].y < 0)
                    Vertices[index].y += map->height;
                else if(Vertices[index].y >= map->height)
                    Vertices[index].y -= map->height;
                Vertices[index].blit = correctMouseBlit(Vertices[index]);
            }
        }
    }
    // check if cursor vertices should change randomly
    if(VertexActivityRandom || VertexFillRandom)
        setupVerticesActivity();
}

template<size_t T_size>
void CMap::calculateVerticesAround(std::array<Point32, T_size>& newVertices, int x, int y)
{
    static_assert(T_size == 1u || T_size == 7u || T_size == 19u, "Only 1, 7 or 19 are allowed");
    bool even = false;
    if(y % 2 == 0)
        even = true;

    newVertices[0].x = x;
    newVertices[0].y = y;

    if(T_size >= 7u)
    {
        newVertices[1].x = x - (even ? 1 : 0);
        if(newVertices[1].x < 0)
            newVertices[1].x += map->width;
        newVertices[1].y = y - 1;
        if(newVertices[1].y < 0)
            newVertices[1].y += map->height;
        newVertices[2].x = x + (even ? 0 : 1);
        if(newVertices[2].x >= map->width)
            newVertices[2].x -= map->width;
        newVertices[2].y = y - 1;
        if(newVertices[2].y < 0)
            newVertices[2].y += map->height;
        newVertices[3].x = x - 1;
        if(newVertices[3].x < 0)
            newVertices[3].x += map->width;
        newVertices[3].y = y;
        newVertices[4].x = x + 1;
        if(newVertices[4].x >= map->width)
            newVertices[4].x -= map->width;
        newVertices[4].y = y;
        newVertices[5].x = x - (even ? 1 : 0);
        if(newVertices[5].x < 0)
            newVertices[5].x += map->width;
        newVertices[5].y = y + 1;
        if(newVertices[5].y >= map->height)
            newVertices[5].y -= map->height;
        newVertices[6].x = x + (even ? 0 : 1);
        if(newVertices[6].x >= map->width)
            newVertices[6].x -= map->width;
        newVertices[6].y = y + 1;
        if(newVertices[6].y >= map->height)
            newVertices[6].y -= map->height;
    }
    if(T_size >= 19)
    {
        newVertices[7].x = x - 1;
        if(newVertices[7].x < 0)
            newVertices[7].x += map->width;
        newVertices[7].y = y - 2;
        if(newVertices[7].y < 0)
            newVertices[7].y += map->height;
        newVertices[8].x = x;
        newVertices[8].y = y - 2;
        if(newVertices[8].y < 0)
            newVertices[8].y += map->height;
        newVertices[9].x = x + 1;
        if(newVertices[9].x >= map->width)
            newVertices[9].x -= map->width;
        newVertices[9].y = y - 2;
        if(newVertices[9].y < 0)
            newVertices[9].y += map->height;
        newVertices[10].x = x - (even ? 2 : 1);
        if(newVertices[10].x < 0)
            newVertices[10].x += map->width;
        newVertices[10].y = y - 1;
        if(newVertices[10].y < 0)
            newVertices[10].y += map->height;
        newVertices[11].x = x + (even ? 1 : 2);
        if(newVertices[11].x >= map->width)
            newVertices[11].x -= map->width;
        newVertices[11].y = y - 1;
        if(newVertices[11].y < 0)
            newVertices[11].y += map->height;
        newVertices[12].x = x - 2;
        if(newVertices[12].x < 0)
            newVertices[12].x += map->width;
        newVertices[12].y = y;
        newVertices[13].x = x + 2;
        if(newVertices[13].x >= map->width)
            newVertices[13].x -= map->width;
        newVertices[13].y = y;
        newVertices[14].x = x - (even ? 2 : 1);
        if(newVertices[14].x < 0)
            newVertices[14].x += map->width;
        newVertices[14].y = y + 1;
        if(newVertices[14].y >= map->height)
            newVertices[14].y -= map->height;
        newVertices[15].x = x + (even ? 1 : 2);
        if(newVertices[15].x >= map->width)
            newVertices[15].x -= map->width;
        newVertices[15].y = y + 1;
        if(newVertices[15].y >= map->height)
            newVertices[15].y -= map->height;
        newVertices[16].x = x - 1;
        if(newVertices[16].x < 0)
            newVertices[16].x += map->width;
        newVertices[16].y = y + 2;
        if(newVertices[16].y >= map->height)
            newVertices[16].y -= map->height;
        newVertices[17].x = x;
        newVertices[17].y = y + 2;
        if(newVertices[17].y >= map->height)
            newVertices[17].y -= map->height;
        newVertices[18].x = x + 1;
        if(newVertices[18].x >= map->width)
            newVertices[18].x -= map->width;
        newVertices[18].y = y + 2;
        if(newVertices[18].y >= map->height)
            newVertices[18].y -= map->height;
    }
}

void CMap::setupVerticesActivity()
{
    int index = 0;
    for(int i = -MAX_CHANGE_SECTION; i <= MAX_CHANGE_SECTION; i++)
    {
        if(abs(i) % 2 == 0)
        {
            for(int j = -MAX_CHANGE_SECTION; j <= MAX_CHANGE_SECTION; j++, index++)
            {
                if(abs(i) <= ChangeSection_ && abs(j) <= ChangeSection_ - (ChangeSectionHexagonMode ? abs(i / 2) : 0))
                {
                    // check if cursor vertices should change randomly
                    if(VertexActivityRandom)
                        Vertices[index].active = (rand() % 2 == 1);
                    else
                        Vertices[index].active = true;

                    // decide which triangle-textures will be filled at this vertex (necessary for border)
                    Vertices[index].fill_rsu = VertexFillRSU || (VertexFillRandom && (rand() % 2 == 1));
                    Vertices[index].fill_usd = VertexFillUSD || (VertexFillRandom && (rand() % 2 == 1));

                    // if we have a ChangeSection greater than zero
                    if(ChangeSection_)
                    {
                        // if we are in hexagon mode
                        if(ChangeSectionHexagonMode)
                        {
                            // if we walk through the upper rows of the cursor field
                            if(i < 0)
                            {
                                // right vertex of the row
                                if(j == ChangeSection_ - abs(i / 2))
                                    Vertices[index].fill_usd = false;
                            }
                            // if we are at the last lower row
                            else if(i == ChangeSection_)
                            {
                                Vertices[index].fill_rsu = false;
                                Vertices[index].fill_usd = false;
                            }
                            // if we walk through the lower rows of the cursor field
                            else // if (i >= 0 && i != ChangeSection)
                            {
                                // left vertex of the row
                                if(j == -ChangeSection_ + abs(i / 2))
                                    Vertices[index].fill_rsu = false;
                                // right vertex of the row
                                else if(j == ChangeSection_ - abs(i / 2))
                                {
                                    Vertices[index].fill_rsu = false;
                                    Vertices[index].fill_usd = false;
                                }
                            }
                        }
                        // we are in square mode
                        else
                        {
                            // if we are at the last lower row or right vertex of the row
                            if(i == ChangeSection_ || j == ChangeSection_)
                            {
                                Vertices[index].fill_rsu = false;
                                Vertices[index].fill_usd = false;
                            }
                            // left vertex of the row
                            else if(j == -ChangeSection_)
                                Vertices[index].fill_rsu = false;
                        }
                    }
                } else
                {
                    Vertices[index].active = false;
                    Vertices[index].fill_rsu = false;
                    Vertices[index].fill_usd = false;
                }
            }
        } else
        {
            for(int j = -MAX_CHANGE_SECTION; j <= MAX_CHANGE_SECTION - 1; j++, index++)
            {
                if(abs(i) <= ChangeSection_
                   && (j < 0 ? abs(j) <= ChangeSection_ - (ChangeSectionHexagonMode ? abs(i / 2) : 0) :
                               j <= ChangeSection_ - 1 - (ChangeSectionHexagonMode ? abs(i / 2) : 0)))
                {
                    // check if cursor vertices should change randomly
                    if(VertexActivityRandom)
                        Vertices[index].active = (rand() % 2 == 1);
                    else
                        Vertices[index].active = true;

                    // decide which triangle-textures will be filled at this vertex (necessary for border)
                    Vertices[index].fill_rsu = VertexFillRSU || (VertexFillRandom && (rand() % 2 == 1));
                    Vertices[index].fill_usd = VertexFillUSD || (VertexFillRandom && (rand() % 2 == 1));

                    // if we have a ChangeSection greater than zero
                    if(ChangeSection_)
                    {
                        // if we are in hexagon mode
                        if(ChangeSectionHexagonMode)
                        {
                            // if we walk through the upper rows of the cursor field
                            if(i < 0)
                            {
                                // right vertex of the row
                                if(j == ChangeSection_ - 1 - abs(i / 2))
                                    Vertices[index].fill_usd = false;
                            }
                            // if we are at the last lower row
                            else if(i == ChangeSection_)
                            {
                                Vertices[index].fill_rsu = false;
                                Vertices[index].fill_usd = false;
                            }
                            // if we walk through the lower rows of the cursor field
                            else // if (i >= 0 && i != ChangeSection)
                            {
                                // left vertex of the row
                                if(j == -ChangeSection_ + abs(i / 2))
                                    Vertices[index].fill_rsu = false;
                                // right vertex of the row
                                else if(j == ChangeSection_ - 1 - abs(i / 2))
                                {
                                    Vertices[index].fill_rsu = false;
                                    Vertices[index].fill_usd = false;
                                }
                            }
                        }
                        // we are in square mode
                        else
                        {
                            // if we are at the last lower row
                            if(i == ChangeSection_)
                            {
                                Vertices[index].fill_rsu = false;
                                Vertices[index].fill_usd = false;
                            }
                            // right vertex of the row
                            else if(j == ChangeSection_ - 1)
                                Vertices[index].fill_usd = false;
                        }
                    }
                } else
                {
                    Vertices[index].active = false;
                    Vertices[index].fill_rsu = false;
                    Vertices[index].fill_usd = false;
                }
            }
        }
    }
    // NOTE: to understand this '-(ChangeSectionHexagonMode ? abs(i/2) : 0)'
    // if we don't change the cursor size in square-mode, but in hexagon mode,
    // at each row there have to be missing as much vertices as the row number is
    // i = row number --> so at the left side of the row there are missing i/2
    // and at the right side there are missing i/2. That makes it look like an hexagon.
}

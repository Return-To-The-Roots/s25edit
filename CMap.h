#ifndef _CMAP_H
#define _CMAP_H

#include "defines.h"
#include <boost/optional.hpp>
#include <Point.h>
#include <SDL.h>
#include <array>
#include <list>
#include <memory>
#include <string>

struct SavedVertex
{
    Position pos;
    // MAX_CHANGE_SECTION * 2 + 1 = number of vertices in one row or col
    //+ 10 because if we raise a vertex then the other vertices will be raised too after 5 times
    // this ranges up to 10 vertices
    //+ 2 because modifications on a vertex will touch building and shading around
    std::array<std::array<MapNode, (MAX_CHANGE_SECTION + 10 + 2) * 2 + 1>, (MAX_CHANGE_SECTION + 10 + 2) * 2 + 1> PointsArroundVertex;
};

class CMap
{
    friend class CDebug;
    friend class CSurface;

private:
    std::string filename_;
    SdlSurface Surf_Map;
    SdlSurface Surf_RightMenubar;
    std::unique_ptr<bobMAP> map;
    DisplayRectangle displayRect;
    bool active;
    int VertexX_, VertexY_;
    bool RenderBuildHelp;
    bool RenderBorders;
    int BitsPerPixel;
    // editor mode variables
    int mode;
    // necessary for release the EDITOR_MODE_CUT (set back to last used mode)
    int lastMode;
    // these variables are used by the callback functions to define what new data should go into the vertex
    int modeContent;
    int modeContent2;
    // is the user currently modifying?
    bool modify;
    // necessary for "undo"- and "do"-function
    bool saveCurrentVertices;
    std::list<SavedVertex> undoBuffer;
    std::list<SavedVertex> redoBuffer;
    // get the number of the triangle nearest to cursor and save it to VertexX and VertexY
    void storeVerticesFromMouse(Uint16 MouseX, Uint16 MouseY, Uint8 MouseState);
    // blitting coords for the mouse cursor
    int MouseBlitX;
    int MouseBlitY;
    // counts the distance from the cursor vertex to the farest vertex that can be involved in changes (0 - only cursor vertex, 1 - six
    // vertices around the cursor vertex ....) (editor mode)
    int ChangeSection_;
    // in some cases we change the ChangeSection manually but want to reset it (this is user friendly)
    int lastChangeSection;
    // decides what to do if user presses '+' or '-', if true, then cursor will increase like an hexagon, otherwise like a square
    bool ChangeSectionHexagonMode;
    // user can decide that only RSU-Triangles will be filled (within the cursor field)
    bool VertexFillRSU;
    // user can decide that only USD-Triangles will be filled (within the cursor field)
    bool VertexFillUSD;
    // user can decide that all triangles will be filled randomly (within the cursor field)
    bool VertexFillRandom;
    // user can set activity to random, so all active cursor vertices (within the ChangeSection) will change each gameloop
    bool VertexActivityRandom;
    // counts how many vertices we have around the cursor (and including the cursor)
    int VertexCounter;
    // array to store all vertices (editor mode) --> after constructing class CMap this will have 'VertexCounter' elements
    std::vector<cursorPoint> Vertices;
    // these are the new (internal) values for player positions (otherwise we had to walk through the objectXXXX-Blocks step by step)
    std::array<Uint16, MAXPLAYERS> PlayerHQx;
    std::array<Uint16, MAXPLAYERS> PlayerHQy;
    // maximum value of height (user can modify this)
    Uint8 MaxRaiseHeight;
    Uint8 MinReduceHeight;
    // lock vertical or horizontal movement
    bool HorizontalMovementLocked;
    bool VerticalMovementLocked;
    boost::optional<Position> startScrollPos;

public:
    CMap(const std::string& filename);
    ~CMap();
    void constructMap(const std::string& filename, int width = 32, int height = 32, MapType type = MAP_GREENLAND,
                      TriangleTerrainType texture = TRIANGLE_TEXTURE_MEADOW1, int border = 4, int border_texture = TRIANGLE_TEXTURE_WATER);
    static std::unique_ptr<bobMAP> generateMap(int width, int height, MapType type, TriangleTerrainType texture, int border,
                                               int border_texture);
    void destructMap();
    void loadMapPics();
    void unloadMapPics();

    void moveMap(Position offset);
    void setMouseData(const SDL_MouseMotionEvent& motion);
    void setMouseData(const SDL_MouseButtonEvent& button);
    void setKeyboardData(const SDL_KeyboardEvent& key);
    void setActive() { active = true; }
    void setInactive() { active = false; }
    bool isActive() const { return active; }
    int getVertexX() const { return VertexX_; }
    int getVertexY() const { return VertexY_; }
    auto getMaxRaiseHeight() const { return MaxRaiseHeight; }
    auto getMinReduceHeight() const { return MinReduceHeight; }
    bool isHorizontalMovementLocked() const { return HorizontalMovementLocked; }
    bool isVerticalMovementLocked() const { return VerticalMovementLocked; }
    bool getRenderBuildHelp() const { return RenderBuildHelp; }
    bool getRenderBorders() const { return RenderBorders; }
    int getBitsPerPixel() const { return BitsPerPixel; }
    void setBitsPerPixel(int bbp)
    {
        BitsPerPixel = bbp;
        Surf_Map.reset();
    }
    void setMode(int mode) { this->mode = mode; }
    int getMode() { return mode; }
    void setModeContent(int modeContent) { this->modeContent = modeContent; }
    void setModeContent2(int modeContent2) { this->modeContent2 = modeContent2; }
    int getModeContent() { return modeContent; }
    int getModeContent2() { return modeContent2; }
    bobMAP* getMap() { return map.get(); }
    SDL_Surface* getSurface()
    {
        render();
        return Surf_Map.get();
    }
    DisplayRectangle getDisplayRect() { return displayRect; }
    void setDisplayRect(const DisplayRectangle& displayRect) { this->displayRect = displayRect; }
    auto& getPlayerHQx() { return PlayerHQx; }
    auto& getPlayerHQy() { return PlayerHQy; }
    const std::string& getFilename() const { return filename_; }
    void setFilename(std::string filename) { filename_ = std::move(filename); }
    std::string getMapname() const { return map->getName(); }
    void setMapname(const std::string& name) { map->setName(name); }
    std::string getAuthor() const { return map->getAuthor(); }
    void setAuthor(const std::string& author) { map->setAuthor(author); }

    void drawMinimap(SDL_Surface* Window);
    void render();
    // get and set some variables necessary for cursor behavior
    void setHexagonMode(bool HexagonMode)
    {
        ChangeSectionHexagonMode = HexagonMode;
        setupVerticesActivity();
    }
    bool getHexagonMode() { return ChangeSectionHexagonMode; }
    void setVertexFillRSU(bool fillRSU)
    {
        VertexFillRSU = fillRSU;
        setupVerticesActivity();
    }
    bool getVertexFillRSU() { return VertexFillRSU; }
    void setVertexFillUSD(bool fillUSD)
    {
        VertexFillUSD = fillUSD;
        setupVerticesActivity();
    }
    bool getVertexFillUSD() { return VertexFillUSD; }
    void setVertexFillRandom(bool fillRandom)
    {
        VertexFillRandom = fillRandom;
        setupVerticesActivity();
    }
    bool getVertexFillRandom() { return VertexFillRandom; }
    void setVertexActivityRandom(bool activityRandom)
    {
        VertexActivityRandom = activityRandom;
        setupVerticesActivity();
    }
    bool getVertexActivityRandom() { return VertexActivityRandom; }

private:
    // returns count of the vertices that are involved in changes (editor mode) -->THIS FUNCTION IS OUTDATED
    int getActiveVertices(int tempChangeSection);
    // this will calculate ALL vertices for the whole square
    void calculateVertices();
    // this will calculate the vertices two sections around one vertex (like a great hexagon) --> necessary to calculate the possible
    // building for a vertex  view this pic to understand the indices
    //              X=7     X=8     X=9
    //          X=10    X=1     X=2     X=11
    //      X=12    X=3     X=0     X=4     X=13
    //          X=14    X=5     X=6     X=15
    //              X=16    X=17    X=18
    template<size_t T_size>
    void calculateVerticesAround(std::array<Point32, T_size>& newVertices, int x, int y);
    // this will setup the 'active' variable of each vertices depending on 'ChangeSection'
    void setupVerticesActivity();
    int correctMouseBlitX(int VertexX, int VertexY);
    int correctMouseBlitY(int VertexX, int VertexY);
    void modifyVertex();
    void modifyHeightRaise(int VertexX, int VertexY);
    void modifyHeightReduce(int VertexX, int VertexY);
    void modifyHeightPlane(int VertexX, int VertexY, Uint8 h);
    void modifyHeightMakeBigHouse(int VertexX, int VertexY);
    void modifyShading(int VertexX, int VertexY);
    void modifyTexture(int VertexX, int VertexY, bool rsu, bool usd);
    void modifyTextureMakeHarbour(int VertexX, int VertexY);
    void modifyObject(int x, int y);
    void modifyAnimal(int VertexX, int VertexY);
    void modifyBuild(int x, int y);
    void modifyResource(int x, int y);
    void modifyPlayer(int VertexX, int VertexY);
    void rotateMap();
    void MirrorMapOnXAxis();
    void MirrorMapOnYAxis();
    void onLeftMouseDown(const Point32& pos);
};

#endif

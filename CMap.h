#ifndef _CMAP_H
    #define _CMAP_H

#include "includes.h"

class CMap
{
    friend class CDebug;

    private:
        SDL_Surface *Surf_Map;
        SDL_Surface *Surf_RightMenubar;
        bobMAP *map;
        struct DisplayRectangle displayRect;
        bool active;
        bool needSurface;
        int VertexX, VertexY;
        bool BuildHelp;
        //editor mode variables
        int mode;
        //necessary for release the EDITOR_MODE_CUT (set back to last used mode)
        int lastMode;
        int modeContent;
        int modeContent2;
        bool modify;
        //get the number of the triangle nearest to cursor and save it to VertexX and VertexY
        void saveVertex(Uint16 MouseX, Uint16 MouseY, Uint8 MouseState);
        //blitting coords for the mouse cursor
        int MouseBlitX;
        int MouseBlitY;
        //counts the distance from the cursor vertex to the farest vertex that can be involved in changes (0 - only cursor vertex, 1 - six vertices around the cursor vertex ....) (editor mode)
        int ChangeSection;
        //decides what to do if user presses '+' or '-', if true, then cursor will increase like an hexagon, otherwise like a square
        bool ChangeSectionHexagonMode;
        //user can decide that only RSU-Triangles will be filled (within the cursor field)
        bool VertexFillRSU;
        //user can decide that only USD-Triangles will be filled (within the cursor field)
        bool VertexFillUSD;
        //user can decide that all triangles will be filled randomly (within the cursor field)
        bool VertexFillRandom;
        //user can set activity to random, so all active cursor vertices (within the ChangeSection) will change each gameloop
        bool VertexActivityRandom;
        //counts how many vertices we have around the cursor (and including the cursor)
        int VertexCounter;
        //array to store all vertices (editor mode) --> after constructing class CMap this will have 'VertexCounter' elements
        struct vertexPoint *Vertices;
        //buffer for texts we maybe need to write
        char textBuffer[50];

    public:
        CMap(char *filename);
        ~CMap();
        void setMouseData(SDL_MouseMotionEvent motion);
        void setMouseData(SDL_MouseButtonEvent button);
        void setKeyboardData(SDL_KeyboardEvent key);
        void setActive(void) { active = true; };
        void setInactive(void) { active = false; };
        bool isActive(void) { return active; };
        int getVertexX(void) { return VertexX; };
        int getVertexY(void) { return VertexY; };
        bool getBuildHelp(void) { return BuildHelp; };
        void setMode(int mode) { this->mode = mode; };
        int getMode(void) { return mode; };
        void setModeContent(int modeContent) { this->modeContent = modeContent; };
        void setModeContent2(int modeContent2) { this->modeContent2 = modeContent2; };
        int getModeContent(void) { return modeContent; };
        int getModeContent2(void) { return modeContent2; };
        bobMAP* getMap(void) { return map; };
        SDL_Surface* getSurface(void) { render(); return Surf_Map; };
        DisplayRectangle getDisplayRect(void) { return displayRect; };
        void setDisplayRect(DisplayRectangle displayRect) { this->displayRect = displayRect; };
        void drawMinimap(SDL_Surface *Window);
        bool render(void);
        //get and set some variables necessary for cursor behavior
        void setHexagonMode(bool HexagonMode) { ChangeSectionHexagonMode = HexagonMode; setupVerticesActivity(); };
        bool getHexagonMode(void) { return ChangeSectionHexagonMode; };
        void setVertexFillRSU(bool fillRSU) { VertexFillRSU = fillRSU; setupVerticesActivity(); };
        bool getVertexFillRSU(void) { return VertexFillRSU; };
        void setVertexFillUSD(bool fillUSD) { VertexFillUSD = fillUSD; setupVerticesActivity(); };
        bool getVertexFillUSD(void) { return VertexFillUSD; };
        void setVertexFillRandom(bool fillRandom) { VertexFillRandom = fillRandom; setupVerticesActivity(); };
        bool getVertexFillRandom(void) { return VertexFillRandom; };
        void setVertexActivityRandom(bool activityRandom) { VertexActivityRandom = activityRandom; setupVerticesActivity(); };
        bool getVertexActivityRandom(void) { return VertexActivityRandom; };

    private:
        //returns count of the vertices that are involved in changes (editor mode) -->THIS FUNCTION IS OUTDATED
        int getActiveVertices(int tempChangeSection);
        //this will calculate ALL vertices for the whole square
        void calculateVertices(void);
        //this will calculate the vertices two sections around one vertex (like a great hexagon) --> necessary to calculate the possible building for a vertex
        //view this pic to understand the indices
        //              X=7     X=8     X=9
        //          X=10    X=1     X=2     X=11
        //      X=12    X=3     X=0     X=4     X=13
        //          X=14    X=5     X=6     X=15
        //              X=16    X=17    X=18
        void calculateVerticesAround(struct vertexPoint Vertices[], int VertexX, int VertexY, int ChangeSection);
        //this will setup the 'active' variable of each vertices depending on 'ChangeSection'
        void setupVerticesActivity(void);
        int correctMouseBlitX(int VertexX, int VertexY);
        int correctMouseBlitY(int VertexX, int VertexY);
        void modifyVertex(void);
        void modifyHeight(int VertexX, int VertexY);
        //this modifies a whole hexagon!
        void modifyTexture(int VertexX, int VertexY, bool rsu, bool usd);
        void modifyObject(int VertexX, int VertexY);
        //this modifies a whole hexagon!
        void modifyBuild(int VertexX, int VertexY);
};

#endif

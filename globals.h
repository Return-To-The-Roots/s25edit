#ifndef _GLOBALS_H
#define _GLOBALS_H

#include "gameData/WorldDescription.h"
#include <SDL.h>
#include <vector>

class CGame;
struct bobBMP;
struct bobSHADOW;
struct bobPAL;

namespace global {
// array for all pictures
extern std::vector<bobBMP> bmpArray;
// array for all shadows
extern std::vector<bobSHADOW> shadowArray;
// array for all palettes
extern std::vector<bobPAL> palArray;
// the game object
extern CGame* s2;
// Path to game data (must not be empty!)
extern std::string gameDataFilePath;
// Path where maps will be stored (must not be empty!)
extern std::string userMapsPath;
extern WorldDescription worldDesc;
} // namespace global

extern unsigned char TRIANGLE_HEIGHT;
extern unsigned char TRIANGLE_WIDTH;
extern unsigned char TRIANGLE_INCREASE;

extern Uint8 gouData[3][256][256];

#endif

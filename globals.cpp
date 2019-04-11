#include "defines.h"
#include "globals.h"

// array for all pictures
std::vector<bobBMP> global::bmpArray(MAXBOBBMP);
// array for all shadows
std::vector<bobSHADOW> global::shadowArray(MAXBOBSHADOW);
// array for all palettes
std::vector<bobPAL> global::palArray(MAXBOBPAL);
// the game object
CGame* global::s2;

std::string global::gameDataFilePath(".");
std::string global::userMapsPath("./WORLDS");
WorldDescription global::worldDesc;

unsigned char TRIANGLE_HEIGHT = 28;
unsigned char TRIANGLE_WIDTH = 56;
unsigned char TRIANGLE_INCREASE = 5;

// three-dimensional array for the GOUx.DAT-Files (3 files * 256 * 256 values)
Uint8 gouData[3][256][256];

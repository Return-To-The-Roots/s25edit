#include "globals.h"

// array for all pictures
bobBMP global::bmpArray[MAXBOBBMP];
// array for all shadows
bobSHADOW global::shadowArray[MAXBOBSHADOW];
// array for all palettes
bobPAL global::palArray[MAXBOBPAL];
// the game object
CGame* global::s2;

unsigned char TRIANGLE_HEIGHT = 28;
unsigned char TRIANGLE_WIDTH = 56;
unsigned char TRIANGLE_INCREASE = 5;

// three-dimensional array for the GOUx.DAT-Files (3 files * 256 * 256 values)
Uint8 gouData[3][256][256];

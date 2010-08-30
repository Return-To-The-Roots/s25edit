#include "globals.h"

//array for all pictures
bobBMP global::bmpArray[MAXBOBBMP];
//array for all shadows
bobSHADOW global::shadowArray[MAXBOBSHADOW];
//array for all palettes
bobPAL global::palArray[MAXBOBPAL];
//the game object
CGame* global::s2;


unsigned char TRIANGLE_HEIGHT = 28;
unsigned char TRIANGLE_WIDTH = 56;
unsigned char TRIANGLE_INCREASE = 5;

#ifndef LEVEL_H_
#define LEVEL_H_

#include "typedef.h"

#define MAPLEN 240

/**
 * Buffer in RAM where the levels are stored.
 */
extern u8 tilemap[MAPLEN];

/**
 * Load a new level into RAM and set the tilemap pointer.
 */
void __fastcall__ load_level(u8 number);

#endif /* LEVEL_H_ */

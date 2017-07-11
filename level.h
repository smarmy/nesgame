#ifndef LEVEL_H_
#define LEVEL_H_

#include "typedef.h"

/**
 * Pointer to the current level.
 */
extern const u8* tilemap;

/**
 * Load a new level into RAM and set the tilemap pointer.
 */
void __fastcall__ load_level(u8 number);

#endif /* LEVEL_H_ */

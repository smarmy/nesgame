#ifndef LEVEL_H_
#define LEVEL_H_

#include "typedef.h"

#define MAPLEN 240

#define TILE_EMPTY  0x0
#define TILE_BRICK  0x1
#define TILE_LADDER 0x2
#define TILE_KEY    0x3
#define TILE_LOCK   0x4
#define TILE_DOOR   0x5
#define TILE_LAVA_SURFACE   0x6
#define TILE_LAVA_BODY      0x7
#define TILE_DIRT_SURFACE   0x8
#define TILE_DIRT_BODY      0x9
#define TILE_BUSH   0xA
#define TILE_CLOUD  0xB
#define TILE_BRIDGE 0xC
#define TILE_PILLAR_TOP   0xD
#define TILE_PILLAR_MID   0xE
#define TILE_PILLAR_BOT   0xF
#define TILE_SPIKES 0x10
#define TILE_FRAGILE  0x11

/**
 * Buffer in RAM where the levels are stored.
 */
extern u8 tilemap[MAPLEN];

/**
 * Load a new level into RAM and set the tilemap pointer.
 */
void __fastcall__ load_level(u8 number);

/**
 * Remove a tile from the tilemap in RAM. Also buffers an update
 * of the nametable to the PPU.
 */
void __fastcall__ remove_tile(u8 tile_index);

#endif /* LEVEL_H_ */

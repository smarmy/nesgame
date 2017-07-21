#include "asm.h"
#include "colcheck.h"
#include "level.h"
#include "object.h"
#include "gamedata.h"

/* Defined in asm */
u8 __fastcall__ is_solid(u8 tile_index);

static void __fastcall__ unlock(u8 tile_index)
{
  if (keys > 0)
  {
    play_sound(4, 0xF0);
    remove_tile(tile_index);
    keys--;
  }
  else
  {
    play_sound(1, 0xA0);
  }
}

/*****************************************************************************
 * Calculate tile index:
 * index = (y/map_width) * map_width + (x/map_width)
 * index = (y>>4)<<4 + (x>>4)
 *
 * Can be optimized as
 *
 * index = (y & 0xF0) + (x>>4)
 *
 * since left shift by 4 and then right shift by 4 just removes the bottom
 * 4 bits.
 *****************************************************************************/

static u8 tile_index_1;
static u8 tile_index_2;

u8 __fastcall__ colcheck_up(u8 obj_index)
{
  tile_index_1 = ((fix2i(objects_y[obj_index])+objects_bbox_y1[obj_index]-1) & 0xF0) + ((fix2i(objects_x[obj_index]) + objects_bbox_x1[obj_index]) >> 4);
  tile_index_2 = ((fix2i(objects_y[obj_index])+objects_bbox_y1[obj_index]-1) & 0xF0) + ((fix2i(objects_x[obj_index]) + objects_bbox_x2[obj_index]) >> 4);

  if (is_solid(tile_index_1) == 0) return 1;
  if (is_solid(tile_index_2) == 0) return 1;

  return 0;
}

u8 __fastcall__ colcheck_right(u8 obj_index)
{
  tile_index_1 = ((fix2i(objects_y[obj_index])+objects_bbox_y1[obj_index]+1) & 0xF0) + ((fix2i(objects_x[obj_index]) + objects_bbox_x2[obj_index] + 1) >> 4);
  tile_index_2 = ((fix2i(objects_y[obj_index])+objects_bbox_y2[obj_index]-1) & 0xF0) + ((fix2i(objects_x[obj_index]) + objects_bbox_x2[obj_index] + 1) >> 4);

  if (is_solid(tile_index_1) == 0) return tile_index_1;
  if (is_solid(tile_index_2) == 0) return tile_index_2;

  if (obj_index == O_PLAYER)
  {
    if (tilemap[tile_index_1] == TILE_LOCK) { unlock(tile_index_1); return tile_index_1; }
    if (tilemap[tile_index_2] == TILE_LOCK) { unlock(tile_index_2); return tile_index_2; }
  }

  return 0;
}

u8 __fastcall__ colcheck_left(u8 obj_index)
{
  tile_index_1 = ((fix2i(objects_y[obj_index])+objects_bbox_y1[obj_index]+1) & 0xF0) + ((fix2i(objects_x[obj_index]) + objects_bbox_x1[obj_index] - 1) >> 4);
  tile_index_2 = ((fix2i(objects_y[obj_index])+objects_bbox_y2[obj_index]-1) & 0xF0) + ((fix2i(objects_x[obj_index]) + objects_bbox_x1[obj_index] - 1) >> 4);

  if (is_solid(tile_index_1) == 0) return tile_index_1;
  if (is_solid(tile_index_2) == 0) return tile_index_2;

  if (obj_index == O_PLAYER)
  {
    if (tilemap[tile_index_1] == TILE_LOCK) { unlock(tile_index_1); return tile_index_1; }
    if (tilemap[tile_index_2] == TILE_LOCK) { unlock(tile_index_2); return tile_index_2; }
  }

  return 0;
}

u8 __fastcall__ stairs_check(u8 obj_index)
{
  tile_index_1 = ((fix2i(objects_y[obj_index])) & 0xF0) + ((fix2i(objects_x[obj_index]) + objects_bbox_x1[obj_index]) >> 4);
  tile_index_2 = ((fix2i(objects_y[obj_index])) & 0xF0) + ((fix2i(objects_x[obj_index]) + objects_bbox_x2[obj_index]) >> 4);

  if (tilemap[tile_index_1] == TILE_LADDER) return 1;
  if (tilemap[tile_index_2] == TILE_LADDER) return 1;

  return 0;
}

u8 __fastcall__ tile_check(u8 obj_index, u8 tile)
{
  tile_index_1 = ((fix2i(objects_y[obj_index])+2) & 0xF0) + ((fix2i(objects_x[obj_index]) + objects_bbox_x1[obj_index]) >> 4);
  tile_index_2 = ((fix2i(objects_y[obj_index])+15) & 0xF0) + ((fix2i(objects_x[obj_index]) + objects_bbox_x2[obj_index]) >> 4);

  if (tilemap[tile_index_1] == tile) return tile_index_1;
  if (tilemap[tile_index_2] == tile) return tile_index_2;

  return 0;
}

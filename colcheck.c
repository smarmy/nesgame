#include "asm.h"
#include "colcheck.h"
#include "level.h"
#include "object.h"
#include "gamedata.h"

static u8 __fastcall__ is_solid(u8 tile_index)
{
  switch (tilemap[tile_index])
  {
    case TILE_BRICK:
    case TILE_DIRT_SURFACE:
    case TILE_DIRT_BODY:
    case TILE_BRIDGE:
      return 0;
  }
  return 1;
}

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

u8 __fastcall__ colcheck_down(u8 obj_index)
{
  u8 tile_index_1 = (((fix2i(objects.y[obj_index])+objects.bbox_y2[obj_index]+1) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + objects.bbox_x1[obj_index]) >> 4);
  u8 tile_index_2 = (((fix2i(objects.y[obj_index])+objects.bbox_y2[obj_index]+1) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + objects.bbox_x2[obj_index]) >> 4);

  if (is_solid(tile_index_1) == 0) return 1;
  if (is_solid(tile_index_2) == 0) return 1;

  return 0;
}

u8 __fastcall__ colcheck_up(u8 obj_index)
{
  u8 tile_index_1 = (((fix2i(objects.y[obj_index])+objects.bbox_y1[obj_index]-1) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + objects.bbox_x1[obj_index]) >> 4);
  u8 tile_index_2 = (((fix2i(objects.y[obj_index])+objects.bbox_y1[obj_index]-1) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + objects.bbox_x2[obj_index]) >> 4);

  if (is_solid(tile_index_1) == 0) return 1;
  if (is_solid(tile_index_2) == 0) return 1;

  return 0;
}

u8 __fastcall__ colcheck_right(u8 obj_index)
{
  u8 tile_index_1 = (((fix2i(objects.y[obj_index])+objects.bbox_y1[obj_index]+1) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + objects.bbox_x2[obj_index] + 1) >> 4);
  u8 tile_index_2 = (((fix2i(objects.y[obj_index])+objects.bbox_y2[obj_index]-1) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + objects.bbox_x2[obj_index] + 1) >> 4);

  if (is_solid(tile_index_1) == 0) return 1;
  if (is_solid(tile_index_2) == 0) return 1;

  if (obj_index == O_PLAYER)
  {
    if (tilemap[tile_index_1] == TILE_LOCK) { unlock(tile_index_1); return 1; }
    if (tilemap[tile_index_2] == TILE_LOCK) { unlock(tile_index_2); return 1; }
  }

  return 0;
}

u8 __fastcall__ colcheck_left(u8 obj_index)
{
  u8 tile_index_1 = (((fix2i(objects.y[obj_index])+objects.bbox_y1[obj_index]+1) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + objects.bbox_x1[obj_index] - 1) >> 4);
  u8 tile_index_2 = (((fix2i(objects.y[obj_index])+objects.bbox_y2[obj_index]-1) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + objects.bbox_x1[obj_index] - 1) >> 4);

  if (is_solid(tile_index_1) == 0) return 1;
  if (is_solid(tile_index_2) == 0) return 1;

  if (obj_index == O_PLAYER)
  {
    if (tilemap[tile_index_1] == TILE_LOCK) { unlock(tile_index_1); return 1; }
    if (tilemap[tile_index_2] == TILE_LOCK) { unlock(tile_index_2); return 1; }
  }

  return 0;
}

u8 __fastcall__ stairs_check(u8 obj_index)
{
  u8 tile_index_1 = (((fix2i(objects.y[obj_index])) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + objects.bbox_x1[obj_index]) >> 4);
  u8 tile_index_2 = (((fix2i(objects.y[obj_index])) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + objects.bbox_x2[obj_index]) >> 4);

  if (tilemap[tile_index_1] == TILE_LADDER) return 1;
  if (tilemap[tile_index_2] == TILE_LADDER) return 1;

  return 0;
}

u8 __fastcall__ tile_check(u8 obj_index, u8 tile)
{
  u8 tile_index_1 = (((fix2i(objects.y[obj_index])+2) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + objects.bbox_x1[obj_index]) >> 4);
  u8 tile_index_2 = (((fix2i(objects.y[obj_index])+15) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + objects.bbox_x2[obj_index]) >> 4);

  if (tilemap[tile_index_1] == tile) return tile_index_1;
  if (tilemap[tile_index_2] == tile) return tile_index_2;

  return 0;
}

u8 __fastcall__ colcheck_objects(u8 obj_index_1, u8 obj_index_2)
{
  static u8 o1x, o1y;
  static u8 o2x, o2y;
  static u8 o1w, o1h;
  static u8 o2w, o2h;

  o1x = fix2i(objects.x[obj_index_1]) + objects.bbox_x1[obj_index_1];
  o1y = fix2i(objects.y[obj_index_1]) + objects.bbox_y1[obj_index_1];
  o2x = fix2i(objects.x[obj_index_2]) + objects.bbox_x1[obj_index_2];
  o2y = fix2i(objects.y[obj_index_2]) + objects.bbox_y1[obj_index_2];
  o1w = objects.bbox_x2[obj_index_1] - objects.bbox_x1[obj_index_1];
  o1h = objects.bbox_y2[obj_index_1] - objects.bbox_y1[obj_index_1];
  o2w = objects.bbox_x2[obj_index_2] - objects.bbox_x1[obj_index_2];
  o2h = objects.bbox_y2[obj_index_2] - objects.bbox_y1[obj_index_2];

  if ((o1x + o1w) < o2x) return 0;
  if ((o1y + o1h) < o2y) return 0;
  if ((o2x + o2w) < o1x) return 0;
  if ((o2y + o2h) < o1y) return 0;

  return 1;
}

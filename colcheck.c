#include "asm.h"
#include "colcheck.h"
#include "level.h"
#include "object.h"
#include "gamedata.h"

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
  u8 tile_index_1 = (((fix2i(objects.y[obj_index])+17) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + 6) >> 4);
  u8 tile_index_2 = (((fix2i(objects.y[obj_index])+17) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + 10) >> 4);

  if (tilemap[tile_index_1] == TILE_BRICK) return 1;
  if (tilemap[tile_index_2] == TILE_BRICK) return 1;

  return 0;
}

u8 __fastcall__ colcheck_up(u8 obj_index)
{
  u8 tile_index_1 = (((fix2i(objects.y[obj_index])-1) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + 6) >> 4);
  u8 tile_index_2 = (((fix2i(objects.y[obj_index])-1) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + 10) >> 4);

  if (tilemap[tile_index_1] == TILE_BRICK) return 1;
  if (tilemap[tile_index_2] == TILE_BRICK) return 1;

  return 0;
}

u8 __fastcall__ colcheck_right(u8 obj_index)
{
  u8 tile_index_1 = (((fix2i(objects.y[obj_index])+2) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + 17) >> 4);
  u8 tile_index_2 = (((fix2i(objects.y[obj_index])+15) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + 17) >> 4);

  if (tilemap[tile_index_1] == TILE_BRICK) return 1;
  if (tilemap[tile_index_2] == TILE_BRICK) return 1;

  if (obj_index == O_PLAYER)
  {
    if (tilemap[tile_index_1] == TILE_LOCK) { unlock(tile_index_1); return 1; }
    if (tilemap[tile_index_2] == TILE_LOCK) { unlock(tile_index_2); return 1; }
  }

  return 0;
}

u8 __fastcall__ colcheck_left(u8 obj_index)
{
  u8 tile_index_1 = (((fix2i(objects.y[obj_index])+2) >> 4) << 4) + ((fix2i(objects.x[obj_index]) - 1) >> 4);
  u8 tile_index_2 = (((fix2i(objects.y[obj_index])+15) >> 4) << 4) + ((fix2i(objects.x[obj_index]) - 1) >> 4);

  if (tilemap[tile_index_1] == TILE_BRICK) return 1;
  if (tilemap[tile_index_2] == TILE_BRICK) return 1;

  if (obj_index == O_PLAYER)
  {
    if (tilemap[tile_index_1] == TILE_LOCK) { unlock(tile_index_1); return 1; }
    if (tilemap[tile_index_2] == TILE_LOCK) { unlock(tile_index_2); return 1; }
  }

  return 0;
}

u8 __fastcall__ stairs_check(u8 obj_index)
{
  u8 tile_index_1 = (((fix2i(objects.y[obj_index])) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + 6) >> 4);
  u8 tile_index_2 = (((fix2i(objects.y[obj_index])) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + 10) >> 4);

  if (tilemap[tile_index_1] == TILE_LADDER) return 1;
  if (tilemap[tile_index_2] == TILE_LADDER) return 1;

  return 0;
}

u8 __fastcall__ tile_check(u8 obj_index, u8 tile)
{
  u8 tile_index_1 = (((fix2i(objects.y[obj_index])+2) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + 6) >> 4);
  u8 tile_index_2 = (((fix2i(objects.y[obj_index])+15) >> 4) << 4) + ((fix2i(objects.x[obj_index]) + 10) >> 4);

  if (tilemap[tile_index_1] == tile) return tile_index_1;
  if (tilemap[tile_index_2] == tile) return tile_index_2;

  return 0;
}

#include "ppu.h"
#include "level.h"

/**
 * Pointer to ROM where levels should be loaded.
 */
extern const u8 LEVELS[];

static const u8 tiletable[] =
{
  0, 0, 0, 0,             /* empty  0x0 */
  0x1, 0x1, 0x2, 0x2,     /* brick  0x1 */
  0xA, 0xB, 0xA, 0xB,     /* ladder 0x2 */
  0xC, 0xD, 0xE, 0xF,     /* key    0x3 */
  0x10, 0x11, 0x12, 0x13, /* lock   0x4 */
  0x6, 0x7, 0x8, 0x9,     /* door   0x5 */
};

u8 tilemap[MAPLEN];

static void __fastcall__ store_nametable(void)
{
  u8 loop;
  u8 tile_index;
  u8 col_cnt;

  u16 offset = 0;
  u16 base_address = 0x2000;
  u16 address;

  col_cnt = 0;

  for (loop = 0; loop < MAPLEN; loop++)
  {
    tile_index = tilemap[loop];

    /* Multpliy by 4 to get index in tiletable. */
    tile_index = tile_index << 2;

    address = base_address + offset;

    ppuwrite(address, tiletable[tile_index+0]);
    ppuwrite(address+1, tiletable[tile_index+1]);
    ppuwrite(address+32, tiletable[tile_index+2]);
    ppuwrite(address+33, tiletable[tile_index+3]);

    col_cnt++;

    if (col_cnt == 16)
    {
      col_cnt = 0;
      offset += 34;
    }
    else
    {
      offset += 2;
    }
  }
}

static void __fastcall__ load_tilemap(const u8* lvlptr)
{
  static u8 i, j;
  static u8* map_ptr;
  static u8 rle_cnt;
  static u8 rle_byte;

  /* attributes, width and height are hardcoded right now. */
  lvlptr += 3;

  i = 0;
  map_ptr = tilemap;

  while (i < MAPLEN)
  {
    rle_cnt = *(lvlptr++);
    rle_byte = *(lvlptr++);

    for (j = 0; j < rle_cnt; j++, i++)
    {
      *(map_ptr++) = rle_byte;
    }
  }
}

void __fastcall__ load_level(u8 number)
{
  static u8 num_levels;
  static u8 loop;
  static const u8* ptr;

  ptr = LEVELS;
  num_levels = *(ptr++);

  for (loop = 0; loop < num_levels; loop++)
  {
    if (loop == number)
    {
      load_tilemap(ptr);
      store_nametable();
      return;
    }

    /* skip ahead. */
    ptr += 3; /* go past attributes, width, height. */
    for (;;)
    {
      /* Read until end marker. */
      if ((*ptr) == 0xFF && (*(ptr + 1)) == 0xFF)
        break;
      ptr++;
    }
  }
}

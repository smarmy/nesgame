#include "ppu.h"
#include "object.h"
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
  0x3, 0x4, 0x5, 0x5,     /* lava surface 0x6 */
  0x5, 0x5, 0x5, 0x5,     /* lava body 0x7 */
  0x14, 0x15, 0x16, 0x17, /* dirt surface 0x8 */
  0x18, 0x19, 0x16, 0x17, /* dirt body 0x9 */
  0x1A, 0x1B, 0x1C, 0x1D, /* bush 0xA */
  0x1E, 0x1F, 0x20, 0x21, /* cloud 0xB */
  0x22, 0x23, 0x24, 0x25, /* bridge 0xC */
  0x26, 0x27, 0x28, 0x29, /* pillar top 0xD */
  0x28, 0x29, 0x28, 0x29, /* pillar mid 0xE */
  0x28, 0x29, 0x2A, 0x2B, /* pillar bot 0xF */
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
  static u8 objcount;

  static u8 otype, x, y;

  /* attributes, width and height are hardcoded right now. */
  lvlptr += 3;

  /* read level objects, reserving a slot for the player. */
  num_objects = 1;
  objcount = *(lvlptr++);

  for (i = 0; i < objcount; i++)
  {
    otype = *(lvlptr++);
    x = *(lvlptr++);
    y = *(lvlptr++);

    if (otype == O_PLAYER)
    {
      /* Always write player info to first object. */
      objects.x[O_PLAYER] = fixed(x, 0);
      objects.y[O_PLAYER] = fixed(y, 0);
    }
    else
    {
      create_object(otype, x, y);
    }
  }

  /* read palettes. */
  PPUADDR = 0x3f;
  PPUADDR = 0x00;
  for (i = 0; i < 32; i++)
  {
    PPUDATA = *(lvlptr++);
  }

  /* read nametable attributes. */
  PPUADDR = 0x23;
  PPUADDR = 0xC0;
  for (i = 0; i < 64; i++)
  {
    PPUDATA = *(lvlptr++);
  }

  /* read tiles. */
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
      {
        ptr += 2;
        break;
      }
      ptr++;
    }
  }
}

void __fastcall__ remove_tile(u8 tile_index)
{
  static const u16 base_address = 0x2000;
  static u16 offset;
  static u8 loop;
  static u8 col_cnt;

  tilemap[tile_index] = 0;

  /* setup offset to PPU address ... */
  col_cnt = 0;
  offset = base_address;
  for (loop = 0; loop < tile_index; loop++)
  {
    col_cnt++;
    if (col_cnt == 16)
    {
      offset += 34;
      col_cnt = 0;
    }
    else
    {
      offset += 2;
    }
  }

  *(VRAMBUFFER+1) = (u8)(offset >> 8);
  *(VRAMBUFFER+2) = (u8)(offset & 0xFF);
  *(VRAMBUFFER+3) = 0;

  offset++;
  *(VRAMBUFFER+4) = (u8)(offset >> 8);
  *(VRAMBUFFER+5) = (u8)(offset & 0xFF);
  *(VRAMBUFFER+6) = 0;

  offset += 31;
  *(VRAMBUFFER+7) = (u8)(offset >> 8);
  *(VRAMBUFFER+8) = (u8)(offset & 0xFF);
  *(VRAMBUFFER+9) = 0;

  offset++;
  *(VRAMBUFFER+10) = (u8)(offset >> 8);
  *(VRAMBUFFER+11) = (u8)(offset & 0xFF);
  *(VRAMBUFFER+12) = 0;

  *(VRAMBUFFER) = 4;
}

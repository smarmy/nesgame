#include "asm.h"
#include "typedef.h"
#include "sprite.h"
#include "fixed.h"
#include "object.h"

#define MAX(A, B) (A > B ? A : B)
#define MIN(A, B) (A < B ? A : B)

static const u8 pal[] = 
{ 
  0x3f, 0x01, 0x11, 0x21, 0x3f, 0x01, 0x11, 0x21, 0x3f, 0x01, 0x11, 0x21, 0x3f, 0x01, 0x11, 0x21, /* background. */
  0x3f, 0x07, 0x26, 0x3d, 0x3f, 0x01, 0x11, 0x21, 0x3f, 0x01, 0x11, 0x21, 0x3f, 0x01, 0x11, 0x21, /* sprites. */
};

static const u8 tilemap[] =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x01, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static u8 vram_buf[960];

static const u8 tiletable[] =
{
  0, 0, 0, 0,
  0x2, 0x3, 0x12, 0x13,
  0x8, 0x9, 0x18, 0x19,
};

void __fastcall__ load_palette(void);
void __fastcall__ setup_sprites(void);
void __fastcall__ load_tilemap(void);
void __fastcall__ copy_vram(void);
void __fastcall__ walk(u8 gamepad_state);
void __fastcall__ climb(u8 gamepad_state);
u8 __fastcall__ colcheck(void);
u8 __fastcall__ colcheck_up(void);
u8 __fastcall__ colcheck_right(void);
u8 __fastcall__ colcheck_left(void);
u8 __fastcall__ stairs_check(void);

#define LEFT    64
#define RIGHT   0
#define DOWN    1
#define UP      2

#define STATE_WALK  0
#define STATE_CLIMB 1
#define STATE_JUMP  2

object_t player =
{
    fixed(128, 0), fixed(128, 0), /* x, y */
    0, 0,                         /* hspeed, vspeed. */
    0, RIGHT,                     /* sprite_mirrored, dir */
    STATE_WALK, 0,                /* state, sprite_index */
};

#define PLAYER_SPRITE_STAND 0
#define PLAYER_SPRITE_CLIMB 1
#define PLAYER_SPRITE_WALK  2
static u8 player_sprite_index_table[] =
{
  0, 1, 16, 17,
  2, 3, 18, 19,
  4, 5, 20, 21
};

static const u8 gravity = 25;

void main()
{
  static u8 gamepad_state;

  /* Turn off PPU. */
  PPUCTRL = 0;
  PPUMASK = 0;

  load_palette();
  load_tilemap();
  copy_vram();

  setup_sprites();

  /* Reset scroll. */
  PPUSCROLL = 0;
  PPUSCROLL = 0;

  /* Turn on PPU. */
  PPUCTRL = 0x88;
  PPUMASK = 0x1E;

  while (1)
  {
    gamepad_state = check_gamepad();

    switch (player.state)
    {
      case STATE_WALK:
      case STATE_JUMP:
        walk(gamepad_state);
        break;
      case STATE_CLIMB:
        climb(gamepad_state);
        break;
    }

    setup_sprites();
    wait_vblank();
  }
}

void __fastcall__ load_palette(void)
{
  u8 loop;

  /* Write palette. */
  PPUADDR = 0x3f;
  PPUADDR = 0x00;

  for (loop = 0; loop < sizeof(pal); loop++)
  {
    PPUDATA = pal[loop];
  }
}

void __fastcall__ setup_sprites(void)
{
  static u8 sprite_index0;
  static u8 sprite_index1;
  static u8 sprite_index2;
  static u8 sprite_index3;

  sprite_index0 = player_sprite_index_table[(player.sprite_index << 2) + 0];
  sprite_index1 = player_sprite_index_table[(player.sprite_index << 2) + 1];
  sprite_index2 = player_sprite_index_table[(player.sprite_index << 2) + 2];
  sprite_index3 = player_sprite_index_table[(player.sprite_index << 2) + 3];

  if (player.sprite_mirrored == 0)
  {
    SPRITE(0)->x = fix2i(player.x);
    SPRITE(0)->y = fix2i(player.y);
    SPRITE(0)->attributes = 0;
    SPRITE(0)->index = sprite_index0;

    SPRITE(1)->x = fix2i(player.x) + 8;
    SPRITE(1)->y = fix2i(player.y);
    SPRITE(1)->attributes = 0;
    SPRITE(1)->index = sprite_index1;

    SPRITE(2)->x = fix2i(player.x);
    SPRITE(2)->y = fix2i(player.y) + 8;
    SPRITE(2)->attributes = 0;
    SPRITE(2)->index = sprite_index2;

    SPRITE(3)->x = fix2i(player.x) + 8;
    SPRITE(3)->y = fix2i(player.y) + 8;
    SPRITE(3)->attributes = 0;
    SPRITE(3)->index = sprite_index3;
  }
  else
  {
    SPRITE(0)->x = fix2i(player.x);
    SPRITE(0)->y = fix2i(player.y);
    SPRITE(0)->attributes = LEFT;
    SPRITE(0)->index = sprite_index1;

    SPRITE(1)->x = fix2i(player.x) + 8;
    SPRITE(1)->y = fix2i(player.y);
    SPRITE(1)->attributes = LEFT;
    SPRITE(1)->index = sprite_index0;

    SPRITE(2)->x = fix2i(player.x);
    SPRITE(2)->y = fix2i(player.y) + 8;
    SPRITE(2)->attributes = LEFT;
    SPRITE(2)->index = sprite_index3;

    SPRITE(3)->x = fix2i(player.x) + 8;
    SPRITE(3)->y = fix2i(player.y) + 8;
    SPRITE(3)->attributes = LEFT;
    SPRITE(3)->index = sprite_index2;
  }
}

void __fastcall__ load_tilemap(void)
{
  u8 loop;
  u8 tile_index;
  u8 col_cnt;
  u8* vram_ptr = vram_buf;

  col_cnt = 0;

  for (loop = 0; loop < sizeof(tilemap); loop++)
  {
    tile_index = tilemap[loop];

    /* Multpliy by 4 to get index in tiletable. */
    tile_index = tile_index << 2;

    *(vram_ptr+0)  = tiletable[tile_index+0];
    *(vram_ptr+1)  = tiletable[tile_index+1];
    *(vram_ptr+32) = tiletable[tile_index+2];
    *(vram_ptr+33) = tiletable[tile_index+3];

    col_cnt++;

    if (col_cnt == 16)
    {
      col_cnt = 0;
      vram_ptr += 34;
    }
    else
    {
      vram_ptr += 2;
    }
  }
}

void __fastcall__ copy_vram(void)
{
  u16 loop;

  PPUADDR = 0x20;
  PPUADDR = 0x00;

  for (loop = 0; loop < sizeof(vram_buf); loop++)
  {
    PPUDATA = vram_buf[loop];
  }
}

void __fastcall__ climb(u8 gamepad_state)
{
  static u16 tmp;
  static u8 flip_counter;

  if ((gamepad_state & PAD_A) || stairs_check() == 0)
  {
    player.state = STATE_WALK;
    player.sprite_index = PLAYER_SPRITE_STAND;
    return;
  }

  if (gamepad_state & PAD_UP)
  {
    player.vspeed = fixed(1, 127);
    player.dir = UP;

    flip_counter++;
  }
  else if (gamepad_state & PAD_DOWN)
  {
    player.vspeed = fixed(1, 127);
    player.dir = DOWN;

    flip_counter++;
  }

  player.sprite_mirrored = flip_counter & 8;

  if (player.vspeed > 0)
  {
    if (player.dir == UP)
    {
      tmp = player.y - player.vspeed;

      while (player.y > tmp)
      {
        if (colcheck_up())
        {
          break;
        }

        player.y -= fixed(1, 0);
      }
    }
    else
    {
      tmp = player.y + player.vspeed;

      while (player.y < tmp)
      {
        if (colcheck())
        {
          player.state = STATE_WALK;
          player.sprite_index = PLAYER_SPRITE_STAND;
          break;
        }

        player.y += fixed(1, 0);
      }
    }

    player.vspeed = 0;
  }

  if (gamepad_state & PAD_LEFT)
  {
    player.hspeed = fixed(1, 127);
    player.dir = LEFT;
  }
  else if (gamepad_state & PAD_RIGHT)
  {
    player.hspeed = fixed(1, 127);
    player.dir = RIGHT;
  }

  if (player.hspeed > 0)
  {
    if (player.dir == LEFT)
    {
      tmp = player.x - player.hspeed;

      while (player.x > tmp)
      {
        if (colcheck_left())
        {
          break;
        }

        player.x -= fixed(1, 0);
      }
    }
    else
    {
      tmp = player.x + player.hspeed;

      while (player.x < tmp)
      {
        if (colcheck_right())
        {
          break;
        }

        player.x += fixed(1, 0);
      }
    }

    player.hspeed = 0;
  }
}

void __fastcall__ walk(u8 gamepad_state)
{
  static u16 tmp;
  static u8 walk_counter = 0;

  /* Check if there's a ladder here. */
  if (gamepad_state & PAD_UP)
  {
    if (stairs_check())
    {
      player.hspeed = 0;
      player.vspeed = 0;
      player.state = STATE_CLIMB;
      player.dir = UP;
      player.sprite_mirrored = 0;
      player.sprite_index = PLAYER_SPRITE_CLIMB;
      return;
    }
  }

  if (gamepad_state & PAD_RIGHT)
  {
    player.hspeed = fixed(1, 127);
    player.dir = RIGHT;
    player.sprite_mirrored = 0;
  }
  else if (gamepad_state & PAD_LEFT)
  {
    player.hspeed = fixed(1, 127);
    player.dir = LEFT;
    player.sprite_mirrored = 1;
  }
  else
  {
    player.hspeed = 0;
  }

  if ((gamepad_state & PAD_A) && colcheck())
  {
    player.state = STATE_JUMP;
    player.vspeed = fixed(2, 0);
  }

  if (player.state == STATE_WALK)
  {
    if (!colcheck())
    {
      player.sprite_index = PLAYER_SPRITE_WALK;
      player.vspeed += gravity;
      player.vspeed = MIN(fixed(2, 0), player.vspeed);

      /* Update position. */
      tmp = player.y + player.vspeed;
      while (player.y < tmp)
      {
        player.y += fixed(1, 0);

        if (colcheck())
        {
          player.vspeed = 0;
          break;
        }
      }
    }
    else
    {
      player.vspeed = 0;
    }
  }
  else
  {
    player.sprite_index = PLAYER_SPRITE_WALK;
    player.vspeed -= gravity;

    tmp = player.y - player.vspeed;
    while (player.y > tmp)
    {
      if (colcheck_up())
      {
        player.vspeed = 0;
        player.state = STATE_WALK;
        break;
      }

      player.y -= fixed(1, 0);
    }

    if ((i16)player.vspeed <= 0)
    {
      player.vspeed = 0;
      player.state = STATE_WALK;
    }
  }

  /* Move player by speed. */
  if (player.dir == LEFT)
  {
    /*player.x -= player.hspeed;*/
    tmp = player.x - player.hspeed;
    while (player.x > tmp)
    {
      if (colcheck_left())
      {
        player.hspeed = 0;
        break;
      }

      player.x -= fixed(1, 0);
    }
  }
  else
  {
    /*player.x += player.hspeed;*/
    tmp = player.x + player.hspeed;
    while (player.x < tmp)
    {
      if (colcheck_right())
      {
        player.hspeed = 0;
        break;
      }

      player.x += fixed(1, 0);
    }
  }

  if (player.hspeed > 0 && player.vspeed == 0)
  {
    walk_counter++;
    if (walk_counter & 8) player.sprite_index = PLAYER_SPRITE_WALK;
    else player.sprite_index = PLAYER_SPRITE_STAND;
  }
  else if (player.hspeed == 0 && player.vspeed == 0)
  {
    player.sprite_index = PLAYER_SPRITE_STAND;
  }
}

u8 __fastcall__ colcheck(void)
{
  u8 tile_index_1 = (((fix2i(player.y)+17) >> 4) << 4) + ((fix2i(player.x) + 6) >> 4);
  u8 tile_index_2 = (((fix2i(player.y)+17) >> 4) << 4) + ((fix2i(player.x) + 10) >> 4);

  if (tilemap[tile_index_1] == 1) return 1;
  if (tilemap[tile_index_2] == 1) return 1;

  return 0;
}

u8 __fastcall__ colcheck_up(void)
{
  u8 tile_index_1 = (((fix2i(player.y)-1) >> 4) << 4) + ((fix2i(player.x) + 6) >> 4);
  u8 tile_index_2 = (((fix2i(player.y)-1) >> 4) << 4) + ((fix2i(player.x) + 10) >> 4);

  if (tilemap[tile_index_1] == 1) return 1;
  if (tilemap[tile_index_2] == 1) return 1;

  return 0;
}

u8 __fastcall__ colcheck_right(void)
{
  u8 tile_index_1 = (((fix2i(player.y)+2) >> 4) << 4) + ((fix2i(player.x) + 17) >> 4);
  u8 tile_index_2 = (((fix2i(player.y)+15) >> 4) << 4) + ((fix2i(player.x) + 17) >> 4);

  if (tilemap[tile_index_1] == 1) return 1;
  if (tilemap[tile_index_2] == 1) return 1;

  return 0;
}

u8 __fastcall__ colcheck_left(void)
{
  u8 tile_index_1 = (((fix2i(player.y)+2) >> 4) << 4) + ((fix2i(player.x) - 1) >> 4);
  u8 tile_index_2 = (((fix2i(player.y)+15) >> 4) << 4) + ((fix2i(player.x) - 1) >> 4);

  if (tilemap[tile_index_1] == 1) return 1;
  if (tilemap[tile_index_2] == 1) return 1;

  return 0;
}

u8 __fastcall__ stairs_check(void)
{
  u8 tile_index_1 = (((fix2i(player.y)) >> 4) << 4) + ((fix2i(player.x) + 6) >> 4);
  u8 tile_index_2 = (((fix2i(player.y)) >> 4) << 4) + ((fix2i(player.x) + 10) >> 4);

  if (tilemap[tile_index_1] == 2) return 1;
  if (tilemap[tile_index_2] == 2) return 1;

  return 0;
}

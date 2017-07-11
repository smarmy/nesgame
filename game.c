#include "asm.h"
#include "typedef.h"
#include "sprite.h"
#include "fixed.h"
#include "object.h"
#include "level.h"
#include "ppu.h"

#define MAX(A, B) (A > B ? A : B)
#define MIN(A, B) (A < B ? A : B)

static const u8 pal[] = 
{ 
  0x3f, 0x01, 0x11, 0x21, 0x3f, 0x01, 0x11, 0x21, 0x3f, 0x01, 0x11, 0x21, 0x3f, 0x01, 0x11, 0x21, /* background. */
  0x3f, 0x07, 0x26, 0x3d, 0x3f, 0x01, 0x11, 0x21, 0x3f, 0x01, 0x11, 0x21, 0x3f, 0x01, 0x11, 0x21, /* sprites. */
};

void __fastcall__ load_palette(void);
void __fastcall__ setup_sprites(void);
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

#define PLAYER_SPRITE_STAND 0
#define PLAYER_SPRITE_CLIMB 1
#define PLAYER_SPRITE_WALK  2

static const u8 gravity = 25;

void main()
{
  static u8 gamepad_state;

  /* Turn off PPU. */
  PPUCTRL = 0;
  PPUMASK = 0;

  objects.x[0] = fixed(128, 0);
  objects.y[0] = fixed(128, 0);
  objects.hspeed[0] = 0;
  objects.vspeed[0] = 0;
  objects.sprite_mirrored[0] = 0;
  objects.dir[0] = RIGHT;
  objects.state[0] = STATE_WALK;
  objects.sprite_index[0] = 0;
  num_objects++;

  load_palette();
  load_level(0);

  /* Reset scroll. */
  PPUSCROLL = 0;
  PPUSCROLL = 0;

  /* Turn on PPU. */
  PPUCTRL = 0x88;
  PPUMASK = 0x1E;

  while (1)
  {
    gamepad_state = check_gamepad();

    switch (objects.state[0])
    {
      case STATE_WALK:
      case STATE_JUMP:
        walk(gamepad_state);
        break;
      case STATE_CLIMB:
        climb(gamepad_state);
        break;
    }

    update_objects();
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

}

void __fastcall__ climb(u8 gamepad_state)
{
  static u16 tmp;
  static u8 flip_counter;

  if ((gamepad_state & PAD_A) || stairs_check() == 0)
  {
    objects.state[0] = STATE_WALK;
    objects.sprite_index[0] = PLAYER_SPRITE_STAND;
    return;
  }

  if (gamepad_state & PAD_UP)
  {
    objects.vspeed[0] = fixed(1, 127);
    objects.dir[0] = UP;

    flip_counter++;
  }
  else if (gamepad_state & PAD_DOWN)
  {
    objects.vspeed[0] = fixed(1, 127);
    objects.dir[0] = DOWN;

    flip_counter++;
  }

  objects.sprite_mirrored[0] = flip_counter & 8;

  if (objects.vspeed[0] > 0)
  {
    if (objects.dir[0] == UP)
    {
      tmp = objects.y[0] - objects.vspeed[0];

      while (objects.y[0] > tmp)
      {
        if (colcheck_up())
        {
          break;
        }

        objects.y[0] -= fixed(1, 0);
      }
    }
    else
    {
      tmp = objects.y[0] + objects.vspeed[0];

      while (objects.y[0] < tmp)
      {
        if (colcheck())
        {
          objects.state[0] = STATE_WALK;
          objects.sprite_index[0] = PLAYER_SPRITE_STAND;
          break;
        }

        objects.y[0] += fixed(1, 0);
      }
    }

    objects.vspeed[0] = 0;
  }

  if (gamepad_state & PAD_LEFT)
  {
    objects.hspeed[0] = fixed(1, 127);
    objects.dir[0] = LEFT;
  }
  else if (gamepad_state & PAD_RIGHT)
  {
    objects.hspeed[0] = fixed(1, 127);
    objects.dir[0] = RIGHT;
  }

  if (objects.hspeed[0] > 0)
  {
    if (objects.dir[0] == LEFT)
    {
      tmp = objects.x[0] - objects.hspeed[0];

      while (objects.x[0] > tmp)
      {
        if (colcheck_left())
        {
          break;
        }

        objects.x[0] -= fixed(1, 0);
      }
    }
    else
    {
      tmp = objects.x[0] + objects.hspeed[0];

      while (objects.x[0] < tmp)
      {
        if (colcheck_right())
        {
          break;
        }

        objects.x[0] += fixed(1, 0);
      }
    }

    objects.hspeed[0] = 0;
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
      objects.hspeed[0] = 0;
      objects.vspeed[0] = 0;
      objects.state[0] = STATE_CLIMB;
      objects.dir[0] = UP;
      objects.sprite_mirrored[0] = 0;
      objects.sprite_index[0] = PLAYER_SPRITE_CLIMB;
      return;
    }
  }

  if (gamepad_state & PAD_RIGHT)
  {
    objects.hspeed[0] = fixed(1, 127);
    objects.dir[0] = RIGHT;
    objects.sprite_mirrored[0] = 0;
  }
  else if (gamepad_state & PAD_LEFT)
  {
    objects.hspeed[0] = fixed(1, 127);
    objects.dir[0] = LEFT;
    objects.sprite_mirrored[0] = 1;
  }
  else
  {
    objects.hspeed[0] = 0;
  }

  if ((gamepad_state & PAD_A) && colcheck())
  {
    objects.state[0] = STATE_JUMP;
    objects.vspeed[0] = fixed(2, 0);
  }

  if (objects.state[0] == STATE_WALK)
  {
    if (!colcheck())
    {
      objects.sprite_index[0] = PLAYER_SPRITE_WALK;
      objects.vspeed[0] += gravity;
      objects.vspeed[0] = MIN(fixed(2, 0), objects.vspeed[0]);

      /* Update position. */
      tmp = objects.y[0] + objects.vspeed[0];
      while (objects.y[0] < tmp)
      {
        objects.y[0] += fixed(1, 0);

        if (colcheck())
        {
          objects.vspeed[0] = 0;
          break;
        }
      }
    }
    else
    {
      objects.vspeed[0] = 0;
    }
  }
  else
  {
    objects.sprite_index[0] = PLAYER_SPRITE_WALK;
    objects.vspeed[0] -= gravity;

    tmp = objects.y[0] - objects.vspeed[0];
    while (objects.y[0] > tmp)
    {
      if (colcheck_up())
      {
        objects.vspeed[0] = 0;
        objects.state[0] = STATE_WALK;
        break;
      }

      objects.y[0] -= fixed(1, 0);
    }

    if ((i16)objects.vspeed[0] <= 0)
    {
      objects.vspeed[0] = 0;
      objects.state[0] = STATE_WALK;
    }
  }

  /* Move player by speed. */
  if (objects.dir[0] == LEFT)
  {
    /*objects.x[0] -= objects.hspeed[0];*/
    tmp = objects.x[0] - objects.hspeed[0];
    while (objects.x[0] > tmp)
    {
      if (colcheck_left())
      {
        objects.hspeed[0] = 0;
        break;
      }

      objects.x[0] -= fixed(1, 0);
    }
  }
  else
  {
    /*objects.x[0] += objects.hspeed[0];*/
    tmp = objects.x[0] + objects.hspeed[0];
    while (objects.x[0] < tmp)
    {
      if (colcheck_right())
      {
        objects.hspeed[0] = 0;
        break;
      }

      objects.x[0] += fixed(1, 0);
    }
  }

  if (objects.hspeed[0] > 0 && objects.vspeed[0] == 0)
  {
    walk_counter++;
    if (walk_counter & 8) objects.sprite_index[0] = PLAYER_SPRITE_WALK;
    else objects.sprite_index[0] = PLAYER_SPRITE_STAND;
  }
  else if (objects.hspeed[0] == 0 && objects.vspeed[0] == 0)
  {
    objects.sprite_index[0] = PLAYER_SPRITE_STAND;
  }
}

u8 __fastcall__ colcheck(void)
{
  u8 tile_index_1 = (((fix2i(objects.y[0])+17) >> 4) << 4) + ((fix2i(objects.x[0]) + 6) >> 4);
  u8 tile_index_2 = (((fix2i(objects.y[0])+17) >> 4) << 4) + ((fix2i(objects.x[0]) + 10) >> 4);

  if (tilemap[tile_index_1] == 1) return 1;
  if (tilemap[tile_index_2] == 1) return 1;

  return 0;
}

u8 __fastcall__ colcheck_up(void)
{
  u8 tile_index_1 = (((fix2i(objects.y[0])-1) >> 4) << 4) + ((fix2i(objects.x[0]) + 6) >> 4);
  u8 tile_index_2 = (((fix2i(objects.y[0])-1) >> 4) << 4) + ((fix2i(objects.x[0]) + 10) >> 4);

  if (tilemap[tile_index_1] == 1) return 1;
  if (tilemap[tile_index_2] == 1) return 1;

  return 0;
}

u8 __fastcall__ colcheck_right(void)
{
  u8 tile_index_1 = (((fix2i(objects.y[0])+2) >> 4) << 4) + ((fix2i(objects.x[0]) + 17) >> 4);
  u8 tile_index_2 = (((fix2i(objects.y[0])+15) >> 4) << 4) + ((fix2i(objects.x[0]) + 17) >> 4);

  if (tilemap[tile_index_1] == 1) return 1;
  if (tilemap[tile_index_2] == 1) return 1;

  return 0;
}

u8 __fastcall__ colcheck_left(void)
{
  u8 tile_index_1 = (((fix2i(objects.y[0])+2) >> 4) << 4) + ((fix2i(objects.x[0]) - 1) >> 4);
  u8 tile_index_2 = (((fix2i(objects.y[0])+15) >> 4) << 4) + ((fix2i(objects.x[0]) - 1) >> 4);

  if (tilemap[tile_index_1] == 1) return 1;
  if (tilemap[tile_index_2] == 1) return 1;

  return 0;
}

u8 __fastcall__ stairs_check(void)
{
  u8 tile_index_1 = (((fix2i(objects.y[0])) >> 4) << 4) + ((fix2i(objects.x[0]) + 6) >> 4);
  u8 tile_index_2 = (((fix2i(objects.y[0])) >> 4) << 4) + ((fix2i(objects.x[0]) + 10) >> 4);

  if (tilemap[tile_index_1] == 2) return 1;
  if (tilemap[tile_index_2] == 2) return 1;

  return 0;
}

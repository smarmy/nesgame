#include "asm.h"
#include "typedef.h"
#include "sprite.h"
#include "fixed.h"
#include "object.h"
#include "level.h"
#include "ppu.h"
#include "colcheck.h"
#include "gamedata.h"

#define MAX(A, B) (A > B ? A : B)
#define MIN(A, B) (A < B ? A : B)

void __fastcall__ walk(u8 gamepad_state);
void __fastcall__ climb(u8 gamepad_state);

#define STATE_WALK  0
#define STATE_CLIMB 1
#define STATE_JUMP  2

#define PLAYER_SPRITE_STAND 0
#define PLAYER_SPRITE_CLIMB 1
#define PLAYER_SPRITE_WALK  2

static const u8 gravity = 25;
u8 keys = 0;

void main()
{
  static u8 tile_check_index;
  static u8 gamepad_state;

  /* Turn off PPU. */
  PPUCTRL = 0;
  PPUMASK = 0;

  create_object(O_PLAYER, 128, 128);
  create_object(O_BAT, 96, 112);

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

    /* Update player based on input. */
    switch (objects.state[O_PLAYER])
    {
      case STATE_WALK:
      case STATE_JUMP:
        walk(gamepad_state);
        break;
      case STATE_CLIMB:
        climb(gamepad_state);
        break;
    }

    /* Check collisions with special tiles. */
    tile_check_index = tile_check(O_PLAYER, TILE_KEY);
    if (tile_check_index != 0)
    {
      remove_tile(tile_check_index);
      keys++;
    }

    update_objects();
    wait_vblank();
  }
}

void __fastcall__ climb(u8 gamepad_state)
{
  static u16 tmp;
  static u8 flip_counter;

  if ((gamepad_state & PAD_A) || stairs_check(O_PLAYER) == 0)
  {
    objects.state[O_PLAYER] = STATE_WALK;
    objects.sprite_index[O_PLAYER] = PLAYER_SPRITE_STAND;
    return;
  }

  if (gamepad_state & PAD_UP)
  {
    objects.vspeed[O_PLAYER] = fixed(1, 127);
    objects.dir[O_PLAYER] = UP;

    flip_counter++;
  }
  else if (gamepad_state & PAD_DOWN)
  {
    objects.vspeed[O_PLAYER] = fixed(1, 127);
    objects.dir[O_PLAYER] = DOWN;

    flip_counter++;
  }

  objects.sprite_mirrored[O_PLAYER] = flip_counter & 8;

  if (objects.vspeed[O_PLAYER] > 0)
  {
    if (objects.dir[O_PLAYER] == UP)
    {
      tmp = objects.y[O_PLAYER] - objects.vspeed[O_PLAYER];

      while (objects.y[O_PLAYER] > tmp)
      {
        if (colcheck_up(O_PLAYER))
        {
          break;
        }

        objects.y[O_PLAYER] -= fixed(1, 0);
      }
    }
    else
    {
      tmp = objects.y[O_PLAYER] + objects.vspeed[O_PLAYER];

      while (objects.y[O_PLAYER] < tmp)
      {
        if (colcheck_down(O_PLAYER))
        {
          objects.state[O_PLAYER] = STATE_WALK;
          objects.sprite_index[O_PLAYER] = PLAYER_SPRITE_STAND;
          break;
        }

        objects.y[O_PLAYER] += fixed(1, 0);
      }
    }

    objects.vspeed[O_PLAYER] = 0;
  }

  if (gamepad_state & PAD_LEFT)
  {
    objects.hspeed[O_PLAYER] = fixed(1, 127);
    objects.dir[O_PLAYER] = LEFT;
  }
  else if (gamepad_state & PAD_RIGHT)
  {
    objects.hspeed[O_PLAYER] = fixed(1, 127);
    objects.dir[O_PLAYER] = RIGHT;
  }

  if (objects.hspeed[O_PLAYER] > 0)
  {
    if (objects.dir[O_PLAYER] == LEFT)
    {
      tmp = objects.x[O_PLAYER] - objects.hspeed[O_PLAYER];

      while (objects.x[O_PLAYER] > tmp)
      {
        if (colcheck_left(O_PLAYER))
        {
          break;
        }

        objects.x[O_PLAYER] -= fixed(1, 0);
      }
    }
    else
    {
      tmp = objects.x[O_PLAYER] + objects.hspeed[O_PLAYER];

      while (objects.x[O_PLAYER] < tmp)
      {
        if (colcheck_right(O_PLAYER))
        {
          break;
        }

        objects.x[O_PLAYER] += fixed(1, 0);
      }
    }

    objects.hspeed[O_PLAYER] = 0;
  }
}

void __fastcall__ walk(u8 gamepad_state)
{
  static u16 tmp;
  static u8 walk_counter = 0;

  /* Check if there's a ladder here. */
  if (gamepad_state & PAD_UP)
  {
    if (stairs_check(O_PLAYER))
    {
      objects.hspeed[O_PLAYER] = 0;
      objects.vspeed[O_PLAYER] = 0;
      objects.state[O_PLAYER] = STATE_CLIMB;
      objects.dir[O_PLAYER] = UP;
      objects.sprite_mirrored[O_PLAYER] = 0;
      objects.sprite_index[O_PLAYER] = PLAYER_SPRITE_CLIMB;
      return;
    }
  }

  if (gamepad_state & PAD_RIGHT)
  {
    objects.hspeed[O_PLAYER] = fixed(1, 127);
    objects.dir[O_PLAYER] = RIGHT;
    objects.sprite_mirrored[O_PLAYER] = 0;
  }
  else if (gamepad_state & PAD_LEFT)
  {
    objects.hspeed[O_PLAYER] = fixed(1, 127);
    objects.dir[O_PLAYER] = LEFT;
    objects.sprite_mirrored[O_PLAYER] = 1;
  }
  else
  {
    objects.hspeed[O_PLAYER] = 0;
  }

  if ((gamepad_state & PAD_A) && colcheck_down(O_PLAYER))
  {
    objects.state[O_PLAYER] = STATE_JUMP;
    objects.vspeed[O_PLAYER] = fixed(2, 0);
  }

  if (objects.state[O_PLAYER] == STATE_WALK)
  {
    if (!colcheck_down(O_PLAYER))
    {
      objects.sprite_index[O_PLAYER] = PLAYER_SPRITE_WALK;
      objects.vspeed[O_PLAYER] += gravity;
      objects.vspeed[O_PLAYER] = MIN(fixed(2, 0), objects.vspeed[O_PLAYER]);

      /* Update position. */
      tmp = objects.y[O_PLAYER] + objects.vspeed[O_PLAYER];
      while (objects.y[O_PLAYER] < tmp)
      {
        objects.y[O_PLAYER] += fixed(1, 0);

        if (colcheck_down(O_PLAYER))
        {
          objects.vspeed[O_PLAYER] = 0;
          break;
        }
      }
    }
    else
    {
      objects.vspeed[O_PLAYER] = 0;
    }
  }
  else
  {
    objects.sprite_index[O_PLAYER] = PLAYER_SPRITE_WALK;
    objects.vspeed[O_PLAYER] -= gravity;

    tmp = objects.y[O_PLAYER] - objects.vspeed[O_PLAYER];
    while (objects.y[O_PLAYER] > tmp)
    {
      if (colcheck_up(O_PLAYER))
      {
        objects.vspeed[O_PLAYER] = 0;
        objects.state[O_PLAYER] = STATE_WALK;
        break;
      }

      objects.y[O_PLAYER] -= fixed(1, 0);
    }

    if ((i16)objects.vspeed[O_PLAYER] <= 0)
    {
      objects.vspeed[O_PLAYER] = 0;
      objects.state[O_PLAYER] = STATE_WALK;
    }
  }

  /* Move player by speed. */
  if (objects.dir[O_PLAYER] == LEFT)
  {
    /*objects.x[O_PLAYER] -= objects.hspeed[O_PLAYER];*/
    tmp = objects.x[O_PLAYER] - objects.hspeed[O_PLAYER];
    while (objects.x[O_PLAYER] > tmp)
    {
      if (colcheck_left(O_PLAYER))
      {
        objects.hspeed[O_PLAYER] = 0;
        break;
      }

      objects.x[O_PLAYER] -= fixed(1, 0);
    }
  }
  else
  {
    /*objects.x[O_PLAYER] += objects.hspeed[O_PLAYER];*/
    tmp = objects.x[O_PLAYER] + objects.hspeed[O_PLAYER];
    while (objects.x[O_PLAYER] < tmp)
    {
      if (colcheck_right(O_PLAYER))
      {
        objects.hspeed[O_PLAYER] = 0;
        break;
      }

      objects.x[O_PLAYER] += fixed(1, 0);
    }
  }

  if (objects.hspeed[O_PLAYER] > 0 && objects.vspeed[O_PLAYER] == 0)
  {
    walk_counter++;
    if (walk_counter & 8) objects.sprite_index[O_PLAYER] = PLAYER_SPRITE_WALK;
    else objects.sprite_index[O_PLAYER] = PLAYER_SPRITE_STAND;
  }
  else if (objects.hspeed[O_PLAYER] == 0 && objects.vspeed[O_PLAYER] == 0)
  {
    objects.sprite_index[O_PLAYER] = PLAYER_SPRITE_STAND;
  }
}

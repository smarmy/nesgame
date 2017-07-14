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

static u8 __fastcall__ check_movement(u8 gamepad_state);
static void __fastcall__ walk(u8 gamepad_state);
static void __fastcall__ jump(u8 gamepad_state);
static void __fastcall__ climb(u8 gamepad_state);
static void __fastcall__ hurt(void);
static u8 __fastcall__ move_player_horiz(void);
static u8 __fastcall__ move_player_vertical(void);

#define PLAYER_STATE_WALK  0
#define PLAYER_STATE_CLIMB 1
#define PLAYER_STATE_JUMP  2
#define PLAYER_STATE_HURT  3

#define PLAYER_SPRITE_STAND 0
#define PLAYER_SPRITE_CLIMB 1
#define PLAYER_SPRITE_WALK  2
#define PLAYER_SPRITE_HURT  1

u8 keys = 0;
u8 current_level = 0;

void main()
{
  static u8 tile_check_index;
  static u8 gamepad_state;

  /* Turn off PPU. */
  PPUCTRL = 0;
  PPUMASK = 0;

  load_level(current_level);

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
      case PLAYER_STATE_WALK:
        walk(gamepad_state);
        break;
      case PLAYER_STATE_JUMP:
        jump(gamepad_state);
        break;
      case PLAYER_STATE_CLIMB:
        climb(gamepad_state);
        break;
      case PLAYER_STATE_HURT:
        hurt();
        break;
    }

    /* Check collisions with special tiles. */
    tile_check_index = tile_check(O_PLAYER, TILE_KEY);
    if (tile_check_index != 0)
    {
      play_sound(4, 0xC9);

      remove_tile(tile_check_index);
      keys++;
      goto end_of_update;
    }

    tile_check_index = tile_check(O_PLAYER, TILE_DOOR);
    if (tile_check_index != 0)
    {
      current_level++;

      /* Turn off PPU. */
      PPUCTRL = 0;
      PPUMASK = 0;

      load_level(current_level);

      /* Reset scroll. */
      PPUSCROLL = 0;
      PPUSCROLL = 0;

      /* Turn on PPU. */
      PPUCTRL = 0x88;
      PPUMASK = 0x1E;

      goto end_of_update;
    }

end_of_update:
    update_objects();
    wait_vblank();
  }
}

static u8 __fastcall__ check_movement(u8 gamepad_state)
{
  /* Check if there's a ladder here. */
  if (gamepad_state & PAD_UP)
  {
    if (stairs_check(O_PLAYER))
    {
      objects.hspeed[O_PLAYER] = 0;
      objects.vspeed[O_PLAYER] = 0;
      objects.state[O_PLAYER] = PLAYER_STATE_CLIMB;
      objects.hdir[O_PLAYER] = UP;
      objects.sprite_mirrored[O_PLAYER] = 0;
      objects.sprite_index[O_PLAYER] = PLAYER_SPRITE_CLIMB;
      return 1;
    }
  }

  if (gamepad_state & PAD_RIGHT)
  {
    objects.hspeed[O_PLAYER] = fixed(1, 127);
    objects.hdir[O_PLAYER] = RIGHT;
    objects.sprite_mirrored[O_PLAYER] = 0;
  }
  else if (gamepad_state & PAD_LEFT)
  {
    objects.hspeed[O_PLAYER] = fixed(1, 127);
    objects.hdir[O_PLAYER] = LEFT;
    objects.sprite_mirrored[O_PLAYER] = 1;
  }
  else
  {
    objects.hspeed[O_PLAYER] = 0;
  }

  return 0;
}

static void __fastcall__ climb(u8 gamepad_state)
{
  static u8 flip_counter;

  if ((gamepad_state & PAD_A) || stairs_check(O_PLAYER) == 0)
  {
    objects.state[O_PLAYER] = PLAYER_STATE_WALK;
    objects.sprite_index[O_PLAYER] = PLAYER_SPRITE_STAND;
    objects.vdir[O_PLAYER] = DOWN;
    return;
  }

  if (gamepad_state & PAD_UP)
  {
    objects.vspeed[O_PLAYER] = fixed(1, 127);
    objects.vdir[O_PLAYER] = UP;

    flip_counter++;
  }
  else if (gamepad_state & PAD_DOWN)
  {
    objects.vspeed[O_PLAYER] = fixed(1, 127);
    objects.vdir[O_PLAYER] = DOWN;

    flip_counter++;
  }

  objects.sprite_mirrored[O_PLAYER] = flip_counter & 8;

  if (objects.vspeed[O_PLAYER] > 0)
  {
    if (move_player_vertical() == 0 && objects.vdir[O_PLAYER] == DOWN)
    {
      objects.state[O_PLAYER] = PLAYER_STATE_WALK;
      objects.sprite_index[O_PLAYER] = PLAYER_SPRITE_STAND;
    }

    objects.vspeed[O_PLAYER] = 0;
  }

  if (gamepad_state & PAD_LEFT)
  {
    objects.hspeed[O_PLAYER] = fixed(1, 127);
    objects.hdir[O_PLAYER] = LEFT;
  }
  else if (gamepad_state & PAD_RIGHT)
  {
    objects.hspeed[O_PLAYER] = fixed(1, 127);
    objects.hdir[O_PLAYER] = RIGHT;
  }

  if (objects.hspeed[O_PLAYER] > 0)
  {
    move_player_horiz();
    objects.hspeed[O_PLAYER] = 0;
  }
}

static void __fastcall__ walk(u8 gamepad_state)
{
  static u8 walk_counter = 0;

  if (check_movement(gamepad_state))
    return;

  if ((gamepad_state & PAD_A) && colcheck_down(O_PLAYER))
  {
    objects.state[O_PLAYER] = PLAYER_STATE_JUMP;
    objects.vspeed[O_PLAYER] = fixed(2, 0);
    objects.vdir[O_PLAYER] = UP;
    return;
  }

  if (!colcheck_down(O_PLAYER))
  {
    objects.sprite_index[O_PLAYER] = PLAYER_SPRITE_WALK;
    objects.vspeed[O_PLAYER] += GRAVITY;
    objects.vspeed[O_PLAYER] = MIN(fixed(2, 0), objects.vspeed[O_PLAYER]);
    objects.vdir[O_PLAYER] = DOWN;

    /* Update position. */
    move_player_vertical();
  }
  else
  {
    objects.vspeed[O_PLAYER] = 0;
  }

  move_player_horiz();

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

static void __fastcall__ jump(u8 gamepad_state)
{
  /* Gamepad checks. */
  if (check_movement(gamepad_state))
    return;

  /* Physics. */
  objects.sprite_index[O_PLAYER] = PLAYER_SPRITE_WALK;
  objects.vspeed[O_PLAYER] -= GRAVITY;

  if (move_player_vertical() == 0)
  {
    objects.state[O_PLAYER] = PLAYER_STATE_WALK;
    objects.vdir[O_PLAYER] = DOWN;
  }

  if ((i16)objects.vspeed[O_PLAYER] <= 0)
  {
    objects.vspeed[O_PLAYER] = 0;
    objects.state[O_PLAYER] = PLAYER_STATE_WALK;
  }

  move_player_horiz();
}

static void __fastcall__ hurt(void)
{
  switch (objects.vdir[O_PLAYER])
  {
    case UP:
      break;
    case DOWN:
      break;
  }

  move_player_horiz();
}

/* Return 1 on collision. */
static u8 __fastcall__ move_player_horiz(void)
{
  static u16 tmp;

  /* Move player by speed. */
  if (objects.hdir[O_PLAYER] == LEFT)
  {
    tmp = objects.x[O_PLAYER] - objects.hspeed[O_PLAYER];
    while (objects.x[O_PLAYER] > tmp)
    {
      if (colcheck_left(O_PLAYER))
      {
        objects.hspeed[O_PLAYER] = 0;
        return 0;
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
        objects.hspeed[O_PLAYER] = 0;
        return 0;
      }

      objects.x[O_PLAYER] += fixed(1, 0);
    }
  }

  return 1;
}

/* Return 1 on collision or speed underflow. */
static u8 __fastcall__ move_player_vertical(void)
{
  static u16 tmp;

  if (objects.vdir[O_PLAYER] == DOWN)
  {
    tmp = objects.y[O_PLAYER] + objects.vspeed[O_PLAYER];
    while (objects.y[O_PLAYER] < tmp)
    {
      objects.y[O_PLAYER] += fixed(1, 0);

      if (colcheck_down(O_PLAYER))
      {
        objects.vspeed[O_PLAYER] = 0;
        return 0;
      }
    }
  }
  else
  {
    tmp = objects.y[O_PLAYER] - objects.vspeed[O_PLAYER];
    while (objects.y[O_PLAYER] > tmp)
    {
      if (colcheck_up(O_PLAYER))
      {
        objects.vspeed[O_PLAYER] = 0;
        return 0;
      }

      objects.y[O_PLAYER] -= fixed(1, 0);
    }
  }

  return 1;
}

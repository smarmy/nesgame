#include "asm.h"
#include "typedef.h"
#include "sprite.h"
#include "fixed.h"
#include "object.h"
#include "level.h"
#include "ppu.h"
#include "colcheck.h"
#include "gamedata.h"

static u8 __fastcall__ check_movement(u8 gamepad_state);
static void __fastcall__ walk(u8 gamepad_state);
static void __fastcall__ jump(u8 gamepad_state);
static void __fastcall__ climb(u8 gamepad_state);
static void __fastcall__ hurt(void);
static void __fastcall__ dead(void);
static u8 __fastcall__ move_player_horiz(void);
static u8 __fastcall__ move_player_vertical(void);
static void __fastcall__ check_object_collisions(void);
static u8 __fastcall__ transition(void);
static void __fastcall__ display_life(void);

#define PLAYER_STATE_WALK  0
#define PLAYER_STATE_CLIMB 1
#define PLAYER_STATE_JUMP  2
#define PLAYER_STATE_HURT  3
#define PLAYER_STATE_DEAD  4

#define PLAYER_SPRITE_STAND 0
#define PLAYER_SPRITE_CLIMB 4
#define PLAYER_SPRITE_WALK  8
#define PLAYER_SPRITE_HURT  4

static u8 jump_button_pressed = 0;
static u8 jumps = 1;

u8 keys = 0;
u8 current_level = 0;
u8 max_jumps = 1;
u8 player_life = 3;

static void reset()
{
  keys = 0;
  current_level = 0;
  max_jumps = 1;
  player_life = 3;
  num_objects = 0;

  /* Turn off PPU. */
  PPUCTRL = 0;
  PPUMASK = 0;

  create_object(O_PLAYER, 80, 176);
  load_level(current_level);

  /* Reset scroll. */
  PPUSCROLL = 0;
  PPUSCROLL = 0;

  /* Turn on PPU. */
  PPUCTRL = 0x88;
  PPUMASK = 0x1E;
}

void main()
{
  static u8 tile_check_index;
  static u8 gamepad_state;

  /* Turn off PPU. */
  PPUCTRL = 0;
  PPUMASK = 0;

  create_object(O_PLAYER, 80, 176);
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
      case PLAYER_STATE_DEAD:
        dead();
        goto end_of_update;
    }

    if (transition() == 0)
    {
      goto end_of_update;
    }

    check_object_collisions();

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
    display_life();
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
      objects.sprite_attribute[O_PLAYER] &= ~ATTR_MIRRORED;
      objects.sprite_index[O_PLAYER] = PLAYER_SPRITE_CLIMB;
      return 1;
    }
  }

  if (gamepad_state & PAD_RIGHT)
  {
    objects.hspeed[O_PLAYER] = fixed(1, 127);
    objects.hdir[O_PLAYER] = RIGHT;
    objects.sprite_attribute[O_PLAYER] &= ~ATTR_MIRRORED;
  }
  else if (gamepad_state & PAD_LEFT)
  {
    objects.hspeed[O_PLAYER] = fixed(1, 127);
    objects.hdir[O_PLAYER] = LEFT;
    objects.sprite_attribute[O_PLAYER] |= ATTR_MIRRORED;
  }
  else
  {
    objects.hspeed[O_PLAYER] = 0;
  }

  /* Check jumping. */
  if ((gamepad_state & PAD_A))
  {
    static u8 allow_jump;
    static u8 collision;

    collision = colcheck_down(O_PLAYER);

    if (jumps == 0 && collision == 0)
      allow_jump = 0;
    else if (collision == 0 && max_jumps == 2 && jumps > 0)
      allow_jump = 1;
    else if (collision == 1)
      allow_jump = 1;
    else
      allow_jump = 0;

    if (jump_button_pressed == 0 && allow_jump == 1)
    {
      jump_button_pressed = 1;
      jumps--;

      objects.state[O_PLAYER] = PLAYER_STATE_JUMP;
      objects.vspeed[O_PLAYER] = fixed(2, 0);
      objects.vdir[O_PLAYER] = UP;
      return 1;
    }
    else if (jump_button_pressed == 0)
    {
      jump_button_pressed = 1;
    }
  }
  else
  {
    jump_button_pressed = 0;
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

  if (flip_counter & 8)
    objects.sprite_attribute[O_PLAYER] |= ATTR_MIRRORED;
  else
    objects.sprite_attribute[O_PLAYER] &= ~ATTR_MIRRORED;

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

  if (!colcheck_down(O_PLAYER))
  {
    if (jumps > 1)
      jumps--;

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
    jumps = max_jumps;
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
  objects.hspeed[O_PLAYER] = fixed(1, 127);

  if (objects.vdir[O_PLAYER] == UP)
  {
    objects.vspeed[O_PLAYER] -= GRAVITY;
    objects.vspeed[O_PLAYER] -= GRAVITY;
    objects.vspeed[O_PLAYER] -= GRAVITY;

    if ((i16)objects.vspeed[O_PLAYER] <= 0)
    {
      objects.vspeed[O_PLAYER] = 0;
      objects.vdir[O_PLAYER] = DOWN;
    }
  }
  else
  {
    objects.vspeed[O_PLAYER] += GRAVITY;

    objects.vspeed[O_PLAYER] = MIN(fixed(2, 0), objects.vspeed[O_PLAYER]);
  }

  if (move_player_vertical() == 0)
  {
    if (objects.vdir[O_PLAYER] == UP)
    {
      objects.vdir[O_PLAYER] = DOWN;
    }
    else
    {
      if (player_life > 0)
      {
        objects.state[O_PLAYER] = PLAYER_STATE_WALK;
      }
      else
      {
        kill_player();
      }
    }
  }

  move_player_horiz();
}

static void __fastcall__ dead(void)
{
  objects.counter[O_PLAYER]--;
  if (objects.counter[O_PLAYER] == 0)
  {
    /* TODO: reset to checkpoint, when that is implemeted. */
    reset();
  }
}

/* Return 0 on collision. */
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

/* Return 0 on collision. */
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

void __fastcall__ hurt_player(u8 hdir)
{
  if (objects.state[O_PLAYER] == PLAYER_STATE_HURT)
    return;

  objects.state[O_PLAYER] = PLAYER_STATE_HURT;
  objects.vspeed[O_PLAYER] = fixed(3, 0);
  objects.hspeed[O_PLAYER] = fixed(1, 127);
  objects.vdir[O_PLAYER] = UP;
  objects.hdir[O_PLAYER] = hdir;

  player_life--;

  play_sound(2, 0xAA);
}

void __fastcall__ kill_player(void)
{
  objects.state[O_PLAYER] = PLAYER_STATE_DEAD;
  objects.sprite_index[O_PLAYER] = 34;
  objects.counter[O_PLAYER] = 64;
  player_life = 0;
}

static void __fastcall__ check_object_collisions(void)
{
  static u8 i;

  for (i = 1; i < num_objects; i++)
  {
    switch (objects.type[i])
    {
      case O_BAT:
      case O_SKELETON:
      case O_BONE:
      case O_FLAME:
        if (colcheck_objects(O_PLAYER, i))
        {
          hurt_player(objects.x[i] > objects.x[O_PLAYER]);
        }
        break;
    }
  }
}

static u8 __fastcall__ transition(void)
{
  static u8 x, y;

  x = fix2i(objects.x[O_PLAYER]);
  y = fix2i(objects.y[O_PLAYER]);

  if (x > 240)
  {
    current_level++;
    objects.x[O_PLAYER] = fixed(16, 0);
  }
  else if (x < 8)
  {
    current_level--;
    objects.x[O_PLAYER] = fixed(240, 0);
  }
  else if (y > 230)
  {
    current_level += LEVELS_PER_ROW;
    objects.y[O_PLAYER] = fixed(16, 0);
  }
  else if (y < 16)
  {
    if (current_level < 6)
      return 1;

    current_level -= LEVELS_PER_ROW;
    objects.y[O_PLAYER] = fixed(230, 0);
  }
  else
  {
    return 1;
  }

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

  return 0;
}

static void __fastcall__ display_life(void)
{
  static u8 i;
  static const u8 life_first_sprite = 0x35;
  static sprite_t* sprite;

  for (i = 0; i < MAX_LIFE; i++)
  {
    sprite = SPRITE(life_first_sprite + i);
    sprite->attributes = 0;
    sprite->index = 33;

    if (i < player_life)
    {
      sprite->x = i << 3;
      sprite->y = 8;
    }
    else
    {
      sprite->y = 0;
    }
  }
}

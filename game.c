#include "asm.h"
#include "typedef.h"
#include "sprite.h"
#include "fixed.h"
#include "object.h"
#include "level.h"
#include "ppu.h"
#include "colcheck.h"
#include "gamedata.h"
#include "text.h"

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

/* Defined in assembly. */
void __fastcall__ display_life(void);

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
static u8 bullet_counter = 0;

static u8 fall_delay = 0;

u8 current_level = 0;
u8 max_jumps = 1;
u8 player_life = 3;
u8 num_bullets = 0;
u8 has_gun = 0;
u8 treasure_states[9] = {0,0,0,0,0,0,0,0,0};
u8 checkpoint_level = 0;
u8 checkpoint_x = 0;
u8 checkpoint_y = 0;

static void reset()
{
  treasure_states[0] = 0;
  treasure_states[1] = 0;
  treasure_states[2] = 0;
  treasure_states[3] = 0;
  treasure_states[4] = 0;
  treasure_states[5] = 0;
  treasure_states[6] = 0;
  treasure_states[7] = 0;
  treasure_states[8] = 0;
  treasure_states[9] = 0;

  current_level = 0;
  max_jumps = 1;
  player_life = 3;
  num_objects = 0;
  num_bullets = 0;
  has_gun = 0;

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

static void reset_checkpoint()
{
  u8 i;

  if (checkpoint_level == 0)
  {
    reset();
    return;
  }

  current_level = checkpoint_level;

  player_life = 3;
  num_objects = 0;
  num_bullets = 0;

  /* Turn off PPU. */
  PPUCTRL = 0;
  PPUMASK = 0;

  create_object(O_PLAYER, checkpoint_x, checkpoint_y);
  load_level(current_level);

  /* Remove checkpoints when reseting. */
  for (i = 0; i < num_objects; i++)
  {
    if (objects_type[i] == O_CHECKPOINT)
    {
      remove_object(i);
    }
  }

  /* Reset scroll. */
  PPUSCROLL = 0;
  PPUSCROLL = 0;

  /* Turn on PPU. */
  PPUCTRL = 0x88;
  PPUMASK = 0x1E;
}

static void titlescreen()
{
  u8 i;

  /* Turn off PPU. */
  PPUCTRL = 0;
  PPUMASK = 0;

  /* set up a palette. */
  ppuwrite(0x3f00, 0x0d);
  ppuwrite(0x3f01, 0x00);
  ppuwrite(0x3f02, 0x10);
  ppuwrite(0x3f03, 0x20);

  /* clear nametable. */
  PPUADDR = 0x23;
  PPUADDR = 0xC0;
  for (i = 0; i < 64; i++)
  {
    PPUDATA = 0;
  }

  print_text_2(0, 1,   "********************************");
  print_text_2(0, 2,   "*                              *");
  print_text_2(0, 3,   "*                              *");
  print_text_2(0, 4,   "*                              *");
  print_text_2(0, 5,   "*                              *");
  print_text_2(0, 6,   "*                              *");
  print_text_2(0, 7,   "*                              *");
  print_text_2(0, 8,   "*                              *");
  print_text_2(0, 9,   "*                              *");
  print_text_2(0, 10,  "*                              *");
  print_text_2(0, 11,  "*                              *");
  print_text_2(0, 12,  "*                              *");
  print_text_2(0, 13,  "*                              *");
  print_text_2(0, 14,  "*     WELCOME TREASURE MAN     *");
  print_text_2(0, 15,  "*                              *");
  print_text_2(0, 16,  "*                              *");
  print_text_2(0, 17,  "*         PRESS START          *");
  print_text_2(0, 18,  "*                              *");
  print_text_2(0, 19,  "*                              *");
  print_text_2(0, 20,  "*                              *");
  print_text_2(0, 21,  "*                              *");
  print_text_2(0, 22,  "*                              *");
  print_text_2(0, 23,  "*                              *");
  print_text_2(0, 24,  "*                              *");
  print_text_2(0, 25,  "*                              *");
  print_text_2(0, 26,  "* MADE FOR AWFUL GAME JAM 2017 *");
  print_text_2(0, 27,  "*                              *");
  print_text_2(0, 28,  "********************************");

  /* Reset scroll. */
  PPUSCROLL = 0;
  PPUSCROLL = 0;

  /* Turn on PPU. */
  PPUCTRL = 0x88;
  PPUMASK = 0x1E;

  while (1)
  {
    static u8 gamepad_state;
    gamepad_state = check_gamepad();

    if (gamepad_state & PAD_START)
    {
      break;
    }
  }
}

static void winscreen()
{
  u8 i;

  /* Turn off PPU. */
  PPUCTRL = 0;
  PPUMASK = 0;

  /* set up a palette. */
  ppuwrite(0x3f00, 0x0d);
  ppuwrite(0x3f01, 0x00);
  ppuwrite(0x3f02, 0x10);
  ppuwrite(0x3f03, 0x20);

  /* clear nametable. */
  PPUADDR = 0x23;
  PPUADDR = 0xC0;
  for (i = 0; i < 64; i++)
  {
    PPUDATA = 0;
  }

  print_text_2(0, 1,   "********************************");
  print_text_2(0, 2,   "*                              *");
  print_text_2(0, 3,   "*                              *");
  print_text_2(0, 4,   "*                              *");
  print_text_2(0, 5,   "*                              *");
  print_text_2(0, 6,   "*                              *");
  print_text_2(0, 7,   "*                              *");
  print_text_2(0, 8,   "*                              *");
  print_text_2(0, 9,   "*                              *");
  print_text_2(0, 10,  "*                              *");
  print_text_2(0, 11,  "*     ALL TREASURES HAS BEEN   *");
  print_text_2(0, 12,  "*     COLLECTED.               *");
  print_text_2(0, 13,  "*                              *");
  print_text_2(0, 14,  "*     PEACE IS RESTORED AT     *");
  print_text_2(0, 15,  "*     LAST.                    *");
  print_text_2(0, 16,  "*                              *");
  print_text_2(0, 17,  "*                              *");
  print_text_2(0, 18,  "*                              *");
  print_text_2(0, 19,  "*                              *");
  print_text_2(0, 20,  "*                              *");
  print_text_2(0, 21,  "*                              *");
  print_text_2(0, 22,  "*                              *");
  print_text_2(0, 23,  "*                              *");
  print_text_2(0, 24,  "*                              *");
  print_text_2(0, 25,  "*                              *");
  print_text_2(0, 26,  "*                              *");
  print_text_2(0, 27,  "*                              *");
  print_text_2(0, 28,  "********************************");

  /* Reset scroll. */
  PPUSCROLL = 0;
  PPUSCROLL = 0;

  /* Turn on PPU. */
  PPUCTRL = 0x88;
  PPUMASK = 0x1E;

  clear_sprites();

  for (i = 0; i < MAX_TREASURES; i++)
  {
    treasure_states[i] = 0;
  }

  num_objects = 0;
  create_object(O_TREASURE_1, 16, 16);
  create_object(O_TREASURE_2, 48, 32);
  create_object(O_TREASURE_3, 64, 200);
  create_object(O_TREASURE_4, 197, 133);
  create_object(O_TREASURE_5, 230, 75);
  create_object(O_TREASURE_7, 150, 150);
  create_object(O_TREASURE_8, 199, 121);
  create_object(O_TREASURE_9, 45, 140);

  while (1)
  {
    update_objects();
    wait_vblank();
  }
}

void main()
{
  static u8 gamepad_state;

  clear_sprites();
  titlescreen();
  reset();

  while (1)
  {
    gamepad_state = check_gamepad();

    if (bullet_counter > 0)
      bullet_counter--;
    if (fall_delay > 1)
      fall_delay--;

    /* Update player based on input. */
    switch (objects_state[O_PLAYER])
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

end_of_update:
    clear_sprites();
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
      objects_hspeed[O_PLAYER] = 0;
      objects_vspeed[O_PLAYER] = 0;
      objects_state[O_PLAYER] = PLAYER_STATE_CLIMB;
      objects_hdir[O_PLAYER] = UP;
      objects_sprite_attribute[O_PLAYER] &= ~ATTR_MIRRORED;
      objects_sprite_index[O_PLAYER] = PLAYER_SPRITE_CLIMB;
      return 1;
    }
  }

  if (gamepad_state & PAD_RIGHT)
  {
    objects_hspeed[O_PLAYER] = fixed(1, 127);
    objects_hdir[O_PLAYER] = RIGHT;
    objects_sprite_attribute[O_PLAYER] &= ~ATTR_MIRRORED;
  }
  else if (gamepad_state & PAD_LEFT)
  {
    objects_hspeed[O_PLAYER] = fixed(1, 127);
    objects_hdir[O_PLAYER] = LEFT;
    objects_sprite_attribute[O_PLAYER] |= ATTR_MIRRORED;
  }
  else
  {
    objects_hspeed[O_PLAYER] = 0;
  }

  if ((gamepad_state & PAD_B) && has_gun == 1 && bullet_counter == 0 && num_bullets < 3)
  {
    objects_sprite_index[O_PLAYER] = 39;
    if (objects_state[O_PLAYER] == PLAYER_SPRITE_WALK)
      objects_hspeed[O_PLAYER] = 0;

    create_object(O_BULLET, fix2i(objects_x[O_PLAYER])+8, fix2i(objects_y[O_PLAYER])+4);
    objects_hdir[num_objects-1] = objects_hdir[O_PLAYER];
    bullet_counter = 8;

    play_sound(4, 0xB0);

    return 1;
  }

  /* Check jumping. */
  if ((gamepad_state & PAD_A))
  {
    static u8 allow_jump;
    static u8 collision;

    collision = colcheck_down(O_PLAYER) || fall_delay > 1;

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
      fall_delay = 1;

      jump_button_pressed = 1;
      jumps--;

      play_sound(4, 0xFE);

      objects_state[O_PLAYER] = PLAYER_STATE_JUMP;
      objects_vspeed[O_PLAYER] = fixed(2, 0);
      objects_vdir[O_PLAYER] = UP;
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
    objects_state[O_PLAYER] = PLAYER_STATE_WALK;
    objects_sprite_index[O_PLAYER] = PLAYER_SPRITE_STAND;
    objects_vdir[O_PLAYER] = DOWN;
    return;
  }

  if (gamepad_state & PAD_UP)
  {
    objects_vspeed[O_PLAYER] = fixed(1, 127);
    objects_vdir[O_PLAYER] = UP;

    flip_counter++;

    /*if (flip_counter & 8) play_sound(4, 0x80);*/
  }
  else if (gamepad_state & PAD_DOWN)
  {
    objects_vspeed[O_PLAYER] = fixed(1, 127);
    objects_vdir[O_PLAYER] = DOWN;

    flip_counter++;

    /*if (flip_counter & 8) play_sound(4, 0x80);*/
  }

  if (flip_counter & 8)
    objects_sprite_attribute[O_PLAYER] |= ATTR_MIRRORED;
  else
    objects_sprite_attribute[O_PLAYER] &= ~ATTR_MIRRORED;

  if (objects_vspeed[O_PLAYER] > 0)
  {
    if (move_player_vertical() == 0 && objects_vdir[O_PLAYER] == DOWN)
    {
      objects_state[O_PLAYER] = PLAYER_STATE_WALK;
      objects_sprite_index[O_PLAYER] = PLAYER_SPRITE_STAND;
    }

    objects_vspeed[O_PLAYER] = 0;
  }

  if (gamepad_state & PAD_LEFT)
  {
    objects_hspeed[O_PLAYER] = fixed(1, 127);
    objects_hdir[O_PLAYER] = LEFT;
  }
  else if (gamepad_state & PAD_RIGHT)
  {
    objects_hspeed[O_PLAYER] = fixed(1, 127);
    objects_hdir[O_PLAYER] = RIGHT;
  }

  if (objects_hspeed[O_PLAYER] > 0)
  {
    move_player_horiz();
    objects_hspeed[O_PLAYER] = 0;
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

    objects_sprite_index[O_PLAYER] = PLAYER_SPRITE_WALK;
    objects_vspeed[O_PLAYER] += GRAVITY;
    objects_vspeed[O_PLAYER] = MIN(fixed(2, 0), objects_vspeed[O_PLAYER]);
    objects_vdir[O_PLAYER] = DOWN;

    /* Update position. */
    move_player_vertical();
  }
  else
  {
    objects_vspeed[O_PLAYER] = 0;
    jumps = max_jumps;
  }

  move_player_horiz();

  if (objects_hspeed[O_PLAYER] > 0 && objects_vspeed[O_PLAYER] == 0)
  {
    walk_counter++;
    if (walk_counter & 8) objects_sprite_index[O_PLAYER] = PLAYER_SPRITE_WALK;
    else objects_sprite_index[O_PLAYER] = PLAYER_SPRITE_STAND;
  }
  else if (objects_hspeed[O_PLAYER] == 0 && objects_vspeed[O_PLAYER] == 0)
  {
    objects_sprite_index[O_PLAYER] = PLAYER_SPRITE_STAND;
  }
}

static void __fastcall__ jump(u8 gamepad_state)
{
  /* Gamepad checks. */
  if (check_movement(gamepad_state))
    return;

  /* Physics. */
  objects_sprite_index[O_PLAYER] = PLAYER_SPRITE_WALK;
  objects_vspeed[O_PLAYER] -= GRAVITY;

  if (move_player_vertical() == 0)
  {
    objects_state[O_PLAYER] = PLAYER_STATE_WALK;
    objects_vdir[O_PLAYER] = DOWN;
  }

  if ((i16)objects_vspeed[O_PLAYER] <= 0)
  {
    objects_vspeed[O_PLAYER] = 0;
    objects_state[O_PLAYER] = PLAYER_STATE_WALK;
  }

  move_player_horiz();
}

static void __fastcall__ hurt(void)
{
  objects_hspeed[O_PLAYER] = fixed(1, 127);

  if (objects_vdir[O_PLAYER] == UP)
  {
    objects_vspeed[O_PLAYER] -= GRAVITY;
    objects_vspeed[O_PLAYER] -= GRAVITY;
    objects_vspeed[O_PLAYER] -= GRAVITY;

    if ((i16)objects_vspeed[O_PLAYER] <= 0)
    {
      objects_vspeed[O_PLAYER] = 0;
      objects_vdir[O_PLAYER] = DOWN;
    }
  }
  else
  {
    objects_vspeed[O_PLAYER] += GRAVITY;

    objects_vspeed[O_PLAYER] = MIN(fixed(2, 0), objects_vspeed[O_PLAYER]);
  }

  if (move_player_vertical() == 0)
  {
    if (objects_vdir[O_PLAYER] == UP)
    {
      objects_vdir[O_PLAYER] = DOWN;
    }
    else
    {
      if (player_life > 0)
      {
        objects_state[O_PLAYER] = PLAYER_STATE_WALK;
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
  objects_counter[O_PLAYER]--;
  if (objects_counter[O_PLAYER] == 0)
  {
    reset_checkpoint();
  }
}

/* Return 0 on collision. */
static u8 __fastcall__ move_player_horiz(void)
{
  static u16 tmp;

  /* Move player by speed. */
  if (objects_hdir[O_PLAYER] == LEFT)
  {
    tmp = objects_x[O_PLAYER] - objects_hspeed[O_PLAYER];
    while (objects_x[O_PLAYER] > tmp)
    {
      if (colcheck_left(O_PLAYER))
      {
        objects_hspeed[O_PLAYER] = 0;
        return 0;
      }

      objects_x[O_PLAYER] -= fixed(1, 0);
    }
  }
  else
  {
    tmp = objects_x[O_PLAYER] + objects_hspeed[O_PLAYER];
    while (objects_x[O_PLAYER] < tmp)
    {
      if (colcheck_right(O_PLAYER))
      {
        objects_hspeed[O_PLAYER] = 0;
        return 0;
      }

      objects_x[O_PLAYER] += fixed(1, 0);
    }
  }

  return 1;
}

/* Return 0 on collision. */
static u8 __fastcall__ move_player_vertical(void)
{
  static u16 tmp;

  if (objects_vdir[O_PLAYER] == DOWN)
  {
    if (fall_delay == 0)
      fall_delay = 7;

    tmp = objects_y[O_PLAYER] + objects_vspeed[O_PLAYER];
    while (objects_y[O_PLAYER] < tmp)
    {
      objects_y[O_PLAYER] += fixed(1, 0);

      if (colcheck_down(O_PLAYER))
      {
        objects_vspeed[O_PLAYER] = 0;
        fall_delay = 0;
        return 0;
      }
    }
  }
  else
  {
    tmp = objects_y[O_PLAYER] - objects_vspeed[O_PLAYER];
    while (objects_y[O_PLAYER] > tmp)
    {
      if (colcheck_up(O_PLAYER))
      {
        objects_vspeed[O_PLAYER] = 0;
        return 0;
      }

      objects_y[O_PLAYER] -= fixed(1, 0);
    }
  }

  return 1;
}

void __fastcall__ hurt_player(u8 hdir)
{
  if (objects_state[O_PLAYER] == PLAYER_STATE_HURT)
    return;

  objects_state[O_PLAYER] = PLAYER_STATE_HURT;
  objects_vspeed[O_PLAYER] = fixed(3, 0);
  objects_hspeed[O_PLAYER] = fixed(1, 127);
  objects_vdir[O_PLAYER] = UP;
  objects_hdir[O_PLAYER] = hdir;

  player_life--;

  play_sound(2, 0xAA);
}

void __fastcall__ kill_player(void)
{
  objects_state[O_PLAYER] = PLAYER_STATE_DEAD;
  objects_sprite_index[O_PLAYER] = 34;
  objects_counter[O_PLAYER] = 64;
  player_life = 0;
}

static void __fastcall__ check_object_collisions(void)
{
  static u8 i;

  for (i = 1; i < num_objects; i++)
  {
    switch (objects_type[i])
    {
      case O_BAT:
      case O_SKELETON:
      case O_BONE:
      case O_FLAME:
      case O_SPIKEBALL:
        if (colcheck_objects(O_PLAYER, i))
        {
          hurt_player(objects_x[i] > objects_x[O_PLAYER]);
        }
        break;
      case O_DOUBLEJUMP:
        if (colcheck_objects(O_PLAYER, i))
        {
          max_jumps = 2;
          print_text(2, 8, "YOU FOUND DOUBLE JUMP!!");
          play_sound(4, 0xC9);
        }
        break;
      case O_GUN:
        if (colcheck_objects(O_PLAYER, i))
        {
          has_gun = 1;
          print_text(2, 8, "YOU FOUND GUN!!");
          play_sound(4, 0xC9);
        }
        break;
      case O_CHECKPOINT:
        if (colcheck_objects(O_PLAYER, i))
        {
          remove_object(i);
          print_text(2, 8, "CHECKPOINT REACHED!!");
          checkpoint_level = current_level;
          checkpoint_x = fix2i(objects_x[O_PLAYER]);
          checkpoint_y = fix2i(objects_y[O_PLAYER]);
        }
        break;
      case O_TREASURE_1:
        if (colcheck_objects(O_PLAYER, i))
        {
          static char remaining_treasures[2];
          static u8 treasure_count;
          static u8 loop_c;

          u8 treasure_index = objects_state[i];
          treasure_states[treasure_index] = 1;

          treasure_count = 0;
          for (loop_c = 0; loop_c < MAX_TREASURES; loop_c++)
          {
            if (treasure_states[loop_c] == 1) treasure_count++;
          }

          if (treasure_count == MAX_TREASURES)
          {
            winscreen();
          }
          else
          {
            play_sound(4, 0xC9);
          }

          remaining_treasures[0] = '0' + (MAX_TREASURES - treasure_count);
          remaining_treasures[1] = 0;

          print_text(2, 8, "YOU GOT TREASURE!!");
          wait_vblank();
          print_text(2, 9, "REMAINING:");
          wait_vblank();
          print_text(14, 9, remaining_treasures);
          wait_vblank();
        }
        break;
    }
  }
}

static u8 __fastcall__ transition(void)
{
  static u8 x, y;

  x = fix2i(objects_x[O_PLAYER]);
  y = fix2i(objects_y[O_PLAYER]);

  if (x > 240)
  {
    current_level++;
    objects_x[O_PLAYER] = fixed(16, 0);
  }
  else if (x < 8)
  {
    current_level--;
    objects_x[O_PLAYER] = fixed(240, 0);
  }
  else if (y > 216)
  {
    current_level += LEVELS_PER_ROW;
    objects_y[O_PLAYER] = fixed(16, 0);
  }
  else if (y < 16)
  {
    if (current_level < 6)
      return 1;

    current_level -= LEVELS_PER_ROW;
    objects_y[O_PLAYER] = fixed(216, 0);
  }
  else
  {
    return 1;
  }

  /* Turn off PPU. */
  PPUCTRL = 0;
  PPUMASK = 0;

  num_bullets = 0;
  load_level(current_level);

  /* Reset scroll. */
  PPUSCROLL = 0;
  PPUSCROLL = 0;

  /* Turn on PPU. */
  PPUCTRL = 0x88;
  PPUMASK = 0x1E;

  return 0;
}

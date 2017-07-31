#include "asm.h"
#include "sprite.h"
#include "object.h"
#include "level.h"
#include "gamedata.h"
#include "colcheck.h"

fixed_t objects_x[MAX_OBJECTS];
fixed_t objects_y[MAX_OBJECTS];
fixed_t objects_hspeed[MAX_OBJECTS];
fixed_t objects_vspeed[MAX_OBJECTS];
u8 objects_sprite_attribute[MAX_OBJECTS];
u8 objects_hdir[MAX_OBJECTS];
u8 objects_vdir[MAX_OBJECTS];
u8 objects_bbox_x1[MAX_OBJECTS];
u8 objects_bbox_y1[MAX_OBJECTS];
u8 objects_bbox_x2[MAX_OBJECTS];
u8 objects_bbox_y2[MAX_OBJECTS];
u8 objects_life[MAX_OBJECTS];
u8 objects_state[MAX_OBJECTS];
u8 objects_counter[MAX_OBJECTS];
u8 objects_sprite_index[MAX_OBJECTS];
u8 objects_type[MAX_OBJECTS];

u8 num_objects = 0;

static u8 spritenum;
static u8 need_cleanup = 0;

static void __fastcall__ update_sprite(u8 index)
{
  static sprite_t* sprite;
  static u8 sprite_index;
  static u8 x, y;

  sprite_index = objects_sprite_index[index];
  x = fix2i(objects_x[index]);
  y = fix2i(objects_y[index]);

  if (objects_sprite_attribute[index] & ATTR_HIDDEN)
  {
    sprite = SPRITE(spritenum);
    sprite->y = 0;
    sprite->index = 0xFF;
    spritenum++;

    if ((objects_sprite_attribute[index] & ATTR_16x16) == 0)
      return;

    sprite = SPRITE(spritenum);
    sprite->y = 0;
    sprite->index = 0xFF;
    spritenum++;

    sprite = SPRITE(spritenum);
    sprite->y = 0;
    sprite->index = 0xFF;
    spritenum++;

    sprite = SPRITE(spritenum);
    sprite->y = 0;
    sprite->index = 0xFF;
    spritenum++;

    return;
  }

  if ((objects_sprite_attribute[index] & ATTR_MIRRORED) == 0)
  {
    sprite = SPRITE(spritenum);
    sprite->x = x;
    sprite->y = y;
    sprite->attributes = 0;
    sprite->index = sprite_index;
    spritenum++;
    sprite_index++;

    if ((objects_sprite_attribute[index] & ATTR_16x16) == 0)
      return;

    sprite = SPRITE(spritenum);
    sprite->x = x + 8;
    sprite->y = y;
    sprite->attributes = 0;
    sprite->index = sprite_index;
    spritenum++;
    sprite_index++;

    sprite = SPRITE(spritenum);
    sprite->x = x;
    sprite->y = y + 8;
    sprite->attributes = 0;
    sprite->index = sprite_index;
    spritenum++;
    sprite_index++;

    sprite = SPRITE(spritenum);
    sprite->x = x + 8;
    sprite->y = y + 8;
    sprite->attributes = 0;
    sprite->index = sprite_index;
    spritenum++;
  }
  else
  {
    sprite = SPRITE(spritenum);
    sprite->x = x + 8;
    sprite->y = y;
    sprite->attributes = ATTR_MIRRORED;
    sprite->index = sprite_index;
    spritenum++;
    sprite_index++;

    if ((objects_sprite_attribute[index] & ATTR_16x16) == 0)
    {
      sprite->x = x;  /* don't displace 8x8 sprites. */
      return;
    }

    sprite = SPRITE(spritenum);
    sprite->x = x;
    sprite->y = y;
    sprite->attributes = ATTR_MIRRORED;
    sprite->index = sprite_index;
    spritenum++;
    sprite_index++;

    sprite = SPRITE(spritenum);
    sprite->x = x + 8;
    sprite->y = y + 8;
    sprite->attributes = ATTR_MIRRORED;
    sprite->index = sprite_index;
    spritenum++;
    sprite_index++;

    sprite = SPRITE(spritenum);
    sprite->x = x;
    sprite->y = y + 8;
    sprite->attributes = ATTR_MIRRORED;
    sprite->index = sprite_index;
    spritenum++;
  }
}

static void __fastcall__ update_bat(u8 index)
{
  objects_state[index]++;

  if (objects_state[index] & 8)
    objects_sprite_index[index] = 16;
  else
    objects_sprite_index[index] = 12;

  if (objects_hdir[index] == RIGHT)
  {
    if (colcheck_right(index) || fix2i(objects_x[index]) > 240)
    {
      objects_hdir[index] = LEFT;
      objects_sprite_attribute[index] |= ATTR_MIRRORED;
    }
    else
    {
      objects_x[index] += objects_hspeed[index];
    }
  }
  else
  {
    if (colcheck_left(index) || fix2i(objects_x[index]) < 16)
    {
      objects_hdir[index] = RIGHT;
      objects_sprite_attribute[index] &= ~ATTR_MIRRORED;
    }
    else
    {
      objects_x[index] -= objects_hspeed[index];
    }
  }
}

static void __fastcall__ update_skeleton(u8 index)
{
  objects_counter[index]++;

  if (objects_counter[index] == 127)
    objects_state[index]++;

  if (objects_state[index] == 1)
  {
    create_object(O_BONE, fix2i(objects_x[index])+6, fix2i(objects_y[index])+6);
    objects_hdir[num_objects-1] = objects_hdir[index];
    objects_vdir[num_objects-1] = UP;
    objects_hspeed[num_objects-1] = fixed(1, 0);
    objects_vspeed[num_objects-1] = fixed(2, 127);

    play_sound(2, 0xDE);

    objects_state[index] = 0;
  }

  if (objects_x[index] < objects_x[O_PLAYER])
  {
    objects_hdir[index] = RIGHT;
    objects_sprite_attribute[index] &= ~ATTR_MIRRORED;
  }
  else
  {
    objects_hdir[index] = LEFT;
    objects_sprite_attribute[index] |= ATTR_MIRRORED;
  }
}

static void __fastcall__ update_bone(u8 index)
{
  if (objects_vdir[index] == UP)
  {
    objects_vspeed[index] -= GRAVITY;
    objects_y[index] -= objects_vspeed[index];

    if ((i16)objects_vspeed[index] <= 0)
    {
      objects_vspeed[index] = 0;
      objects_vdir[index] = DOWN;
    }
  }
  else
  {
    objects_vspeed[index] += GRAVITY;
    objects_vspeed[index] = MIN(fixed(2, 0), objects_vspeed[index]);
    objects_y[index] += objects_vspeed[index];
  }

  if (objects_hdir[index] == RIGHT)
  {
    objects_x[index] += objects_hspeed[index];
  }
  else
  {
    objects_x[index] -= objects_hspeed[index];
  }

  objects_state[index]++;
  if (objects_state[index] & 8)
    objects_sprite_attribute[index] |= ATTR_MIRRORED;
  else
    objects_sprite_attribute[index] &= ~(ATTR_MIRRORED);

  if (fix2i(objects_y[index]) >= 240)
  {
    remove_object(index);
  }
}

static void __fastcall__ update_flame(u8 index)
{
  static fixed_t vspeed;

  switch (objects_state[index])
  {
    case 0:
      objects_counter[index]++;

      if (objects_counter[index] == 64)
      {
        objects_state[index] = 1;
        objects_vspeed[index] = fixed(4, 0);
        objects_vdir[index] = UP;
        objects_sprite_attribute[index] &= ~ATTR_HIDDEN;
        objects_counter[index] = fix2i(objects_y[index]);
      }
      return;
  }

  if (objects_vdir[index] == UP)
  {
    vspeed = objects_vspeed[index];

    vspeed -= GRAVITY;
    objects_y[index] -= vspeed;

    if ((i16)vspeed <= 0)
    {
      vspeed = 0;
      objects_vdir[index] = DOWN;
    }

    objects_vspeed[index] = vspeed;
  }
  else
  {
    vspeed = objects_vspeed[index];

    vspeed += GRAVITY;
    vspeed = MIN(fixed(2, 0), vspeed);
    objects_y[index] += vspeed;

    objects_vspeed[index] = vspeed;

    if (fix2i(objects_y[index]) >= objects_counter[index])
    {
      objects_y[index] = fixed(objects_counter[index], 0);
      objects_state[index] = 0;
      objects_sprite_attribute[index] |= ATTR_HIDDEN;
    }
  }
}

static void __fastcall__ update_bullet(u8 index)
{
#define BULLET_SPEED  3

  static u8 dir, x;
  static u8 i;
  static u8 tile_index;

  dir = objects_hdir[index];
  x = fix2i(objects_x[index]);

  switch (dir)
  {
    case RIGHT:
      if ((tile_index = colcheck_right(index)))
      {
        if (tilemap[tile_index] == TILE_FRAGILE)
          remove_tile(tile_index);

        num_bullets--;
        remove_object(index);
        return;
      }
      x += BULLET_SPEED;
      break;
    case LEFT:
      if ((tile_index = colcheck_left(index)))
      {
        if (tilemap[tile_index] == TILE_FRAGILE)
          remove_tile(tile_index);

        num_bullets--;
        remove_object(index);
        return;
      }
      x -= BULLET_SPEED;
      break;
  }

  if (x < 8) { num_bullets--; remove_object(index); return; }
  if (x > 248) { num_bullets--; remove_object(index); return; }

  objects_x[index] = fixed(x, 0);

  switch (objects_state[index])
  {
    case ENEMY_BULLET:
      if (colcheck_objects(O_PLAYER, index))
      {
        hurt_player(objects_x[i] > objects_x[O_PLAYER]);
        remove_object(index);
      }
      return;
    case PLAYER_BULLET:
      for (i = 1; i < num_objects; i++)
      {
        if (i == index) continue;
        if (objects_type[i] == O_NOTHING) continue;
        if (colcheck_objects(index, i) == 1)
        {
          play_sound(4, 0x40);
          remove_object(index);
          num_bullets--;
          if (objects_life[i] > 0)
          {
            objects_life[i]--;
            if (objects_life[i] == 0)
              remove_object(i);
          }
          break;
        }
      }
      return;
  }
}

static void __fastcall__ update_spikeball(u8 index)
{
  if (objects_hdir[index] == RIGHT)
  {
    if (colcheck_right(index) || fix2i(objects_x[index]) > 240)
    {
      objects_hdir[index] = LEFT;
    }
    else
    {
      objects_x[index] += objects_hspeed[index];
    }
  }
  else
  {
    if (colcheck_left(index) || fix2i(objects_x[index]) < 16)
    {
      objects_hdir[index] = RIGHT;
    }
    else
    {
      objects_x[index] -= objects_hspeed[index];
    }
  }
}

static void __fastcall__ animate_powerup(u8 index)
{
#define COUNTER_MAX 16

  if (objects_vdir[index] == UP)
  {
    if (objects_counter[index] == COUNTER_MAX)
    {
      objects_vdir[index] = DOWN;
      objects_counter[index] = 0;
      return;
    }
    objects_y[index] -= 32;
    objects_counter[index]++;
  }
  else
  {
    if (objects_counter[index] == COUNTER_MAX)
    {
      objects_vdir[index] = UP;
      objects_counter[index] = 0;
      return;
    }
    objects_y[index] += 32;
    objects_counter[index]++;
  }

#undef COUNTER_MAX
}

static void __fastcall__ update_doublejump(u8 index)
{
  if (max_jumps == 2)
  {
    remove_object(index);
    return;
  }

  animate_powerup(index);
}

static void __fastcall__ update_gun(u8 index)
{
  if (has_gun == 1)
  {
    remove_object(index);
    return;
  }

  animate_powerup(index);
}

static void __fastcall__ update_treasure(u8 index)
{
  static u8 state;

  state = objects_state[index];

  if (treasure_states[state] == 1)
  {
    remove_object(index);
    return;
  }

  animate_powerup(index);
}

static void __fastcall__ update_object(u8 index)
{
  switch (objects_type[index])
  {
    case O_PLAYER:
      return;
    case O_BAT:
      update_bat(index);
      break;
    case O_SKELETON:
      update_skeleton(index);
      break;
    case O_BONE:
      update_bone(index);
      break;
    case O_FLAME:
      update_flame(index);
      break;
    case O_BULLET:
      update_bullet(index);
      break;
    case O_SPIKEBALL:
      update_spikeball(index);
      break;
    case O_DOUBLEJUMP:
      update_doublejump(index);
      break;
    case O_GUN:
      update_gun(index);
      break;
    case O_TREASURE_1:
      /* All treasures will use O_TREASURE_1 ID. */
      update_treasure(index);
      break;
  }
}

void __fastcall__ update_objects(void)
{
  static u8 index;
  static u8 j;

  spritenum = 0;
  for (index = 0; index < num_objects; index++)
  {
    update_object(index);
    update_sprite(index);
  }

  if (need_cleanup == 0)
    return;

  /* clean up unused objects_ */
  need_cleanup = 0;

  for (index = 0; index < num_objects;)
  {
    if (objects_type[index] == O_NOTHING)
    {
      for (j = index; j < num_objects - 1; j++)
      {
        objects_x[j] = objects_x[j + 1];
        objects_y[j] = objects_y[j + 1];
        objects_hspeed[j] = objects_hspeed[j + 1];
        objects_vspeed[j] = objects_vspeed[j + 1];
        objects_sprite_attribute[j] = objects_sprite_attribute[j + 1];
        objects_hdir[j] = objects_hdir[j + 1];
        objects_vdir[j] = objects_vdir[j + 1];
        objects_state[j] = objects_state[j + 1];
        objects_counter[j] = objects_counter[j + 1];
        objects_sprite_index[j] = objects_sprite_index[j + 1];
        objects_type[j] = objects_type[j + 1];
      }

      num_objects--;
    }
    else
    {
      index++;
    }
  }
}

void __fastcall__ create_object(u8 type, u8 x, u8 y)
{
  objects_x[num_objects] = fixed(x, 0);
  objects_y[num_objects] = fixed(y, 0);
  objects_hdir[num_objects] = 0;
  objects_vdir[num_objects] = 0;
  objects_hspeed[num_objects] = 0;
  objects_vspeed[num_objects] = 0;
  objects_sprite_index[num_objects] = 0;
  objects_sprite_attribute[num_objects] = ATTR_16x16; /* default to using 16x16 sprites. */
  objects_state[num_objects] = 0;
  objects_counter[num_objects] = 0;
  objects_type[num_objects] = type;
  objects_life[num_objects] = 0;

#define BBOX(x1, x2, y1, y2) \
  objects_bbox_x1[num_objects] = x1; \
  objects_bbox_x2[num_objects] = x2; \
  objects_bbox_y1[num_objects] = y1; \
  objects_bbox_y2[num_objects] = y2

  switch (type)
  {
    case O_PLAYER:
      BBOX(4, 12, 0, 16);
      break;

    /* Enemies. */

    case O_BAT:
      objects_hspeed[num_objects] = fixed(1, 127);
      objects_sprite_index[num_objects] = 12;
      objects_life[num_objects] = 3;
      BBOX(3, 14, 2, 12);
      break;
    case O_SKELETON:
      objects_sprite_index[num_objects] = 20;
      objects_life[num_objects] = 3;
      BBOX(6, 10, 0, 16);
      break;
    case O_BONE:
      objects_sprite_index[num_objects] = 28;
      objects_sprite_attribute[num_objects] &= ~ATTR_16x16;
      BBOX(0, 0, 8, 8);
      break;
    case O_FLAME:
      objects_sprite_index[num_objects] = 29;
      objects_vdir[num_objects] = UP;
      objects_sprite_attribute[num_objects] |= ATTR_HIDDEN;
      BBOX(2, 14, 2, 14);
      break;
    case O_BULLET:
      objects_sprite_index[num_objects] = 38;
      objects_sprite_attribute[num_objects] &= ~ATTR_16x16;
      num_bullets++;
      BBOX(3, 5, 3, 5);
      break;
    case O_SPIKEBALL:
      objects_hspeed[num_objects] = fixed(1, 0);
      objects_sprite_index[num_objects] = 43;
      BBOX(1, 15, 1, 15);
      break;

    /* Powerups */

    case O_DOUBLEJUMP:
      BBOX(4, 12, 4, 12);
      objects_sprite_index[num_objects] = 47;
      break;
    case O_GUN:
      BBOX(4, 12, 4, 12);
      objects_sprite_index[num_objects] = 51;
      break;
    case O_CHECKPOINT:
      BBOX(4, 12, 4, 12);
      objects_sprite_index[num_objects] = 91;
      break;

    /* Treasures */

    case O_TREASURE_1:
      BBOX(4, 12, 4, 12);
      objects_sprite_index[num_objects] = 55;
      objects_state[num_objects] = 0;
      objects_type[num_objects] = O_TREASURE_1;
      break;
    case O_TREASURE_2:
      BBOX(4, 12, 4, 12);
      objects_sprite_index[num_objects] = 59;
      objects_state[num_objects] = 1;
      objects_type[num_objects] = O_TREASURE_1;
      break;
    case O_TREASURE_3:
      BBOX(4, 12, 4, 12);
      objects_sprite_index[num_objects] = 63;
      objects_state[num_objects] = 2;
      objects_type[num_objects] = O_TREASURE_1;
      break;
    case O_TREASURE_4:
      BBOX(4, 12, 4, 12);
      objects_sprite_index[num_objects] = 67;
      objects_state[num_objects] = 3;
      objects_type[num_objects] = O_TREASURE_1;
      break;
    case O_TREASURE_5:
      BBOX(4, 12, 4, 12);
      objects_sprite_index[num_objects] = 71;
      objects_state[num_objects] = 4;
      objects_type[num_objects] = O_TREASURE_1;
      break;
    case O_TREASURE_6:
      BBOX(4, 12, 4, 12);
      objects_sprite_index[num_objects] = 75;
      objects_state[num_objects] = 5;
      objects_type[num_objects] = O_TREASURE_1;
      break;
    case O_TREASURE_7:
      BBOX(4, 12, 4, 12);
      objects_sprite_index[num_objects] = 79;
      objects_state[num_objects] = 6;
      objects_type[num_objects] = O_TREASURE_1;
      break;
    case O_TREASURE_8:
      BBOX(4, 12, 4, 12);
      objects_sprite_index[num_objects] = 83;
      objects_state[num_objects] = 7;
      objects_type[num_objects] = O_TREASURE_1;
      break;
    case O_TREASURE_9:
      BBOX(4, 12, 4, 12);
      objects_sprite_index[num_objects] = 87;
      objects_state[num_objects] = 8;
      objects_type[num_objects] = O_TREASURE_1;
      break;
  }

  num_objects++;
}

void __fastcall__ remove_object(u8 index)
{
  objects_type[index] = O_NOTHING;
  need_cleanup = 1;
}

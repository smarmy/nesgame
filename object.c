#include "asm.h"
#include "sprite.h"
#include "object.h"
#include "level.h"
#include "gamedata.h"
#include "colcheck.h"

object_t objects;
u8 num_objects = 0;

static u8 spritenum;
static u8 need_cleanup = 0;

static void __fastcall__ update_sprite(u8 index)
{
  static sprite_t* sprite;
  static u8 sprite_index;
  static u8 x, y;

  sprite_index = objects.sprite_index[index];
  x = fix2i(objects.x[index]);
  y = fix2i(objects.y[index]);

  if (objects.sprite_attribute[index] & ATTR_HIDDEN)
  {
    sprite = SPRITE(spritenum);
    sprite->y = 0;
    spritenum++;

    if ((objects.sprite_attribute[index] & ATTR_16x16) == 0)
      return;

    sprite = SPRITE(spritenum);
    sprite->y = 0;
    spritenum++;

    sprite = SPRITE(spritenum);
    sprite->y = 0;
    spritenum++;

    sprite = SPRITE(spritenum);
    sprite->y = 0;
    spritenum++;

    return;
  }

  if ((objects.sprite_attribute[index] & ATTR_MIRRORED) == 0)
  {
    sprite = SPRITE(spritenum);
    sprite->x = x;
    sprite->y = y;
    sprite->attributes = 0;
    sprite->index = sprite_index;
    spritenum++;
    sprite_index++;

    if ((objects.sprite_attribute[index] & ATTR_16x16) == 0)
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

    if ((objects.sprite_attribute[index] & ATTR_16x16) == 0)
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
  objects.state[index]++;

  if (objects.state[index] & 8)
    objects.sprite_index[index] = 16;
  else
    objects.sprite_index[index] = 12;

  if (objects.hdir[index] == RIGHT)
  {
    if (colcheck_right(index) || fix2i(objects.x[index]) > 240)
    {
      objects.hdir[index] = LEFT;
      objects.sprite_attribute[index] |= ATTR_MIRRORED;
    }
    else
    {
      objects.x[index] += objects.hspeed[index];
    }
  }
  else
  {
    if (colcheck_left(index) || fix2i(objects.x[index]) < 16)
    {
      objects.hdir[index] = RIGHT;
      objects.sprite_attribute[index] &= ~ATTR_MIRRORED;
    }
    else
    {
      objects.x[index] -= objects.hspeed[index];
    }
  }
}

static void __fastcall__ update_skeleton(u8 index)
{
  objects.counter[index]++;

  if (objects.counter[index] == 127)
    objects.state[index]++;

  if (objects.state[index] == 1)
  {
    create_object(O_BONE, fix2i(objects.x[index])+6, fix2i(objects.y[index])+6);
    objects.hdir[num_objects-1] = objects.hdir[index];
    objects.vdir[num_objects-1] = UP;
    objects.hspeed[num_objects-1] = fixed(1, 0);
    objects.vspeed[num_objects-1] = fixed(2, 127);

    play_sound(2, 0xDE);

    objects.state[index] = 0;
  }

  if (objects.x[index] < objects.x[O_PLAYER])
  {
    objects.hdir[index] = RIGHT;
    objects.sprite_attribute[index] &= ~ATTR_MIRRORED;
  }
  else
  {
    objects.hdir[index] = LEFT;
    objects.sprite_attribute[index] |= ATTR_MIRRORED;
  }
}

static void __fastcall__ update_bone(u8 index)
{
  if (objects.vdir[index] == UP)
  {
    objects.vspeed[index] -= GRAVITY;
    objects.y[index] -= objects.vspeed[index];

    if ((i16)objects.vspeed[index] <= 0)
    {
      objects.vspeed[index] = 0;
      objects.vdir[index] = DOWN;
    }
  }
  else
  {
    objects.vspeed[index] += GRAVITY;
    objects.vspeed[index] = MIN(fixed(2, 0), objects.vspeed[index]);
    objects.y[index] += objects.vspeed[index];
  }

  if (objects.hdir[index] == RIGHT)
  {
    objects.x[index] += objects.hspeed[index];
  }
  else
  {
    objects.x[index] -= objects.hspeed[index];
  }

  objects.state[index]++;
  if (objects.state[index] & 8)
    objects.sprite_attribute[index] |= ATTR_MIRRORED;
  else
    objects.sprite_attribute[index] &= ~(ATTR_MIRRORED);

  if (fix2i(objects.y[index]) >= 240)
  {
    remove_object(index);
  }
}

static void __fastcall__ update_flame(u8 index)
{
  static u8 tmp;

  switch (objects.state[index])
  {
    case 0:
      objects.counter[index]++;

      if (objects.counter[index] == 64)
      {
        objects.state[index] = 1;
        objects.vspeed[index] = fixed(4, 0);
        objects.vdir[index] = UP;
        objects.sprite_attribute[index] &= ~ATTR_HIDDEN;
        objects.counter[index] = 0;
      }
      return;
  }

  if (objects.vdir[index] == UP)
  {
    objects.vspeed[index] -= GRAVITY;
    objects.y[index] -= objects.vspeed[index];

    if ((i16)objects.vspeed[index] <= 0)
    {
      objects.vspeed[index] = 0;
      objects.vdir[index] = DOWN;
    }
  }
  else
  {
    objects.vspeed[index] += GRAVITY;
    objects.vspeed[index] = MIN(fixed(2, 0), objects.vspeed[index]);
    objects.y[index] += objects.vspeed[index];

    tmp = tile_check(index, TILE_LAVA_BODY);
    if (tmp == 0) return;

    objects.y[index] = fixed((tmp >> 4) << 4, 0);
    objects.state[index] = 0;
    objects.sprite_attribute[index] |= ATTR_HIDDEN;
  }
}

static void __fastcall__ update_object(u8 index)
{
  switch (objects.type[index])
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

  /* clean up unused objects. */
  need_cleanup = 0;

  for (index = 0; index < num_objects;)
  {
    if (objects.type[index] == O_NOTHING)
    {
      for (j = index; j < num_objects - 1; j++)
      {
        objects.x[index] = objects.x[index + 1];
        objects.y[index] = objects.y[index + 1];
        objects.hspeed[index] = objects.hspeed[index + 1];
        objects.vspeed[index] = objects.vspeed[index + 1];
        objects.sprite_attribute[index] = objects.sprite_attribute[index + 1];
        objects.hdir[index] = objects.hdir[index + 1];
        objects.vdir[index] = objects.vdir[index + 1];
        objects.state[index] = objects.state[index + 1];
        objects.counter[index] = objects.counter[index + 1];
        objects.sprite_index[index] = objects.sprite_index[index + 1];
        objects.type[index] = objects.type[index + 1];
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
  objects.x[num_objects] = fixed(x, 0);
  objects.y[num_objects] = fixed(y, 0);
  objects.hdir[num_objects] = 0;
  objects.vdir[num_objects] = 0;
  objects.hspeed[num_objects] = 0;
  objects.vspeed[num_objects] = 0;
  objects.sprite_index[num_objects] = 0;
  objects.sprite_attribute[num_objects] = ATTR_16x16; /* default to using 16x16 sprites. */
  objects.state[num_objects] = 0;
  objects.counter[num_objects] = 0;
  objects.type[num_objects] = type;

#define BBOX(x1, x2, y1, y2) \
  objects.bbox_x1[num_objects] = x1; \
  objects.bbox_x2[num_objects] = x2; \
  objects.bbox_y1[num_objects] = y1; \
  objects.bbox_y2[num_objects] = y2

  switch (type)
  {
    case O_PLAYER:
      BBOX(4, 12, 0, 16);
      break;
    case O_BAT:
      objects.hspeed[num_objects] = fixed(1, 127);
      objects.sprite_index[num_objects] = 12;
      BBOX(3, 14, 2, 12);
      break;
    case O_SKELETON:
      objects.sprite_index[num_objects] = 20;
      BBOX(6, 10, 0, 16);
      break;
    case O_BONE:
      objects.sprite_index[num_objects] = 28;
      objects.sprite_attribute[num_objects] &= ~ATTR_16x16;
      BBOX(0, 0, 8, 8);
      break;
    case O_FLAME:
      objects.sprite_index[num_objects] = 29;
      objects.vdir[num_objects] = UP;
      objects.sprite_attribute[num_objects] |= ATTR_HIDDEN;
      BBOX(2, 14, 2, 14);
      break;
  }

  num_objects++;
}

void __fastcall__ remove_object(u8 index)
{
  objects.type[index] = O_NOTHING;
  need_cleanup = 1;
}

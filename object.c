#include "sprite.h"
#include "object.h"
#include "colcheck.h"

object_t objects;
u8 num_objects = 0;

static u8 spritenum;

static void __fastcall__ update_sprite(u8 index)
{
  static sprite_t* sprite;
  static u8 sprite_index;
  static u8 x, y;

  sprite_index = objects.sprite_index[index] << 2;
  x = fix2i(objects.x[index]);
  y = fix2i(objects.y[index]);

  if (objects.sprite_mirrored[index] == 0)
  {
    sprite = SPRITE(spritenum);
    sprite->x = x;
    sprite->y = y;
    sprite->attributes = 0;
    sprite->index = sprite_index;
    spritenum++;
    sprite_index++;

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

static void __fastcall__ update_object(u8 index)
{
  switch (objects.type[index])
  {
    case O_PLAYER:
      return;
    case O_BAT:
      objects.state[index]++;

      if (objects.state[index] & 8)
        objects.sprite_index[index] = 4;
      else
        objects.sprite_index[index] = 3;

      if (objects.dir[index] == RIGHT)
      {
        if (colcheck_right(index))
        {
          objects.dir[index] = LEFT;
          objects.sprite_mirrored[index] = 1;
        }
        else
        {
          objects.x[index] += objects.hspeed[index];
        }
      }
      else
      {
        if (colcheck_left(index))
        {
          objects.dir[index] = RIGHT;
          objects.sprite_mirrored[index] = 0;
        }
        else
        {
          objects.x[index] -= objects.hspeed[index];
        }
      }
      break;
  }
}

void __fastcall__ update_objects(void)
{
  static u8 index;

  spritenum = 0;
  for (index = 0; index < num_objects; index++)
  {
    update_object(index);
    update_sprite(index);
  }
}

void __fastcall__ create_object(u8 type, u8 x, u8 y)
{
  objects.x[num_objects] = fixed(x, 0);
  objects.y[num_objects] = fixed(y, 0);
  objects.dir[num_objects] = 0;
  objects.hspeed[num_objects] = 0;
  objects.vspeed[num_objects] = 0;
  objects.sprite_index[num_objects] = 0;
  objects.sprite_mirrored[num_objects] = 0;
  objects.state[num_objects] = 0;
  objects.type[num_objects] = type;

  switch (type)
  {
    case O_BAT:
      objects.hspeed[num_objects] = fixed(1, 127);
      objects.sprite_index[num_objects] = 3;
      break;
  }

  num_objects++;
}

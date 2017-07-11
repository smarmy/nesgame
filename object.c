#include "sprite.h"
#include "object.h"

object_t objects[MAX_OBJECTS];
u8 num_objects = 0;

static u8 sprite_index_table[] =
{
  0, 1, 16, 17,
  2, 3, 18, 19,
  4, 5, 20, 21
};

void __fastcall__ update_objects(void)
{
  static u8 index;

  static u8 spritenum;
  static u8 spritenum_0;
  static u8 spritenum_1;
  static u8 spritenum_2;
  static u8 spritenum_3;

  static u8 sprite_index0;
  static u8 sprite_index1;
  static u8 sprite_index2;
  static u8 sprite_index3;

  for (index = 0, spritenum = 0; index < num_objects; index++, spritenum += 4)
  {
#define O objects[index]

    spritenum_0 = spritenum+0;
    spritenum_1 = spritenum+1;
    spritenum_2 = spritenum+2;
    spritenum_3 = spritenum+3;

    sprite_index0 = sprite_index_table[(O.sprite_index << 2) + 0];
    sprite_index1 = sprite_index_table[(O.sprite_index << 2) + 1];
    sprite_index2 = sprite_index_table[(O.sprite_index << 2) + 2];
    sprite_index3 = sprite_index_table[(O.sprite_index << 2) + 3];

    if (O.sprite_mirrored == 0)
    {
      SPRITE(spritenum_0)->x = fix2i(O.x);
      SPRITE(spritenum_0)->y = fix2i(O.y);
      SPRITE(spritenum_0)->attributes = 0;
      SPRITE(spritenum_0)->index = sprite_index0;

      SPRITE(spritenum_1)->x = fix2i(O.x) + 8;
      SPRITE(spritenum_1)->y = fix2i(O.y);
      SPRITE(spritenum_1)->attributes = 0;
      SPRITE(spritenum_1)->index = sprite_index1;

      SPRITE(spritenum_2)->x = fix2i(O.x);
      SPRITE(spritenum_2)->y = fix2i(O.y) + 8;
      SPRITE(spritenum_2)->attributes = 0;
      SPRITE(spritenum_2)->index = sprite_index2;

      SPRITE(spritenum_3)->x = fix2i(O.x) + 8;
      SPRITE(spritenum_3)->y = fix2i(O.y) + 8;
      SPRITE(spritenum_3)->attributes = 0;
      SPRITE(spritenum_3)->index = sprite_index3;
    }
    else
    {
      SPRITE(spritenum_0)->x = fix2i(O.x);
      SPRITE(spritenum_0)->y = fix2i(O.y);
      SPRITE(spritenum_0)->attributes = ATTR_MIRRORED;
      SPRITE(spritenum_0)->index = sprite_index1;

      SPRITE(spritenum_1)->x = fix2i(O.x) + 8;
      SPRITE(spritenum_1)->y = fix2i(O.y);
      SPRITE(spritenum_1)->attributes = ATTR_MIRRORED;
      SPRITE(spritenum_1)->index = sprite_index0;

      SPRITE(spritenum_2)->x = fix2i(O.x);
      SPRITE(spritenum_2)->y = fix2i(O.y) + 8;
      SPRITE(spritenum_2)->attributes = ATTR_MIRRORED;
      SPRITE(spritenum_2)->index = sprite_index3;

      SPRITE(spritenum_3)->x = fix2i(O.x) + 8;
      SPRITE(spritenum_3)->y = fix2i(O.y) + 8;
      SPRITE(spritenum_3)->attributes = ATTR_MIRRORED;
      SPRITE(spritenum_3)->index = sprite_index2;
    }
#undef O
  }
}

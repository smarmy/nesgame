#ifndef SPRITE_H
#define SPRITE_H

#include "typedef.h"

typedef struct sprite_t
{
  u8 y;
  u8 index;
  u8 attributes;
  u8 x;
} sprite_t;

#define SPRITE(X) (((sprite_t*)0x200) + X )

#define ATTR_MIRRORED 64

#endif

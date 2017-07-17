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

#define ATTR_PAL_LO   1
#define ATTR_PAL_HI   2
#define ATTR_16x16    4   /* use one of the unused sprite attributes. */
#define ATTR_HIDDEN   8   /* use one of the unused sprite attributes. */
#define ATTR_UNUSED2  16
#define ATTR_PRIORITY 32
#define ATTR_MIRRORED 64
#define ATTR_V_MIRROR 128

/**
 * Implemented in assembly.
 */
void __fastcall__ clear_sprites(void);

#endif

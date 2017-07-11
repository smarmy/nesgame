#ifndef OBJECT_H_
#define OBJECT_H_

#include "fixed.h"
#include "typedef.h"

typedef struct object_t
{
  fixed_t x;
  fixed_t y;
  fixed_t hspeed;
  fixed_t vspeed;
  u8 sprite_mirrored;
  u8 dir;
  u8 state;
  u8 sprite_index;
} object_t;

#define MAX_OBJECTS 10
extern u8 num_objects;
extern object_t objects[MAX_OBJECTS];

void __fastcall__ update_objects(void);

#endif /* OBJECT_H_ */

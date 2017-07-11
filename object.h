#ifndef OBJECT_H_
#define OBJECT_H_

#include "fixed.h"
#include "typedef.h"

#define MAX_OBJECTS 10

typedef struct object_t
{
  fixed_t x[MAX_OBJECTS];
  fixed_t y[MAX_OBJECTS];
  fixed_t hspeed[MAX_OBJECTS];
  fixed_t vspeed[MAX_OBJECTS];
  u8 sprite_mirrored[MAX_OBJECTS];
  u8 dir[MAX_OBJECTS];
  u8 state[MAX_OBJECTS];
  u8 sprite_index[MAX_OBJECTS];
} object_t;

extern u8 num_objects;
extern object_t objects;

void __fastcall__ update_objects(void);

#endif /* OBJECT_H_ */

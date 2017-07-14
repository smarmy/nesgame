#ifndef OBJECT_H_
#define OBJECT_H_

#include "fixed.h"
#include "typedef.h"

#define MAX_OBJECTS 10

#define O_PLAYER 0
#define O_BAT    1

typedef struct object_t
{
  fixed_t x[MAX_OBJECTS];
  fixed_t y[MAX_OBJECTS];
  fixed_t hspeed[MAX_OBJECTS];
  fixed_t vspeed[MAX_OBJECTS];
  u8 sprite_mirrored[MAX_OBJECTS];
  u8 hdir[MAX_OBJECTS];
  u8 vdir[MAX_OBJECTS];
  u8 state[MAX_OBJECTS];
  u8 sprite_index[MAX_OBJECTS];
  u8 type[MAX_OBJECTS];
} object_t;

extern u8 num_objects;
extern object_t objects;

void __fastcall__ update_objects(void);
void __fastcall__ create_object(u8 type, u8 x, u8 y);

#endif /* OBJECT_H_ */

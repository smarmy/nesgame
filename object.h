/*
 * object.h
 *
 *  Created on: 11 juli 2017
 *      Author: thene_000
 */

#ifndef OBJECT_H_
#define OBJECT_H_

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

#endif /* OBJECT_H_ */

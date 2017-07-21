#ifndef OBJECT_H_
#define OBJECT_H_

#include "fixed.h"
#include "typedef.h"

#define PLAYER_BULLET 0
#define ENEMY_BULLET  1

#define MAX_OBJECTS 30

#define O_PLAYER    0
#define O_BAT       1
#define O_SKELETON  2
#define O_BONE      3
#define O_FLAME     4
#define O_BULLET    5
#define O_SPIKEBALL 6
#define O_DOUBLEJUMP 7
#define O_NOTHING   255

extern fixed_t objects_x[MAX_OBJECTS];
extern fixed_t objects_y[MAX_OBJECTS];
extern fixed_t objects_hspeed[MAX_OBJECTS];
extern fixed_t objects_vspeed[MAX_OBJECTS];
extern u8 objects_sprite_attribute[MAX_OBJECTS];
extern u8 objects_hdir[MAX_OBJECTS];
extern u8 objects_vdir[MAX_OBJECTS];
extern u8 objects_bbox_x1[MAX_OBJECTS];
extern u8 objects_bbox_y1[MAX_OBJECTS];
extern u8 objects_bbox_x2[MAX_OBJECTS];
extern u8 objects_bbox_y2[MAX_OBJECTS];
extern u8 objects_life[MAX_OBJECTS];
extern u8 objects_state[MAX_OBJECTS];
extern u8 objects_counter[MAX_OBJECTS];
extern u8 objects_sprite_index[MAX_OBJECTS];
extern u8 objects_type[MAX_OBJECTS];

extern u8 num_objects;

void __fastcall__ update_objects(void);
void __fastcall__ create_object(u8 type, u8 x, u8 y);
void __fastcall__ remove_object(u8 index);

#endif /* OBJECT_H_ */

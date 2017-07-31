#ifndef GAMEDATA_H_
#define GAMEDATA_H_

#include "typedef.h"

#define MAX_TREASURES 9
#define MAX_GAME_LIFE 5

extern u8 current_level;
extern u8 max_jumps;
extern u8 player_life;
extern u8 game_life;
extern u8 num_bullets;
extern u8 has_gun;
extern u8 checkpoint_level;
extern u8 checkpoint_x;
extern u8 checkpoint_y;

extern u8 treasure_states[MAX_TREASURES];

#define LEVELS_PER_ROW  6
#define MAX_LIFE        10
#define GRAVITY 25

void __fastcall__ hurt_player(u8 hdir);
void __fastcall__ kill_player(void);

#endif /* GAMEDATA_H_ */

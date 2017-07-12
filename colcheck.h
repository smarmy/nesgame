#ifndef COLCHECK_H_
#define COLCHECK_H_

#include "typedef.h"

u8 __fastcall__ colcheck_down(u8 obj_index);
u8 __fastcall__ colcheck_up(u8 obj_index);
u8 __fastcall__ colcheck_right(u8 obj_index);
u8 __fastcall__ colcheck_left(u8 obj_index);
u8 __fastcall__ stairs_check(u8 obj_index);
u8 __fastcall__ tile_check(u8 obj_index, u8 tile);

#endif /* COLCHECK_H_ */

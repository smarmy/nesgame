#ifndef ASM_H
#define ASM_H

#include "typedef.h"

#define PAD_A       (128)
#define PAD_B       (64)
#define PAD_SELECT  (32)
#define PAD_START   (16)
#define PAD_UP      (8)
#define PAD_DOWN    (4)
#define PAD_LEFT    (2)
#define PAD_RIGHT   (1)

/**
 * Button state is saved as
 * 7 - A
 * 6 - B
 * 5 - Select
 * 4 - Start
 * 3 - Up
 * 2 - Down
 * 1 - Left
 * 0 - Right
 */
u8 __fastcall__ check_gamepad(void);

/**
 * Wait for vblank period to finish.
 */
void __fastcall__ wait_vblank(void);

#endif

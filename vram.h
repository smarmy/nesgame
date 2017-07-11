#ifndef VRAM_H_
#define VRAM_H_

#include "typedef.h"

/**
 * Buffer to store nametable updates.
 */
extern u8* VRAM;

void __fastcall__ copy_vram(void);

#endif /* VRAM_H_ */

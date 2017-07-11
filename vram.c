#include "vram.h"

static u8 vram_buf[960];
u8* VRAM = vram_buf;

void __fastcall__ copy_vram(void)
{
  u16 loop;

  PPUADDR = 0x20;
  PPUADDR = 0x00;

  for (loop = 0; loop < sizeof(vram_buf); loop++)
  {
    PPUDATA = vram_buf[loop];
  }
}

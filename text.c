#include "ppu.h"
#include "text.h"

static const u16 base_address = 0x2000;
static u16 offset;

void __fastcall__ print_text(u8 x, u8 y, const char* text)
{
  static u8 len;
  static u8 index;

  offset = base_address + (y << 5) + x;

  for (len = 0, index = 1; *text; ++text, ++len, ++offset, index += 3)
  {
    *(VRAMBUFFER+index+0) = (u8)(offset >> 8);
    *(VRAMBUFFER+index+1) = (u8)(offset & 0xFF);
    *(VRAMBUFFER+index+2) = 96 + (*text);
  }

  *(VRAMBUFFER) = len;
}

void __fastcall__ print_text_2(u8 x, u8 y, const char* text)
{
  offset = base_address + (y << 5) + x;

  for (; *text; ++text, ++offset)
  {
    ppuwrite(offset, 96+(*text));
  }
}

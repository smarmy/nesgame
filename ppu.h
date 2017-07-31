#ifndef PPU_H_
#define PPU_H_

#include "typedef.h"

#define PPUCTRL   (*(u8*)0x2000)
#define PPUMASK   (*(u8*)0x2001)
#define PPUSTATUS (*(u8*)0x2002)
#define OAMADDR   (*(u8*)0x2003)
#define OAMDATA   (*(u8*)0x2004)
#define PPUSCROLL (*(u8*)0x2005)
#define PPUADDR   (*(u8*)0x2006)
#define PPUDATA   (*(u8*)0x2007)
#define OAM_DMA   (*(u8*)0x4014)

#define ppuwrite(Address, Byte) \
    PPUADDR = (u8)((Address) >> 8); \
    PPUADDR = (u8)((Address) & 0xFF); \
    PPUDATA = (Byte)

#define VRAMBUFFER ((u8*)0x0300)

#define turn_off_ppu() \
    PPUCTRL = 0; \
    PPUMASK = 0

#define turn_on_ppu() \
    PPUSCROLL = 0; \
    PPUSCROLL = 0; \
    PPUCTRL = 0x88; \
    PPUMASK = 0x1E

#endif /* PPU_H_ */

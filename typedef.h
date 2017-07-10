#ifndef TYPEDEF_H
#define TYPEDEF_H

typedef unsigned char u8;
typedef signed char i8;
typedef unsigned int u16;
typedef signed int i16;

#define PPUCTRL   (*(u8*)0x2000)
#define PPUMASK   (*(u8*)0x2001)
#define PPUSTATUS (*(u8*)0x2002)
#define OAMADDR   (*(u8*)0x2003)
#define OAMDATA   (*(u8*)0x2004)
#define PPUSCROLL (*(u8*)0x2005)
#define PPUADDR   (*(u8*)0x2006)
#define PPUDATA   (*(u8*)0x2007)
#define OAM_DMA   (*(u8*)0x4014)

#endif


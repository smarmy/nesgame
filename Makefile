cc65=../bin/cc65.exe
ca65=../bin/ca65.exe
ld65=../bin/ld65.exe

CFLAGS=-Oi -T

all:
	$(cc65) $(CFLAGS) game.c
	$(cc65) $(CFLAGS) level.c
	$(cc65) $(CFLAGS) object.c
	$(cc65) $(CFLAGS) colcheck.c
	$(ca65) crt0.s -I ../asminc/
	$(ca65) asm.s -I ../asminc/
	$(ca65) game.s -I ../asminc/
	$(ca65) level.s -I ../asminc/
	$(ca65) object.s -I ../asminc/
	$(ca65) colcheck.s -I ../asminc/
	$(ld65) -C nes.cfg -o game.nes game.o level.o object.o colcheck.o crt0.o asm.o -L ../lib/ nes.lib
cc65=../bin/cc65.exe
ca65=../bin/ca65.exe
ld65=../bin/ld65.exe

all:
	$(cc65) -Oi game.c
	$(ca65) crt0.s -I ../asminc/
	$(ca65) asm.s -I ../asminc/
	$(ca65) game.s -I ../asminc/
	$(ld65) -C nes.cfg -o game.nes game.o crt0.o asm.o -L ../lib/ nes.lib
;
; Startup code for cc65 (NES version)
;
; by Groepaz/Hitmen <groepaz@gmx.net>
; based on code by Ullrich von Bassewitz <uz@cc65.org>
;

        .export         _exit
        .export         __STARTUP__ : absolute = 1      ; Mark as startup
        .import         initlib, donelib, callmain
        .import         push0, _main, zerobss, copydata
        .import         ppubuf_flush

        ; Linker generated symbols
        .import         __RAM_START__, __RAM_SIZE__
        .import         __SRAM_START__, __SRAM_SIZE__
        .import         __ROM0_START__, __ROM0_SIZE__
        .import         __STARTUP_LOAD__,__STARTUP_RUN__, __STARTUP_SIZE__
        .import         __CODE_LOAD__,__CODE_RUN__, __CODE_SIZE__
        .import         __RODATA_LOAD__,__RODATA_RUN__, __RODATA_SIZE__

        .include        "zeropage.inc"
        .include        "constants.inc"

; ------------------------------------------------------------------------
; 16 bytes INES header

.segment        "HEADER"

;    +--------+------+------------------------------------------+
;    | Offset | Size | Content(s)                               |
;    +--------+------+------------------------------------------+
;    |   0    |  3   | 'NES'                                    |
;    |   3    |  1   | $1A                                      |
;    |   4    |  1   | 16K PRG-ROM page count                   |
;    |   5    |  1   | 8K CHR-ROM page count                    |
;    |   6    |  1   | ROM Control Byte #1                      |
;    |        |      |   %####vTsM                              |
;    |        |      |    |  ||||+- 0=Horizontal mirroring      |
;    |        |      |    |  ||||   1=Vertical mirroring        |
;    |        |      |    |  |||+-- 1=SRAM enabled              |
;    |        |      |    |  ||+--- 1=512-byte trainer present  |
;    |        |      |    |  |+---- 1=Four-screen mirroring     |
;    |        |      |    |  |                                  |
;    |        |      |    +--+----- Mapper # (lower 4-bits)     |
;    |   7    |  1   | ROM Control Byte #2                      |
;    |        |      |   %####0000                              |
;    |        |      |    |  |                                  |
;    |        |      |    +--+----- Mapper # (upper 4-bits)     |
;    |  8-15  |  8   | $00                                      |
;    | 16-..  |      | Actual 16K PRG-ROM pages (in linear      |
;    |  ...   |      | order). If a trainer exists, it precedes |
;    |  ...   |      | the first PRG-ROM page.                  |
;    | ..-EOF |      | CHR-ROM pages (in ascending order).      |
;    +--------+------+------------------------------------------+

        .byte   $4e,$45,$53,$1a ; "NES"^Z
        .byte   2               ; ines prg  - Specifies the number of 16k prg banks.
        .byte   1               ; ines chr  - Specifies the number of 8k chr banks.
        .byte   %00000011       ; ines mir  - Specifies VRAM mirroring of the banks.
        .byte   %00000000       ; ines map  - Specifies the NES mapper used.
        .byte   0,0,0,0,0,0,0,0 ; 8 zeroes


; ------------------------------------------------------------------------
; Place the startup code in a special segment.

.segment        "STARTUP"

start:

; Set up the CPU and System-IRQ.

        sei
        cld

; Clear the BSS data.

        jsr     zerobss

; Initialize the data.
        jsr     copydata

; Set up the stack.

        lda     #<(__SRAM_START__ + __SRAM_SIZE__)
        ldx     #>(__SRAM_START__ + __SRAM_SIZE__)
        sta     sp
        stx     sp+1            ; Set argument stack ptr

; Clear PPU buffer count memory.
        lda #0
        sta $0300

; Call the module constructors.

        jsr     initlib

; Push the command-line arguments; and, call main().

        jsr     callmain

; Call the module destructors. This is also the exit() entry.

_exit:  jsr     donelib         ; Run module destructors

; Reset the NES.

        jmp start

; ------------------------------------------------------------------------
; System V-Blank Interrupt
; Updates PPU Memory (buffered).
; Updates VBLANK_FLAG and tickcount.
; ------------------------------------------------------------------------

nmi:    pha
        tya
        pha
        txa
        pha

        ; Begin OAM transfer
        lda #$00
        sta OAMADDR
        lda #$02
        sta OAM_DMA

        ; Perform nametable updates
        ; $0300: Number of updates
        ; $0301: PPUADDR hi
        ; $0302: PPUADDR lo
        ; $0303: tileId
        ;     .
        ;     .
        ;     .
        ldx $0300
        beq @audio
        ldy #$00
@loop:
        iny
        lda $0300, y
        sta PPUADDR
        iny
        lda $0300, y
        sta PPUADDR
        iny
        lda $0300, y
        sta PPUDATA
        dex
        bne @loop
        stx $0300

        ; Reset scroll (why?)
        stx PPUSCROLL
        stx PPUSCROLL
@audio:
        ; Check audio
        lda $0400
        beq @exit_nmi
        dec $0400
        bne @exit_nmi
@stop_sound:
        lda #%00000000
        sta $4015
        sta $4000
        sta $4002
        sta $4003
@exit_nmi:
        pla
        tax
        pla
        tay
        pla

; Interrupt exit

irq:
        rti

; ------------------------------------------------------------------------
; hardware vectors
; ------------------------------------------------------------------------

.segment "VECTORS"

        .word   nmi         ; $fffa vblank nmi
        .word   start       ; $fffc reset
        .word   irq         ; $fffe irq / brk

; ------------------------------------------------------------------------
; character data
; ------------------------------------------------------------------------

.segment "CHARS"

        .incbin         "chr.chr", $0, $2000

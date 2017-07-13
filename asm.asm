
        .export _check_gamepad
        .export _wait_vblank
        .export _play_sound

        .import popa

        .include "zeropage.inc"
        .include "constants.inc"

; ----------------------------------------------------------------------------
; General info:
; - Lo byte of return value saved into A
; - Hi byte of return value saved into X
; - cc65 always expect both to be set correctly due to integer promotions.
; ----------------------------------------------------------------------------

; ----------------------------------------------------------------------------
; u8 __fastcall__ check_gamepad(void);
;
; Button state is saved as
; 7 - A
; 6 - B
; 5 - Select
; 4 - Start
; 3 - Up
; 2 - Down
; 1 - Left
; 0 - Right

_check_gamepad:
        lda #$01
        sta $4016
        lda #$00
        sta $4016

        sta tmp1
        ldx #8
@loop:
        lda $4016
        lsr A           ; Shift bit -> carry
        rol tmp1        ; Rotate carry -> tmp1
        dex
        bne @loop

        lda tmp1
        rts

; ----------------------------------------------------------------------------
; void __fastcall__ wait_vblank(void);

_wait_vblank:
        bit PPUSTATUS
        bpl _wait_vblank
        rts

; ----------------------------------------------------------------------------
; void __fastcall__ play_sound(u8 duration, u8 tone);

_play_sound:
        tax         ; save tone
        jsr popa    ; pop duration from stack into a
        sta $0400

        lda #%00000001
        sta $4015

        lda #%01111111
        sta $4000

        txa    ; tone
        sta $4002
        lda #$00
        sta $4003

        rts

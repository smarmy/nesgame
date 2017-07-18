
        .export _check_gamepad
        .export _wait_vblank
        .export _play_sound
        .export _clear_sprites
        .export _display_life

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

; ----------------------------------------------------------------------------
; void __fastcall__ clear_sprites(void);

_clear_sprites:
        ldx #64
        ldy #0
        lda #0
@loop:
        sta $0200, y
        dex
        beq @done
        iny
        iny
        iny
        iny
        jmp @loop
@done:
        rts

; ----------------------------------------------------------------------------
; void __fastcall__ display_life(void);

        .import _player_life

MAX_LIFE = 10
FIRST_SPRITE = $35
HEART_SPRITE = 33

_display_life:
        ldx #0                ; Loop counter and X position of sprite
        ldy #0                ; Index in sprite array.

@loop:  ; Sprite y coord
        cpx _player_life      ; if loop counter (X) < _player_life, show sprite
        bcc @show_sprite

        lda #0
        jmp @1
@show_sprite:
        lda #8
@1:     sta $0200+(FIRST_SPRITE*4), y
        iny

        ; Sprite index
        lda #33
        sta $0200+(FIRST_SPRITE*4), y
        iny

        ; Sprite attribute
        lda #0
        sta $0200+(FIRST_SPRITE*4), y
        iny

        ; Sprite x coord
        txa
        asl a               ; Multiply by 8
        asl a
        asl a
        sta $0200+(FIRST_SPRITE*4), y
        iny

        inx
        cpx #MAX_LIFE
        bne @loop

        rts

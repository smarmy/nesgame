
        .export _check_gamepad
        .export _wait_vblank
        .export _play_sound
        .export _clear_sprites
        .export _colcheck_objects

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
; u8 __fastcall__ colcheck_objects(u8 obj_index_1, u8 obj_index_2)

.proc   _colcheck_objects: near

.macro  add_bbox    index_var, coord_addr, bbox_addr, dst_addr
        lda index_var           ; Fetch index to objects array
        asl a                   ; Multiply by 2 to get 16 bit index
        tay
        lda coord_addr+1, y     ; +1 to address to get odd bytes (high address)
        sta dst_addr

        lda index_var           ; Fetch index to objects array
        tay
        lda bbox_addr, y
        clc
        adc dst_addr            ; object_x + object_bbox_x1
        sta dst_addr
.endmacro

.macro  sub_bbox    index_var, bbox_addr_2, bbox_addr_1, dst_addr
        lda index_var           ; Fetch index to objects array
        tay
        lda bbox_addr_1, y
        sta dst_addr            ; object_bbox_x1 -> variable

        lda index_var           ; Fetch index to objects array
        tay
        lda bbox_addr_2, y
        sec
        sbc dst_addr            ; object_bbox_x2 - object_bbox_x1
        sta dst_addr
.endmacro

    .import _objects_x
    .import _objects_y
    .import _objects_bbox_x1
    .import _objects_bbox_x2
    .import _objects_bbox_y1
    .import _objects_bbox_y2

.segment    "BSS"

; Define some local variables.
o1x:
    .res    1, $00
o1y:
    .res    1, $00
o2x:
    .res    1, $00
o2y:
    .res    1, $00
o1w:
    .res    1, $00
o1h:
    .res    1, $00
o2w:
    .res    1, $00
o2h:
    .res    1, $00

.segment    "CODE"

        sta tmp2                ; obj_index_2 in tmp2
        jsr popa
        sta tmp1                ; obj_index_1 in tmp1

        ; Compute bounding box positions
        add_bbox tmp1, _objects_x, _objects_bbox_x1, o1x
        add_bbox tmp1, _objects_y, _objects_bbox_y1, o1y
        add_bbox tmp2, _objects_x, _objects_bbox_x1, o2x
        add_bbox tmp2, _objects_y, _objects_bbox_y1, o2y

        ; Compute bounding box sizes
        sub_bbox tmp1, _objects_bbox_x2, _objects_bbox_x1, o1w
        sub_bbox tmp1, _objects_bbox_y2, _objects_bbox_y1, o1h
        sub_bbox tmp2, _objects_bbox_x2, _objects_bbox_x1, o2w
        sub_bbox tmp2, _objects_bbox_y2, _objects_bbox_y1, o2h

        ; if ((o1x + o1w) < o2x) return 0;
        clc
        lda o1x
        adc o1w
        cmp o2x
        bcc @false

        ; if ((o1y + o1h) < o2y) return 0;
        clc
        lda o1y
        adc o1h
        cmp o2y
        bcc @false

        ; if ((o2x + o2w) < o1x) return 0;
        clc
        lda o2x
        adc o2w
        cmp o1x
        bcc @false

        ; if ((o2y + o2h) < o1y) return 0;
        clc
        lda o2y
        adc o2h
        cmp o1y
        bcc @false

@true:
        lda #1
        ldx #0
        rts
@false:
        lda #0
        ldx #0
        rts

.endproc

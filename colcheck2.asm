        .export _colcheck_down
        .export _colcheck_objects

        .import popa
        .import _kill_player
        .import _hurt_player

        .import _tilemap

        .import _objects_x
        .import _objects_y
        .import _objects_bbox_x1
        .import _objects_bbox_x2
        .import _objects_bbox_y1
        .import _objects_bbox_y2
        .import _objects_hdir

        .include "zeropage.inc"
        .include "constants.inc"

; ----------------------------------------------------------------------------
; static u8 is_solid(u8 tile_index)

is_solid:
        tay
        lda _tilemap, y
        cmp #$01
        beq @true
        cmp #$08
        beq @true
        cmp #$09
        beq @true
        cmp #$0C
        beq @true
        cmp #$11
        beq @true
@false:
        lda #1
        rts
@true:
        lda #0
        rts

; ----------------------------------------------------------------------------
; u8 __fastcall__ colcheck_down(u8 obj_index)

;  tile_index_1 = ((fix2i(objects_y[obj_index])+objects_bbox_y2[obj_index]+1) & 0xF0) + ((fix2i(objects_x[obj_index]) + objects_bbox_x1[obj_index]) >> 4);
;  tile_index_2 = ((fix2i(objects_y[obj_index])+objects_bbox_y2[obj_index]+1) & 0xF0) + ((fix2i(objects_x[obj_index]) + objects_bbox_x2[obj_index]) >> 4);


_colcheck_down:
        tax                      ; obj_index -> x

        ; --------------------------------------------------------------------
        ; Calculate left part of bounding box.
        ; --------------------------------------------------------------------
        asl a                    ; Multiply by 2 to get index to 16 bit objects_y array
        tay
        lda _objects_y+1, y      ; +1 to get high byte
        sta tmp1                 ; objects_y[obj_index] -> tmp1
        lda _objects_bbox_y2, x  ; objects_bbox_y2[obj_index]
        sec                      ; Carry bit set
        adc tmp1                 ; objects_y + objects_bbox_y2 + 1
        and #$F0                 ; Remove bottom 4 bits ((y / 16) * 16)
        sta tmp1                 ; tmp1 = tile_index_1
        sta tmp3                 ; tmp3 = ypart

        txa
        asl a
        tay
        lda _objects_x+1, y
        sta tmp4                ; objects_x -> tmp4
        lda _objects_bbox_x1, x
        clc
        adc tmp4                ; objects_x + objects_bbox_x1
        lsr a                   ; Divide by 16
        lsr a
        lsr a
        lsr a
        clc
        adc tmp1                ; Y part + X part
        sta tmp1                ; tile_index_1 -> tmp1

        jsr is_solid
        beq @solid

        ; --------------------------------------------------------------------
        ; Calculate right part of bounding box.
        ; --------------------------------------------------------------------
        ; y-part precalculated (stored in tmp3)
        txa
        asl a
        tay
        lda _objects_x+1, y
        sta tmp4                ; objects_x -> tmp4
        lda _objects_bbox_x2, x
        clc
        adc tmp4                ; objects_x + objects_bbox_x1
        lsr a
        lsr a
        lsr a
        lsr a
        clc
        adc tmp3                ; Y part + X part
        sta tmp2                ; tile_index_1 -> tmp2

        jsr is_solid
        beq @solid

        ; --------------------------------------------------------------------
        ; Check player collision with special tiles.
        ; --------------------------------------------------------------------
        txa                     ; if obj_index == O_PLAYER (0)
        bne @no_collision

        ldy tmp1                ; tile_index_1
        lda _tilemap, y
        cmp #$10
        beq @hurt
        cmp #$06
        beq @kill

        ldy tmp2                ; tile_index_2
        lda _tilemap, y
        cmp #$10
        beq @hurt
        cmp #$06
        beq @kill

        jmp @no_collision
@hurt:
        lda _objects_hdir       ; player hdir first entry in array.
        jsr _hurt_player
        jmp @no_collision
@kill:
        jsr _kill_player
@no_collision:
        lda #0
        ldx #0
        rts
@solid:
        lda #1
        ldx #0
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

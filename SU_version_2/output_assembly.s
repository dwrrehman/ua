	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 13, 0	sdk_version 13, 3
	.section	__TEXT,__literal8,8byte_literals
	.p2align	3                               ; -- Begin function main
lCPI0_0:
	.byte	21                              ; 0x15
	.byte	22                              ; 0x16
	.byte	23                              ; 0x17
	.byte	25                              ; 0x19
	.byte	26                              ; 0x1a
	.byte	27                              ; 0x1b
	.byte	29                              ; 0x1d
	.byte	30                              ; 0x1e
lCPI0_1:
	.short	0                               ; 0x0
	.short	1                               ; 0x1
	.short	3                               ; 0x3
	.short	1028                            ; 0x404
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_main
	.p2align	2
_main:                                  ; @main
	.cfi_startproc
; %bb.0:
	stp	d9, d8, [sp, #-112]!            ; 16-byte Folded Spill
	.cfi_def_cfa_offset 112
	stp	x28, x27, [sp, #16]             ; 16-byte Folded Spill
	stp	x26, x25, [sp, #32]             ; 16-byte Folded Spill
	stp	x24, x23, [sp, #48]             ; 16-byte Folded Spill
	stp	x22, x21, [sp, #64]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #80]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #96]             ; 16-byte Folded Spill
	add	x29, sp, #96
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	.cfi_offset w25, -72
	.cfi_offset w26, -80
	.cfi_offset w27, -88
	.cfi_offset w28, -96
	.cfi_offset b8, -104
	.cfi_offset b9, -112
	sub	sp, sp, #640
	mov	x8, #0
Lloh0:
	adrp	x9, ___stack_chk_guard@GOTPAGE
Lloh1:
	ldr	x9, [x9, ___stack_chk_guard@GOTPAGEOFF]
Lloh2:
	ldr	x9, [x9]
Lloh3:
	adrp	x10, l___const.main.actual_partial_graph@PAGE
Lloh4:
	add	x10, x10, l___const.main.actual_partial_graph@PAGEOFF
	stur	x9, [x29, #-112]
	ldr	x11, [x10]
Lloh5:
	adrp	x20, __MergedGlobals@PAGE
Lloh6:
	add	x20, x20, __MergedGlobals@PAGEOFF
	str	x11, [x20, #48]
	ldrh	w14, [x10, #8]
	strh	w14, [x20, #56]
Lloh7:
	adrp	x9, l___const.main.zerosp_pas_map@PAGE
Lloh8:
	add	x9, x9, l___const.main.zerosp_pas_map@PAGEOFF
	ldr	x10, [x9]
	stur	x10, [x20, #84]
	ldr	w9, [x9, #8]
	str	w9, [x20, #92]
Lloh9:
	adrp	x9, lCPI0_0@PAGE
Lloh10:
	ldr	d0, [x9, lCPI0_0@PAGEOFF]
	str	d0, [x20, #96]
	mov	w9, #31
	strb	w9, [x20, #104]
Lloh11:
	adrp	x9, lCPI0_1@PAGE
Lloh12:
	ldr	d0, [x9, lCPI0_1@PAGEOFF]
	stur	d0, [x20, #58]
	mov	w9, #8993
	sturh	w9, [x20, #105]
	mov	w9, #1
	strb	w9, [x20]
	and	w10, w11, #0xf
	ldrh	w11, [x20, #50]
	and	w11, w11, #0xf
	ldrh	w12, [x20, #52]
	and	w12, w12, #0xf
	ldrh	w13, [x20, #54]
	and	w13, w13, #0xf
	and	w14, w14, #0xf
	ldrh	w15, [x20, #58]
	and	w15, w15, #0xf
	ldrh	w16, [x20, #60]
	and	w16, w16, #0xf
	ldrh	w17, [x20, #62]
	and	w17, w17, #0xf
	ldrh	w0, [x20, #64]
	and	w0, w0, #0xf
	mov	w1, #2
Lloh13:
	adrp	x2, _banned_edges@PAGE
Lloh14:
	add	x2, x2, _banned_edges@PAGEOFF
	mov	w4, #4
	mov	w5, #5
	mov	w6, #6
	mov	w7, #7
	mov	w21, #8
	mov	x28, x1
	ldp	x19, x23, [x20, #8]
	b	LBB0_3
LBB0_1:                                 ;   in Loop: Header=BB0_3 Depth=1
	orr	x23, x23, x22
	adrp	x9, __MergedGlobals@PAGE+16
	str	x23, [x9, __MergedGlobals@PAGEOFF+16]
	add	x8, x8, #1
LBB0_2:                                 ;   in Loop: Header=BB0_3 Depth=1
	add	x9, x20, x8
	ldrb	w28, [x9, #84]
LBB0_3:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB0_5 Depth 2
                                        ;     Child Loop BB0_15 Depth 2
                                        ;     Child Loop BB0_25 Depth 2
                                        ;     Child Loop BB0_34 Depth 2
                                        ;     Child Loop BB0_43 Depth 2
                                        ;     Child Loop BB0_52 Depth 2
                                        ;     Child Loop BB0_61 Depth 2
                                        ;     Child Loop BB0_70 Depth 2
                                        ;     Child Loop BB0_79 Depth 2
	mov	x30, #0
	ubfx	x26, x28, #2, #6
	and	x24, x28, #0x3
	add	x22, x20, x26, lsl #1
	ldrh	w25, [x22, #48]
	and	w27, w25, #0xf
	b	LBB0_5
LBB0_4:                                 ;   in Loop: Header=BB0_5 Depth=2
	add	x9, x30, #3
	cmp	x30, #66
	mov	x30, x9
	b.hs	LBB0_7
LBB0_5:                                 ;   Parent Loop BB0_3 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldrb	w22, [x2, x30]
	cmp	w27, w22
	b.ne	LBB0_4
; %bb.6:                                ;   in Loop: Header=BB0_5 Depth=2
	add	x22, x2, x30
	ldrb	w9, [x22, #2]
	ldrb	w22, [x22, #1]
	cmp	x24, x22
	ccmp	w10, w9, #0, eq
	b.ne	LBB0_4
	b	LBB0_11
LBB0_7:                                 ;   in Loop: Header=BB0_3 Depth=1
	cmp	w28, #3
	b.hi	LBB0_12
; %bb.8:                                ;   in Loop: Header=BB0_3 Depth=1
	and	w22, w25, #0xf
	cmp	w22, #1
	b.eq	LBB0_10
; %bb.9:                                ;   in Loop: Header=BB0_3 Depth=1
	cbnz	w22, LBB0_12
LBB0_10:                                ;   in Loop: Header=BB0_3 Depth=1
	cmp	x24, #1
	b.ne	LBB0_12
LBB0_11:                                ;   in Loop: Header=BB0_3 Depth=1
	mov	x28, #0
	b	LBB0_13
LBB0_12:                                ;   in Loop: Header=BB0_3 Depth=1
	mov	w28, #1
LBB0_13:                                ;   in Loop: Header=BB0_3 Depth=1
	mov	x30, #0
	b	LBB0_15
LBB0_14:                                ;   in Loop: Header=BB0_15 Depth=2
	add	x9, x30, #3
	cmp	x30, #66
	mov	x30, x9
	b.hs	LBB0_17
LBB0_15:                                ;   Parent Loop BB0_3 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldrb	w9, [x2, x30]
	cmp	w27, w9
	b.ne	LBB0_14
; %bb.16:                               ;   in Loop: Header=BB0_15 Depth=2
	add	x9, x2, x30
	ldrb	w22, [x9, #2]
	ldrb	w9, [x9, #1]
	cmp	x24, x9
	ccmp	w11, w22, #0, eq
	b.ne	LBB0_14
	b	LBB0_21
LBB0_17:                                ;   in Loop: Header=BB0_3 Depth=1
	cmp	x26, #1
	b.ne	LBB0_22
; %bb.18:                               ;   in Loop: Header=BB0_3 Depth=1
	ands	w22, w25, #0xf
	b.eq	LBB0_20
; %bb.19:                               ;   in Loop: Header=BB0_3 Depth=1
	cmp	w22, #1
	b.ne	LBB0_22
LBB0_20:                                ;   in Loop: Header=BB0_3 Depth=1
	cmp	x24, #1
	b.ne	LBB0_22
LBB0_21:                                ;   in Loop: Header=BB0_3 Depth=1
	mov	x30, #0
	b	LBB0_23
LBB0_22:                                ;   in Loop: Header=BB0_3 Depth=1
	lsl	x9, x28, #2
	mov	w3, #1
	lsl	x30, x3, x9
	add	x28, x28, #1
LBB0_23:                                ;   in Loop: Header=BB0_3 Depth=1
	mov	x22, #0
	b	LBB0_25
LBB0_24:                                ;   in Loop: Header=BB0_25 Depth=2
	add	x9, x22, #3
	cmp	x22, #66
	mov	x22, x9
	b.hs	LBB0_27
LBB0_25:                                ;   Parent Loop BB0_3 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldrb	w9, [x2, x22]
	cmp	w27, w9
	b.ne	LBB0_24
; %bb.26:                               ;   in Loop: Header=BB0_25 Depth=2
	add	x9, x2, x22
	ldrb	w3, [x9, #2]
	ldrb	w9, [x9, #1]
	cmp	x24, x9
	ccmp	w12, w3, #0, eq
	b.ne	LBB0_24
	b	LBB0_32
LBB0_27:                                ;   in Loop: Header=BB0_3 Depth=1
	cmp	x26, #2
	b.ne	LBB0_31
; %bb.28:                               ;   in Loop: Header=BB0_3 Depth=1
	ands	w22, w25, #0xf
	b.eq	LBB0_30
; %bb.29:                               ;   in Loop: Header=BB0_3 Depth=1
	cmp	w22, #1
	b.ne	LBB0_31
LBB0_30:                                ;   in Loop: Header=BB0_3 Depth=1
	cmp	x24, #1
	b.eq	LBB0_32
LBB0_31:                                ;   in Loop: Header=BB0_3 Depth=1
	lsl	x9, x28, #2
	lsl	x9, x1, x9
	orr	x30, x9, x30
	add	x28, x28, #1
LBB0_32:                                ;   in Loop: Header=BB0_3 Depth=1
	mov	x22, #0
	b	LBB0_34
LBB0_33:                                ;   in Loop: Header=BB0_34 Depth=2
	add	x9, x22, #3
	cmp	x22, #66
	mov	x22, x9
	b.hs	LBB0_36
LBB0_34:                                ;   Parent Loop BB0_3 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldrb	w9, [x2, x22]
	cmp	w27, w9
	b.ne	LBB0_33
; %bb.35:                               ;   in Loop: Header=BB0_34 Depth=2
	add	x9, x2, x22
	ldrb	w3, [x9, #2]
	ldrb	w9, [x9, #1]
	cmp	x24, x9
	ccmp	w13, w3, #0, eq
	b.ne	LBB0_33
	b	LBB0_41
LBB0_36:                                ;   in Loop: Header=BB0_3 Depth=1
	cmp	x26, #3
	b.ne	LBB0_40
; %bb.37:                               ;   in Loop: Header=BB0_3 Depth=1
	ands	w22, w25, #0xf
	b.eq	LBB0_39
; %bb.38:                               ;   in Loop: Header=BB0_3 Depth=1
	cmp	w22, #1
	b.ne	LBB0_40
LBB0_39:                                ;   in Loop: Header=BB0_3 Depth=1
	cmp	x24, #1
	b.eq	LBB0_41
LBB0_40:                                ;   in Loop: Header=BB0_3 Depth=1
	lsl	x9, x28, #2
	mov	w3, #3
	lsl	x9, x3, x9
	orr	x30, x9, x30
	add	x28, x28, #1
LBB0_41:                                ;   in Loop: Header=BB0_3 Depth=1
	mov	x22, #0
	b	LBB0_43
LBB0_42:                                ;   in Loop: Header=BB0_43 Depth=2
	add	x9, x22, #3
	cmp	x22, #66
	mov	x22, x9
	b.hs	LBB0_45
LBB0_43:                                ;   Parent Loop BB0_3 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldrb	w9, [x2, x22]
	cmp	w27, w9
	b.ne	LBB0_42
; %bb.44:                               ;   in Loop: Header=BB0_43 Depth=2
	add	x9, x2, x22
	ldrb	w3, [x9, #2]
	ldrb	w9, [x9, #1]
	cmp	x24, x9
	ccmp	w14, w3, #0, eq
	b.ne	LBB0_42
	b	LBB0_50
LBB0_45:                                ;   in Loop: Header=BB0_3 Depth=1
	cmp	x26, #4
	b.ne	LBB0_49
; %bb.46:                               ;   in Loop: Header=BB0_3 Depth=1
	ands	w22, w25, #0xf
	b.eq	LBB0_48
; %bb.47:                               ;   in Loop: Header=BB0_3 Depth=1
	cmp	w22, #1
	b.ne	LBB0_49
LBB0_48:                                ;   in Loop: Header=BB0_3 Depth=1
	cmp	x24, #1
	b.eq	LBB0_50
LBB0_49:                                ;   in Loop: Header=BB0_3 Depth=1
	lsl	x9, x28, #2
	lsl	x9, x4, x9
	orr	x30, x9, x30
	add	x28, x28, #1
LBB0_50:                                ;   in Loop: Header=BB0_3 Depth=1
	mov	x22, #0
	b	LBB0_52
LBB0_51:                                ;   in Loop: Header=BB0_52 Depth=2
	add	x9, x22, #3
	cmp	x22, #66
	mov	x22, x9
	b.hs	LBB0_54
LBB0_52:                                ;   Parent Loop BB0_3 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldrb	w9, [x2, x22]
	cmp	w27, w9
	b.ne	LBB0_51
; %bb.53:                               ;   in Loop: Header=BB0_52 Depth=2
	add	x9, x2, x22
	ldrb	w3, [x9, #2]
	ldrb	w9, [x9, #1]
	cmp	x24, x9
	ccmp	w15, w3, #0, eq
	b.ne	LBB0_51
	b	LBB0_59
LBB0_54:                                ;   in Loop: Header=BB0_3 Depth=1
	cmp	x26, #5
	b.ne	LBB0_58
; %bb.55:                               ;   in Loop: Header=BB0_3 Depth=1
	ands	w22, w25, #0xf
	b.eq	LBB0_57
; %bb.56:                               ;   in Loop: Header=BB0_3 Depth=1
	cmp	w22, #1
	b.ne	LBB0_58
LBB0_57:                                ;   in Loop: Header=BB0_3 Depth=1
	cmp	x24, #1
	b.eq	LBB0_59
LBB0_58:                                ;   in Loop: Header=BB0_3 Depth=1
	lsl	x9, x28, #2
	lsl	x9, x5, x9
	orr	x30, x9, x30
	add	x28, x28, #1
LBB0_59:                                ;   in Loop: Header=BB0_3 Depth=1
	mov	x22, #0
	b	LBB0_61
LBB0_60:                                ;   in Loop: Header=BB0_61 Depth=2
	add	x9, x22, #3
	cmp	x22, #66
	mov	x22, x9
	b.hs	LBB0_63
LBB0_61:                                ;   Parent Loop BB0_3 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldrb	w9, [x2, x22]
	cmp	w27, w9
	b.ne	LBB0_60
; %bb.62:                               ;   in Loop: Header=BB0_61 Depth=2
	add	x9, x2, x22
	ldrb	w3, [x9, #2]
	ldrb	w9, [x9, #1]
	cmp	x24, x9
	ccmp	w16, w3, #0, eq
	b.ne	LBB0_60
	b	LBB0_68
LBB0_63:                                ;   in Loop: Header=BB0_3 Depth=1
	cmp	x26, #6
	b.ne	LBB0_67
; %bb.64:                               ;   in Loop: Header=BB0_3 Depth=1
	ands	w22, w25, #0xf
	b.eq	LBB0_66
; %bb.65:                               ;   in Loop: Header=BB0_3 Depth=1
	cmp	w22, #1
	b.ne	LBB0_67
LBB0_66:                                ;   in Loop: Header=BB0_3 Depth=1
	cmp	x24, #1
	b.eq	LBB0_68
LBB0_67:                                ;   in Loop: Header=BB0_3 Depth=1
	lsl	x9, x28, #2
	lsl	x9, x6, x9
	orr	x30, x9, x30
	add	x28, x28, #1
LBB0_68:                                ;   in Loop: Header=BB0_3 Depth=1
	mov	x22, #0
	b	LBB0_70
LBB0_69:                                ;   in Loop: Header=BB0_70 Depth=2
	add	x9, x22, #3
	cmp	x22, #66
	mov	x22, x9
	b.hs	LBB0_72
LBB0_70:                                ;   Parent Loop BB0_3 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldrb	w9, [x2, x22]
	cmp	w27, w9
	b.ne	LBB0_69
; %bb.71:                               ;   in Loop: Header=BB0_70 Depth=2
	add	x9, x2, x22
	ldrb	w3, [x9, #2]
	ldrb	w9, [x9, #1]
	cmp	x24, x9
	ccmp	w17, w3, #0, eq
	b.ne	LBB0_69
	b	LBB0_77
LBB0_72:                                ;   in Loop: Header=BB0_3 Depth=1
	cmp	x26, #7
	b.ne	LBB0_76
; %bb.73:                               ;   in Loop: Header=BB0_3 Depth=1
	ands	w22, w25, #0xf
	b.eq	LBB0_75
; %bb.74:                               ;   in Loop: Header=BB0_3 Depth=1
	cmp	w22, #1
	b.ne	LBB0_76
LBB0_75:                                ;   in Loop: Header=BB0_3 Depth=1
	cmp	x24, #1
	b.eq	LBB0_77
LBB0_76:                                ;   in Loop: Header=BB0_3 Depth=1
	lsl	x9, x28, #2
	lsl	x9, x7, x9
	orr	x30, x9, x30
	add	x28, x28, #1
LBB0_77:                                ;   in Loop: Header=BB0_3 Depth=1
	mov	x22, #0
	b	LBB0_79
LBB0_78:                                ;   in Loop: Header=BB0_79 Depth=2
	add	x9, x22, #3
	cmp	x22, #66
	mov	x22, x9
	b.hs	LBB0_81
LBB0_79:                                ;   Parent Loop BB0_3 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldrb	w9, [x2, x22]
	cmp	w27, w9
	b.ne	LBB0_78
; %bb.80:                               ;   in Loop: Header=BB0_79 Depth=2
	add	x9, x2, x22
	ldrb	w3, [x9, #2]
	ldrb	w9, [x9, #1]
	cmp	x24, x9
	ccmp	w0, w3, #0, eq
	b.ne	LBB0_78
	b	LBB0_86
LBB0_81:                                ;   in Loop: Header=BB0_3 Depth=1
	cmp	x26, #8
	b.ne	LBB0_85
; %bb.82:                               ;   in Loop: Header=BB0_3 Depth=1
	ands	w22, w25, #0xf
	b.eq	LBB0_84
; %bb.83:                               ;   in Loop: Header=BB0_3 Depth=1
	cmp	w22, #1
	b.ne	LBB0_85
LBB0_84:                                ;   in Loop: Header=BB0_3 Depth=1
	cmp	x24, #1
	b.eq	LBB0_86
LBB0_85:                                ;   in Loop: Header=BB0_3 Depth=1
	lsl	x9, x28, #2
	lsl	x9, x21, x9
	orr	x30, x9, x30
	add	x28, x28, #1
LBB0_86:                                ;   in Loop: Header=BB0_3 Depth=1
	add	x9, x20, x8, lsl #3
	str	x30, [x9, #216]
	lsl	w9, w8, #2
	and	x9, x9, #0x3c
	lsl	x22, x28, x9
	cmp	x8, #15
	b.ls	LBB0_1
; %bb.87:                               ;   in Loop: Header=BB0_3 Depth=1
	orr	x19, x19, x22
	add	x8, x8, #1
	cmp	x8, #23
	b.ne	LBB0_2
; %bb.88:
Lloh15:
	adrp	x21, __MergedGlobals@PAGE+1
Lloh16:
	add	x21, x21, __MergedGlobals@PAGEOFF+1
	stur	x19, [x21, #7]
	mov	w8, #1
	strb	w8, [x21]
	ubfx	x8, x19, #12, #16
	stur	x8, [x21, #23]
	mov	w8, #23
	str	x8, [sp]
Lloh17:
	adrp	x0, l_.str@PAGE
Lloh18:
	add	x0, x0, l_.str@PAGEOFF
	bl	_printf
	ldur	x8, [x21, #23]
	str	x8, [sp]
Lloh19:
	adrp	x0, l_.str.1@PAGE
Lloh20:
	add	x0, x0, l_.str.1@PAGEOFF
	bl	_printf
	ldrb	w8, [x21]
	cmp	w8, #0
	mov	w8, #12
	csel	x8, x8, xzr, ne
	str	x8, [sp]
Lloh21:
	adrp	x0, l_.str.2@PAGE
Lloh22:
	add	x0, x0, l_.str.2@PAGEOFF
	bl	_printf
	ldur	x8, [x21, #15]
	ldur	x9, [x21, #7]
	stp	x8, x9, [sp]
Lloh23:
	adrp	x0, l_.str.3@PAGE
Lloh24:
	add	x0, x0, l_.str.3@PAGEOFF
	bl	_printf
Lloh25:
	adrp	x0, l_.str.4@PAGE
Lloh26:
	add	x0, x0, l_.str.4@PAGEOFF
	bl	_printf
	mov	w21, #0
Lloh27:
	adrp	x19, l_.str.26@PAGE
Lloh28:
	add	x19, x19, l_.str.26@PAGEOFF
LBB0_89:                                ; =>This Inner Loop Header: Depth=1
	lsr	w8, w21, #2
	add	x8, x20, w8, uxth #1
	ldrh	w8, [x8, #48]
	ubfiz	w9, w21, #2, #2
	lsr	w8, w8, w9
	and	w8, w8, #0xf
	str	x8, [sp]
	mov	x0, x19
	bl	_printf
	add	w8, w21, #1
	cmp	w21, #35
	mov	x21, x8
	b.lo	LBB0_89
; %bb.90:
	mov	w0, #10
	bl	_putchar
Lloh29:
	adrp	x0, l_.str.6@PAGE
Lloh30:
	add	x0, x0, l_.str.6@PAGEOFF
	bl	_printf
	adrp	x21, __MergedGlobals@PAGE
	ldrb	w8, [x21, __MergedGlobals@PAGEOFF]
	cmp	w8, #1
	b.ne	LBB0_93
; %bb.91:
	mov	x22, #0
Lloh31:
	adrp	x19, l_.str.7@PAGE
Lloh32:
	add	x19, x19, l_.str.7@PAGEOFF
	mov	w23, #23
LBB0_92:                                ; =>This Inner Loop Header: Depth=1
	add	x8, x20, x22
	ldrb	w8, [x8, #84]
	str	x8, [sp]
	mov	x0, x19
	bl	_printf
	add	x22, x22, #1
	ldrb	w8, [x20]
	cmp	w8, #0
	csel	x8, x23, xzr, ne
	cmp	x22, x8
	b.lo	LBB0_92
LBB0_93:
Lloh33:
	adrp	x0, l_str.81@PAGE
Lloh34:
	add	x0, x0, l_str.81@PAGEOFF
	bl	_puts
Lloh35:
	adrp	x0, l_.str.9@PAGE
Lloh36:
	add	x0, x0, l_.str.9@PAGEOFF
	bl	_printf
	ldrb	w8, [x21, __MergedGlobals@PAGEOFF]
	cmp	w8, #1
	b.ne	LBB0_98
; %bb.94:
	mov	x21, #0
Lloh37:
	adrp	x19, l_.str.10@PAGE
Lloh38:
	add	x19, x19, l_.str.10@PAGEOFF
	mov	w22, #23
	b	LBB0_96
LBB0_95:                                ;   in Loop: Header=BB0_96 Depth=1
	add	x8, x20, x21, lsl #3
	ldr	x8, [x8, #216]
	str	x8, [sp]
	mov	x0, x19
	bl	_printf
	add	x21, x21, #1
	ldrb	w8, [x20]
	cmp	w8, #0
	csel	x8, x22, xzr, ne
	cmp	x21, x8
	b.hs	LBB0_98
LBB0_96:                                ; =>This Inner Loop Header: Depth=1
	tst	x21, #0x3
	b.ne	LBB0_95
; %bb.97:                               ;   in Loop: Header=BB0_96 Depth=1
	mov	w0, #10
	bl	_putchar
	b	LBB0_95
LBB0_98:
Lloh39:
	adrp	x0, l_str.81@PAGE
Lloh40:
	add	x0, x0, l_str.81@PAGEOFF
	bl	_puts
	mov	x0, #0
	bl	_time
                                        ; kill: def $w0 killed $w0 killed $x0
	bl	_srand
	mov	w25, #1
	mov	w0, #1
	mov	w1, #8
	bl	_calloc
	mov	x26, x0
Lloh41:
	adrp	x20, __MergedGlobals@PAGE+24
Lloh42:
	add	x20, x20, __MergedGlobals@PAGEOFF+24
	str	xzr, [x20, #8]
	stur	wzr, [x20, #42]
	mov	w0, #4096
	mov	w1, #1
	bl	_calloc
	str	x0, [x20, #16]
	movi.2d	v0, #0000000000000000
	stp	q0, q0, [x29, #-144]
	add	x0, sp, #128
	mov	x1, #0
	bl	_gettimeofday
	add	x0, sp, #128
	bl	_localtime
	mov	x3, x0
Lloh43:
	adrp	x2, l_.str.20@PAGE
Lloh44:
	add	x2, x2, l_.str.20@PAGEOFF
	sub	x24, x29, #144
	sub	x0, x29, #144
	mov	w1, #32
	bl	_strftime
	ldr	x19, [x20, #16]
	bl	_rand
	mov	x21, x0
	bl	_rand
	mov	x22, x0
	bl	_rand
	mov	x23, x0
	bl	_rand
                                        ; kill: def $w0 killed $w0 def $x0
	stp	x23, x0, [sp, #24]
	stp	x21, x22, [sp, #8]
	str	x24, [sp]
Lloh45:
	adrp	x2, l_.str.11@PAGE
Lloh46:
	add	x2, x2, l_.str.11@PAGEOFF
	mov	x0, x19
	mov	w1, #4096
	bl	_snprintf
	mov	w8, #0
	ldr	x12, [x20]
	and	x9, x12, #0xf
	sub	x9, x9, #1
	ubfx	x10, x12, #4, #4
	sub	x10, x10, #1
	ubfx	x11, x12, #8, #4
	sub	x11, x11, #1
	ubfx	x12, x12, #12, #4
	sub	x12, x12, #1
	mov	x24, #-1
Lloh47:
	adrp	x13, __MergedGlobals@PAGE+32
Lloh48:
	add	x13, x13, __MergedGlobals@PAGEOFF+32
Lloh49:
	adrp	x21, _queue@PAGE
Lloh50:
	add	x21, x21, _queue@PAGEOFF
	b	LBB0_101
LBB0_99:                                ;   in Loop: Header=BB0_101 Depth=1
	mov	w14, #0
LBB0_100:                               ;   in Loop: Header=BB0_101 Depth=1
	lsl	w14, w25, w14
	add	w8, w8, w14
	sub	x24, x24, #1
LBB0_101:                               ; =>This Inner Loop Header: Depth=1
	ldadd	x25, x14, [x13]
	strh	w8, [x21, x14, lsl #1]
	and	x14, x8, #0xf
	cmp	x9, x14
	b.hi	LBB0_99
; %bb.102:                              ;   in Loop: Header=BB0_101 Depth=1
	ubfx	w14, w8, #4, #4
	cmp	x10, x14
	b.ls	LBB0_104
; %bb.103:                              ;   in Loop: Header=BB0_101 Depth=1
	and	w8, w8, #0xfffffff0
	mov	w14, #4
	b	LBB0_100
LBB0_104:                               ;   in Loop: Header=BB0_101 Depth=1
	ubfx	w14, w8, #8, #4
	cmp	x11, x14
	b.ls	LBB0_106
; %bb.105:                              ;   in Loop: Header=BB0_101 Depth=1
	and	w8, w8, #0xffffff00
	mov	w14, #8
	b	LBB0_100
LBB0_106:                               ;   in Loop: Header=BB0_101 Depth=1
	ubfx	w14, w8, #12, #4
	cmp	x12, x14
	b.ls	LBB0_108
; %bb.107:                              ;   in Loop: Header=BB0_101 Depth=1
	and	w8, w8, #0xfffff000
	mov	w14, #12
	b	LBB0_100
LBB0_108:
	neg	x27, x24
	str	x27, [sp]
Lloh51:
	adrp	x0, l_.str.12@PAGE
Lloh52:
	add	x0, x0, l_.str.12@PAGEOFF
	bl	_printf
	cbz	x24, LBB0_117
; %bb.109:
	mov	x20, #0
	neg	x22, x24
Lloh53:
	adrp	x19, l_.str.13@PAGE
Lloh54:
	add	x19, x19, l_.str.13@PAGEOFF
	b	LBB0_112
LBB0_110:                               ;   in Loop: Header=BB0_112 Depth=1
	mov	w0, #32
	bl	_putchar
	tst	x20, #0x7
	b.eq	LBB0_116
LBB0_111:                               ;   in Loop: Header=BB0_112 Depth=1
	ldrh	w8, [x21, x20, lsl #1]
	str	x8, [sp]
	mov	x0, x19
	bl	_printf
	add	x20, x20, #1
	cmp	x22, x20
	b.eq	LBB0_117
LBB0_112:                               ; =>This Inner Loop Header: Depth=1
	tbnz	w20, #0, LBB0_114
; %bb.113:                              ;   in Loop: Header=BB0_112 Depth=1
	mov	w0, #32
	bl	_putchar
	tst	x20, #0x3
	b.ne	LBB0_115
	b	LBB0_110
LBB0_114:                               ;   in Loop: Header=BB0_112 Depth=1
	tst	x20, #0x3
	b.eq	LBB0_110
LBB0_115:                               ;   in Loop: Header=BB0_112 Depth=1
	tst	x20, #0x7
	b.ne	LBB0_111
LBB0_116:                               ;   in Loop: Header=BB0_112 Depth=1
	mov	w0, #10
	bl	_putchar
	b	LBB0_111
LBB0_117:
	mov	w0, #10
	bl	_putchar
	bl	_getchar
	mov	w8, #4
Lloh55:
	adrp	x0, _main.output_string@PAGE
Lloh56:
	add	x0, x0, _main.output_string@PAGEOFF
	str	x8, [sp]
Lloh57:
	adrp	x2, l_.str.14@PAGE
Lloh58:
	add	x2, x2, l_.str.14@PAGEOFF
	mov	w1, #4096
	bl	_snprintf
	bl	_print
	stp	xzr, xzr, [sp, #112]
	add	x0, sp, #112
	mov	x1, #0
	bl	_gettimeofday
	mov	w0, #8
	bl	_malloc
	mov	x3, x0
	str	xzr, [x0]
Lloh59:
	adrp	x2, _worker_thread@PAGE
Lloh60:
	add	x2, x2, _worker_thread@PAGEOFF
	str	x26, [sp, #88]                  ; 8-byte Folded Spill
	mov	x0, x26
	mov	x1, #0
	bl	_pthread_create
	stp	xzr, xzr, [x29, #-168]
	sturh	wzr, [x29, #-152]
	movi.2d	v0, #0000000000000000
	stp	q0, q0, [sp, #208]
	stp	q0, q0, [sp, #240]
	stp	q0, q0, [sp, #272]
	stp	q0, q0, [sp, #304]
	stp	q0, q0, [sp, #336]
	stp	q0, q0, [sp, #368]
	stp	q0, q0, [sp, #400]
	stp	q0, q0, [sp, #432]
	stp	q0, q0, [sp, #464]
	stp	q0, q0, [sp, #496]
	stp	q0, q0, [sp, #528]
Lloh61:
	adrp	x8, __MergedGlobals@PAGE+32
Lloh62:
	ldr	x28, [x8, __MergedGlobals@PAGEOFF+32]
	cmp	x28, #1
	b.lt	LBB0_122
; %bb.118:
	ucvtf	d0, x27
Lloh63:
	adrp	x21, l_.str.15@PAGE
Lloh64:
	add	x21, x21, l_.str.15@PAGEOFF
Lloh65:
	adrp	x22, l_.str.16@PAGE
Lloh66:
	add	x22, x22, l_.str.16@PAGEOFF
	fmov	d1, #1.00000000
Lloh67:
	adrp	x23, l_.str.17@PAGE
Lloh68:
	add	x23, x23, l_.str.17@PAGEOFF
	fdiv	d8, d1, d0
Lloh69:
	adrp	x20, __MergedGlobals@PAGE+66
Lloh70:
	add	x20, x20, __MergedGlobals@PAGEOFF+66
	neg	x26, x24
Lloh71:
	adrp	x24, l_.str.18@PAGE
Lloh72:
	add	x24, x24, l_.str.18@PAGEOFF
	sub	x19, x29, #168
Lloh73:
	adrp	x25, l_.str.26@PAGE
Lloh74:
	add	x25, x25, l_.str.26@PAGEOFF
LBB0_119:                               ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB0_120 Depth 2
	mov	x0, x21
	bl	_printf
	stp	x28, x27, [sp]
	mov	x0, x22
	bl	_printf
	sub	x8, x26, x28
	ucvtf	d0, x8
	fmul	d0, d0, d8
	str	d0, [sp]
	mov	x0, x23
	bl	_printf
	ldrh	w8, [x20]
	sturh	w8, [x29, #-168]
	ldrh	w8, [x20, #2]
	sturh	w8, [x29, #-166]
	ldrh	w8, [x20, #4]
	sturh	w8, [x29, #-164]
	ldrh	w8, [x20, #6]
	sturh	w8, [x29, #-162]
	ldrh	w8, [x20, #8]
	sturh	w8, [x29, #-160]
	ldrh	w8, [x20, #10]
	sturh	w8, [x29, #-158]
	ldrh	w8, [x20, #12]
	sturh	w8, [x29, #-156]
	ldrh	w8, [x20, #14]
	sturh	w8, [x29, #-154]
	ldrh	w8, [x20, #16]
	sturh	w8, [x29, #-152]
	str	xzr, [sp]
	mov	x0, x24
	bl	_printf
	mov	w28, #0
LBB0_120:                               ;   Parent Loop BB0_119 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ubfx	x8, x28, #2, #16
	ldrh	w8, [x19, x8, lsl #1]
	ubfiz	w9, w28, #2, #2
	lsr	w8, w8, w9
	and	w8, w8, #0xf
	str	x8, [sp]
	mov	x0, x25
	bl	_printf
	add	w8, w28, #1
	cmp	w28, #35
	mov	x28, x8
	b.lo	LBB0_120
; %bb.121:                              ;   in Loop: Header=BB0_119 Depth=1
	mov	w0, #10
	bl	_putchar
	mov	w0, #10
	bl	_putchar
	mov	w0, #10000
	bl	_usleep
Lloh75:
	adrp	x8, __MergedGlobals@PAGE+32
Lloh76:
	ldr	x28, [x8, __MergedGlobals@PAGEOFF+32]
	cmp	x28, #0
	b.gt	LBB0_119
LBB0_122:
Lloh77:
	adrp	x0, l_.str.19@PAGE
Lloh78:
	add	x0, x0, l_.str.19@PAGEOFF
	bl	_puts
	stur	xzr, [x29, #-144]
	ldr	x8, [sp, #88]                   ; 8-byte Folded Reload
	ldr	x0, [x8]
	sub	x1, x29, #144
	bl	_pthread_join
	ldur	x8, [x29, #-144]
	ldur	x0, [x29, #-144]
	ldp	q0, q1, [x8]
	ldp	q2, q3, [sp, #208]
	add.2d	v0, v2, v0
	add.2d	v1, v3, v1
	stp	q0, q1, [sp, #208]
	ldp	q0, q1, [x8, #32]
	ldp	q2, q3, [sp, #240]
	add.2d	v0, v2, v0
	add.2d	v1, v3, v1
	stp	q0, q1, [sp, #240]
	ldp	q0, q1, [x8, #64]
	ldp	q2, q3, [sp, #272]
	add.2d	v0, v2, v0
	add.2d	v1, v3, v1
	stp	q0, q1, [sp, #272]
	ldp	q0, q1, [x8, #96]
	ldp	q2, q3, [sp, #304]
	add.2d	v0, v2, v0
	add.2d	v1, v3, v1
	stp	q0, q1, [sp, #304]
	ldp	q0, q1, [x8, #128]
	ldp	q2, q3, [sp, #336]
	add.2d	v0, v2, v0
	add.2d	v1, v3, v1
	stp	q0, q1, [sp, #336]
	ldp	q0, q1, [x8, #160]
	ldp	q2, q3, [sp, #368]
	add.2d	v0, v2, v0
	add.2d	v1, v3, v1
	stp	q0, q1, [sp, #368]
	ldp	q0, q1, [x8, #192]
	ldp	q2, q3, [sp, #400]
	add.2d	v0, v2, v0
	add.2d	v1, v3, v1
	stp	q0, q1, [sp, #400]
	ldp	q0, q1, [x8, #224]
	ldp	q2, q3, [sp, #432]
	add.2d	v0, v2, v0
	add.2d	v1, v3, v1
	stp	q0, q1, [sp, #432]
	ldp	q0, q1, [x8, #256]
	ldp	q2, q3, [sp, #464]
	add.2d	v0, v2, v0
	add.2d	v1, v3, v1
	stp	q0, q1, [sp, #464]
	ldp	q0, q1, [x8, #288]
	ldp	q2, q3, [sp, #496]
	add.2d	v0, v2, v0
	add.2d	v1, v3, v1
	ldp	q2, q3, [x8, #320]
	stp	q0, q1, [sp, #496]
	ldp	q0, q1, [sp, #528]
	add.2d	v0, v0, v2
	add.2d	v1, v1, v3
	stp	q0, q1, [sp, #528]
	bl	_free
	stp	xzr, xzr, [sp, #96]
	add	x0, sp, #96
	mov	x1, #0
	bl	_gettimeofday
	ldr	x0, [sp, #96]
	ldr	x1, [sp, #112]
	bl	_difftime
	fmov	d8, d0
	movi.2d	v0, #0000000000000000
	stp	q0, q0, [sp, #176]
	stp	q0, q0, [sp, #144]
	add	x0, sp, #96
	bl	_localtime
	mov	x3, x0
Lloh79:
	adrp	x20, l_.str.20@PAGE
Lloh80:
	add	x20, x20, l_.str.20@PAGEOFF
	add	x19, sp, #144
	add	x0, sp, #144
	mov	w1, #32
	mov	x2, x20
	bl	_strftime
	add	x0, sp, #112
	bl	_localtime
	mov	x3, x0
	add	x21, sp, #176
	add	x0, sp, #176
	mov	w1, #32
	mov	x2, x20
	bl	_strftime
	stp	x21, x19, [sp, #72]
	str	d8, [sp, #64]
	mov	w8, #16960
	movk	w8, #15, lsl #16
	mov	w9, #57600
	movk	w9, #1525, lsl #16
	stp	x9, x8, [sp, #48]
	mov	w8, #2
	mov	w9, #1
	stp	x9, x8, [sp, #32]
	stp	xzr, x9, [sp, #16]
Lloh81:
	adrp	x2, l_.str.21@PAGE
Lloh82:
	add	x2, x2, l_.str.21@PAGEOFF
	mov	w8, #4
	stp	x8, x9, [sp]
Lloh83:
	adrp	x25, _main.output_string@PAGE
Lloh84:
	add	x25, x25, _main.output_string@PAGEOFF
	mov	x0, x25
	mov	w1, #4096
	bl	_snprintf
	bl	_print
Lloh85:
	adrp	x8, l_.str.22@PAGE
Lloh86:
	add	x8, x8, l_.str.22@PAGEOFF
	ldr	x9, [x8]
	str	x9, [x25]
	ldur	x8, [x8, #5]
	stur	x8, [x25, #5]
	bl	_print
	mov	x21, #0
	adrp	x19, _main.output_string@PAGE
Lloh87:
	adrp	x22, _pm_spelling@PAGE
Lloh88:
	add	x22, x22, _pm_spelling@PAGEOFF
	add	x23, sp, #208
Lloh89:
	adrp	x20, l_.str.24@PAGE
Lloh90:
	add	x20, x20, l_.str.24@PAGEOFF
	mov	w24, #10
	b	LBB0_124
LBB0_123:                               ;   in Loop: Header=BB0_124 Depth=1
	lsl	x8, x21, #3
	ldr	x9, [x22, x8]
	ldr	x8, [x23, x8]
	stp	x9, x8, [sp]
	mov	x0, x25
	mov	w1, #4096
	mov	x2, x20
	bl	_snprintf
	bl	_print
	add	x21, x21, #1
	cmp	x21, #44
	b.eq	LBB0_127
LBB0_124:                               ; =>This Inner Loop Header: Depth=1
	cbz	x21, LBB0_123
; %bb.125:                              ;   in Loop: Header=BB0_124 Depth=1
	tbnz	w21, #0, LBB0_123
; %bb.126:                              ;   in Loop: Header=BB0_124 Depth=1
	strh	w24, [x19, _main.output_string@PAGEOFF]
	bl	_print
	b	LBB0_123
LBB0_127:
	mov	x8, #25691
	movk	x8, #28271, lsl #16
	movk	x8, #23909, lsl #32
	movk	x8, #10, lsl #48
	str	x8, [x19, _main.output_string@PAGEOFF]
	bl	_print
	ldur	x8, [x29, #-112]
Lloh91:
	adrp	x9, ___stack_chk_guard@GOTPAGE
Lloh92:
	ldr	x9, [x9, ___stack_chk_guard@GOTPAGEOFF]
Lloh93:
	ldr	x9, [x9]
	cmp	x9, x8
	b.ne	LBB0_129
; %bb.128:
	mov	w0, #0
	add	sp, sp, #640
	ldp	x29, x30, [sp, #96]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #80]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #64]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #48]             ; 16-byte Folded Reload
	ldp	x26, x25, [sp, #32]             ; 16-byte Folded Reload
	ldp	x28, x27, [sp, #16]             ; 16-byte Folded Reload
	ldp	d9, d8, [sp], #112              ; 16-byte Folded Reload
	ret
LBB0_129:
	bl	___stack_chk_fail
	.loh AdrpAdd	Lloh13, Lloh14
	.loh AdrpLdr	Lloh11, Lloh12
	.loh AdrpLdr	Lloh9, Lloh10
	.loh AdrpAdd	Lloh7, Lloh8
	.loh AdrpAdd	Lloh5, Lloh6
	.loh AdrpAdd	Lloh3, Lloh4
	.loh AdrpLdrGotLdr	Lloh0, Lloh1, Lloh2
	.loh AdrpAdd	Lloh27, Lloh28
	.loh AdrpAdd	Lloh25, Lloh26
	.loh AdrpAdd	Lloh23, Lloh24
	.loh AdrpAdd	Lloh21, Lloh22
	.loh AdrpAdd	Lloh19, Lloh20
	.loh AdrpAdd	Lloh17, Lloh18
	.loh AdrpAdd	Lloh15, Lloh16
	.loh AdrpAdd	Lloh29, Lloh30
	.loh AdrpAdd	Lloh31, Lloh32
	.loh AdrpAdd	Lloh35, Lloh36
	.loh AdrpAdd	Lloh33, Lloh34
	.loh AdrpAdd	Lloh37, Lloh38
	.loh AdrpAdd	Lloh49, Lloh50
	.loh AdrpAdd	Lloh47, Lloh48
	.loh AdrpAdd	Lloh45, Lloh46
	.loh AdrpAdd	Lloh43, Lloh44
	.loh AdrpAdd	Lloh41, Lloh42
	.loh AdrpAdd	Lloh39, Lloh40
	.loh AdrpAdd	Lloh51, Lloh52
	.loh AdrpAdd	Lloh53, Lloh54
	.loh AdrpLdr	Lloh61, Lloh62
	.loh AdrpAdd	Lloh59, Lloh60
	.loh AdrpAdd	Lloh57, Lloh58
	.loh AdrpAdd	Lloh55, Lloh56
	.loh AdrpAdd	Lloh73, Lloh74
	.loh AdrpAdd	Lloh71, Lloh72
	.loh AdrpAdd	Lloh69, Lloh70
	.loh AdrpAdd	Lloh67, Lloh68
	.loh AdrpAdd	Lloh65, Lloh66
	.loh AdrpAdd	Lloh63, Lloh64
	.loh AdrpLdr	Lloh75, Lloh76
	.loh AdrpAdd	Lloh89, Lloh90
	.loh AdrpAdd	Lloh87, Lloh88
	.loh AdrpAdd	Lloh85, Lloh86
	.loh AdrpAdd	Lloh83, Lloh84
	.loh AdrpAdd	Lloh81, Lloh82
	.loh AdrpAdd	Lloh79, Lloh80
	.loh AdrpAdd	Lloh77, Lloh78
	.loh AdrpLdrGotLdr	Lloh91, Lloh92, Lloh93
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function print_graph_raw
_print_graph_raw:                       ; @print_graph_raw
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #64
	.cfi_def_cfa_offset 64
	stp	x22, x21, [sp, #16]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #32]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #48]             ; 16-byte Folded Spill
	add	x29, sp, #48
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	mov	x19, x0
	mov	w21, #0
Lloh94:
	adrp	x20, l_.str.26@PAGE
Lloh95:
	add	x20, x20, l_.str.26@PAGEOFF
LBB1_1:                                 ; =>This Inner Loop Header: Depth=1
	ubfx	x8, x21, #2, #16
	ldrh	w8, [x19, x8, lsl #1]
	ubfiz	w9, w21, #2, #2
	lsr	w8, w8, w9
	and	w8, w8, #0xf
	str	x8, [sp]
	mov	x0, x20
	bl	_printf
	add	w8, w21, #1
	cmp	w21, #35
	mov	x21, x8
	b.lo	LBB1_1
; %bb.2:
	ldp	x29, x30, [sp, #48]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #64
	ret
	.loh AdrpAdd	Lloh94, Lloh95
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function print
_print:                                 ; @print
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #160
	.cfi_def_cfa_offset 160
	stp	x22, x21, [sp, #112]            ; 16-byte Folded Spill
	stp	x20, x19, [sp, #128]            ; 16-byte Folded Spill
	stp	x29, x30, [sp, #144]            ; 16-byte Folded Spill
	add	x29, sp, #144
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
Lloh96:
	adrp	x8, ___stack_chk_guard@GOTPAGE
Lloh97:
	ldr	x8, [x8, ___stack_chk_guard@GOTPAGEOFF]
Lloh98:
	ldr	x8, [x8]
	stur	x8, [x29, #-40]
	movi.2d	v0, #0000000000000000
	stp	q0, q0, [sp, #64]
	add	x0, sp, #48
	mov	x1, #0
	bl	_gettimeofday
	add	x0, sp, #48
	bl	_localtime
	mov	x3, x0
Lloh99:
	adrp	x2, l_.str.20@PAGE
Lloh100:
	add	x2, x2, l_.str.20@PAGEOFF
	add	x22, sp, #64
	add	x0, sp, #64
	mov	w1, #32
	bl	_strftime
	str	xzr, [sp]
Lloh101:
	adrp	x0, _main.output_filename@PAGE
Lloh102:
	add	x0, x0, _main.output_filename@PAGEOFF
	mov	w1, #9
	bl	_open
	mov	x19, x0
	tbz	w0, #31, LBB2_2
; %bb.1:
	bl	_rand
	mov	x19, x0
	bl	_rand
	mov	x20, x0
	bl	_rand
	mov	x21, x0
	bl	_rand
                                        ; kill: def $w0 killed $w0 def $x0
	stp	x21, x0, [sp, #32]
	stp	x19, x20, [sp, #16]
	mov	w8, #4
Lloh103:
	adrp	x19, _main.output_filename@PAGE
Lloh104:
	add	x19, x19, _main.output_filename@PAGEOFF
	stp	x22, x8, [sp]
Lloh105:
	adrp	x2, l_.str.29@PAGE
Lloh106:
	add	x2, x2, l_.str.29@PAGEOFF
	mov	x0, x19
	mov	w1, #4096
	bl	_snprintf
	mov	w8, #420
	str	x8, [sp]
	mov	x0, x19
	mov	w1, #2569
	bl	_open
	mov	x19, x0
	tbnz	w0, #31, LBB2_5
LBB2_2:
Lloh107:
	adrp	x20, _main.output_string@PAGE
Lloh108:
	add	x20, x20, _main.output_string@PAGEOFF
	mov	x0, x20
	bl	_strlen
	mov	x2, x0
	mov	x0, x19
	mov	x1, x20
	bl	_write
	mov	x0, x19
	bl	_close
	str	x20, [sp]
Lloh109:
	adrp	x0, l_.str.30@PAGE
Lloh110:
	add	x0, x0, l_.str.30@PAGEOFF
	bl	_printf
Lloh111:
	adrp	x8, ___stdoutp@GOTPAGE
Lloh112:
	ldr	x8, [x8, ___stdoutp@GOTPAGEOFF]
Lloh113:
	ldr	x0, [x8]
	bl	_fflush
	ldur	x8, [x29, #-40]
Lloh114:
	adrp	x9, ___stack_chk_guard@GOTPAGE
Lloh115:
	ldr	x9, [x9, ___stack_chk_guard@GOTPAGEOFF]
Lloh116:
	ldr	x9, [x9]
	cmp	x9, x8
	b.ne	LBB2_4
; %bb.3:
	ldp	x29, x30, [sp, #144]            ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #128]            ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #112]            ; 16-byte Folded Reload
	add	sp, sp, #160
	ret
LBB2_4:
	bl	___stack_chk_fail
LBB2_5:
	add	x0, sp, #64
	bl	_print.cold.1
	.loh AdrpAdd	Lloh101, Lloh102
	.loh AdrpAdd	Lloh99, Lloh100
	.loh AdrpLdrGotLdr	Lloh96, Lloh97, Lloh98
	.loh AdrpAdd	Lloh105, Lloh106
	.loh AdrpAdd	Lloh103, Lloh104
	.loh AdrpLdrGotLdr	Lloh114, Lloh115, Lloh116
	.loh AdrpLdrGotLdr	Lloh111, Lloh112, Lloh113
	.loh AdrpAdd	Lloh109, Lloh110
	.loh AdrpAdd	Lloh107, Lloh108
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function worker_thread
_worker_thread:                         ; @worker_thread
	.cfi_startproc
; %bb.0:
	stp	d9, d8, [sp, #-112]!            ; 16-byte Folded Spill
	.cfi_def_cfa_offset 112
	stp	x28, x27, [sp, #16]             ; 16-byte Folded Spill
	stp	x26, x25, [sp, #32]             ; 16-byte Folded Spill
	stp	x24, x23, [sp, #48]             ; 16-byte Folded Spill
	stp	x22, x21, [sp, #64]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #80]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #96]             ; 16-byte Folded Spill
	add	x29, sp, #96
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	.cfi_offset w25, -72
	.cfi_offset w26, -80
	.cfi_offset w27, -88
	.cfi_offset w28, -96
	.cfi_offset b8, -104
	.cfi_offset b9, -112
	sub	sp, sp, #816
Lloh117:
	adrp	x8, ___stack_chk_guard@GOTPAGE
Lloh118:
	ldr	x8, [x8, ___stack_chk_guard@GOTPAGEOFF]
Lloh119:
	ldr	x8, [x8]
	stur	x8, [x29, #-120]
	mov	w20, #16960
	movk	w20, #15, lsl #16
	ldr	x19, [x0]
	mov	w22, #8
	mov	w0, #44
	mov	w1, #8
	bl	_calloc
	str	x0, [sp, #64]                   ; 8-byte Folded Spill
	orr	x0, x20, #0x1
	mov	w1, #8
	bl	_calloc
	mov	x27, x0
	mov	w0, #9
	mov	w1, #2
	bl	_calloc
	mov	x21, x0
Lloh120:
	adrp	x8, __MergedGlobals@PAGE+32
Lloh121:
	add	x8, x8, __MergedGlobals@PAGEOFF+32
	mov	x17, #-1
	ldadd	x17, x8, [x8]
Lloh122:
	adrp	x20, __MergedGlobals@PAGE
Lloh123:
	add	x20, x20, __MergedGlobals@PAGEOFF
	cmp	x8, #1
	str	x19, [sp, #40]                  ; 8-byte Folded Spill
	b.lt	LBB3_223
; %bb.1:
	add	x9, x27, #1953, lsl #12         ; =7999488
	add	x25, x9, #512
	mov	w9, #18
	madd	x9, x19, x9, x20
	add	x9, x9, #66
	stp	x25, x9, [sp, #48]              ; 16-byte Folded Spill
	add	x9, x27, #8
	str	x9, [sp, #176]                  ; 8-byte Folded Spill
	mov	w0, #12
	mov	w1, #15
	mov	w4, #57600
	movk	w4, #1525, lsl #16
	mov	w5, #13
Lloh124:
	adrp	x6, lJTI3_1@PAGE
Lloh125:
	add	x6, x6, lJTI3_1@PAGEOFF
	adrp	x2, __MergedGlobals@PAGE
Lloh126:
	adrp	x3, __MergedGlobals@PAGE+8
Lloh127:
	add	x3, x3, __MergedGlobals@PAGEOFF+8
	movi	d8, #0xffffffffffff0000
Lloh128:
	adrp	x7, lJTI3_0@PAGE
Lloh129:
	add	x7, x7, lJTI3_0@PAGEOFF
	b	LBB3_3
LBB3_2:                                 ;   in Loop: Header=BB3_3 Depth=1
Lloh130:
	adrp	x8, __MergedGlobals@PAGE+32
Lloh131:
	add	x8, x8, __MergedGlobals@PAGEOFF+32
	ldadd	x17, x8, [x8]
	cmp	x8, #1
	b.lt	LBB3_224
LBB3_3:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB3_16 Depth 2
                                        ;     Child Loop BB3_23 Depth 2
                                        ;       Child Loop BB3_54 Depth 3
                                        ;       Child Loop BB3_198 Depth 3
                                        ;     Child Loop BB3_207 Depth 2
                                        ;     Child Loop BB3_216 Depth 2
	mov	x19, #0
Lloh132:
	adrp	x9, _queue@PAGE
Lloh133:
	add	x9, x9, _queue@PAGEOFF
	add	x8, x9, x8, lsl #1
	ldurh	w8, [x8, #-2]
	adrp	x9, __MergedGlobals@PAGE+1
	ldrb	w9, [x9, __MergedGlobals@PAGEOFF+1]
	cmp	w9, #0
	csel	x9, x0, xzr, ne
	lsl	x26, x8, x9
	ubfx	x8, x19, #16, #4
	cmp	w8, #6
	b.hi	LBB3_14
LBB3_4:                                 ;   in Loop: Header=BB3_3 Depth=1
	adr	x9, LBB3_5
	ldrb	w10, [x7, x8]
	add	x9, x9, x10, lsl #2
	br	x9
LBB3_5:                                 ;   in Loop: Header=BB3_3 Depth=1
	ubfx	w9, w19, #20, #4
	cmp	w9, #3
	b.eq	LBB3_7
; %bb.6:                                ;   in Loop: Header=BB3_3 Depth=1
	cbnz	w9, LBB3_8
LBB3_7:                                 ;   in Loop: Header=BB3_3 Depth=1
	ubfx	x9, x19, #24, #8
	orr	w9, w9, #0x4
	and	w9, w9, #0xf
	cmp	w9, #4
	b.eq	LBB3_12
LBB3_8:                                 ;   in Loop: Header=BB3_3 Depth=1
	cmp	w8, #2
	b.ne	LBB3_14
LBB3_9:                                 ;   in Loop: Header=BB3_3 Depth=1
	ubfx	w8, w19, #20, #4
	cmp	w8, #4
	b.eq	LBB3_11
; %bb.10:                               ;   in Loop: Header=BB3_3 Depth=1
	cmp	w8, #1
	b.ne	LBB3_14
LBB3_11:                                ;   in Loop: Header=BB3_3 Depth=1
	ubfx	x8, x19, #24, #8
	orr	w8, w8, #0x4
	and	w8, w8, #0xf
	cmp	w8, #6
	b.ne	LBB3_14
LBB3_12:                                ;   in Loop: Header=BB3_3 Depth=1
	ldrb	w8, [x2, __MergedGlobals@PAGEOFF]
	cmp	w8, #1
	b.ne	LBB3_2
; %bb.13:                               ;   in Loop: Header=BB3_3 Depth=1
	fmov	d0, x19
	and.8b	v0, v0, v8
	fmov	x19, d0
	mov	w8, #4
	b	LBB3_212
LBB3_14:                                ;   in Loop: Header=BB3_3 Depth=1
	ldr	q0, [x20, #48]
	str	q0, [x21]
	ldrh	w8, [x20, #64]
	strh	w8, [x21, #16]
	ldrb	w8, [x20]
	cmp	w8, #1
	b.ne	LBB3_17
; %bb.15:                               ;   in Loop: Header=BB3_3 Depth=1
	mov	x8, #0
	mov	x9, #0
LBB3_16:                                ;   Parent Loop BB3_3 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	add	x10, x20, x9
	ldrb	w10, [x10, #84]
	lsr	w11, w10, #1
	add	x12, x20, x9, lsl #3
	ldr	x12, [x12, #216]
	cmp	x9, #16
	csel	x13, x19, x26, lo
	and	x14, x8, #0x3c
	lsr	x13, x13, x14
	lsl	w13, w13, #2
	and	x13, x13, #0x3c
	lsr	x12, x12, x13
	and	w12, w12, #0xf
	lsl	w10, w10, #2
	and	x10, x10, #0xc
	and	x11, x11, #0x7e
	ldrh	w13, [x21, x11]
	lsl	w10, w12, w10
	orr	w10, w13, w10
	strh	w10, [x21, x11]
	add	x9, x9, #1
	add	x8, x8, #4
	cmp	x9, #23
	b.ne	LBB3_16
LBB3_17:                                ;   in Loop: Header=BB3_3 Depth=1
	ldrh	w8, [x21]
	and	w9, w8, #0xff
	ubfx	x10, x8, #12, #20
	ubfx	x11, x8, #4, #4
	mov	w13, #1
	lsl	w11, w13, w11
	cmp	w9, #16
	csel	w9, wzr, w11, lo
	ands	w11, w1, w8, lsr #8
	lsl	w11, w13, w11
	csel	w11, wzr, w11, eq
	lsl	w10, w13, w10
	cmp	w8, #1, lsl #12                 ; =4096
	csel	w8, wzr, w10, lo
	orr	w8, w8, w9
	orr	w8, w8, w11
	ldrh	w9, [x21, #2]
	ubfx	w10, w9, #4, #4
	ubfx	w11, w9, #8, #4
	lsr	w9, w9, #12
	lsl	w12, w13, w10
	cmp	w10, #1
	csel	w10, wzr, w12, eq
	lsl	w12, w13, w11
	cmp	w11, #1
	csel	w11, wzr, w12, eq
	lsl	w12, w13, w9
	cmp	w9, #1
	csel	w9, wzr, w12, eq
	orr	w8, w9, w8
	orr	w8, w8, w10
	orr	w8, w8, w11
	ldrh	w9, [x21, #4]
	ubfx	w10, w9, #4, #4
	ubfx	w11, w9, #8, #4
	lsr	w9, w9, #12
	lsl	w12, w13, w10
	cmp	w10, #2
	csel	w10, wzr, w12, eq
	lsl	w12, w13, w11
	cmp	w11, #2
	csel	w11, wzr, w12, eq
	lsl	w12, w13, w9
	cmp	w9, #2
	csel	w9, wzr, w12, eq
	orr	w8, w9, w8
	orr	w8, w8, w10
	orr	w8, w8, w11
	ldrh	w9, [x21, #6]
	ubfx	w10, w9, #4, #4
	ubfx	w11, w9, #8, #4
	lsr	w9, w9, #12
	lsl	w12, w13, w10
	cmp	w10, #3
	csel	w10, wzr, w12, eq
	lsl	w12, w13, w11
	cmp	w11, #3
	csel	w11, wzr, w12, eq
	lsl	w12, w13, w9
	cmp	w9, #3
	csel	w9, wzr, w12, eq
	orr	w8, w9, w8
	orr	w8, w8, w10
	orr	w8, w8, w11
	ldrh	w9, [x21, #8]
	ubfx	w10, w9, #4, #4
	ubfx	w11, w9, #8, #4
	lsr	w9, w9, #12
	lsl	w12, w13, w10
	cmp	w10, #4
	csel	w10, wzr, w12, eq
	lsl	w12, w13, w11
	cmp	w11, #4
	csel	w11, wzr, w12, eq
	lsl	w12, w13, w9
	cmp	w9, #4
	csel	w9, wzr, w12, eq
	orr	w8, w9, w8
	orr	w8, w8, w10
	orr	w8, w8, w11
	ldrh	w9, [x21, #10]
	ubfx	w10, w9, #4, #4
	ubfx	w11, w9, #8, #4
	lsr	w9, w9, #12
	lsl	w12, w13, w10
	cmp	w10, #5
	csel	w10, wzr, w12, eq
	lsl	w12, w13, w11
	cmp	w11, #5
	csel	w11, wzr, w12, eq
	lsl	w12, w13, w9
	cmp	w9, #5
	csel	w9, wzr, w12, eq
	orr	w8, w9, w8
	orr	w8, w8, w10
	orr	w8, w8, w11
	ldrh	w11, [x21, #12]
	ubfx	w9, w11, #4, #4
	ubfx	w10, w11, #8, #4
	str	w11, [sp, #88]                  ; 4-byte Folded Spill
	lsr	w11, w11, #12
	lsl	w12, w13, w9
	cmp	w9, #6
	csel	w9, wzr, w12, eq
	lsl	w12, w13, w10
	cmp	w10, #6
	csel	w10, wzr, w12, eq
	lsl	w12, w13, w11
	cmp	w11, #6
	csel	w11, wzr, w12, eq
	orr	w8, w11, w8
	orr	w8, w8, w9
	orr	w8, w8, w10
	ldrh	w11, [x21, #14]
	ubfx	w9, w11, #4, #4
	ubfx	w10, w11, #8, #4
	str	w11, [sp, #84]                  ; 4-byte Folded Spill
	lsr	w11, w11, #12
	lsl	w12, w13, w9
	cmp	w9, #7
	csel	w9, wzr, w12, eq
	lsl	w12, w13, w10
	cmp	w10, #7
	csel	w10, wzr, w12, eq
	lsl	w12, w13, w11
	cmp	w11, #7
	csel	w11, wzr, w12, eq
	orr	w8, w11, w8
	orr	w8, w8, w9
	orr	w8, w8, w10
	ldrh	w11, [x21, #16]
	ubfx	w9, w11, #4, #4
	ubfx	w10, w11, #8, #4
	str	w11, [sp, #80]                  ; 4-byte Folded Spill
	lsr	w11, w11, #12
	lsl	w12, w13, w9
	cmp	w9, #8
	csel	w9, wzr, w12, eq
	lsl	w12, w13, w10
	cmp	w10, #8
	csel	w10, wzr, w12, eq
	lsl	w12, w13, w11
	cmp	w11, #8
	csel	w11, wzr, w12, eq
	orr	w8, w11, w8
	orr	w8, w8, w9
	orr	w8, w8, w10
	mvn	w8, w8
	tst	w8, #0x1ff
	b.ne	LBB3_203
; %bb.18:                               ;   in Loop: Header=BB3_3 Depth=1
	mov	x20, #0
	mov	w7, #1
	b	LBB3_23
LBB3_19:                                ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #16
LBB3_20:                                ;   in Loop: Header=BB3_23 Depth=2
	mov	w22, #8
	mov	x27, x25
	ldr	x25, [sp, #48]                  ; 8-byte Folded Reload
	mov	w0, #12
	mov	w4, #57600
	movk	w4, #1525, lsl #16
	mov	w5, #13
Lloh134:
	adrp	x6, lJTI3_1@PAGE
Lloh135:
	add	x6, x6, lJTI3_1@PAGEOFF
	mov	x7, x28
	mov	x20, x14
LBB3_21:                                ;   in Loop: Header=BB3_23 Depth=2
	ldrh	w9, [x21]
	ldp	x10, x11, [sp, #56]             ; 16-byte Folded Reload
	strh	w9, [x10]
	ldrh	w9, [x21, #2]
	strh	w9, [x10, #2]
	ldrh	w9, [x21, #4]
	strh	w9, [x10, #4]
	ldrh	w9, [x21, #6]
	strh	w9, [x10, #6]
	ldrh	w9, [x21, #8]
	strh	w9, [x10, #8]
	ldrh	w9, [x21, #10]
	strh	w9, [x10, #10]
	ldr	w9, [sp, #88]                   ; 4-byte Folded Reload
	strh	w9, [x10, #12]
	ldr	w9, [sp, #84]                   ; 4-byte Folded Reload
	strh	w9, [x10, #14]
	ldr	w9, [sp, #80]                   ; 4-byte Folded Reload
	strh	w9, [x10, #16]
	lsl	x9, x8, #3
	ldr	x10, [x11, x9]
	add	x10, x10, #1
	str	x10, [x11, x9]
	cbz	x8, LBB3_205
LBB3_22:                                ;   in Loop: Header=BB3_23 Depth=2
	cmp	x20, #8
	add	x8, x20, #1
	cset	w7, lo
	mov	x20, x8
	cmp	x8, #9
	b.eq	LBB3_210
LBB3_23:                                ;   Parent Loop BB3_3 Depth=1
                                        ; =>  This Loop Header: Depth=2
                                        ;       Child Loop BB3_54 Depth 3
                                        ;       Child Loop BB3_198 Depth 3
	ldrh	w13, [x21, x20, lsl #1]
	and	w8, w13, #0xf
	cmp	w8, #2
	b.ne	LBB3_22
; %bb.24:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	x2, #0
	mov	x12, #0
	stp	xzr, xzr, [sp, #192]            ; 16-byte Folded Spill
	mov	x3, #0
	stp	xzr, xzr, [sp, #160]            ; 16-byte Folded Spill
	mov	x9, #0
	mov	w14, #0
	str	wzr, [sp, #188]                 ; 4-byte Folded Spill
	mov	w24, #0
	stp	xzr, xzr, [sp, #144]            ; 16-byte Folded Spill
	mov	w30, #0
	mov	x16, #0
	mov	x28, #0
	str	xzr, [sp, #208]                 ; 8-byte Folded Spill
	str	wzr, [sp, #108]                 ; 4-byte Folded Spill
	stp	xzr, xzr, [sp, #112]            ; 16-byte Folded Spill
	stp	xzr, xzr, [sp, #128]            ; 16-byte Folded Spill
	str	xzr, [x27]
	str	xzr, [x25]
	sturb	wzr, [x29, #-160]
	strb	wzr, [sp, #224]
	mov	w23, #255
	mov	x10, #-1
	and	w8, w13, #0xf
	cmp	w8, #4
	b.hi	LBB3_147
LBB3_25:                                ;   in Loop: Header=BB3_23 Depth=2
	and	x15, x13, #0xf
	adr	x17, LBB3_26
	ldrb	w1, [x6, x15]
	add	x17, x17, x1, lsl #2
	br	x17
LBB3_26:                                ;   in Loop: Header=BB3_23 Depth=2
	mov	w11, #16960
	movk	w11, #15, lsl #16
	cmp	x3, x11
	b.eq	LBB3_226
; %bb.27:                               ;   in Loop: Header=BB3_23 Depth=2
	cbz	x12, LBB3_168
; %bb.28:                               ;   in Loop: Header=BB3_23 Depth=2
	tst	w23, #0xff
	csel	w14, wzr, w14, eq
	ldr	w11, [sp, #188]                 ; 4-byte Folded Reload
	csel	w11, wzr, w11, eq
	str	w11, [sp, #188]                 ; 4-byte Folded Spill
	cmp	x3, #511
	ldr	x17, [sp, #208]                 ; 8-byte Folded Reload
	b.hi	LBB3_31
; %bb.29:                               ;   in Loop: Header=BB3_23 Depth=2
	and	w12, w23, #0xff
	add	x11, sp, #224
	cmp	w12, #2
	b.ne	LBB3_71
; %bb.30:                               ;   in Loop: Header=BB3_23 Depth=2
	ldrb	w12, [x11, x3]
	add	w12, w12, #1
	and	w15, w12, #0xff
	strb	w12, [x11, x3]
	cmp	w15, #29
	b.hi	LBB3_178
LBB3_31:                                ;   in Loop: Header=BB3_23 Depth=2
	and	w12, w17, #0xff
	cmp	w12, #4
	b.ls	LBB3_72
	b	LBB3_129
LBB3_32:                                ;   in Loop: Header=BB3_23 Depth=2
	cbz	x2, LBB3_165
; %bb.33:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w24, #0
	str	xzr, [x25]
	add	x16, x16, #1
	cmp	x16, x4
	b.ne	LBB3_148
	b	LBB3_149
LBB3_34:                                ;   in Loop: Header=BB3_23 Depth=2
	ands	w15, w23, #0xff
	b.eq	LBB3_43
; %bb.35:                               ;   in Loop: Header=BB3_23 Depth=2
	cmp	w15, #2
	ldp	x15, x17, [sp, #200]            ; 16-byte Folded Reload
	b.ne	LBB3_46
	b	LBB3_171
LBB3_36:                                ;   in Loop: Header=BB3_23 Depth=2
	add	w24, w24, #1
	and	w11, w24, #0xff
	cmp	w11, #9
	b.hi	LBB3_166
; %bb.37:                               ;   in Loop: Header=BB3_23 Depth=2
	add	x11, x2, #1
	str	x11, [x25]
	add	x16, x16, #1
	cmp	x16, x4
	b.ne	LBB3_148
	b	LBB3_149
LBB3_38:                                ;   in Loop: Header=BB3_23 Depth=2
	and	w10, w23, #0xff
	cmp	w10, #2
	b.ne	LBB3_167
; %bb.39:                               ;   in Loop: Header=BB3_23 Depth=2
	cbz	x3, LBB3_169
; %bb.40:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	x25, x27
	ldr	x11, [sp, #168]                 ; 8-byte Folded Reload
	cmp	x3, x11
	b.eq	LBB3_50
; %bb.41:                               ;   in Loop: Header=BB3_23 Depth=2
	add	x10, x11, #1
	cmp	x3, x10
	b.eq	LBB3_50
; %bb.42:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w12, #0
	mov	x11, x3
	b	LBB3_51
LBB3_43:                                ;   in Loop: Header=BB3_23 Depth=2
	add	w14, w14, #1
	and	w11, w14, #0xff
	cmp	w11, #9
	ldr	x15, [sp, #200]                 ; 8-byte Folded Reload
	b.hi	LBB3_172
; %bb.44:                               ;   in Loop: Header=BB3_23 Depth=2
	ldr	w11, [sp, #188]                 ; 4-byte Folded Reload
	add	w11, w11, #1
	str	w11, [sp, #188]                 ; 4-byte Folded Spill
	and	w11, w11, #0xff
	cmp	w11, #7
	b.lo	LBB3_74
; %bb.45:                               ;   in Loop: Header=BB3_23 Depth=2
	lsr	x11, x16, #5
	cmp	x11, #3124
	ldr	x17, [sp, #208]                 ; 8-byte Folded Reload
	b.hi	LBB3_173
LBB3_46:                                ;   in Loop: Header=BB3_23 Depth=2
	cmp	x15, #3
	b.eq	LBB3_75
LBB3_47:                                ;   in Loop: Header=BB3_23 Depth=2
	cmp	x15, #2
	b.ne	LBB3_70
; %bb.48:                               ;   in Loop: Header=BB3_23 Depth=2
	ldr	w11, [sp, #160]                 ; 4-byte Folded Reload
	add	w11, w11, #1
	str	w11, [sp, #160]                 ; 4-byte Folded Spill
	and	w11, w11, #0xff
	cmp	w11, #15
	b.hi	LBB3_175
; %bb.49:                               ;   in Loop: Header=BB3_23 Depth=2
	str	wzr, [sp, #156]                 ; 4-byte Folded Spill
	cmn	x10, #1
	b.ne	LBB3_77
	b	LBB3_78
LBB3_50:                                ;   in Loop: Header=BB3_23 Depth=2
	ldr	w12, [sp, #164]                 ; 4-byte Folded Reload
	add	w12, w12, #1
	and	w10, w12, #0xff
	cmp	w10, #49
	b.hi	LBB3_174
LBB3_51:                                ;   in Loop: Header=BB3_23 Depth=2
	str	w30, [sp, #76]                  ; 4-byte Folded Spill
	str	w12, [sp, #164]                 ; 4-byte Folded Spill
	str	x11, [sp, #168]                 ; 8-byte Folded Spill
	str	w14, [sp, #92]                  ; 4-byte Folded Spill
	str	x28, [sp, #96]                  ; 8-byte Folded Spill
	mov	x14, x20
	mov	x28, x7
	ldr	x10, [sp, #192]                 ; 8-byte Folded Reload
	cmn	x10, #1
	b.eq	LBB3_63
; %bb.52:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	x10, #0
	mov	w12, #0
	mov	w2, #0
	mov	w6, #0
	mov	w4, #0
	mov	w24, #0
	mov	w1, #0
	ldr	x20, [x25]
	ldr	x11, [sp, #192]                 ; 8-byte Folded Reload
	add	x17, x11, #1
	mov	x7, #-1
	mov	x27, #-1
	mov	x15, #-1
	b	LBB3_54
LBB3_53:                                ;   in Loop: Header=BB3_54 Depth=3
	add	x10, x10, #1
	mov	x15, x5
	cmp	x17, x10
	b.eq	LBB3_63
LBB3_54:                                ;   Parent Loop BB3_3 Depth=1
                                        ;     Parent Loop BB3_23 Depth=2
                                        ; =>    This Inner Loop Header: Depth=3
	mov	x5, x20
	ldr	x11, [sp, #176]                 ; 8-byte Folded Reload
	ldr	x20, [x11, x10, lsl #3]
	sub	x11, x20, x5
	subs	x0, x5, x20
	csel	x11, x11, x0, lo
	csel	x0, x5, x20, lo
	csel	x22, x5, x20, hi
	mov	x30, #-6148914691236517206
	movk	x30, #43691
	umulh	x0, x0, x30
	lsr	x0, x0, #1
	cmp	x22, #200
	ccmp	x11, x0, #0, hs
	b.hs	LBB3_161
; %bb.55:                               ;   in Loop: Header=BB3_54 Depth=3
	cmp	x5, #8
	csinc	w1, wzr, w1, hs
	and	w11, w1, #0xff
	cmp	w11, #230
	b.hi	LBB3_162
; %bb.56:                               ;   in Loop: Header=BB3_54 Depth=3
	cmp	x5, x15
	csinc	w24, wzr, w24, ne
	and	w11, w24, #0xff
	cmp	w11, #14
	b.hi	LBB3_163
; %bb.57:                               ;   in Loop: Header=BB3_54 Depth=3
	add	x11, x27, w6, uxtb
	cmp	x5, x11
	b.ne	LBB3_60
; %bb.58:                               ;   in Loop: Header=BB3_54 Depth=3
	tst	w6, #0xff
	cset	w6, eq
	add	w4, w4, #1
	and	w11, w4, #0xff
	cmp	w11, #14
	b.hi	LBB3_164
; %bb.59:                               ;   in Loop: Header=BB3_54 Depth=3
	add	x11, x15, #1
	cmp	x5, x11
	csinc	w2, wzr, w2, ne
	and	w11, w2, #0xff
	cmp	w11, #39
	b.ls	LBB3_61
	b	LBB3_19
LBB3_60:                                ;   in Loop: Header=BB3_54 Depth=3
	mov	w4, #0
	mov	w6, #0
	mov	x27, x5
	add	x11, x15, #1
	cmp	x5, x11
	csinc	w2, wzr, w2, ne
	and	w11, w2, #0xff
	cmp	w11, #39
	b.hi	LBB3_19
LBB3_61:                                ;   in Loop: Header=BB3_54 Depth=3
	tbnz	w10, #0, LBB3_53
; %bb.62:                               ;   in Loop: Header=BB3_54 Depth=3
	add	x11, x7, #1
	cmp	x5, x11
	csinc	w12, wzr, w12, ne
	and	w11, w12, #0xff
	mov	x7, x5
	cmp	w11, #79
	b.ls	LBB3_53
	b	LBB3_19
LBB3_63:                                ;   in Loop: Header=BB3_23 Depth=2
	add	x10, x3, #1
	cmp	x10, x9
	b.ne	LBB3_84
; %bb.64:                               ;   in Loop: Header=BB3_23 Depth=2
	ldr	w30, [sp, #76]                  ; 4-byte Folded Reload
	add	w30, w30, #1
	and	w11, w30, #0xff
	cmp	w11, #7
	mov	w22, #8
	mov	x27, x25
	mov	w0, #12
	mov	w4, #57600
	movk	w4, #1525, lsl #16
	mov	w5, #13
Lloh136:
	adrp	x6, lJTI3_1@PAGE
Lloh137:
	add	x6, x6, lJTI3_1@PAGEOFF
	mov	x7, x28
	mov	x20, x14
	ldr	x17, [sp, #208]                 ; 8-byte Folded Reload
	b.hi	LBB3_179
; %bb.65:                               ;   in Loop: Header=BB3_23 Depth=2
	add	x12, x3, #2
	cmp	x12, x9
	b.ne	LBB3_85
LBB3_66:                                ;   in Loop: Header=BB3_23 Depth=2
	ldr	w11, [sp, #148]                 ; 4-byte Folded Reload
	add	w11, w11, #1
	str	w11, [sp, #148]                 ; 4-byte Folded Spill
	and	w11, w11, #0xff
	cmp	w11, #7
	ldr	x25, [sp, #48]                  ; 8-byte Folded Reload
	ldr	x28, [sp, #96]                  ; 8-byte Folded Reload
	ldr	w14, [sp, #92]                  ; 4-byte Folded Reload
	b.hi	LBB3_180
; %bb.67:                               ;   in Loop: Header=BB3_23 Depth=2
	add	x15, x3, #3
	cmp	x15, x9
	b.ne	LBB3_86
LBB3_68:                                ;   in Loop: Header=BB3_23 Depth=2
	ldr	w11, [sp, #144]                 ; 4-byte Folded Reload
	add	w11, w11, #1
	str	w11, [sp, #144]                 ; 4-byte Folded Spill
	and	w11, w11, #0xff
	cmp	w11, #29
	b.hi	LBB3_181
; %bb.69:                               ;   in Loop: Header=BB3_23 Depth=2
	cmp	x12, x9
	b.ne	LBB3_87
	b	LBB3_92
LBB3_70:                                ;   in Loop: Header=BB3_23 Depth=2
	stp	wzr, wzr, [sp, #156]            ; 8-byte Folded Spill
	cmn	x10, #1
	b.ne	LBB3_77
	b	LBB3_78
LBB3_71:                                ;   in Loop: Header=BB3_23 Depth=2
	strb	wzr, [x11, x3]
	and	w12, w17, #0xff
	cmp	w12, #4
	b.hi	LBB3_129
LBB3_72:                                ;   in Loop: Header=BB3_23 Depth=2
	and	x12, x17, #0xff
Lloh138:
	adrp	x11, lJTI3_2@PAGE
Lloh139:
	add	x11, x11, lJTI3_2@PAGEOFF
	adr	x15, LBB3_73
	ldrb	w17, [x11, x12]
	add	x15, x15, x17, lsl #2
	mov	w11, #2
	br	x15
LBB3_73:                                ;   in Loop: Header=BB3_23 Depth=2
	mov	w11, #4
	b	LBB3_130
LBB3_74:                                ;   in Loop: Header=BB3_23 Depth=2
	ldr	x17, [sp, #208]                 ; 8-byte Folded Reload
	cmp	x15, #3
	b.ne	LBB3_47
LBB3_75:                                ;   in Loop: Header=BB3_23 Depth=2
	ldr	w11, [sp, #156]                 ; 4-byte Folded Reload
	add	w11, w11, #1
	str	w11, [sp, #156]                 ; 4-byte Folded Spill
	and	w11, w11, #0xff
	cmp	w11, #23
	b.hi	LBB3_176
; %bb.76:                               ;   in Loop: Header=BB3_23 Depth=2
	str	wzr, [sp, #160]                 ; 4-byte Folded Spill
	cmn	x10, #1
	b.eq	LBB3_78
LBB3_77:                                ;   in Loop: Header=BB3_23 Depth=2
	cmp	x3, x10
	b.hs	LBB3_170
LBB3_78:                                ;   in Loop: Header=BB3_23 Depth=2
	ands	w10, w17, #0xff
	b.eq	LBB3_81
; %bb.79:                               ;   in Loop: Header=BB3_23 Depth=2
	cmp	w10, #2
	b.ne	LBB3_82
; %bb.80:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w10, #3
	str	x10, [sp, #208]                 ; 8-byte Folded Spill
	b	LBB3_83
LBB3_81:                                ;   in Loop: Header=BB3_23 Depth=2
	mov	w10, #1
	str	x10, [sp, #208]                 ; 8-byte Folded Spill
	b	LBB3_83
LBB3_82:                                ;   in Loop: Header=BB3_23 Depth=2
	str	xzr, [sp, #208]                 ; 8-byte Folded Spill
	str	wzr, [sp, #152]                 ; 4-byte Folded Spill
LBB3_83:                                ;   in Loop: Header=BB3_23 Depth=2
	mov	w24, #0
	str	xzr, [sp, #200]                 ; 8-byte Folded Spill
	add	x10, x12, #1
	str	x10, [x27, x3, lsl #3]
	mov	x10, #-1
	add	x16, x16, #1
	cmp	x16, x4
	b.ne	LBB3_148
	b	LBB3_149
LBB3_84:                                ;   in Loop: Header=BB3_23 Depth=2
	mov	w30, #0
	mov	w22, #8
	mov	x27, x25
	mov	w0, #12
	mov	w4, #57600
	movk	w4, #1525, lsl #16
	mov	w5, #13
Lloh140:
	adrp	x6, lJTI3_1@PAGE
Lloh141:
	add	x6, x6, lJTI3_1@PAGEOFF
	mov	x7, x28
	mov	x20, x14
	ldr	x17, [sp, #208]                 ; 8-byte Folded Reload
	add	x12, x3, #2
	cmp	x12, x9
	b.eq	LBB3_66
LBB3_85:                                ;   in Loop: Header=BB3_23 Depth=2
	str	wzr, [sp, #148]                 ; 4-byte Folded Spill
	ldr	x25, [sp, #48]                  ; 8-byte Folded Reload
	ldr	x28, [sp, #96]                  ; 8-byte Folded Reload
	ldr	w14, [sp, #92]                  ; 4-byte Folded Reload
	add	x15, x3, #3
	cmp	x15, x9
	b.eq	LBB3_68
LBB3_86:                                ;   in Loop: Header=BB3_23 Depth=2
	str	wzr, [sp, #144]                 ; 4-byte Folded Spill
	cmp	x12, x9
	b.eq	LBB3_92
LBB3_87:                                ;   in Loop: Header=BB3_23 Depth=2
	cmp	x3, x9
	b.eq	LBB3_92
; %bb.88:                               ;   in Loop: Header=BB3_23 Depth=2
	cmp	x10, x9
	b.eq	LBB3_92
; %bb.89:                               ;   in Loop: Header=BB3_23 Depth=2
	cmp	x15, x9
	b.eq	LBB3_92
; %bb.90:                               ;   in Loop: Header=BB3_23 Depth=2
	add	x10, x3, #4
	cmp	x10, x9
	b.eq	LBB3_92
; %bb.91:                               ;   in Loop: Header=BB3_23 Depth=2
	str	wzr, [sp, #140]                 ; 4-byte Folded Spill
	b	LBB3_94
LBB3_92:                                ;   in Loop: Header=BB3_23 Depth=2
	ldr	w10, [sp, #140]                 ; 4-byte Folded Reload
	add	w10, w10, #1
	str	w10, [sp, #140]                 ; 4-byte Folded Spill
	and	w10, w10, #0xff
	cmp	w10, #150
	b.lo	LBB3_94
; %bb.93:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w10, #41247
	movk	w10, #7, lsl #16
	cmp	x16, x10
	b.hi	LBB3_182
LBB3_94:                                ;   in Loop: Header=BB3_23 Depth=2
	cmp	x3, x9
	b.eq	LBB3_97
; %bb.95:                               ;   in Loop: Header=BB3_23 Depth=2
	add	x10, x3, #5
	cmp	x10, x9
	b.eq	LBB3_97
; %bb.96:                               ;   in Loop: Header=BB3_23 Depth=2
	str	wzr, [sp, #136]                 ; 4-byte Folded Spill
	b	LBB3_99
LBB3_97:                                ;   in Loop: Header=BB3_23 Depth=2
	ldr	w10, [sp, #136]                 ; 4-byte Folded Reload
	add	w10, w10, #1
	str	w10, [sp, #136]                 ; 4-byte Folded Spill
	and	w10, w10, #0xff
	cmp	w10, #80
	b.lo	LBB3_99
; %bb.98:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w10, #41247
	movk	w10, #7, lsl #16
	cmp	x16, x10
	b.hi	LBB3_183
LBB3_99:                                ;   in Loop: Header=BB3_23 Depth=2
	cmp	x3, x9
	b.eq	LBB3_102
; %bb.100:                              ;   in Loop: Header=BB3_23 Depth=2
	add	x10, x3, #6
	cmp	x10, x9
	b.eq	LBB3_102
; %bb.101:                              ;   in Loop: Header=BB3_23 Depth=2
	str	wzr, [sp, #132]                 ; 4-byte Folded Spill
	b	LBB3_104
LBB3_102:                               ;   in Loop: Header=BB3_23 Depth=2
	ldr	w10, [sp, #132]                 ; 4-byte Folded Reload
	add	w10, w10, #1
	str	w10, [sp, #132]                 ; 4-byte Folded Spill
	and	w10, w10, #0xff
	cmp	w10, #80
	b.lo	LBB3_104
; %bb.103:                              ;   in Loop: Header=BB3_23 Depth=2
	mov	w10, #41247
	movk	w10, #7, lsl #16
	cmp	x16, x10
	b.hi	LBB3_184
LBB3_104:                               ;   in Loop: Header=BB3_23 Depth=2
	cmp	x3, x9
	b.eq	LBB3_107
; %bb.105:                              ;   in Loop: Header=BB3_23 Depth=2
	add	x10, x3, #7
	cmp	x10, x9
	b.eq	LBB3_107
; %bb.106:                              ;   in Loop: Header=BB3_23 Depth=2
	str	wzr, [sp, #128]                 ; 4-byte Folded Spill
	b	LBB3_109
LBB3_107:                               ;   in Loop: Header=BB3_23 Depth=2
	ldr	w10, [sp, #128]                 ; 4-byte Folded Reload
	add	w10, w10, #1
	str	w10, [sp, #128]                 ; 4-byte Folded Spill
	and	w10, w10, #0xff
	cmp	w10, #80
	b.lo	LBB3_109
; %bb.108:                              ;   in Loop: Header=BB3_23 Depth=2
	mov	w10, #41247
	movk	w10, #7, lsl #16
	cmp	x16, x10
	b.hi	LBB3_185
LBB3_109:                               ;   in Loop: Header=BB3_23 Depth=2
	cmp	x3, x9
	b.eq	LBB3_112
; %bb.110:                              ;   in Loop: Header=BB3_23 Depth=2
	add	x10, x3, #8
	cmp	x10, x9
	b.eq	LBB3_112
; %bb.111:                              ;   in Loop: Header=BB3_23 Depth=2
	str	wzr, [sp, #124]                 ; 4-byte Folded Spill
	b	LBB3_114
LBB3_112:                               ;   in Loop: Header=BB3_23 Depth=2
	ldr	w10, [sp, #124]                 ; 4-byte Folded Reload
	add	w10, w10, #1
	str	w10, [sp, #124]                 ; 4-byte Folded Spill
	and	w10, w10, #0xff
	cmp	w10, #80
	b.lo	LBB3_114
; %bb.113:                              ;   in Loop: Header=BB3_23 Depth=2
	mov	w10, #41247
	movk	w10, #7, lsl #16
	cmp	x16, x10
	b.hi	LBB3_186
LBB3_114:                               ;   in Loop: Header=BB3_23 Depth=2
	add	x10, x3, #9
	cmp	x10, x9
	b.ne	LBB3_136
; %bb.115:                              ;   in Loop: Header=BB3_23 Depth=2
	ldr	w10, [sp, #120]                 ; 4-byte Folded Reload
	add	w10, w10, #1
	str	w10, [sp, #120]                 ; 4-byte Folded Spill
	and	w10, w10, #0xff
	cmp	w10, #30
	b.lo	LBB3_117
; %bb.116:                              ;   in Loop: Header=BB3_23 Depth=2
	mov	w10, #41247
	movk	w10, #7, lsl #16
	cmp	x16, x10
	b.hi	LBB3_187
LBB3_117:                               ;   in Loop: Header=BB3_23 Depth=2
	add	x10, x3, #10
	cmp	x10, x9
	b.ne	LBB3_137
LBB3_118:                               ;   in Loop: Header=BB3_23 Depth=2
	ldr	w10, [sp, #116]                 ; 4-byte Folded Reload
	add	w10, w10, #1
	str	w10, [sp, #116]                 ; 4-byte Folded Spill
	and	w10, w10, #0xff
	cmp	w10, #30
	b.lo	LBB3_120
; %bb.119:                              ;   in Loop: Header=BB3_23 Depth=2
	mov	w10, #41247
	movk	w10, #7, lsl #16
	cmp	x16, x10
	b.hi	LBB3_188
LBB3_120:                               ;   in Loop: Header=BB3_23 Depth=2
	add	x10, x3, #11
	cmp	x10, x9
	b.ne	LBB3_138
LBB3_121:                               ;   in Loop: Header=BB3_23 Depth=2
	ldr	w10, [sp, #112]                 ; 4-byte Folded Reload
	add	w10, w10, #1
	str	w10, [sp, #112]                 ; 4-byte Folded Spill
	and	w10, w10, #0xff
	cmp	w10, #30
	b.lo	LBB3_123
; %bb.122:                              ;   in Loop: Header=BB3_23 Depth=2
	mov	w10, #41247
	movk	w10, #7, lsl #16
	cmp	x16, x10
	b.hi	LBB3_189
LBB3_123:                               ;   in Loop: Header=BB3_23 Depth=2
	add	x10, x3, #12
	cmp	x10, x9
	b.ne	LBB3_139
LBB3_124:                               ;   in Loop: Header=BB3_23 Depth=2
	ldr	w10, [sp, #108]                 ; 4-byte Folded Reload
	add	w10, w10, #1
	and	w9, w10, #0xff
	cmp	w9, #30
	b.lo	LBB3_126
; %bb.125:                              ;   in Loop: Header=BB3_23 Depth=2
	mov	w9, #41247
	movk	w9, #7, lsl #16
	cmp	x16, x9
	b.hi	LBB3_190
LBB3_126:                               ;   in Loop: Header=BB3_23 Depth=2
	ands	w9, w17, #0xff
	b.ne	LBB3_140
	b	LBB3_143
LBB3_127:                               ;   in Loop: Header=BB3_23 Depth=2
	ldr	w11, [sp, #152]                 ; 4-byte Folded Reload
	add	w11, w11, #1
	str	w11, [sp, #152]                 ; 4-byte Folded Spill
	and	w12, w11, #0xff
	cmp	w12, #7
	b.hi	LBB3_177
LBB3_128:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w11, #0
	b	LBB3_130
LBB3_129:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w11, #0
	str	wzr, [sp, #152]                 ; 4-byte Folded Spill
LBB3_130:                               ;   in Loop: Header=BB3_23 Depth=2
	str	x11, [sp, #208]                 ; 8-byte Folded Spill
	mov	w24, #0
	ldr	x11, [sp, #200]                 ; 8-byte Folded Reload
	add	x11, x11, #1
	str	x11, [sp, #200]                 ; 8-byte Folded Spill
	add	x3, x3, #1
	ldr	x11, [sp, #192]                 ; 8-byte Folded Reload
	cmp	x3, x11
	b.ls	LBB3_147
; %bb.131:                              ;   in Loop: Header=BB3_23 Depth=2
	mov	w11, #16960
	movk	w11, #15, lsl #16
	cmp	x3, x11
	b.hs	LBB3_147
; %bb.132:                              ;   in Loop: Header=BB3_23 Depth=2
	str	xzr, [x27, x3, lsl #3]
	cmp	x3, #511
	b.hi	LBB3_135
; %bb.133:                              ;   in Loop: Header=BB3_23 Depth=2
	add	x11, sp, #224
	strb	wzr, [x11, x3]
	cmp	x3, #19
	b.hi	LBB3_135
; %bb.134:                              ;   in Loop: Header=BB3_23 Depth=2
	mov	w24, #0
	sub	x11, x29, #160
	strb	wzr, [x11, x3]
	str	x3, [sp, #192]                  ; 8-byte Folded Spill
	add	x16, x16, #1
	cmp	x16, x4
	b.ne	LBB3_148
	b	LBB3_149
LBB3_135:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w24, #0
	str	x3, [sp, #192]                  ; 8-byte Folded Spill
	b	LBB3_147
LBB3_136:                               ;   in Loop: Header=BB3_23 Depth=2
	str	wzr, [sp, #120]                 ; 4-byte Folded Spill
	add	x10, x3, #10
	cmp	x10, x9
	b.eq	LBB3_118
LBB3_137:                               ;   in Loop: Header=BB3_23 Depth=2
	str	wzr, [sp, #116]                 ; 4-byte Folded Spill
	add	x10, x3, #11
	cmp	x10, x9
	b.eq	LBB3_121
LBB3_138:                               ;   in Loop: Header=BB3_23 Depth=2
	str	wzr, [sp, #112]                 ; 4-byte Folded Spill
	add	x10, x3, #12
	cmp	x10, x9
	b.eq	LBB3_124
LBB3_139:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w10, #0
	ands	w9, w17, #0xff
	b.eq	LBB3_143
LBB3_140:                               ;   in Loop: Header=BB3_23 Depth=2
	cmp	w9, #3
	b.ne	LBB3_142
; %bb.141:                              ;   in Loop: Header=BB3_23 Depth=2
	ldr	w9, [sp, #152]                  ; 4-byte Folded Reload
	add	w9, w9, #1
	str	w9, [sp, #152]                  ; 4-byte Folded Spill
	and	w9, w9, #0xff
	cmp	w9, #7
	b.ls	LBB3_143
	b	LBB3_177
LBB3_142:                               ;   in Loop: Header=BB3_23 Depth=2
	str	wzr, [sp, #152]                 ; 4-byte Folded Spill
LBB3_143:                               ;   in Loop: Header=BB3_23 Depth=2
	str	w10, [sp, #108]                 ; 4-byte Folded Spill
	mov	w9, #1
	lsl	x9, x9, x3
	cmp	x3, #64
	csel	x9, x9, xzr, lo
	orr	x28, x28, x9
	cmp	x3, #19
	b.hi	LBB3_146
; %bb.144:                              ;   in Loop: Header=BB3_23 Depth=2
	sub	x10, x29, #160
	ldrb	w9, [x10, x3]
	cmp	w9, #249
	b.hi	LBB3_146
; %bb.145:                              ;   in Loop: Header=BB3_23 Depth=2
	str	xzr, [sp, #208]                 ; 8-byte Folded Spill
	mov	w24, #0
	add	w9, w9, #1
	strb	w9, [x10, x3]
	mov	x10, x3
	mov	x9, x3
	mov	x3, #0
	add	x16, x16, #1
	cmp	x16, x4
	b.ne	LBB3_148
	b	LBB3_149
LBB3_146:                               ;   in Loop: Header=BB3_23 Depth=2
	str	xzr, [sp, #208]                 ; 8-byte Folded Spill
	mov	w24, #0
	mov	x10, x3
	mov	x9, x3
	mov	x3, #0
	add	x16, x16, #1
	cmp	x16, x4
	b.ne	LBB3_148
	b	LBB3_149
LBB3_147:                               ;   in Loop: Header=BB3_23 Depth=2
	add	x16, x16, #1
	cmp	x16, x4
	b.eq	LBB3_149
LBB3_148:                               ;   in Loop: Header=BB3_23 Depth=2
	and	w11, w13, #0xffff
	ldr	x2, [x25]
	ldr	x12, [x27, x3, lsl #3]
	cmp	x2, x12
	cset	w15, lo
	lsl	w15, w15, #2
	csel	w15, w22, w15, hi
	csel	w15, w0, w15, eq
	lsr	w11, w11, w15
	and	w11, w11, #0xf
	and	w13, w13, w5
	cmp	w8, #3
	ccmp	w13, #0, #4, ne
	csel	w23, w8, w23, eq
	ldrh	w13, [x21, w11, uxtw #1]
	and	w8, w13, #0xf
	cmp	w8, #4
	b.ls	LBB3_25
	b	LBB3_147
LBB3_149:                               ;   in Loop: Header=BB3_23 Depth=2
	ldr	x8, [sp, #192]                  ; 8-byte Folded Reload
	cmp	x8, #11
	b.lo	LBB3_160
; %bb.150:                              ;   in Loop: Header=BB3_23 Depth=2
	ldr	x9, [x27]
	cmp	x9, #20
	b.lo	LBB3_160
; %bb.151:                              ;   in Loop: Header=BB3_23 Depth=2
	ldr	x8, [x27, #8]
	cmp	x8, #20
	b.lo	LBB3_160
; %bb.152:                              ;   in Loop: Header=BB3_23 Depth=2
	ldr	x10, [x27, #16]
	cmp	x10, #20
	b.lo	LBB3_160
; %bb.153:                              ;   in Loop: Header=BB3_23 Depth=2
	ldr	x12, [x27, #24]
	cmp	x12, #20
	b.lo	LBB3_160
; %bb.154:                              ;   in Loop: Header=BB3_23 Depth=2
	ldr	x13, [x27, #32]
	cmp	x13, #20
	b.lo	LBB3_160
; %bb.155:                              ;   in Loop: Header=BB3_23 Depth=2
	ldr	x14, [x27, #40]
	cmp	x14, #20
	b.lo	LBB3_160
; %bb.156:                              ;   in Loop: Header=BB3_23 Depth=2
	ldr	x15, [x27, #48]
	cmp	x15, #20
	b.lo	LBB3_160
; %bb.157:                              ;   in Loop: Header=BB3_23 Depth=2
	ldr	x11, [x27, #56]
	cmp	x11, #20
	b.lo	LBB3_160
; %bb.158:                              ;   in Loop: Header=BB3_23 Depth=2
	ldr	x11, [x27, #64]
	cmp	x11, #20
	b.lo	LBB3_160
; %bb.159:                              ;   in Loop: Header=BB3_23 Depth=2
	ldr	x11, [x27, #72]
	cmp	x11, #20
	b.hs	LBB3_191
LBB3_160:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #19
	b	LBB3_21
LBB3_161:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #18
	b	LBB3_20
LBB3_162:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #17
	b	LBB3_20
LBB3_163:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #14
	b	LBB3_20
LBB3_164:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #15
	b	LBB3_20
LBB3_165:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #2
	b	LBB3_21
LBB3_166:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #4
	b	LBB3_21
LBB3_167:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #5
	b	LBB3_21
LBB3_168:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #7
	b	LBB3_21
LBB3_169:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #1
	b	LBB3_21
LBB3_170:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #6
	b	LBB3_21
LBB3_171:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #3
	b	LBB3_21
LBB3_172:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #10
	b	LBB3_21
LBB3_173:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #11
	b	LBB3_21
LBB3_174:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #8
	mov	x27, x25
	ldr	x25, [sp, #48]                  ; 8-byte Folded Reload
	b	LBB3_21
LBB3_175:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #12
	b	LBB3_21
LBB3_176:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #13
	b	LBB3_21
LBB3_177:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #20
	b	LBB3_21
LBB3_178:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #9
	b	LBB3_21
LBB3_179:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #22
	ldr	x25, [sp, #48]                  ; 8-byte Folded Reload
	b	LBB3_21
LBB3_180:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #23
	b	LBB3_21
LBB3_181:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #24
	b	LBB3_21
LBB3_182:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #25
	b	LBB3_21
LBB3_183:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #26
	b	LBB3_21
LBB3_184:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #27
	b	LBB3_21
LBB3_185:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #28
	b	LBB3_21
LBB3_186:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #29
	b	LBB3_21
LBB3_187:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #30
	b	LBB3_21
LBB3_188:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #31
	b	LBB3_21
LBB3_189:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #32
	b	LBB3_21
LBB3_190:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #33
	b	LBB3_21
LBB3_191:                               ;   in Loop: Header=BB3_23 Depth=2
	add	x11, x13, x12
	add	x11, x11, x14
	add	x11, x11, x15
	lsr	x11, x11, #2
	add	x12, x11, x11, lsl #1
	lsr	x13, x12, #1
	cmp	x9, x13
	b.ls	LBB3_193
; %bb.192:                              ;   in Loop: Header=BB3_23 Depth=2
	cmp	x12, #199
	b.hi	LBB3_195
	b	LBB3_196
LBB3_193:                               ;   in Loop: Header=BB3_23 Depth=2
	cmp	x10, x13
	ccmp	x8, x13, #2, ls
	b.ls	LBB3_196
; %bb.194:                              ;   in Loop: Header=BB3_23 Depth=2
	cmp	x12, #199
	b.ls	LBB3_196
LBB3_195:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #21
	b	LBB3_21
LBB3_196:                               ;   in Loop: Header=BB3_23 Depth=2
	ldr	x8, [sp, #192]                  ; 8-byte Folded Reload
	cmp	x8, #100
	b.lo	LBB3_201
; %bb.197:                              ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #1
LBB3_198:                               ;   Parent Loop BB3_3 Depth=1
                                        ;     Parent Loop BB3_23 Depth=2
                                        ; =>    This Inner Loop Header: Depth=3
	lsr	x9, x28, x8
	tbz	w9, #0, LBB3_202
; %bb.199:                              ;   in Loop: Header=BB3_198 Depth=3
	add	x8, x8, #1
	cmp	x8, #64
	b.ne	LBB3_198
; %bb.200:                              ;   in Loop: Header=BB3_23 Depth=2
	sub	x8, x29, #176
	ldur	q0, [x8, #17]
	movi.16b	v1, #5
	cmhi.16b	v0, v1, v0
	umov.b	w8, v0[1]
	and	w8, w8, #0x1
	umov.b	w9, v0[0]
	and	w9, w9, #0x1
	bfi	w9, w8, #1, #1
	umov.b	w8, v0[2]
	and	w8, w8, #0x1
	bfi	w9, w8, #2, #1
	umov.b	w8, v0[3]
	and	w8, w8, #0x1
	bfi	w9, w8, #3, #1
	umov.b	w8, v0[4]
	and	w8, w8, #0x1
	bfi	w9, w8, #4, #1
	umov.b	w8, v0[5]
	and	w8, w8, #0x1
	bfi	w9, w8, #5, #1
	umov.b	w8, v0[6]
	and	w8, w8, #0x1
	orr	w8, w9, w8, lsl #6
	umov.b	w9, v0[7]
	and	w9, w9, #0x1
	orr	w8, w8, w9, lsl #7
	umov.b	w9, v0[8]
	and	w9, w9, #0x1
	orr	w8, w8, w9, lsl #8
	umov.b	w9, v0[9]
	and	w9, w9, #0x1
	orr	w8, w8, w9, lsl #9
	umov.b	w9, v0[10]
	and	w9, w9, #0x1
	orr	w8, w8, w9, lsl #10
	umov.b	w9, v0[11]
	and	w9, w9, #0x1
	orr	w8, w8, w9, lsl #11
	umov.b	w9, v0[12]
	and	w9, w9, #0x1
	orr	w8, w8, w9, lsl #12
	umov.b	w9, v0[13]
	and	w9, w9, #0x1
	orr	w8, w8, w9, lsl #13
	umov.b	w9, v0[14]
	and	w9, w9, #0x1
	orr	w8, w8, w9, lsl #14
	umov.b	w9, v0[15]
	orr	w8, w8, w9, lsl #15
	and	w8, w8, #0xffff
	ldurb	w9, [x29, #-143]
	ldurb	w10, [x29, #-142]
	ldurb	w11, [x29, #-141]
	cmp	w11, #5
	ccmp	w10, #5, #0, hs
	ccmp	w9, #5, #0, hs
	ccmp	w8, #0, #0, hs
	mov	w8, #35
	csel	x8, x8, xzr, ne
	b	LBB3_21
LBB3_201:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	x8, #0
	b	LBB3_21
LBB3_202:                               ;   in Loop: Header=BB3_23 Depth=2
	mov	w8, #34
	b	LBB3_21
LBB3_203:                               ;   in Loop: Header=BB3_3 Depth=1
	ldr	x9, [sp, #64]                   ; 8-byte Folded Reload
	ldr	x8, [x9, #296]
	add	x8, x8, #1
	str	x8, [x9, #296]
	ldrb	w8, [x2, __MergedGlobals@PAGEOFF]
	cmp	w8, #1
	b.ne	LBB3_2
; %bb.204:                              ;   in Loop: Header=BB3_3 Depth=1
	mov	w8, #0
	b	LBB3_212
LBB3_205:                               ;   in Loop: Header=BB3_3 Depth=1
	tbz	w7, #0, LBB3_210
; %bb.206:                              ;   in Loop: Header=BB3_3 Depth=1
	ldr	x8, [sp, #40]                   ; 8-byte Folded Reload
Lloh142:
	adrp	x9, __MergedGlobals@PAGE
Lloh143:
	add	x9, x9, __MergedGlobals@PAGEOFF
	add	x8, x9, x8, lsl #3
	ldr	x23, [x8, #40]
	sub	x8, x29, #176
	movi.2d	v0, #0000000000000000
	stp	q0, q0, [x8, #16]
	add	x0, sp, #224
	mov	x1, #0
	bl	_gettimeofday
	add	x0, sp, #224
	bl	_localtime
	mov	x3, x0
	sub	x0, x29, #160
	mov	w1, #32
Lloh144:
	adrp	x2, l_.str.20@PAGE
Lloh145:
	add	x2, x2, l_.str.20@PAGEOFF
	bl	_strftime
	mov	x8, #0
	movi.2d	v0, #0000000000000000
	stp	q0, q0, [sp, #256]
	stp	q0, q0, [sp, #224]
	add	x11, sp, #224
LBB3_207:                               ;   Parent Loop BB3_3 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	lsr	x9, x8, #1
	and	x9, x9, #0x7ffffffffffffffe
	ldrh	w9, [x21, x9]
	ubfiz	w10, w8, #2, #2
	lsr	w9, w9, w10
	mov	w10, #48
	bfxil	w10, w9, #0, #4
	strb	w10, [x11, x8]
	add	x8, x8, #1
	cmp	x8, #36
	b.ne	LBB3_207
; %bb.208:                              ;   in Loop: Header=BB3_3 Depth=1
	strb	wzr, [sp, #260]
	ubfx	w8, w20, #2, #6
	sub	x9, x29, #176
	stp	xzr, xzr, [x9]
	str	x8, [sp]
	sub	x0, x29, #176
	mov	w1, #16
Lloh146:
	adrp	x2, l_.str.32@PAGE
Lloh147:
	add	x2, x2, l_.str.32@PAGEOFF
	bl	_snprintf
	str	xzr, [sp]
	mov	x0, x23
	mov	w1, #9
	bl	_open
	tbnz	w0, #31, LBB3_220
; %bb.209:                              ;   in Loop: Header=BB3_3 Depth=1
	mov	x28, x0
Lloh148:
	adrp	x20, l_.str.36@PAGE
Lloh149:
	add	x20, x20, l_.str.36@PAGEOFF
	b	LBB3_222
LBB3_210:                               ;   in Loop: Header=BB3_3 Depth=1
	mov	w8, #0
Lloh150:
	adrp	x20, __MergedGlobals@PAGE
Lloh151:
	add	x20, x20, __MergedGlobals@PAGEOFF
	mov	x17, #-1
	mov	w1, #15
LBB3_211:                               ;   in Loop: Header=BB3_3 Depth=1
	adrp	x2, __MergedGlobals@PAGE
Lloh152:
	adrp	x3, __MergedGlobals@PAGE+8
Lloh153:
	add	x3, x3, __MergedGlobals@PAGEOFF+8
Lloh154:
	adrp	x7, lJTI3_0@PAGE
Lloh155:
	add	x7, x7, lJTI3_0@PAGEOFF
LBB3_212:                               ;   in Loop: Header=BB3_3 Depth=1
	ldr	x9, [x3, #8]
	lsl	w10, w8, #2
	lsr	x11, x19, x10
	and	w11, w11, #0xf
	lsr	x12, x9, x10
	and	w12, w12, #0xf
	sub	w12, w12, #1
	cmp	w12, w11
	b.le	LBB3_215
; %bb.213:                              ;   in Loop: Header=BB3_3 Depth=1
	mov	w8, #1
	lsl	x8, x8, x10
LBB3_214:                               ;   in Loop: Header=BB3_3 Depth=1
	mov	x9, x8
	mov	x8, #0
	add	x26, x8, x26
	add	x19, x9, x19
	ubfx	x8, x19, #16, #4
	cmp	w8, #6
	b.ls	LBB3_4
	b	LBB3_14
LBB3_215:                               ;   in Loop: Header=BB3_3 Depth=1
	ldr	x11, [x3]
	ldrb	w12, [x2, __MergedGlobals@PAGEOFF]
	cmp	w12, #0
	mov	w12, #22
	csinv	w12, w12, wzr, ne
	mov	w13, #1
LBB3_216:                               ;   Parent Loop BB3_3 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	cmp	w12, w8, uxtb
	b.le	LBB3_2
; %bb.217:                              ;   in Loop: Header=BB3_216 Depth=2
	lsl	x10, x1, x10
	tst	w13, #0x1
	csinv	x13, x17, x10, ne
	and	x26, x26, x13
	csinv	x10, x17, x10, eq
	and	x19, x19, x10
	add	w8, w8, #1
	and	w14, w8, #0xff
	cmp	w14, #16
	cset	w13, lo
	csel	x15, x19, x26, lo
	ubfiz	w10, w8, #2, #4
	lsr	x15, x15, x10
	and	w15, w15, #0xf
	csel	x16, x9, x11, lo
	lsr	x16, x16, x10
	and	w16, w16, #0xf
	sub	w16, w16, #1
	cmp	w16, w15
	b.le	LBB3_216
; %bb.218:                              ;   in Loop: Header=BB3_3 Depth=1
	mov	w8, #1
	lsl	x8, x8, x10
	cmp	w14, #15
	b.ls	LBB3_214
; %bb.219:                              ;   in Loop: Header=BB3_3 Depth=1
	mov	x9, #0
	add	x26, x8, x26
	add	x19, x9, x19
	ubfx	x8, x19, #16, #4
	cmp	w8, #6
	b.ls	LBB3_4
	b	LBB3_14
LBB3_220:                               ;   in Loop: Header=BB3_3 Depth=1
	bl	_rand
	mov	x20, x0
	bl	_rand
	mov	x24, x0
	bl	_rand
	mov	x28, x0
	bl	_rand
                                        ; kill: def $w0 killed $w0 def $x0
	stp	x28, x0, [sp, #24]
	stp	x20, x24, [sp, #8]
	sub	x8, x29, #160
	str	x8, [sp]
	mov	x0, x23
	mov	w1, #4096
Lloh156:
	adrp	x2, l_.str.11@PAGE
Lloh157:
	add	x2, x2, l_.str.11@PAGEOFF
	bl	_snprintf
	mov	w8, #420
	str	x8, [sp]
	mov	x0, x23
	mov	w1, #2569
	bl	_open
	tbnz	w0, #31, LBB3_227
; %bb.221:                              ;   in Loop: Header=BB3_3 Depth=1
	mov	x28, x0
Lloh158:
	adrp	x20, l_.str.35@PAGE
Lloh159:
	add	x20, x20, l_.str.35@PAGEOFF
LBB3_222:                               ;   in Loop: Header=BB3_3 Depth=1
	add	x1, sp, #224
	mov	x0, x28
	mov	w2, #36
	bl	_write
	sub	x1, x29, #176
	mov	x0, x28
	mov	w2, #3
	bl	_write
	sub	x1, x29, #160
	mov	x0, x28
	mov	w2, #17
	bl	_write
	mov	x0, x28
Lloh160:
	adrp	x1, l_.str.23@PAGE
Lloh161:
	add	x1, x1, l_.str.23@PAGEOFF
	mov	w2, #1
	bl	_write
	mov	x0, x28
	bl	_close
	add	x8, sp, #224
	stp	x8, x23, [sp, #16]
	sub	x8, x29, #160
	stp	x8, x20, [sp]
Lloh162:
	adrp	x0, l_.str.34@PAGE
Lloh163:
	add	x0, x0, l_.str.34@PAGEOFF
	bl	_printf
	mov	w8, #0
Lloh164:
	adrp	x20, __MergedGlobals@PAGE
Lloh165:
	add	x20, x20, __MergedGlobals@PAGEOFF
	mov	x17, #-1
	mov	w0, #12
	mov	w1, #15
	mov	w4, #57600
	movk	w4, #1525, lsl #16
	mov	w5, #13
Lloh166:
	adrp	x6, lJTI3_1@PAGE
Lloh167:
	add	x6, x6, lJTI3_1@PAGEOFF
	b	LBB3_211
LBB3_223:
	mov	x26, #0
	mov	x19, #0
LBB3_224:
	mov	x0, x27
	bl	_free
	mov	x0, x21
	bl	_free
	ldr	x8, [sp, #40]                   ; 8-byte Folded Reload
	add	x8, x20, x8, lsl #2
	strh	w19, [x8, #66]
	strh	w26, [x8, #68]
	ldur	x8, [x29, #-120]
Lloh168:
	adrp	x9, ___stack_chk_guard@GOTPAGE
Lloh169:
	ldr	x9, [x9, ___stack_chk_guard@GOTPAGEOFF]
Lloh170:
	ldr	x9, [x9]
	cmp	x9, x8
	b.ne	LBB3_228
; %bb.225:
	ldr	x0, [sp, #64]                   ; 8-byte Folded Reload
	add	sp, sp, #816
	ldp	x29, x30, [sp, #96]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #80]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #64]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #48]             ; 16-byte Folded Reload
	ldp	x26, x25, [sp, #32]             ; 16-byte Folded Reload
	ldp	x28, x27, [sp, #16]             ; 16-byte Folded Reload
	ldp	d9, d8, [sp], #112              ; 16-byte Folded Reload
	ret
LBB3_226:
	mov	x0, x21
	bl	_worker_thread.cold.2
LBB3_227:
	sub	x0, x29, #160
	add	x1, sp, #224
	mov	x2, x23
	bl	_worker_thread.cold.1
LBB3_228:
	bl	___stack_chk_fail
	.loh AdrpAdd	Lloh122, Lloh123
	.loh AdrpAdd	Lloh120, Lloh121
	.loh AdrpLdrGotLdr	Lloh117, Lloh118, Lloh119
	.loh AdrpAdd	Lloh128, Lloh129
	.loh AdrpAdd	Lloh126, Lloh127
	.loh AdrpAdd	Lloh124, Lloh125
	.loh AdrpAdd	Lloh130, Lloh131
	.loh AdrpAdd	Lloh132, Lloh133
	.loh AdrpAdd	Lloh134, Lloh135
	.loh AdrpAdd	Lloh136, Lloh137
	.loh AdrpAdd	Lloh138, Lloh139
	.loh AdrpAdd	Lloh140, Lloh141
	.loh AdrpAdd	Lloh144, Lloh145
	.loh AdrpAdd	Lloh142, Lloh143
	.loh AdrpAdd	Lloh146, Lloh147
	.loh AdrpAdd	Lloh148, Lloh149
	.loh AdrpAdd	Lloh150, Lloh151
	.loh AdrpAdd	Lloh154, Lloh155
	.loh AdrpAdd	Lloh152, Lloh153
	.loh AdrpAdd	Lloh156, Lloh157
	.loh AdrpAdd	Lloh158, Lloh159
	.loh AdrpAdd	Lloh166, Lloh167
	.loh AdrpAdd	Lloh164, Lloh165
	.loh AdrpAdd	Lloh162, Lloh163
	.loh AdrpAdd	Lloh160, Lloh161
	.loh AdrpLdrGotLdr	Lloh168, Lloh169, Lloh170
	.cfi_endproc
	.section	__TEXT,__const
lJTI3_0:
	.byte	(LBB3_5-LBB3_5)>>2
	.byte	(LBB3_14-LBB3_5)>>2
	.byte	(LBB3_9-LBB3_5)>>2
	.byte	(LBB3_14-LBB3_5)>>2
	.byte	(LBB3_5-LBB3_5)>>2
	.byte	(LBB3_14-LBB3_5)>>2
	.byte	(LBB3_9-LBB3_5)>>2
lJTI3_1:
	.byte	(LBB3_26-LBB3_26)>>2
	.byte	(LBB3_36-LBB3_26)>>2
	.byte	(LBB3_34-LBB3_26)>>2
	.byte	(LBB3_38-LBB3_26)>>2
	.byte	(LBB3_32-LBB3_26)>>2
lJTI3_2:
	.byte	(LBB3_128-LBB3_73)>>2
	.byte	(LBB3_130-LBB3_73)>>2
	.byte	(LBB3_129-LBB3_73)>>2
	.byte	(LBB3_73-LBB3_73)>>2
	.byte	(LBB3_127-LBB3_73)>>2
                                        ; -- End function
	.section	__TEXT,__text,regular,pure_instructions
	.p2align	2                               ; -- Begin function print.cold.1
_print.cold.1:                          ; @print.cold.1
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48
	.cfi_def_cfa_offset 48
	stp	x20, x19, [sp, #16]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	mov	x19, x0
Lloh171:
	adrp	x0, l_.str.27@PAGE
Lloh172:
	add	x0, x0, l_.str.27@PAGEOFF
	bl	_perror
Lloh173:
	adrp	x8, _main.output_filename@PAGE
Lloh174:
	add	x8, x8, _main.output_filename@PAGEOFF
	stp	x19, x8, [sp]
Lloh175:
	adrp	x0, l_.str.28@PAGE
Lloh176:
	add	x0, x0, l_.str.28@PAGEOFF
	bl	_printf
Lloh177:
	adrp	x8, ___stdoutp@GOTPAGE
Lloh178:
	ldr	x8, [x8, ___stdoutp@GOTPAGEOFF]
Lloh179:
	ldr	x0, [x8]
	bl	_fflush
	bl	_abort
	.loh AdrpLdrGotLdr	Lloh177, Lloh178, Lloh179
	.loh AdrpAdd	Lloh175, Lloh176
	.loh AdrpAdd	Lloh173, Lloh174
	.loh AdrpAdd	Lloh171, Lloh172
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function worker_thread.cold.1
_worker_thread.cold.1:                  ; @worker_thread.cold.1
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #80
	.cfi_def_cfa_offset 80
	stp	x22, x21, [sp, #32]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #48]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #64]             ; 16-byte Folded Spill
	add	x29, sp, #64
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	mov	x19, x2
	mov	x20, x1
	mov	x21, x0
Lloh180:
	adrp	x0, l_.str.27@PAGE
Lloh181:
	add	x0, x0, l_.str.27@PAGEOFF
	bl	_perror
	stp	x20, x19, [sp, #8]
	str	x21, [sp]
Lloh182:
	adrp	x0, l_.str.33@PAGE
Lloh183:
	add	x0, x0, l_.str.33@PAGEOFF
	bl	_printf
Lloh184:
	adrp	x8, ___stdoutp@GOTPAGE
Lloh185:
	ldr	x8, [x8, ___stdoutp@GOTPAGEOFF]
Lloh186:
	ldr	x0, [x8]
	bl	_fflush
	bl	_abort
	.loh AdrpLdrGotLdr	Lloh184, Lloh185, Lloh186
	.loh AdrpAdd	Lloh182, Lloh183
	.loh AdrpAdd	Lloh180, Lloh181
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function worker_thread.cold.2
_worker_thread.cold.2:                  ; @worker_thread.cold.2
	.cfi_startproc
; %bb.0:
	stp	x20, x19, [sp, #-32]!           ; 16-byte Folded Spill
	.cfi_def_cfa_offset 32
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	add	x29, sp, #16
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	mov	x19, x0
Lloh187:
	adrp	x0, l_.str.31@PAGE
Lloh188:
	add	x0, x0, l_.str.31@PAGEOFF
	bl	_puts
	mov	x0, x19
	bl	_print_graph_raw
	mov	w0, #10
	bl	_putchar
	bl	_abort
	.loh AdrpAdd	Lloh187, Lloh188
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__const
l___const.main.zerosp_pas_map:          ; @__const.main.zerosp_pas_map
	.ascii	"\002\003\006\007\t\n\013\r\016\017\021\023"
	.space	20

	.p2align	1                               ; @__const.main.actual_partial_graph
l___const.main.actual_partial_graph:
	.short	16                              ; 0x10
	.short	1                               ; 0x1
	.short	2                               ; 0x2
	.short	3                               ; 0x3
	.short	1028                            ; 0x404
	.short	0                               ; 0x0
	.short	0                               ; 0x0
	.short	0                               ; 0x0
	.short	0                               ; 0x0

_banned_edges:                          ; @banned_edges
	.asciz	"\000\002\001\001\002\001\002\002\001\003\002\001\004\002\001\000\001\003\000\002\003\000\003\003\001\001\004\001\002\004\001\003\004\002\001\002\002\002\002\002\003\002\003\001\003\003\002\003\003\003\003\004\001\004\004\003\004\004\003\000\004\003\003\000\002\000\000\003"

	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"pas_count = %hhu\n\n"

l_.str.1:                               ; @.str.1
	.asciz	"job_modulus = 0x%llx\n\n"

l_.str.2:                               ; @.str.2
	.asciz	"job_placement_in_PAS = %hhu\n\n"

l_.str.3:                               ; @.str.3
	.asciz	"mod0 = 0x%016llx, mod1 = 0x%016llx \n\n"

l_.str.4:                               ; @.str.4
	.asciz	"partial_graph[] = "

l_.str.6:                               ; @.str.6
	.asciz	"pas_map[] = { "

l_.str.7:                               ; @.str.7
	.asciz	"%hhu, "

l_.str.9:                               ; @.str.9
	.asciz	"decode[] = { "

l_.str.10:                              ; @.str.10
	.asciz	"0x%llx, "

.zerofill __DATA,__bss,_main.output_filename,4096,0 ; @main.output_filename
.zerofill __DATA,__bss,_main.output_string,4096,3 ; @main.output_string
l_.str.11:                              ; @.str.11
	.asciz	"%s_%08x%08x%08x%08x_z.txt"

.zerofill __DATA,__bss,_queue,4096,1    ; @queue
l_.str.12:                              ; @.str.12
	.asciz	"printing jobs: (%llu total jobs)\n"

l_.str.13:                              ; @.str.13
	.asciz	"%04hx "

l_.str.14:                              ; @.str.14
	.asciz	"SU: searching [D=%u] space....\n"

l_.str.15:                              ; @.str.15
	.asciz	"\033[H\033[2J"

l_.str.16:                              ; @.str.16
	.asciz	"----------------- jobs remaining %llu / %llu -------------------\n"

l_.str.17:                              ; @.str.17
	.asciz	"\n\t complete %1.10lf%%\n\n"

l_.str.18:                              ; @.str.18
	.asciz	" %5llu : "

l_.str.19:                              ; @.str.19
	.asciz	"\nmain: joining threads...\n"

l_.str.20:                              ; @.str.20
	.asciz	"1%Y%m%d%u.%H%M%S"

l_.str.21:                              ; @.str.21
	.asciz	"su: D = %u space:\n\t thread_count = %u\n\t machine_index = %u\n\t machine0_throughput = %u\n\t machine1_throughput = %u\n\t display_rate = %u\n\t execution_limit = %llu\n\t array_size = %llu\n\t in %10.2lfs [%s:%s]\n\n"

l_.str.22:                              ; @.str.22
	.asciz	"\npm counts:\n"

l_.str.23:                              ; @.str.23
	.asciz	"\n"

l_.str.24:                              ; @.str.24
	.asciz	"%6s: %-8lld\t\t"

	.section	__DATA,__const
	.p2align	3                               ; @pm_spelling
_pm_spelling:
	.quad	l_.str.37
	.quad	l_.str.38
	.quad	l_.str.39
	.quad	l_.str.40
	.quad	l_.str.41
	.quad	l_.str.42
	.quad	l_.str.43
	.quad	l_.str.44
	.quad	l_.str.45
	.quad	l_.str.46
	.quad	l_.str.47
	.quad	l_.str.48
	.quad	l_.str.49
	.quad	l_.str.50
	.quad	l_.str.51
	.quad	l_.str.52
	.quad	l_.str.53
	.quad	l_.str.54
	.quad	l_.str.55
	.quad	l_.str.56
	.quad	l_.str.57
	.quad	l_.str.58
	.quad	l_.str.59
	.quad	l_.str.60
	.quad	l_.str.61
	.quad	l_.str.62
	.quad	l_.str.63
	.quad	l_.str.64
	.quad	l_.str.65
	.quad	l_.str.66
	.quad	l_.str.67
	.quad	l_.str.68
	.quad	l_.str.69
	.quad	l_.str.70
	.quad	l_.str.71
	.quad	l_.str.72
	.quad	l_.str.73
	.quad	l_.str.74
	.quad	l_.str.75
	.quad	l_.str.76
	.quad	l_.str.77
	.quad	l_.str.78
	.quad	l_.str.79
	.quad	l_.str.80

	.section	__TEXT,__cstring,cstring_literals
l_.str.26:                              ; @.str.26
	.asciz	"%hhu"

l_.str.27:                              ; @.str.27
	.asciz	"create openat file"

l_.str.28:                              ; @.str.28
	.asciz	"print: [%s]: failed to create filename = \"%s\"\n"

l_.str.29:                              ; @.str.29
	.asciz	"%s_D%u_%08x%08x%08x%08x_output.txt"

l_.str.30:                              ; @.str.30
	.asciz	"%s"

l_.str.31:                              ; @.str.31
	.asciz	"FEA condition violated by a z value: "

l_.str.32:                              ; @.str.32
	.asciz	" %hhu "

l_.str.33:                              ; @.str.33
	.asciz	"[%s]: [z=%s]: failed to create filename = \"%s\"\n"

l_.str.34:                              ; @.str.34
	.asciz	"[%s]: write: %s z = %s to file \"%s\"\n"

l_.str.35:                              ; @.str.35
	.asciz	"created"

l_.str.36:                              ; @.str.36
	.asciz	"wrote"

l_.str.37:                              ; @.str.37
	.asciz	"z_is_good"

l_.str.38:                              ; @.str.38
	.asciz	"pm_zr5"

l_.str.39:                              ; @.str.39
	.asciz	"pm_zr6"

l_.str.40:                              ; @.str.40
	.asciz	"pm_ndi"

l_.str.41:                              ; @.str.41
	.asciz	"pm_sndi"

l_.str.42:                              ; @.str.42
	.asciz	"pm_pco"

l_.str.43:                              ; @.str.43
	.asciz	"pm_per"

l_.str.44:                              ; @.str.44
	.asciz	"pm_ns0"

l_.str.45:                              ; @.str.45
	.asciz	"pm_oer"

l_.str.46:                              ; @.str.46
	.asciz	"pm_rsi"

l_.str.47:                              ; @.str.47
	.asciz	"pm_h0"

l_.str.48:                              ; @.str.48
	.asciz	"pm_h0s"

l_.str.49:                              ; @.str.49
	.asciz	"pm_h1"

l_.str.50:                              ; @.str.50
	.asciz	"pm_h2"

l_.str.51:                              ; @.str.51
	.asciz	"pm_rmv"

l_.str.52:                              ; @.str.52
	.asciz	"pm_ormv"

l_.str.53:                              ; @.str.53
	.asciz	"pm_imv"

l_.str.54:                              ; @.str.54
	.asciz	"pm_csm"

l_.str.55:                              ; @.str.55
	.asciz	"pm_lmv"

l_.str.56:                              ; @.str.56
	.asciz	"pm_fse"

l_.str.57:                              ; @.str.57
	.asciz	"pm_pair"

l_.str.58:                              ; @.str.58
	.asciz	"pm_ls0"

l_.str.59:                              ; @.str.59
	.asciz	"pm_bdl1"

l_.str.60:                              ; @.str.60
	.asciz	"pm_bdl2"

l_.str.61:                              ; @.str.61
	.asciz	"pm_bdl3"

l_.str.62:                              ; @.str.62
	.asciz	"pm_bdl4"

l_.str.63:                              ; @.str.63
	.asciz	"pm_bdl5"

l_.str.64:                              ; @.str.64
	.asciz	"pm_bdl6"

l_.str.65:                              ; @.str.65
	.asciz	"pm_bdl7"

l_.str.66:                              ; @.str.66
	.asciz	"pm_bdl8"

l_.str.67:                              ; @.str.67
	.asciz	"pm_bdl9"

l_.str.68:                              ; @.str.68
	.asciz	"pm_bdl10"

l_.str.69:                              ; @.str.69
	.asciz	"pm_bdl11"

l_.str.70:                              ; @.str.70
	.asciz	"pm_bdl12"

l_.str.71:                              ; @.str.71
	.asciz	"pm_erp1"

l_.str.72:                              ; @.str.72
	.asciz	"pm_erp2"

l_.str.73:                              ; @.str.73
	.asciz	"pm_ga_rdo"

l_.str.74:                              ; @.str.74
	.asciz	"pm_ga_uo"

l_.str.75:                              ; @.str.75
	.asciz	"pm_ga_5u1"

l_.str.76:                              ; @.str.76
	.asciz	"pm_ga_6u2"

l_.str.77:                              ; @.str.77
	.asciz	"pm_ga_3u5"

l_.str.78:                              ; @.str.78
	.asciz	"pm_ga_3u1"

l_.str.79:                              ; @.str.79
	.asciz	"pm_ga_sndi"

l_.str.80:                              ; @.str.80
	.asciz	"pm_ga_h"

l_str.81:                               ; @str.81
	.asciz	"}"

.zerofill __DATA,__bss,__MergedGlobals,1240,3 ; @_MergedGlobals
.subsections_via_symbols

/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Recreation of the echo UNIX utility
 *
 *  Copyright (C) 2026 by Diego Evaristo <di.diegoevaristo@gmail.com>
 */

.section .text
.globl main
.type main, @function
main:
	addi sp, sp, -16
	sd s0, 8(sp)
	sd ra, 16(sp)

	addi s0, a1, 8
.loop:
	ld a0, 0(s0)
	beq a0, zero, .end_loop

	call print
	la a0, space_str
	call print

	addi s0, s0, 8
	j .loop
.end_loop:
	la a0, newline_str
	call print

	ld s0, 8(sp)
	ld ra, 16(sp)
	addi sp, sp, 16
	li a0, 0
	ret

.section .rodata
	space_str: .string " "
	newline_str: .string "\n"

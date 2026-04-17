/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Recreation of the cat UNIX utility
 *
 *  Copyright (C) 2026 by Diego Evaristo <di.diegoevaristo@gmail.com>
 */

.section .text
.globl main
.type main, @function
main:
	addi sp, sp, -2048
	addi sp, sp, -2048
	addi sp, sp, -32
	sd ra, 0(sp)
	sd s0, 8(sp)
	sd s1, 16(sp)
	sd s2, 24(sp)

	addi s0, a1, 8
.loop:
	ld a0, 0(s0)
	beq a0, zero, .end_loop

	li a1, 0 # O_RDONLY
	li a2, 0
	call open
	mv s1, a0
	blt s1, zero, .open_err
.cat_loop:
	mv a0, s1
	addi a1, sp, 24
	li a2, 4096
	call read
	blt a0, zero, .read_err
	beq a0, zero, .end_cat_loop

	mv s2, a0
	mv a2, s2
	addi a1, sp, 24
	li a0, 1
	call write
	bne s2, a0, .write_err
	j .cat_loop
.end_cat_loop:

	addi s0, s0, 8
	j .loop
.end_loop:

	ld ra, 0(sp)
	ld s0, 8(sp)
	ld s1, 16(sp)
	ld s2, 24(sp)
	addi sp, sp, 2047
	addi sp, sp, 2047
	addi sp, sp, 26
	li a0, 0
	ret

.open_err:
	ld a0, 0(s0)
	la a1, openerr
	call err
.read_err:
	ld a0, 0(s0)
	la a1, readerr
	call err
.write_err:
	ld a0, 0(s0)
	la a1, writeerr
	call err

.type openerr, @function
err:
	mv s0, a0
	mv a0, a1
	call print

	mv a0, s0
	call print

	la a0, newline
	call print

	li a0, 1
	call exit

.section .rodata
	newline: .string "\n"
	openerr: .string "open() failed: "
	readerr: .string "read() failed: "
	writeerr: .string "write() failed: "

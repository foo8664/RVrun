/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Recreation of the xxd UNIX utility
 *
 *  Copyright (C) 2026 by Diego Oliveira <di.diegoevaristo@gmail.com>
 */

.section .text
.globl main
.type main, @function
main:
	addi sp, sp, -72
	sd ra, 8(sp)
	sd s0, 16(sp)
	sd s1, 24(sp)

	addi s0, a1, 8

	li a1, 0 # O_RDONLY
	ld a0, 0(s0)
	beq a0, zero, .args_err
	call open
	mv s1, a0
	blt s1, zero, .open_err
.loop:
	mv a0, s1
	addi a1, sp, 41
	li a2, 8
	call read
	blt a0, zero, .read_err
	beq a0, zero, .end_loop

	addi a1, sp, 41
	add a1, a1, a0
	sb zero, 0(a1)
	addi a0, sp, 41
	addi a1, sp, 66
	call hexstr

	addi a0, sp, 66
	call print
	la a0, newline
	call print
	j .loop
.end_loop:
	mv a0, s1
	call close

	ld ra, 8(sp)
	ld s0, 16(sp)
	ld s1, 24(sp)
	addi sp, sp, 72
	li a0, 0
	ret

.args_err:
	la a0, noargserr
	la a1, emptystr
	call err
.read_err:
	la a0, readerr
	ld a1, 0(s0)
	call err
.open_err:
	la a0, openerr
	ld a1, 0(s0)
	call err

# void hexstr(const unsigned char src[9], char dst[25])
.type hexstr, @function
hexstr:

.hexstr_loop:
	lb t0, 0(a0)
	beq t0, zero, .hexstr_end_loop
	li t1, 9
	and t2, t0, 0x0f
	bge t2, t1, .hexstr_hexchar1
.hexstr_hexnum1:
	addi t2, t2, 0x30 # '0'
	sb t2, 1(a1)
	j .hexstr_hex2
.hexstr_hexchar1:
	addi t2, t2, -10
	addi t2, t2, 0x61 # 'a'
	sb t2, 1(a1)

.hexstr_hex2:
	and t2, t0, 0xf0
	srli t2, t2, 4
	bge t2, t1, .hexstr_hexchar2
.hexstr_hexnum2:
	addi t2, t2, 0x30 # '0'
	sb t2, 0(a1)
	j .hexstr_addspace
.hexstr_hexchar2:
	addi t2, t2, -10
	addi t2, t2, 0x60 # 'a'
	sb t2, 0(a1)

.hexstr_addspace:
	li t2, 0x20 # ' '
	sb t2, 2(a1)

	addi a0, a0, 1
	addi a1, a1, 3
	j .hexstr_loop
.hexstr_end_loop:
	sb zero, 0(a1)
	ret

# void err(const char *msg1, const char *msg2) __attribute__((noreturn));
.type err, @function
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
	newline:	.string "\n"
	openerr:	.string "open() failed: "
	readerr:	.string "read() failed: "
	noargserr:	.string "xxd: must pass an argument to the program"
	emptystr:	.string ""

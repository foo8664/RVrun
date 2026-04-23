/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Common functions for RvRun's example programs
 *
 *  Copyright (C) 2026 by Diego Evaristo <di.diegoevaristo@gmail.com>
 */

.section .text
.global _start
.type _start, @function
_start:
	la gp, __global_pointer$
	ld a0, 0(sp)
	addi a1, sp, 8
	call main
	call exit

# ssize_t print(const char *str)
.type print, @function
.globl print
print:
	addi sp, sp, -16
	sd s0, 0(sp)
	sd ra, 8(sp)

	mv s0, a0
	call strlen
	mv a2, a0
	li a0, 1
	mv a1, s0
	call write

	ld s0, 0(sp)
	ld ra, 8(sp)
	addi sp, sp, 16
	ret

# void printnum(uint64_t num)
.type printnum, @function
.globl printnum
printnum:
	addi sp, sp, -64
	sd ra, 0(sp)

	addi a1, sp, 8
	li t0, 10
.printnum_loop:
	rem t1, a0, t0 # spec recommends div+rem (opposite order), but RvRun doesn't care.
	div a0, a0, t0
	addi t1, t1, 0x30 # '0'
	sb t1, 0(a1)
	addi a1, a1, 1

	bne a0, zero, .printnum_loop
.printnum_endloop:
	sb zero, 0(a1)
	addi a0, sp, 8
	addi a1, a1, -1
	call reverse
	addi a0, sp, 8
	call print

	ld ra, 0(sp)
	addi sp, sp, 64
	ret

# uint64_t getnum(void)
# Sets a1 to -1 or 1 according to the sign of the result
.type getnum, @function
.globl getnum
getnum:
	addi sp, sp, -32
	sd ra, 0(sp)
	sd s0, 8(sp)
	sd s1, 16(sp)
	sd s2, 24(sp)

	li s2, 1
	li s0, 0
	li s1, 10
	call getchar
	li t0, 0x2d # '-'
	bne a0, t0, .getnum_notneg
	li s2, -1
	j .getnum_loop
.getnum_notneg:
	addi s0, a0, -0x30 # -'0'
.getnum_loop:
	call getchar
	beq a0, s1, .getnum_endloop # s1 is also '\n'
	addi a0, a0, -0x30 # -'0'
	bge a0, s1, .getnum_endloop # if not a number
	blt a0, zero, .getnum_endloop # if not a number
	mul s0, s0, s1
	add s0, s0, a0
	j .getnum_loop
.getnum_endloop:
	mv a0, s0
	mv a1, s2
	ld ra, 0(sp)
	ld s0, 8(sp)
	ld s1, 16(sp)
	ld s2, 24(sp)
	addi sp, sp, 32
	ret

# float getfloat(void)
.type getfloat, @function
.globl getfloat
getfloat:
	addi sp, sp, -16
	fsw fs0, 0(sp)
	sd ra, 8(sp)

	call getnum
	fcvt.s.w fs0, a0
	fcvt.s.w ft0, a1
	fmul.s fs0, fs0, ft0

	li t0, 10
	li t1, 1
	li t2, 0
.getfloat_decimal_loop:
	call getchar
	beq a0, t0, .getfloat_decimal_endloop
	beq a0, zero, .getfloat_decimal_endloop
	addi a0, a0, -0x30 # - '0'
	bge a0, t0, .getfloat_decimal_endloop

	mul t2, t2, t0
	add t2, t2, a0
	mul t1, t1, t0
	j .getfloat_decimal_loop
.getfloat_decimal_endloop:

	fcvt.s.w ft0, t2
	fcvt.s.w ft1, t1
	fdiv.s ft0, ft0, ft1
	fabs.s ft1, fs0
	fadd.s fa0, ft1, ft0
	fsgnj.s fa0, fa0, fs0

	flw fs0, 0(sp)
	ld ra, 8(sp)
	addi sp, sp, 16
	ret

# void printfloat(float f)
.type printfloat, @function
.globl printfloat
printfloat:
	addi sp, sp, -16
	fsw fs0, 0(sp)
	sd ra, 8(sp)

	fmv.s fs0, fa0

	fcvt.s.w ft0, zero
	flt.s t0, fs0, ft0
	beq t0, zero, .printfloat_isnotneg
	li t0, 0x2d # '-'
	sb t0, 5(sp)
	sb zero, 6(sp)
	addi a0, sp, 5
	call print
	fneg.s fs0, fs0
.printfloat_isnotneg:

	fcvt.w.s a0, fs0
	call printnum

	li t0, 0x2e # '.'
	sb t0, 5(sp)
	sb zero, 6(sp)
	addi a0, sp, 5
	call print

	li t0, 10
	li t1, 0
	li t2, 5
	fcvt.s.w ft0, t0
	fcvt.w.s t3, fs0
	fcvt.s.w ft1, t3
	fsub.s ft1, fs0, ft1 # ft1 = decimal part of fs0
.printfloat_loop:
	fmul.s ft1, ft1, ft0
	fcvt.w.s a0, ft1
	remu a0, a0, t0
	addi a0, a0, 0x30 # '0'
	sb a0, 5(sp)

	li a0, 1
	addi a1, sp, 5
	li a2, 1
	li a7, 64
	ecall

	addi t1, t1, 1
	blt t1, t2, .printfloat_loop
.printfloat_endloop:

	flw fs0, 0(sp)
	ld ra, 8(sp)
	addi sp, sp, 16
	ret

# void reverse(char *beg, char *end);
.type reverse, @function
.globl reverse
reverse:
	bge a0, a1, .reverse_endloop

	lb t0, 0(a0)
	lb t1, 0(a1)
	sb t1, 0(a0)
	sb t0, 0(a1)

	addi a0, a0, 1
	addi a1, a1, -1

	j reverse
.reverse_endloop:
	ret

# Returns exits if EOF
.type getchar, @function
.globl getchar
getchar:
	addi sp, sp, -16
	sd ra, 0(sp)

	li a0, 0
	addi a1, sp, 8
	li a2, 1
	call read

	bge zero, a0, .getchar_failure
	lb a0, 8(sp)
	ld ra, 0(sp)
	addi sp, sp, 16
	ret
.getchar_failure:
	la a0, getchar_fail
	call print
	li a0, 1
	call exit


.type strlen, @function
.globl strlen
strlen:
	mv t0, a0
	li a0, 0
.strlen_loop:
	lb t1, 0(t0)
	beq t1, zero, .strlen_ret
	addi t0, t0, 1
	addi a0, a0, 1
	j .strlen_loop
.strlen_ret:
	ret

# float pow(float a, unsigned b): a^b
.type pow, @function
.globl pow
pow:
	li t0, 1
	fmv.s ft0, fa0
	fcvt.s.w fa0, t0
	beq a0, zero, .pow_endloop
.pow_loop:
	fmul.s fa0, fa0, ft0
	addi t0, t0, 1
	bge a0, t0, .pow_loop
.pow_endloop:
	ret

# unsigned fac(unsigned n): n!
.type fac, @function
.globl fac
fac:
	addi t0, a0, -1
	beq t0, zero, .fac_endloop
.fac_loop:
	mul a0, a0, t0
	addi t0, t0, -1
	blt zero, t0, .fac_loop
.fac_endloop:
	ret

.type openat, @function
.globl openat
openat:
	li a7, 56 # openat(2)
	ecall
	ret

.globl open
.type open, @function
open:
	mv a3, a2
	mv a2, a1
	mv a1, a0
	li a0, -100 # AT_FDCWD
	li a7, 56
	ecall
	ret

.type read, @function
.globl read
read:
	li a7, 63 # read(2)
	ecall
	ret

.type write, @function
.globl write
write:
	li a7, 64 # write(2)
	ecall
	ret

.type close, @function
.globl close
close:
	li a7, 57
	ecall
	ret

.type exit, @function
.globl exit
exit:
	li a7, 94 # exit_group(2)
	ecall

.section .rodata
	getchar_fail: .string "getchar() failed (might be EOF)\n"

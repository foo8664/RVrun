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
	beq a0, zero, .printnum_endloop
	rem t1, a0, t0 # spec recommends div+rem (opposite order), but RvRun doesn't care.
	div a0, a0, t0
	addi t1, t1, 0x30 # '0'
	sb t1, 0(a1)
	addi a1, a1, 1

	j .printnum_loop
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
.type getnum, @function
.globl getnum
getnum:
	addi sp, sp, -24
	sd ra, 0(sp)
	sd s0, 8(sp)
	sd s1, 16(sp)

	li s0, 0
	li s1, 10
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
	ld ra, 0(sp)
	ld s0, 8(sp)
	ld s1, 16(sp)
	addi sp, sp, 24
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

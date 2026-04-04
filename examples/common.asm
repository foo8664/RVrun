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
	sd s0, 8(sp)
	sd ra, 16(sp)

	mv s0, a0
	call strlen
	mv a2, a0
	li a0, 1
	mv a1, s0
	call write

	ld s0, 8(sp)
	ld ra, 16(sp)
	addi sp, sp, 16
	ret

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
	mv a1, a0
	mv a2, a1
	mv a3, a2
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

.type exit, @function
.globl exit
exit:
	li a7, 94 # exit_group(2)
	ecall

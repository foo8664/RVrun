.section .text
.globl main
.type main, @function
main:
	call getnum
	call fib
	call printnum
	la a0, newline
	call print
	li a0, 0
	call exit

.type fib, @function
fib:
	li t0, 0
	li t1, 1
	addi a0, a0, -1
.fib_loop:
	add t2, t0, t1
	mv t0, t1
	mv t1, t2
	addi a0, a0, -1
	bge a0, zero, .fib_loop
	mv a0, t0
	ret

.section .rodata
	newline: .string "\n"

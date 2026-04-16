
.section .text
.globl main
.type main, @function
main:
	call getnum
	mv s0, a0
	call getnum

	mv a1, a0
	mv a0, s0
	call gcd
	call printnum
	la a0, newline
	call print

	li a0, 0
	call exit

.type gcd, @function
gcd:
	beq a1, zero, .gcd_endloop
	rem a0, a0, a1
	mv t0, a0
	mv a0, a1
	mv a1, t0
	j gcd
.gcd_endloop:
	ret

.section .rodata
	newline: .string "\n"

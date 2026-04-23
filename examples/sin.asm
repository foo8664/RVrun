.section .text
.globl main
.type main, @function
main:
	addi sp, sp, -32
	sd ra, 0(sp)
	sd s0, 8(sp)
	fsw fs0, 16(sp)
	fsw fs1, 20(sp)
	fsw fs2, 24(sp)
	fsw fs3, 28(sp)

	call getfloat
	li s0, 1
	fmv.s fs0, fa0
	fcvt.s.w fs1, zero
	fcvt.s.w fs2, s0

# sin(x) = x - (x³/3!) + (x⁵/5!) - (x⁷/7!) + ...
.sin_loop:
	mv a0, s0
	fmv.s fa0, fs0
	call pow
	fmv.s fs3, fa0

	mv a0, s0
	call fac

	fcvt.s.w ft0, a0
	fdiv.s ft0, fs3, ft0
	fmul.s ft0, ft0, fs2
	fadd.s fs1, fs1, ft0

	fneg.s fs2, fs2
	addi s0, s0, 2

	li t0, 15
	blt s0, t0, .sin_loop

	la a0, msg_p1
	call print
	fmv.s fa0, fs0
	call printfloat
	la a0, msg_p2
	call print
	fmv.s fa0, fs1
	call printfloat
	la a0, msg_p3
	call print

	ld ra, 0(sp)
	ld s0, 8(sp)
	flw fs0, 16(sp)
	flw fs1, 20(sp)
	flw fs2, 24(sp)
	flw fs3, 28(sp)
	addi sp, sp, 32

	mv a0, zero
	ret

.section .rodata
	msg_p1: .string "sin("
	msg_p2: .string " radians) = "
	msg_p3: .string "\n"

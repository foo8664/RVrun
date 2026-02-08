#include <errno.h>
#include <stdint.h>
#include "riscv.h"
#include "proc.h"
#include "debug.h"
#include "rv_i.h"

static void R_getfields(insn_t insn, enum ABI_REG *rd, enum ABI_REG *rs1,
			enum ABI_REG *rs2)
	__attribute__((nonnull));


static void R_getfields(insn_t insn, enum ABI_REG *rd, enum ABI_REG *rs1,
			enum ABI_REG *rs2)
{
	*rd = (enum ABI_REG)((insn & 0xf80) >> 7);
	*rs1 = (enum ABI_REG)((insn & 0xf8000) >> 15);
	*rs2 = (enum ABI_REG)((insn & 0xf00000) >> 20);
}

int insn_add(struct proc *proc, insn_t insn)
{
	enum ABI_REG rs1;
	enum ABI_REG rs2;
	enum ABI_REG rd;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, getreg(proc, rs1) + getreg(proc, rs2));
	dbg_log("add: setting x%d to x%d + x%d = %ld", rd, rs1, rs2,
		getreg(proc, rd));
	return 0;
}

int insn_slt(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, (ireg_t)getreg(proc, rs1) < (ireg_t)getreg(proc, rs2));
	dbg_log("stl: Setting x%d to %ld: x%d < x%d?", rd, getreg(proc, rd),
		rs1, rs2);
	return 0;
}

int insn_sltu(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, (ureg_t)getreg(proc, rs1) < (ureg_t)getreg(proc, rs2));
	dbg_log("stlu: Setting x%d to %ld: x%d < x%d?", rd, getreg(proc, rd),
		rs1, rs2);
	return 0;
}

int insn_and(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, getreg(proc, rs1) & getreg(proc, rs2));
	dbg_log("and: Setting x%d = x%d & x%d = 0x%lx", rd, rs1, rs2,
		getreg(proc, rd));
	return 0;
}

int insn_or(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, getreg(proc, rs1) | getreg(proc, rs2));
	dbg_log("and: Setting x%d = x%d | x%d = 0x%lx", rd, rs1, rs2,
		getreg(proc, rd));
	return 0;
}

int insn_xor(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, getreg(proc, rs1) ^ getreg(proc, rs2));
	dbg_log("xor: Setting x%d = x%d ^ x%d = 0x%lx", rd, rs1, rs2,
		getreg(proc, rd));
	return 0;
}

int insn_sll(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, (reg_t)((ureg_t)getreg(proc, rs1) <<
				(ureg_t)(getreg(proc, rs2) & 0x3f)));
	dbg_log("sll: Setting x%d = x%d << x%d = 0x%lx", rd, rs1, rs2,
		getreg(proc, rd));
	return 0;
}

int insn_srl(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, (reg_t)((ureg_t)getreg(proc, rs1) >>
				(ureg_t)(getreg(proc, rs2) & 0x3f)));
	dbg_log("srl: Setting x%d = x%d >> x%d = 0x%lx", rd, rs1, rs2,
		getreg(proc, rd));
	return 0;
}

int insn_sra(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, (reg_t)((ireg_t)getreg(proc, rs1) >>
				(ireg_t)(getreg(proc, rs2) & 0x3f)));
	dbg_log("sra: Setting x%d = x%d >> x%d = 0x%lx", rd, rs1, rs2,
		getreg(proc, rd));
	return 0;
}

int insn_sub(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, getreg(proc, rs1) - getreg(proc, rs2));
	dbg_log("sub: Setting x%d = x%d - x%d = %ld", rd, rs1, rs2,
		getreg(proc, rd));
	return 0;
}

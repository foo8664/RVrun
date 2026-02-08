#include <errno.h>
#include <stdint.h>
#include "riscv.h"
#include "proc.h"
#include "debug.h"
#include "rv_i.h"

static void R_getfields(insn_t insn, enum ABI_REG *rd, enum ABI_REG *rs1,
			enum ABI_REG *rs2, uint8_t *funct7)
	__attribute__((nonnull));


static void R_getfields(insn_t insn, enum ABI_REG *rd, enum ABI_REG *rs1,
			enum ABI_REG *rs2, uint8_t *funct7)
{
	*rd = (enum ABI_REG)((insn & 0xf80) >> 7);
	*rs1 = (enum ABI_REG)((insn & 0xf8000) >> 15);
	*rs2 = (enum ABI_REG)((insn & 0xf00000) >> 20);
	*funct7 = (uint8_t)((insn & 0xff000000) >> 25);
}

int insn_add(struct proc *proc, insn_t insn)
{
	enum ABI_REG rs1;
	enum ABI_REG rs2;
	enum ABI_REG rd;
	uint8_t funct7;

	R_getfields(insn, &rd, &rs1, &rs2, &funct7);
	if (funct7 != 0) {
		err_log("add: invalid funct7 (0x%hhx)", funct7);
		errno = ENOSYS;
		return -1;
	}

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
	uint8_t funct7;

	R_getfields(insn, &rd, &rs1, &rs2, &funct7);
	if (funct7 != 0) {
		err_log("slt: invalid funct7 (0x%hhx)", funct7);
		errno = ENOSYS;
		return -1;
	}

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
	uint8_t funct7;

	R_getfields(insn, &rd, &rs1, &rs2, &funct7);
	if (funct7 != 0) {
		err_log("sltu: invalid funct7 (0x%hhx)", funct7);
		errno = ENOSYS;
		return -1;
	}

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
	uint8_t funct7;

	R_getfields(insn, &rd, &rs1, &rs2, &funct7);
	if (funct7 != 0) {
		err_log("and: invalid funct7 (0x%hhx)", funct7);
		errno = ENOSYS;
		return -1;
	}

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
	uint8_t funct7;

	R_getfields(insn, &rd, &rs1, &rs2, &funct7);
	if (funct7 != 0) {
		err_log("or: invalid funct7 (0x%hhx)", funct7);
		errno = ENOSYS;
		return -1;
	}

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
	uint8_t funct7;

	R_getfields(insn, &rd, &rs1, &rs2, &funct7);
	if (funct7 != 0) {
		err_log("xor: invalid funct7 (0x%hhx)", funct7);
		errno = ENOSYS;
		return -1;
	}

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
	uint8_t funct7;

	R_getfields(insn, &rd, &rs1, &rs2, &funct7);
	if (funct7 != 0) {
		err_log("sll: invalid funct7 (0x%hhx)", funct7);
		errno = ENOSYS;
		return -1;
	}

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
	uint8_t funct7;

	R_getfields(insn, &rd, &rs1, &rs2, &funct7);
	if (funct7 != 0) {
		err_log("srl: invalid funct7 (0x%hhx)", funct7);
		errno = ENOSYS;
		return -1;
	}

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
	uint8_t funct7;

	R_getfields(insn, &rd, &rs1, &rs2, &funct7);
	if (funct7 != 0x20) {
		err_log("sra: invalid funct7 (0x%hhx)", funct7);
		errno = ENOSYS;
		return -1;
	}

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
	uint8_t funct7;

	R_getfields(insn, &rd, &rs1, &rs2, &funct7);
	if (funct7 != 0x20) {
		err_log("sub: invalid funct7 (0x%hhx)", funct7);
		errno = ENOSYS;
		return -1;
	}

	mvreg(proc, rd, getreg(proc, rs1) - getreg(proc, rs2));
	dbg_log("sub: Setting x%d = x%d - x%d = %ld", rd, rs1, rs2,
		getreg(proc, rd));
	return 0;
}

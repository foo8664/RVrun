/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Rv64I instructions
 *
 *  Copyright (C) 2026 by Diego Oliveira Evaristo <di.diegoevaristo@gmail.com>
 */

#include <errno.h>
#include <stdint.h>
#include "riscv.h"
#include "proc.h"
#include "debug.h"
#include "rv_i.h"

// For ecall
#include "syscall.h"
#include "sysnums.h"

static inline void R_getfields(insn_t insn, enum ABI_REG *rd, enum ABI_REG *rs1,
			       enum ABI_REG *rs2) __attribute__((nonnull));
static inline void I_getfields(insn_t insn, enum ABI_REG *rd, enum ABI_REG *rs1,
			       uint64_t *imm) __attribute__((nonnull));
static inline void U_getfields(insn_t insn, enum ABI_REG *rd, uint64_t *imm)
			       __attribute__((nonnull));
static inline uint64_t extend64(uint32_t i32) __attribute__((nonnull));


static inline void R_getfields(insn_t insn, enum ABI_REG *rd, enum ABI_REG *rs1,
			       enum ABI_REG *rs2)
{
	*rd =  (enum ABI_REG)((insn & 0xf80)	>> 7);
	*rs1 = (enum ABI_REG)((insn & 0xf8000)	>> 15);
	*rs2 = (enum ABI_REG)((insn & 0xf00000)	>> 20);
}

static inline void I_getfields(insn_t insn, enum ABI_REG *rd, enum ABI_REG *rs1,
			       uint64_t *imm)
{
	*rd =  (enum ABI_REG)((insn & 0xf80)	>> 7);
	*rs1 = (enum ABI_REG)((insn & 0xf8000)	>> 15);
	*imm =  (uint64_t)((insn & 0xfff00000)	>> 20);
	*imm |= (uint64_t)((insn & 0x80000000)  ? 0xfffffffffffff000lu : 0lu);
}

static inline void U_getfields(insn_t insn, enum ABI_REG *rd, uint64_t *imm)
{
	*rd =	(enum ABI_REG)((insn & 0xf80) >> 7);
	*imm =	(uint64_t)(insn & 0xfffff000);
	*imm |= (uint64_t)((insn & 0x80000000) ? 0xffffffff00000000lu : 0lu);
}

static inline uint64_t extend64(uint32_t i32)
{
	return i32 & 0x80000000 ? (uint64_t)i32 | (~0lu << 32) : i32;
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

int insn_addw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, extend64((uint32_t)(getreg(proc, rs1) +
					    getreg(proc, rs2))));
	dbg_log("addw: setting x%d = x%d + x%d", rd, rs1, rs2);
	return 0;
}

int insn_subw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, extend64((uint32_t)(getreg(proc, rs1) -
					    getreg(proc, rs2))));
	dbg_log("addw: setting x%d = x%d - x%d", rd, rs1, rs2);
	return 0;
}

int insn_sllw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, extend64((uint32_t)((ureg_t) getreg(proc, rs1) <<
					    (ureg_t)(getreg(proc, rs2) & 0x1f))));
	dbg_log("addw: setting x%d = x%d << x%d", rd, rs1, rs2);
	return 0;
}

int insn_srlw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, extend64((uint32_t)((ureg_t) getreg(proc, rs1) >>
					    (ureg_t)(getreg(proc, rs2) & 0x1f))));
	dbg_log("addw: setting x%d = x%d >> x%d", rd, rs1, rs2);
	return 0;
}

int insn_sraw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, extend64((uint32_t)((ireg_t) getreg(proc, rs1) >>
					    (ireg_t)(getreg(proc, rs2) & 0x1f))));
	dbg_log("sraw: setting x%d = x%d >> x%d", rd, rs1, rs2);
	return 0;
}

int insn_andi(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)(getreg(proc, rs1) & imm));
	dbg_log("andi: Setting x%d = x%d & 0x%lx", rd, rs1, imm);
	return 0;
}

int insn_ori(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)(getreg(proc, rs1) | imm));
	dbg_log("ori: Setting x%d = x%d | 0x%lx", rd, rs1, imm);
	return 0;
}

int insn_xori(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)(getreg(proc, rs1) ^ imm));
	dbg_log("xori: Setting x%d = x%d ^ 0x%lx", rd, rs1, imm);
	return 0;
}

int insn_addi(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)((uint64_t)getreg(proc, rs1) + imm));
	dbg_log("addi: Setting x%d = x%d + %ld", rd, rs1, (int64_t)imm);
	return 0;
}

int insn_slli(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)((uint64_t)getreg(proc, rs1) << (imm & 0x3f)));
	dbg_log("slli: Setting x%d = x%d << %d", rd, rs1, (int)(imm & 0x3f));
	return 0;
}

int insn_srli(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)((uint64_t)getreg(proc, rs1) >> (imm & 0x3f)));
	dbg_log("srli: Setting x%d = x%d >> %d", rd, rs1, (int)(imm & 0x3f));
	return 0;
}

int insn_srai(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)((int64_t)getreg(proc, rs1) >>
				(int64_t)(imm & 0x3f)));
	dbg_log("srai: Setting x%d = x%d >> %d", rd, rs1, (int)(imm & 0x3f));
	return 0;
}

int insn_slti(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)((ireg_t)getreg(proc, rs1) < (ireg_t)imm));
	dbg_log("slti: Setting x%d = x%d < %ld", rd, rs1, (int64_t)imm);
	return 0;
}

int insn_sltiu(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)((ureg_t)getreg(proc, rs1) < (ureg_t)imm));
	dbg_log("sltiu: Setting x%d = x%d < %lu", rd, rs1, imm);
	return 0;
}

int insn_addiw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)extend64((uint32_t)(getreg(proc, rd) + imm)));
	dbg_log("addiw: x%d = x%d + %d", rd, rs1, (int32_t)imm);
	return 0;
}

int insn_slliw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)extend64((uint32_t)(getreg(proc, rd) <<
						   (imm & 0x1f))));
	dbg_log("slliw: x%d = x%d << %u", rd, rs1, (unsigned)(imm & 0x1f));
	return 0;
}

int insn_srliw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)extend64((uint32_t)(getreg(proc, rd) >>
						   (imm & 0x1f))));
	dbg_log("srliw: x%d = x%d >> %u", rd, rs1, (unsigned)(imm & 0x1f));
	return 0;
}

int insn_sraiw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)extend64((uint32_t)((int32_t)(getreg(proc, rd)
						  >> (int32_t)(imm & 0x1f)))));
	dbg_log("sraiw: x%d = x%d >> %u", rd, rs1, (unsigned)(imm & 0x1f));
	return 0;
}

int insn_lui(struct proc *proc, insn_t insn)
{
	uint64_t imm;
	enum ABI_REG rd;

	U_getfields(insn, &rd, &imm);
	mvreg(proc, rd, (reg_t)imm);
	dbg_log("lui: Setting x%d = %ld", rd, imm);
	return 0;
}

int insn_auipc(struct proc *proc, insn_t insn)
{
	uint64_t imm;
	enum ABI_REG rd;

	U_getfields(insn, &rd, &imm);
	mvreg(proc, rd, (reg_t)(proc->pc + imm));
	dbg_log("auipc: x%d = 0x%lx + 0x%lx", rd, proc->pc, imm);
	return 0;
}

// Ignores rd and rs1, only implements linux system calls
int insn_ecall(struct proc *proc, insn_t insn)
{
	int sysnum;
	(void)insn;

	sysnum = (int)getreg(proc, REG_A7);

	dbg_log("ecall: syscall %d", sysnum);

#define ADD_SYSCALL(sc) case __NR_##sc: return rvsys_##sc(proc)
	switch (sysnum) {
		ADD_SYSCALL(exit);
		ADD_SYSCALL(exit_group);
		ADD_SYSCALL(write);
		ADD_SYSCALL(read);
		ADD_SYSCALL(openat);
		ADD_SYSCALL(close);
	}
#undef ADD_SYSCALL

	errno = ENOSYS;
	return -1;
}

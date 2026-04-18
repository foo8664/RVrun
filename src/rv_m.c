/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Rv64M Instructions
 *
 *  Copyright (C) 2026 by Diego Evaristo <di.diegoevaristo@gmail.com>
 */

#include <stdint.h>
#include "debug.h"
#include "riscv.h"
#include "proc.h"
#include "rv_m.h"


static inline void getfields(insn_t insn, enum ABI_REG *rd, enum ABI_REG *rs1, enum ABI_REG *rs2)
{
	*rd = (enum ABI_REG)((insn & 0xf80) >> 7);
	*rs1 = (enum ABI_REG)((insn & 0xf8000) >> 15);
	*rs2 = (enum ABI_REG)((insn & 0x1f00000) >> 20);
}

static inline int64_t imulh(int64_t a, int64_t b)
{
	int64_t c;
	__asm__("movq %1, %%rax;\n\t"
		"movq %2, %%rdx;\n\t"
		"imulq %%rdx;\n\t"
		"movq %%rdx, %0"
		: "=r"(c)
		: "r"(a), "r"(b)
		: "%rax", "%rdx"
	);

	return c;
}

static inline uint64_t umulh(uint64_t a, uint64_t b)
{
	uint64_t c;
	__asm__("movq %1, %%rax;\n\t"
		"movq %2, %%rdx;\n\t"
		"mulq %%rdx;\n\t"
		"movq %%rdx, %0"
		: "=r"(c)
		: "r"(a), "r"(b)
		: "%rax", "%rdx"
	);

	return c;
}

int insn_mul(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, (reg_t)((int64_t)getreg(proc, rs1) * (int64_t)getreg(proc, rs2)));

	return 0;
}

int insn_mulw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, (reg_t)extend32to64((uint32_t)((int32_t)getreg(proc, rs1) *
						       (int32_t)getreg(proc, rs2))));

	return 0;
}

int insn_mulh(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, (reg_t)(imulh((int64_t)getreg(proc, rs1),
				      (int64_t)getreg(proc, rs2))));
	return 0;
}

int insn_mulhu(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, (reg_t)(umulh((uint64_t)getreg(proc, rs1),
				      (uint64_t)getreg(proc, rs2))));

	return 0;
}

int insn_mulhsu(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;
	uint64_t c;

	getfields(insn, &rd, &rs1, &rs2);
	if ((int64_t)getreg(proc, rs1) < 0) {
		c = -(umulh(-(uint64_t)getreg(proc, rs1), (uint64_t)getreg(proc, rs2)));
		mvreg(proc, rd, (reg_t)(c ? c : ~0lu));
	} else {
		mvreg(proc, rd, (reg_t)(umulh((uint64_t)getreg(proc, rs1),
					      (uint64_t)getreg(proc, rs2))));
	}

	return 0;
}

int insn_div(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	getfields(insn, &rd, &rs1, &rs2);
	if (UNLIKELY(getreg(proc, rs2) == 0)) {
		mvreg(proc, rd, (reg_t)-1);
		warn_log("0x%lx: div: Division by zero (%s = %ld)", proc->pc,
			 reg2abi(rd), getreg(proc, rd));
		return 0;
	} else if (UNLIKELY(getreg(proc, rs1) == (reg_t)-(1llu << (XLEN - 1))
			 && getreg(proc, rs2) == (reg_t)-1)) {
		mvreg(proc, rd, (reg_t)-(1llu << (XLEN - 1)));
		warn_log("0x%lx: div: Division overflew (%s = %lx)", proc->pc,
			 reg2abi(rd), getreg(proc, rd));
		return 0;
	}

	mvreg(proc, rd, (reg_t)((int64_t)getreg(proc, rs1) / (int64_t)getreg(proc, rs2)));
	return 0;
}

int insn_divu(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	getfields(insn, &rd, &rs1, &rs2);
	if (UNLIKELY(getreg(proc, rs2) == 0)) {
		mvreg(proc, rd, (reg_t)(~0llu));
		warn_log("0x%lx: divu: Division by zero (%s = %ld)", proc->pc,
			 reg2abi(rd), getreg(proc, rd));
		return 0;
	}

	mvreg(proc, rd, (reg_t)((uint64_t)getreg(proc, rs1) / (uint64_t)getreg(proc, rs2)));
	return 0;
}

int insn_divw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	getfields(insn, &rd, &rs1, &rs2);
	if (UNLIKELY(getreg(proc, rs2) == 0)) {
		mvreg(proc, rd, (reg_t)-1);
		warn_log("0x%lx: divw: Division by zero (%s = %ld)", proc->pc,
			 reg2abi(rd), getreg(proc, rd));
		return 0;
	} else if (UNLIKELY(getreg(proc, rs1) == (reg_t)-(1llu << (32 - 1)) &&
			    getreg(proc, rs2) == (reg_t)-1)) {
		mvreg(proc, rd, 0xffffffff80000000llu);
		warn_log("0x%lx: divw: Division overflew (%s = %lx)", proc->pc,
			 reg2abi(rd), getreg(proc, rd));
		return 0;
	}

	mvreg(proc, rd, extend32to64((uint32_t)((int32_t)getreg(proc, rs1) /
						(int32_t)getreg(proc, rs2))));
	return 0;
}

int insn_divuw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	getfields(insn, &rd, &rs1, &rs2);
	if (UNLIKELY(getreg(proc, rs2) == 0)) {
		mvreg(proc, rd, (reg_t)(~0llu));
		warn_log("0x%lx: divuw: Division by zero (%s = %ld)", proc->pc,
			 reg2abi(rd), getreg(proc, rd));
		return 0;
	}

	mvreg(proc, rd, extend32to64((uint32_t)((uint32_t)getreg(proc, rs1) /
						 (uint32_t)getreg(proc, rs2))));
	return 0;
}

int insn_rem(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	getfields(insn, &rd, &rs1, &rs2);
	if (UNLIKELY(getreg(proc, rs2) == 0)) {
		mvreg(proc, rd, getreg(proc, rs1));
		warn_log("0x%lx: rem: Division by zero (%s = %ld)", proc->pc,
			 reg2abi(rd), getreg(proc, rd));
		return 0;
	} else if (UNLIKELY(getreg(proc, rs1) == (reg_t)-(1llu << (XLEN - 1)) &&
			    getreg(proc, rs2) == (reg_t)-1)) {
		mvreg(proc, rd, 0);
		warn_log("0x%lx: rem: Division overflew (%s = %ld)", proc->pc,
			 reg2abi(rd), getreg(proc, rd));
		return 0;
	}

	mvreg(proc, rd, (reg_t)((int64_t)getreg(proc, rs1) % (int64_t)getreg(proc, rs2)));
	return 0;
}

int insn_remu(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	getfields(insn, &rd, &rs1, &rs2);
	if (UNLIKELY(getreg(proc, rs2) == 0)) {
		mvreg(proc, rd, getreg(proc, rs1));
		warn_log("0x%lx: remu: Division by zero (%s = %ld)", proc->pc,
			 reg2abi(rd), getreg(proc, rd));
		return 0;
	}

	mvreg(proc, rd, (reg_t)((uint64_t)getreg(proc, rs1) % (uint64_t)getreg(proc, rs2)));
	return 0;
}

int insn_remw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	getfields(insn, &rd, &rs1, &rs2);
	if (UNLIKELY(getreg(proc, rs2) == 0)) {
		mvreg(proc, rd, extend32to64((uint32_t)getreg(proc, rs1)));
		warn_log("0x%lx: remw: Division by zero (%s = %ld)", proc->pc,
			 reg2abi(rd), getreg(proc, rd));
		return 0;
	} else if (UNLIKELY(getreg(proc, rs1) == (reg_t)-(1llu << (32 - 1)) &&
			    getreg(proc, rs2) == (reg_t)-1)) {
		mvreg(proc, rd, 0);
		warn_log("0x%lx: remw: Division overflew (%s = %ld)", proc->pc,
			 reg2abi(rd), getreg(proc, rd));
		return 0;
	}

	mvreg(proc, rd, extend32to64((uint32_t)((int32_t)getreg(proc, rs1) %
						(int32_t)getreg(proc, rs2))));
	return 0;
}

int insn_remuw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	getfields(insn, &rd, &rs1, &rs2);
	if (UNLIKELY(getreg(proc, rs2) == 0)) {
		mvreg(proc, rd, extend32to64((uint32_t)getreg(proc, rs1)));
		warn_log("0x%lx: remwu: Division by zero (%s = %ld)", proc->pc,
			 reg2abi(rd), getreg(proc, rd));
		return 0;
	}

	mvreg(proc, rd, zextend32to64((uint32_t)((uint32_t)getreg(proc, rs1) %
						 (uint32_t)getreg(proc, rs2))));
	return 0;
}

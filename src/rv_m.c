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


#ifdef HAVE_RV64M
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
		"movq %2, %%rsi;\n\t"
		"imulq %%rsi;\n\t"
		"movq %%rdx, %0"
		: "=r"(c)
		: "r"(a), "r"(b)
		: "%rax", "%rsi", "%rdx"
	);

	return c;
}

static inline uint64_t umulh(uint64_t a, uint64_t b)
{
	uint64_t c;
	__asm__("movq %1, %%rax;\n\t"
		"movq %2, %%rsi;\n\t"
		"mulq %%rsi;\n\t"
		"movq %%rdx, %0"
		: "=r"(c)
		: "r"(a), "r"(b)
		: "%rax", "%rsi", "%rdx"
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

	dbg_log("0x%lx: mul: %s (%ld) = %s (%ld) * %s (%ld)", proc->pc, reg2abi(rd),
		getreg(proc, rd), reg2abi(rs1), getreg(proc, rs1), reg2abi(rs2),
		getreg(proc, rs2));
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

	dbg_log("0x%lx: mulw: %s (%ld) = (%s (%ld) * %s (%ld))", proc->pc,
		reg2abi(rd), getreg(proc, rd), reg2abi(rs1), getreg(proc, rs1),
		reg2abi(rs2), getreg(proc, rs2));
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
	dbg_log("0x%lx: mulh: %s (%ld) = %s (%ld) * %s (%ld)", proc->pc,
		reg2abi(rd), getreg(proc, rd), reg2abi(rs1), getreg(proc, rs1),
		reg2abi(rs2), getreg(proc, rs2));
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

	dbg_log("0x%lx: mulhu: %s (%lu) = %s (%lu) * %s (%lu)", proc->pc,
		reg2abi(rd), getreg(proc, rd), reg2abi(rs1), getreg(proc, rs1),
		reg2abi(rs2), getreg(proc, rs2));
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
		c = -(umulh((uint64_t)-getreg(proc, rs1), (uint64_t)getreg(proc, rs2)));
		mvreg(proc, rd, (reg_t)(c ? c : ~0lu));
	} else {
		mvreg(proc, rd, (reg_t)(umulh((uint64_t)getreg(proc, rs1),
					      (uint64_t)getreg(proc, rs2))));
	}

	dbg_log("0x%lx: mulhsu: %s (%ld) = %s (%ld) * %s (%lu)", proc->pc,
		reg2abi(rd), getreg(proc, rd), reg2abi(rs1), getreg(proc, rs1),
		reg2abi(rs2), getreg(proc, rs2));
	return 0;
}
#endif // HAVE_RV64M

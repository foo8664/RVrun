/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Rv64I instructions
 *
 *  Copyright (C) 2026 by Diego Evaristo <di.diegoevaristo@gmail.com>
 */

#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "riscv.h"
#include "proc.h"
#include "debug.h"
#include "common.h"
#include "rv_i.h"

#include "memory.h"

// For ecall
#include "syscall.h"
#include "sysnums.h"

#ifdef HAVE_RV64I

// Get info from OP-Code fields
static inline void R_getfields(insn_t insn, enum ABI_REG *rd, enum ABI_REG *rs1,
			       enum ABI_REG *rs2) ATTRIBUTE(nonnull);
static inline void I_getfields(insn_t insn, enum ABI_REG *rd, enum ABI_REG *rs1,
			       uint64_t *imm) ATTRIBUTE(nonnull);
static inline void U_getfields(insn_t insn, enum ABI_REG *rd, uint64_t *imm)
				ATTRIBUTE(nonnull);
static inline void J_getfields(insn_t insn, enum ABI_REG *rd, uint64_t *imm)
				ATTRIBUTE(nonnull);
static inline void B_getfields(insn_t insn, enum ABI_REG *rs1, enum ABI_REG *rs2,
			       uint64_t *imm) ATTRIBUTE(nonnull);
static inline void S_getfields(insn_t insn, enum ABI_REG *rs1, enum ABI_REG *rs2,
		               uint64_t *imm) ATTRIBUTE(nonnull);


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

static inline void J_getfields(insn_t insn, enum ABI_REG *rd, uint64_t *imm)
{
	*rd = (enum ABI_REG)((insn & 0xf80)	>> 7);
	*imm =  (uint64_t)((insn & 0xffc00000)	>> 20);
	*imm |= (uint64_t)((insn & 0x100000)	>> 9);
	*imm |= (uint64_t)((insn & 0xff000)	>> 0);
	*imm |= (uint64_t)((insn & (1u << 31))	>> 11);
	*imm |= (uint64_t)((insn & (1u << 31)) ? (0xffffffffffflu << 20) : 0lu);
}

static inline void B_getfields(insn_t insn, enum ABI_REG *rs1, enum ABI_REG *rs2,
			       uint64_t *imm)
{
	*rs1 = (enum ABI_REG)((insn & 0xf8000) >> 15);
	*rs2 = (enum ABI_REG)((insn & 0x1f00000) >> 20);
	*imm =  (uint64_t)((insn & (0xf  << 8))  >> 7);
	*imm |= (uint64_t)((insn & (0x3f << 25)) >> 20);
	*imm |= (uint64_t)((insn & (0x1  << 7))  << 4);
	*imm |= (uint64_t)((insn & (0x1u << 31u)) ? (~0lu << 12) : 0lu);
}

static inline void S_getfields(insn_t insn, enum ABI_REG *rs1, enum ABI_REG *rs2,
		               uint64_t *imm)
{
	*rs1 = (enum ABI_REG)((insn & 0xf8000)	>> 15);
	*rs2 = (enum ABI_REG)((insn & 0xf00000)	>> 20);
	*imm =  (uint64_t)((insn & 0xf80)	>> 7);
	*imm |= (uint64_t)((insn & 0xfe000000)	>> 20);
	*imm |= (uint64_t)((insn & (0x1u << 31u)) ? (~0lu << 12) : 0lu);
}


int insn_add(struct proc *proc, insn_t insn)
{
	enum ABI_REG rs1;
	enum ABI_REG rs2;
	enum ABI_REG rd;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, getreg(proc, rs1) + getreg(proc, rs2));
	dbg_log("0x%lx: add: setting %s to %s + %s = %ld", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), reg2abi(rs2), getreg(proc, rd));
	return 0;
}

int insn_slt(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, (ireg_t)getreg(proc, rs1) < (ireg_t)getreg(proc, rs2));
	dbg_log("0x%lx: stl: Setting %s to %ld: %s < %s?", (unsigned long)proc->pc,
		reg2abi(rd), getreg(proc, rd), reg2abi(rs1), reg2abi(rs2));
	return 0;
}

int insn_sltu(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, (ureg_t)getreg(proc, rs1) < (ureg_t)getreg(proc, rs2));
	dbg_log("0x%lx: stlu: Setting %s to %ld: %s < %s?", (unsigned long)proc->pc,
		reg2abi(rd), getreg(proc, rd), reg2abi(rs1), reg2abi(rs2));
	return 0;
}

int insn_and(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, getreg(proc, rs1) & getreg(proc, rs2));
	dbg_log("0x%lx: and: Setting %s = %s & %s = 0x%lx", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), reg2abi(rs2), getreg(proc, rd));
	return 0;
}

int insn_or(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, getreg(proc, rs1) | getreg(proc, rs2));
	dbg_log("0x%lx: and: Setting %s = %s | %s = 0x%lx", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), reg2abi(rs2), getreg(proc, rd));
	return 0;
}

int insn_xor(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, getreg(proc, rs1) ^ getreg(proc, rs2));
	dbg_log("0x%lx: xor: Setting %s = %s ^ %s = 0x%lx", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), reg2abi(rs2), getreg(proc, rd));
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
	dbg_log("0x%lx: sll: Setting %s = x%s << %s = 0x%lx", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), reg2abi(rs2), getreg(proc, rd));
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
	dbg_log("0x%lx: srl: Setting %s = %s >> %s = 0x%lx", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), reg2abi(rs2), getreg(proc, rd));
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
	dbg_log("0x%lx: sra: Setting %s = %s >> %s = 0x%lx", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), reg2abi(rs2), getreg(proc, rd));
	return 0;
}

int insn_sub(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, getreg(proc, rs1) - getreg(proc, rs2));
	dbg_log("0x%lx: sub: Setting %s = %s - %s = %ld", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), reg2abi(rs2), getreg(proc, rd));
	return 0;
}

int insn_addw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, extend32to64((uint32_t)(getreg(proc, rs1) +
					    getreg(proc, rs2))));
	dbg_log("0x%lx: addw: setting %s = %s + %s", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), reg2abi(rs2));
	return 0;
}

int insn_subw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, extend32to64((uint32_t)(getreg(proc, rs1) -
					    getreg(proc, rs2))));
	dbg_log("0x%lx: addw: setting %s = %s - %s", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), reg2abi(rs2));
	return 0;
}

int insn_sllw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, extend32to64((uint32_t)((ureg_t) getreg(proc, rs1) <<
					    (ureg_t)(getreg(proc, rs2) & 0x1f))));
	dbg_log("0x%lx: addw: setting %s = %s << %s", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), reg2abi(rs2));
	return 0;
}

int insn_srlw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, extend32to64((uint32_t)((ureg_t) getreg(proc, rs1) >>
					    (ureg_t)(getreg(proc, rs2) & 0x1f))));
	dbg_log("0x%lx: addw: setting %s = %s >> %s", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), reg2abi(rs2));
	return 0;
}

int insn_sraw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, extend32to64((uint32_t)((ireg_t) getreg(proc, rs1) >>
					    (ireg_t)(getreg(proc, rs2) & 0x1f))));
	dbg_log("0x%lx: sraw: setting %s = %s >> %s", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), reg2abi(rs2));
	return 0;
}

int insn_andi(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)(getreg(proc, rs1) & imm));
	dbg_log("0x%lx: andi: Setting %s = %s & 0x%lx", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), imm);
	return 0;
}

int insn_ori(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)(getreg(proc, rs1) | imm));
	dbg_log("0x%lx: ori: Setting %s = %s | 0x%lx", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), imm);
	return 0;
}

int insn_xori(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)(getreg(proc, rs1) ^ imm));
	dbg_log("0x%lx: xori: Setting %s = %s ^ 0x%lx", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), imm);
	return 0;
}

int insn_addi(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)((uint64_t)getreg(proc, rs1) + imm));
	dbg_log("0x%lx: addi: Setting %s = %s + %ld", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), (int64_t)imm);
	return 0;
}

int insn_slli(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)((uint64_t)getreg(proc, rs1) << (imm & 0x3f)));
	dbg_log("0x%lx: slli: Setting %s = %s << %d", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), (int)(imm & 0x3f));
	return 0;
}

int insn_srli(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)((uint64_t)getreg(proc, rs1) >> (imm & 0x3f)));
	dbg_log("0x%lx: srli: Setting %s = %s >> %d", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), (int)(imm & 0x3f));
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
	dbg_log("0x%lx: srai: Setting %s = %s >> %d", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), (int)(imm & 0x3f));
	return 0;
}

int insn_slti(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)((ireg_t)getreg(proc, rs1) < (ireg_t)imm));
	dbg_log("0x%lx: slti: Setting %s = %s < %ld", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), (int64_t)imm);
	return 0;
}

int insn_sltiu(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)((ureg_t)getreg(proc, rs1) < (ureg_t)imm));
	dbg_log("0x%lx: sltiu: Setting %s = %s < %lu", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), imm);
	return 0;
}

int insn_addiw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)extend32to64((uint32_t)(getreg(proc, rd) + imm)));
	dbg_log("0x%lx: addiw: %s = %s + %d", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), (int32_t)imm);
	return 0;
}

int insn_slliw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)extend32to64((uint32_t)(getreg(proc, rd) <<
						   (imm & 0x1f))));
	dbg_log("0x%lx: slliw: %s = %s << %u", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), (unsigned)(imm & 0x1f));
	return 0;
}

int insn_srliw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)extend32to64((uint32_t)(getreg(proc, rd) >>
						   (imm & 0x1f))));
	dbg_log("0x%lx: srliw: %s = %s >> %u", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), (unsigned)(imm & 0x1f));
	return 0;
}

int insn_sraiw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)extend32to64((uint32_t)((int32_t)(getreg(proc, rd)
						  >> (int32_t)(imm & 0x1f)))));
	dbg_log("0x%lx: sraiw: %s = %s >> %u", (unsigned long)proc->pc,
		reg2abi(rd), reg2abi(rs1), (unsigned)(imm & 0x1f));
	return 0;
}

int insn_lui(struct proc *proc, insn_t insn)
{
	uint64_t imm;
	enum ABI_REG rd;

	U_getfields(insn, &rd, &imm);
	mvreg(proc, rd, (reg_t)imm);
	dbg_log("0x%lx: lui: Setting %s = %ld", (unsigned long)proc->pc,
		reg2abi(rd), imm);
	return 0;
}

int insn_auipc(struct proc *proc, insn_t insn)
{
	uint64_t imm;
	enum ABI_REG rd;

	U_getfields(insn, &rd, &imm);
	mvreg(proc, rd, (reg_t)(proc->pc + imm));
	dbg_log("0x%lx: auipc: %s = 0x%lx + 0x%lx", (unsigned long)proc->pc,
		reg2abi(rd), proc->pc, imm);
	return 0;
}

int insn_jal(struct proc *proc, insn_t insn)
{
	uint64_t imm;
	enum ABI_REG rd;

	J_getfields(insn, &rd, &imm);
	mvreg(proc, rd, (reg_t)(proc->pc + 4));
	dbg_log("0x%lx: jal: %s = 0x%lx, pc += 0x%lx", (unsigned long)proc->pc,
		reg2abi(rd), getreg(proc, rd), imm);
	proc->pc += imm - 4; // Cycle always adds size of current instruction
	if (UNLIKELY(proc->pc % IALIGN != 0)) {
		err_log("pc (0x%lx) is not bit-aligned to IALIGN (%u)",
			proc->pc, IALIGN * 8);
		panic("Instruction-address-misaligned exception");
	}
	return 0;
}

int insn_jalr(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)(proc->pc + 4));
	dbg_log("0x%lx: jalr: %s = 0x%lx. pc = %s (0x%lx) + 0x%lx", (unsigned long)proc->pc,
		reg2abi(rd), getreg(proc, rd), reg2abi(rs1), getreg(proc, rs1), imm);

	// Cycle always adds size of current instruction
	proc->pc = (rvaddr_t)(getreg(proc, rs1) + imm - 4);
	if (UNLIKELY(proc->pc % IALIGN != 0)) {
		err_log("pc (0x%lx) is not bit-aligned to IALIGN (%u)",
			proc->pc, IALIGN * 8);
		panic("Instruction-address-misaligned exception");
	}
	return 0;
}

int insn_beq(struct proc *proc, insn_t insn)
{
	enum ABI_REG rs1;
	enum ABI_REG rs2;
	uint64_t imm;

	B_getfields(insn, &rs1, &rs2, &imm);
	dbg_log("beq: Comparing %s (0x%lx) to %s (0x%lx), pc: 0x%lx, offset: 0x%lx",
		reg2abi(rs1), getreg(proc, rs1), reg2abi(rs2), getreg(proc, rs2),
		proc->pc, imm);

	if (getreg(proc, rs1) == getreg(proc, rs2)) {
		// Cycle always adds size of current instruction
		proc->pc += imm - 4;
		if (UNLIKELY(proc->pc % IALIGN != 0)) {
			err_log("pc (0x%lx) is not bit-aligned to IALIGN (%u)",
				proc->pc, IALIGN * 8);
			panic("Instruction-address-misaligned exception");
		}
	}

	return 0;
}

int insn_bne(struct proc *proc, insn_t insn)
{
	enum ABI_REG rs1;
	enum ABI_REG rs2;
	uint64_t imm;

	B_getfields(insn, &rs1, &rs2, &imm);
	dbg_log("bne: Comparing %s (0x%lx) to %s (0x%lx), pc: 0x%lx, offset: 0x%lx",
		reg2abi(rs1), getreg(proc, rs1), reg2abi(rs2), getreg(proc, rs2),
		proc->pc, imm);

	if (getreg(proc, rs1) != getreg(proc, rs2)) {
		// Cycle always adds size of current instruction
		proc->pc += imm - 4;
		if (UNLIKELY(proc->pc % IALIGN != 0)) {
			err_log("pc (0x%lx) is not bit-aligned to IALIGN (%u)",
				proc->pc, IALIGN * 8);
			panic("Instruction-address-misaligned exception");
		}
	}

	return 0;
}

int insn_blt(struct proc *proc, insn_t insn)
{
	enum ABI_REG rs1;
	enum ABI_REG rs2;
	uint64_t imm;

	B_getfields(insn, &rs1, &rs2, &imm);
	dbg_log("blt: Comparing %s (0x%lx) to %s (0x%lx), pc: 0x%lx, offset: 0x%lx",
		reg2abi(rs1), getreg(proc, rs1), reg2abi(rs2), getreg(proc, rs2),
		proc->pc, imm);

	if ((ireg_t)getreg(proc, rs1) < (ireg_t)getreg(proc, rs2)) {
		// Cycle always adds size of current instruction
		proc->pc += imm - 4;
		if (UNLIKELY(proc->pc % IALIGN != 0)) {
			err_log("pc (0x%lx) is not bit-aligned to IALIGN (%u)",
				proc->pc, IALIGN * 8);
			panic("Instruction-address-misaligned exception");
		}
	}

	return 0;
}

int insn_bltu(struct proc *proc, insn_t insn)
{
	enum ABI_REG rs1;
	enum ABI_REG rs2;
	uint64_t imm;

	B_getfields(insn, &rs1, &rs2, &imm);
	dbg_log("bltu: Comparing %s (0x%lx) to %s (0x%lx), pc: 0x%lx, offset: 0x%lx",
		reg2abi(rs1), getreg(proc, rs1), reg2abi(rs2), getreg(proc, rs2),
		proc->pc, imm);

	if ((ureg_t)getreg(proc, rs1) < (ureg_t)getreg(proc, rs2)) {
		// Cycle always adds size of current instruction
		proc->pc += imm - 4;
		if (UNLIKELY(proc->pc % IALIGN != 0)) {
			err_log("pc (0x%lx) is not bit-aligned to IALIGN (%u)",
				proc->pc, IALIGN * 8);
			panic("Instruction-address-misaligned exception");
		}
	}

	return 0;
}

int insn_bge(struct proc *proc, insn_t insn)
{
	enum ABI_REG rs1;
	enum ABI_REG rs2;
	uint64_t imm;

	B_getfields(insn, &rs1, &rs2, &imm);
	dbg_log("bge: Comparing %s (0x%lx) to %s (0x%lx), pc: 0x%lx, offset: 0x%lx",
		reg2abi(rs1), getreg(proc, rs1), reg2abi(rs2), getreg(proc, rs2),
		proc->pc, imm);

	if ((ireg_t)getreg(proc, rs1) >= (ireg_t)getreg(proc, rs2)) {
		// Cycle always adds size of current instruction
		proc->pc += imm - 4;
		if (UNLIKELY(proc->pc % IALIGN != 0)) {
			err_log("pc (0x%lx) is not bit-aligned to IALIGN (%u)",
				proc->pc, IALIGN * 8);
			panic("Instruction-address-misaligned exception");
		}
	}

	return 0;
}

int insn_bgeu(struct proc *proc, insn_t insn)
{
	enum ABI_REG rs1;
	enum ABI_REG rs2;
	uint64_t imm;

	B_getfields(insn, &rs1, &rs2, &imm);
	dbg_log("bgeu: Comparing %s (0x%lx) to %s (0x%lx), pc: 0x%lx, offset: 0x%lx",
		reg2abi(rs1), getreg(proc, rs1), reg2abi(rs2), getreg(proc, rs2),
		proc->pc, imm);

	if ((ureg_t)getreg(proc, rs1) >= (ureg_t)getreg(proc, rs2)) {
		// Cycle always adds size of current instruction
		proc->pc += imm - 4;
		if (UNLIKELY(proc->pc % IALIGN != 0)) {
			err_log("pc (0x%lx) is not bit-aligned to IALIGN (%u)",
				proc->pc, IALIGN * 8);
			panic("Instruction-address-misaligned exception");
		}
	}

	return 0;
}

int insn_lb(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;
	uint8_t val;

	I_getfields(insn, &rd, &rs1, &imm);
	if (UNLIKELY(memload(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1)) {
		err_log("lb: Could not load address 0x%lx: %s",
			getreg(proc, rs1) + imm, strerror(errno));
		return -1;
	}

	dbg_log("0x%lx: lb: Loading %hhd from 0x%lx into %s", (unsigned long)proc->pc,
		(int8_t)val, getreg(proc, rs1) + imm, reg2abi(rd));
	mvreg(proc, rd, (reg_t)extend8to64(val));
	return 0;
}

int insn_lh(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;
	uint16_t val;

	I_getfields(insn, &rd, &rs1, &imm);
	if (UNLIKELY(memload(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1)) {
		err_log("lh: Could not load address 0x%lx: %s",
			getreg(proc, rs1) + imm, strerror(errno));
		return -1;
	}

	dbg_log("0x%lx: lh: Loading %hd from 0x%lx into %s", (unsigned long)proc->pc,
		(int16_t)val, getreg(proc, rs1) + imm, reg2abi(rd));
	mvreg(proc, rd, (reg_t)extend16to64(val));
	return 0;
}

int insn_lw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;
	uint32_t val;

	I_getfields(insn, &rd, &rs1, &imm);
	if (UNLIKELY(memload(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1)) {
		err_log("lw: Could not load address 0x%lx: %s",
			getreg(proc, rs1) + imm, strerror(errno));
		return -1;
	}

	dbg_log("0x%lx: lw: Loading %d from 0x%lx into %s", (unsigned long)proc->pc,
		(int32_t)val, getreg(proc, rs1) + imm, reg2abi(rd));
	mvreg(proc, rd, (reg_t)extend32to64(val));
	return 0;
}

int insn_ld(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;
	uint64_t val;

	I_getfields(insn, &rd, &rs1, &imm);
	if (UNLIKELY(memload(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1)) {
		err_log("ld: Could not load address 0x%lx: %s",
			getreg(proc, rs1) + imm, strerror(errno));
		return -1;
	}

	dbg_log("0x%lx: ld: Loading %ld from 0x%lx into %s", (unsigned long)proc->pc,
		(int64_t)val, getreg(proc, rs1) + imm, reg2abi(rd));
	mvreg(proc, rd, (reg_t)val);
	return 0;
}

int insn_lwu(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;
	uint32_t val;

	I_getfields(insn, &rd, &rs1, &imm);
	if (UNLIKELY(memload(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1)) {
		err_log("lwu: Could not load address 0x%lx: %s",
			getreg(proc, rs1) + imm, strerror(errno));
		return -1;
	}

	dbg_log("0x%lx: lwu: Loading %u from 0x%lx into %s", (unsigned long)proc->pc,
		val, getreg(proc, rs1) + imm, reg2abi(rd));
	mvreg(proc, rd, (reg_t)zextend32to64(val));
	return 0;
}

int insn_lhu(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;
	uint16_t val;

	I_getfields(insn, &rd, &rs1, &imm);
	if (UNLIKELY(memload(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1)) {
		err_log("lhu: Could not load address 0x%lx: %s",
			getreg(proc, rs1) + imm, strerror(errno));
		return -1;
	}

	dbg_log("0x%lx: lhu: Loading %hu from 0x%lx into %s", (unsigned long)proc->pc,
		val, getreg(proc, rs1) + imm, reg2abi(rd));
	mvreg(proc, rd, (reg_t)zextend16to64(val));
	return 0;
}

int insn_lbu(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;
	uint8_t val;

	I_getfields(insn, &rd, &rs1, &imm);
	if (UNLIKELY(memload(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1)) {
		err_log("lbu: Could not load address 0x%lx: %s",
			getreg(proc, rs1) + imm, strerror(errno));
		return -1;
	}

	dbg_log("0x%lx: lbu: Loading %hhu from 0x%lx into %s", (unsigned long)proc->pc,
		val, getreg(proc, rs1) + imm, reg2abi(rd));
	mvreg(proc, rd, (reg_t)zextend8to64(val));
	return 0;
}

int insn_sb(struct proc *proc, insn_t insn)
{
	enum ABI_REG rs1;
	enum ABI_REG rs2;
	uint64_t imm;
	uint8_t val;

	S_getfields(insn, &rs1, &rs2, &imm);
	val = (uint8_t)(getreg(proc, rs2) & 0xff);

	dbg_log("0x%lx: sb: Storing %hhu from %s into 0x%lx", (unsigned long)proc->pc,
		val, reg2abi(rs2), getreg(proc, rs1) + imm);
	if (UNLIKELY(memstore(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1)) {
		err_log("sb: Could not store address 0x%lx: %s",
			getreg(proc, rs1) + imm, strerror(errno));
		return -1;
	}

	return 0;
}

int insn_sh(struct proc *proc, insn_t insn)
{
	enum ABI_REG rs1;
	enum ABI_REG rs2;
	uint64_t imm;
	uint16_t val;

	S_getfields(insn, &rs1, &rs2, &imm);
	val = (uint16_t)(getreg(proc, rs2) & 0xffff);

	dbg_log("0x%lx: sh: Storing %hu from %s into 0x%lx", (unsigned long)proc->pc,
		val, reg2abi(rs2), getreg(proc, rs1) + imm);
	if (UNLIKELY(memstore(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1)) {
		err_log("sh: Could not store address 0x%lx: %s",
			getreg(proc, rs1) + imm, strerror(errno));
		return -1;
	}

	return 0;
}

int insn_sw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rs1;
	enum ABI_REG rs2;
	uint64_t imm;
	uint32_t val;

	S_getfields(insn, &rs1, &rs2, &imm);
	val = (uint32_t)(getreg(proc, rs2) & 0xffffffff);

	dbg_log("0x%lx: sw: Storing %u from %s into 0x%lx", (unsigned long)proc->pc,
		val, reg2abi(rs2), getreg(proc, rs1) + imm);
	if (UNLIKELY(memstore(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1)) {
		err_log("sw: Could not store address 0x%lx: %s",
			getreg(proc, rs1) + imm, strerror(errno));
		return -1;
	}

	return 0;
}

int insn_sd(struct proc *proc, insn_t insn)
{
	enum ABI_REG rs1;
	enum ABI_REG rs2;
	uint64_t imm;
	uint64_t val;

	S_getfields(insn, &rs1, &rs2, &imm);
	val = (uint64_t)getreg(proc, rs2);

	dbg_log("0x%lx: sd: Storing %lu from %s into 0x%lx", (unsigned long)proc->pc,
		val, reg2abi(rs2), getreg(proc, rs1) + imm);
	if (UNLIKELY(memstore(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1)) {
		err_log("sd: Could not store address 0x%lx: %s",
			getreg(proc, rs1) + imm, strerror(errno));
		return -1;
	}

	return 0;
}

int insn_fence(struct proc *proc, insn_t insn)
{
	(void)proc;
	(void)insn;

	dbg_log("fence: NOP");
	return 0;
}

int insn_ebreak(struct proc *proc, insn_t insn)
{
	enum ABI_REG i;

	(void)insn;

	dbg_log("EBREAK: Dumping proc's registers:");
	for (i = 1; i < 32; ++i) {
		printf("%s = 0x%lx\t\t\t", reg2abi(i), (unsigned long)getreg(proc, i));
		if ((i & 1) == 0)
			putchar('\n');
	}
	putchar('\n');

	return 0;
}

// Ignores rd and rs1, only implements linux system calls
int insn_ecall(struct proc *proc, insn_t insn)
{
	int sysnum;
	(void)insn;

	sysnum = (int)getreg(proc, REG_A7);

	dbg_log("0x%lx: ecall: syscall %d", (unsigned long)proc->pc, sysnum);

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
#endif // HAVE_RV64I

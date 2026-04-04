/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Rv64I instructions
 *
 *  Copyright (C) 2026 by Diego Oliveira <di.diegoevaristo@gmail.com>
 */

#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "riscv.h"
#include "proc.h"
#include "debug.h"
#include "rv_i.h"

#include "memory.h"

// For ecall
#include "syscall.h"
#include "sysnums.h"

// Get info from OP-Code fields
static inline void R_getfields(insn_t insn, enum ABI_REG *rd, enum ABI_REG *rs1,
			       enum ABI_REG *rs2) __attribute__((nonnull));
static inline void I_getfields(insn_t insn, enum ABI_REG *rd, enum ABI_REG *rs1,
			       uint64_t *imm)	__attribute__((nonnull));
static inline void U_getfields(insn_t insn, enum ABI_REG *rd, uint64_t *imm)
						__attribute__((nonnull));
static inline void J_getfields(insn_t insn, enum ABI_REG *rd, uint64_t *imm)
						__attribute__((nonnull));
static inline void B_getfields(insn_t insn, enum ABI_REG *rs1, enum ABI_REG *rs2,
			       uint64_t *imm)	__attribute__((nonnull));
static inline void S_getfields(insn_t insn, enum ABI_REG *rs1, enum ABI_REG *rs2,
		               uint64_t *imm)	__attribute__((nonnull));

// Sign-extend/Zero-extend values
static inline uint64_t extend32to64(uint32_t i32);
static inline uint64_t extend16to64(uint16_t i16);
static inline uint64_t extend8to64(uint8_t i8);
static inline uint64_t zextend32to64(uint32_t i32);
static inline uint64_t zextend16to64(uint16_t i16);
static inline uint64_t zextend8to64(uint8_t i8);


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


static inline uint64_t extend32to64(uint32_t i32)
{
	return i32 & 0x80000000 ? (uint64_t)i32 | (~0lu << 32) : i32;
}

static inline uint64_t extend16to64(uint16_t i16)
{
	return i16 & 0x8000 ? (uint64_t)i16 | (~0lu << 16) : i16;
}

static inline uint64_t extend8to64(uint8_t i8)
{
	return i8 & 0x80 ? (uint64_t)i8 | (~0lu << 8) : i8;
}

static inline uint64_t zextend32to64(uint32_t u32)
{
	return (uint64_t)u32;
}

static inline uint64_t zextend16to64(uint16_t u16)
{
	return (uint64_t)u16;
}

static inline uint64_t zextend8to64(uint8_t u8)
{
	return (uint64_t)u8;
}


int insn_add(struct proc *proc, insn_t insn)
{
	enum ABI_REG rs1;
	enum ABI_REG rs2;
	enum ABI_REG rd;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, getreg(proc, rs1) + getreg(proc, rs2));
	dbg_log("add: setting %s to %s + %s = %ld", reg2abi(rd), reg2abi(rs1),
		reg2abi(rs2), getreg(proc, rd));
	return 0;
}

int insn_slt(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, (ireg_t)getreg(proc, rs1) < (ireg_t)getreg(proc, rs2));
	dbg_log("stl: Setting %s to %ld: %s < %s?", reg2abi(rd), getreg(proc, rd),
		reg2abi(rs1), reg2abi(rs2));
	return 0;
}

int insn_sltu(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, (ureg_t)getreg(proc, rs1) < (ureg_t)getreg(proc, rs2));
	dbg_log("stlu: Setting %s to %ld: %s < %s?", reg2abi(rd), getreg(proc, rd),
		reg2abi(rs1), reg2abi(rs2));
	return 0;
}

int insn_and(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, getreg(proc, rs1) & getreg(proc, rs2));
	dbg_log("and: Setting %s = %s & %s = 0x%lx", reg2abi(rd), reg2abi(rs1),
		reg2abi(rs2), getreg(proc, rd));
	return 0;
}

int insn_or(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, getreg(proc, rs1) | getreg(proc, rs2));
	dbg_log("and: Setting %s = %s | %s = 0x%lx", reg2abi(rd), reg2abi(rs1),
		reg2abi(rs2), getreg(proc, rd));
	return 0;
}

int insn_xor(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, getreg(proc, rs1) ^ getreg(proc, rs2));
	dbg_log("xor: Setting %s = %s ^ %s = 0x%lx", reg2abi(rd), reg2abi(rs1),
		reg2abi(rs2), getreg(proc, rd));
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
	dbg_log("sll: Setting %s = x%s << %s = 0x%lx", reg2abi(rd), reg2abi(rs1),
		reg2abi(rs2), getreg(proc, rd));
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
	dbg_log("srl: Setting %s = %s >> %s = 0x%lx", reg2abi(rd), reg2abi(rs1),
		reg2abi(rs2), getreg(proc, rd));
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
	dbg_log("sra: Setting %s = %s >> %s = 0x%lx", reg2abi(rd), reg2abi(rs1),
		reg2abi(rs2), getreg(proc, rd));
	return 0;
}

int insn_sub(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	enum ABI_REG rs2;

	R_getfields(insn, &rd, &rs1, &rs2);
	mvreg(proc, rd, getreg(proc, rs1) - getreg(proc, rs2));
	dbg_log("sub: Setting %s = %s - %s = %ld", reg2abi(rd), reg2abi(rs1),
		reg2abi(rs2), getreg(proc, rd));
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
	dbg_log("addw: setting %s = %s + %s", reg2abi(rd), reg2abi(rs1),
		reg2abi(rs2));
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
	dbg_log("addw: setting %s = %s - %s", reg2abi(rd), reg2abi(rs1),
		reg2abi(rs2));
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
	dbg_log("addw: setting %s = %s << %s", reg2abi(rd), reg2abi(rs1),
		reg2abi(rs2));
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
	dbg_log("addw: setting %s = %s >> %s", reg2abi(rd), reg2abi(rs1),
		reg2abi(rs2));
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
	dbg_log("sraw: setting %s = %s >> %s", reg2abi(rd), reg2abi(rs1),
		reg2abi(rs2));
	return 0;
}

int insn_andi(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)(getreg(proc, rs1) & imm));
	dbg_log("andi: Setting %s = %s & 0x%lx", reg2abi(rd), reg2abi(rs1), imm);
	return 0;
}

int insn_ori(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)(getreg(proc, rs1) | imm));
	dbg_log("ori: Setting %s = %s | 0x%lx", reg2abi(rd), reg2abi(rs1), imm);
	return 0;
}

int insn_xori(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)(getreg(proc, rs1) ^ imm));
	dbg_log("xori: Setting %s = %s ^ 0x%lx", reg2abi(rd), reg2abi(rs1), imm);
	return 0;
}

int insn_addi(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)((uint64_t)getreg(proc, rs1) + imm));
	dbg_log("addi: Setting %s = %s + %ld", reg2abi(rd), reg2abi(rs1),
		(int64_t)imm);
	return 0;
}

int insn_slli(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)((uint64_t)getreg(proc, rs1) << (imm & 0x3f)));
	dbg_log("slli: Setting %s = %s << %d", reg2abi(rd), reg2abi(rs1),
		(int)(imm & 0x3f));
	return 0;
}

int insn_srli(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)((uint64_t)getreg(proc, rs1) >> (imm & 0x3f)));
	dbg_log("srli: Setting %s = %s >> %d", reg2abi(rd), reg2abi(rs1),
		(int)(imm & 0x3f));
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
	dbg_log("srai: Setting %s = %s >> %d", reg2abi(rd), reg2abi(rs1),
		(int)(imm & 0x3f));
	return 0;
}

int insn_slti(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)((ireg_t)getreg(proc, rs1) < (ireg_t)imm));
	dbg_log("slti: Setting %s = %s < %ld", reg2abi(rd), reg2abi(rs1),
		(int64_t)imm);
	return 0;
}

int insn_sltiu(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)((ureg_t)getreg(proc, rs1) < (ureg_t)imm));
	dbg_log("sltiu: Setting %s = %s < %lu", reg2abi(rd), reg2abi(rs1), imm);
	return 0;
}

int insn_addiw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	uint64_t imm;

	I_getfields(insn, &rd, &rs1, &imm);
	mvreg(proc, rd, (reg_t)extend32to64((uint32_t)(getreg(proc, rd) + imm)));
	dbg_log("addiw: %s = %s + %d", reg2abi(rd), reg2abi(rs1),
		(int32_t)imm);
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
	dbg_log("slliw: %s = %s << %u", reg2abi(rd), reg2abi(rs1),
		(unsigned)(imm & 0x1f));
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
	dbg_log("srliw: %s = %s >> %u", reg2abi(rd), reg2abi(rs1),
		(unsigned)(imm & 0x1f));
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
	dbg_log("sraiw: %s = %s >> %u", reg2abi(rd), reg2abi(rs1),
		(unsigned)(imm & 0x1f));
	return 0;
}

int insn_lui(struct proc *proc, insn_t insn)
{
	uint64_t imm;
	enum ABI_REG rd;

	U_getfields(insn, &rd, &imm);
	mvreg(proc, rd, (reg_t)imm);
	dbg_log("lui: Setting %s = %ld", reg2abi(rd), imm);
	return 0;
}

int insn_auipc(struct proc *proc, insn_t insn)
{
	uint64_t imm;
	enum ABI_REG rd;

	U_getfields(insn, &rd, &imm);
	mvreg(proc, rd, (reg_t)(proc->pc + imm));
	dbg_log("auipc: %s = 0x%lx + 0x%lx", reg2abi(rd), proc->pc, imm);
	return 0;
}

int insn_jal(struct proc *proc, insn_t insn)
{
	uint64_t imm;
	enum ABI_REG rd;

	J_getfields(insn, &rd, &imm);
	mvreg(proc, rd, (reg_t)(proc->pc + 4));
	dbg_log("jal: %s = 0x%lx, pc += 0x%lx", reg2abi(rd), getreg(proc, rd), imm);
	proc->pc += imm - 4; // Cycle always adds size of current instruction
	if (proc->pc % IALIGN != 0) {
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
	dbg_log("jalr: %s = 0x%lx. pc = %s (0x%lx) + 0x%lx", reg2abi(rd),
		getreg(proc, rd), reg2abi(rs1), getreg(proc, rs1), imm);

	// Cycle always adds size of current instruction
	proc->pc = (rvaddr_t)(getreg(proc, rs1) + imm - 4);
	if (proc->pc % IALIGN != 0) {
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
		if (proc->pc % IALIGN != 0) {
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
		if (proc->pc % IALIGN != 0) {
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
		if (proc->pc % IALIGN != 0) {
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
		if (proc->pc % IALIGN != 0) {
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

	if ((ireg_t)getreg(proc, rs1) > (ireg_t)getreg(proc, rs2)) {
		// Cycle always adds size of current instruction
		proc->pc += imm - 4;
		if (proc->pc % IALIGN != 0) {
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

	if ((ureg_t)getreg(proc, rs1) > (ureg_t)getreg(proc, rs2)) {
		// Cycle always adds size of current instruction
		proc->pc += imm - 4;
		if (proc->pc % IALIGN != 0) {
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
	if (memload(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1) {
		err_log("lb: Could not load address 0x%lx: %s",
			getreg(proc, rs1) + imm, strerror(errno));
		return -1;
	}

	dbg_log("lb: Loading %hhd from 0x%lx into %s", (int8_t)val,
		getreg(proc, rs1) + imm, reg2abi(rd));
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
	if (memload(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1) {
		err_log("lh: Could not load address 0x%lx: %s",
			getreg(proc, rs1) + imm, strerror(errno));
		return -1;
	}

	dbg_log("lh: Loading %hd from 0x%lx into %s", (int16_t)val,
		getreg(proc, rs1) + imm, reg2abi(rd));
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
	if (memload(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1) {
		err_log("lw: Could not load address 0x%lx: %s",
			getreg(proc, rs1) + imm, strerror(errno));
		return -1;
	}

	dbg_log("lw: Loading %d from 0x%lx into %s", (int32_t)val,
		getreg(proc, rs1) + imm, reg2abi(rd));
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
	if (memload(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1) {
		err_log("ld: Could not load address 0x%lx: %s",
			getreg(proc, rs1) + imm, strerror(errno));
		return -1;
	}

	dbg_log("ld: Loading %ld from 0x%lx into %s", (int64_t)val,
		getreg(proc, rs1) + imm, reg2abi(rd));
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
	if (memload(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1) {
		err_log("lwu: Could not load address 0x%lx: %s",
			getreg(proc, rs1) + imm, strerror(errno));
		return -1;
	}

	dbg_log("lwu: Loading %u from 0x%lx into %s", val,
		getreg(proc, rs1) + imm, reg2abi(rd));
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
	if (memload(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1) {
		err_log("lhu: Could not load address 0x%lx: %s",
			getreg(proc, rs1) + imm, strerror(errno));
		return -1;
	}

	dbg_log("lhu: Loading %hu from 0x%lx into %s", val,
		getreg(proc, rs1) + imm, reg2abi(rd));
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
	if (memload(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1) {
		err_log("lbu: Could not load address 0x%lx: %s",
			getreg(proc, rs1) + imm, strerror(errno));
		return -1;
	}

	dbg_log("lbu: Loading %hhu from 0x%lx into %s", val,
		getreg(proc, rs1) + imm, reg2abi(rd));
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

	dbg_log("sb: Storing %hhu from %s into 0x%lx", val, reg2abi(rs2),
		getreg(proc, rs1) + imm);
	if (memstore(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1) {
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

	dbg_log("sh: Storing %hu from %s into 0x%lx", val, reg2abi(rs2),
		getreg(proc, rs1) + imm);
	if (memstore(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1) {
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

	dbg_log("sw: Storing %u from %s into 0x%lx", val, reg2abi(rs2),
		getreg(proc, rs1) + imm);
	if (memstore(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1) {
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

	dbg_log("sd: Storing %lu from %s into 0x%lx", val, reg2abi(rs2),
		getreg(proc, rs1) + imm);
	if (memstore(proc->mem, (rvaddr_t)(getreg(proc, rs1) + imm), &val) == -1) {
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
	int i;

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

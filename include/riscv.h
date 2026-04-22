/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * RISC-V constants and typedefs
 *
 *  Copyright (C) 2026 by Diego Evaristo <di.diegoevaristo@gmail.com>
 */

#ifndef RISCV_H
#define RISCV_H

#define XLEN 64
#define IALIGN 4 // In bytes

#include <stdint.h>
typedef uint64_t reg_t;
typedef uint64_t ureg_t;
typedef int64_t ireg_t;
typedef uint64_t rvaddr_t;
typedef uint32_t insn_t;
typedef uint16_t csr_t;
typedef float freg_t;

enum ABI_REG {
	REG_ZERO=0,
	REG_RA,
	REG_SP,
	REG_GP,
	REG_TP,
	REG_T0,
	REG_T1,
	REG_T2,
	REG_S0,
	REG_S1,
	REG_A0,
	REG_A1,
	REG_A2,
	REG_A3,
	REG_A4,
	REG_A5,
	REG_A6,
	REG_A7,
	REG_S2,
	REG_S3,
	REG_S4,
	REG_S5,
	REG_S6,
	REG_S7,
	REG_S8,
	REG_S9,
	REG_S10,
	REG_S11,
	REG_T3,
	REG_T4,
	REG_T5,
	REG_T6,
};

enum ABI_FREG {
	FREG_T0=0,
	FREG_T1,
	FREG_T2,
	FREG_T3,
	FREG_T4,
	FREG_T5,
	FREG_T6,
	FREG_T7,
	FREG_S0,
	FREG_S1,
	FREG_A0,
	FREG_A1,
	FREG_A2,
	FREG_A3,
	FREG_A4,
	FREG_A5,
	FREG_A6,
	FREG_A7,
	FREG_S2,
	FREG_S3,
	FREG_S4,
	FREG_S5,
	FREG_S6,
	FREG_S7,
	FREG_S8,
	FREG_S9,
	FREG_S10,
	FREG_S11,
	FREG_T8,
	FREG_T9,
	FREG_T10,
	FREG_T11,
};

#include "common.h"

// Convert register number to abi name
const char *reg2abi(enum ABI_REG r) ATTRIBUTE(const, leaf);
const char *freg2abi(enum ABI_FREG r) ATTRIBUTE(const, leaf);

// Sign-extend/Zero-extend values
static inline uint64_t extend32to64(uint32_t i32) ATTRIBUTE(const);
static inline uint64_t extend16to64(uint16_t i16) ATTRIBUTE(const);
static inline uint64_t extend8to64(uint8_t i8) ATTRIBUTE(const);
static inline uint64_t zextend32to64(uint32_t i32) ATTRIBUTE(const);
static inline uint64_t zextend16to64(uint16_t i16) ATTRIBUTE(const);
static inline uint64_t zextend8to64(uint8_t i8) ATTRIBUTE(const);

// Get instruction fields
static inline uint8_t insn2rd(insn_t insn) ATTRIBUTE(const);
static inline uint8_t insn2rs1(insn_t insn) ATTRIBUTE(const);
static inline uint8_t insn2rs2(insn_t insn) ATTRIBUTE(const);

// Inline functions
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

static inline uint8_t insn2rd(insn_t insn)
{
	return (uint8_t)((insn & 0xf80) >> 7);
}

static inline uint8_t insn2rs1(insn_t insn)
{
	return (uint8_t)((insn & 0xf8000) >> 15);
}

static inline uint8_t insn2rs2(insn_t insn)
{
	return (uint8_t)((insn & 0x1f00000) >> 20);
}

#endif // RISCV_H

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

#include "common.h"

// Convert register number to abi name
const char *reg2abi(enum ABI_REG r) ATTRIBUTE(const, leaf);

// Sign-extend/Zero-extend values
static inline uint64_t extend32to64(uint32_t i32);
static inline uint64_t extend16to64(uint16_t i16);
static inline uint64_t extend8to64(uint8_t i8);
static inline uint64_t zextend32to64(uint32_t i32);
static inline uint64_t zextend16to64(uint16_t i16);
static inline uint64_t zextend8to64(uint8_t i8);


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

#endif // RISCV_H

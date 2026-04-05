/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * RISC-V related operations.
 *
 *  Copyright (C) 2026 by Diego Evaristo <di.diegoevaristo@gmail.com>
 */
#include <assert.h>
#include "riscv.h"

static const char *abi_reg[32] = {
	[REG_ZERO] = "zero",
	[REG_RA] = "ra",
	[REG_SP] = "sp",
	[REG_GP] = "gp",
	[REG_TP] = "tp",
	[REG_T0] = "t0",
	[REG_T1] = "t1",
	[REG_T2] = "t2",
	[REG_S0] = "s0",
	[REG_S1] = "s1",
	[REG_A0] = "a0",
	[REG_A1] = "a1",
	[REG_A2] = "a2",
	[REG_A3] = "a3",
	[REG_A4] = "a4",
	[REG_A5] = "a5",
	[REG_A6] = "a6",
	[REG_A7] = "a7",
	[REG_S2] = "s2",
	[REG_S3] = "s3",
	[REG_S4] = "s4",
	[REG_S5] = "s5",
	[REG_S6] = "s6",
	[REG_S7] = "s7",
	[REG_S8] = "s8",
	[REG_S9] = "s9",
	[REG_S10] = "s10",
	[REG_S11] = "s11",
	[REG_T3] = "t3",
	[REG_T4] = "t4",
	[REG_T5] = "t5",
	[REG_T6] = "t6",
};

const char *reg2abi(enum ABI_REG r)
{
	assert(r >= 0 && r < 32);
	return abi_reg[r];
}

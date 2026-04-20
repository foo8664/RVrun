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

static const char *abi_freg[32] = {
	[FREG_T0] = "ft0",
	[FREG_T1] = "ft1",
	[FREG_T2] = "ft2",
	[FREG_T3] = "ft3",
	[FREG_T4] = "ft4",
	[FREG_T5] = "ft5",
	[FREG_T6] = "ft6",
	[FREG_T7] = "ft7",
	[FREG_S0] = "fs0",
	[FREG_S1] = "fs1",
	[FREG_A0] = "fa0",
	[FREG_A1] = "fa1",
	[FREG_A2] = "fa2",
	[FREG_A3] = "fa3",
	[FREG_A4] = "fa4",
	[FREG_A5] = "fa5",
	[FREG_A6] = "fa6",
	[FREG_A7] = "fa7",
	[FREG_S2] = "fs2",
	[FREG_S3] = "fs3",
	[FREG_S4] = "fs4",
	[FREG_S5] = "fs5",
	[FREG_S6] = "fs6",
	[FREG_S7] = "fs7",
	[FREG_S8] = "fs8",
	[FREG_S9] = "fs9",
	[FREG_S10] = "fs10",
	[FREG_S11] = "fs11",
	[FREG_T8] = "ft8",
	[FREG_T9] = "ft9",
	[FREG_T10] = "ft10",
	[FREG_T11] = "ft11",
};

const char *reg2abi(enum ABI_REG r)
{
	assert(r >= 0 && r < 32);
	return abi_reg[r];
}

const char *freg2abi(enum ABI_FREG r)
{
	assert(r >= 0 && r < 32);
	return abi_freg[r];
}

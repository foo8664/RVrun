/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Rv64M Instructions
 *
 *  Copyright (C) 2026 by Diego Evaristo <di.diegoevaristo@gmail.com>
 */

#ifndef RVRUN_RV64M_H
#define RVRUN_RV64M_H

#include "riscv.h"
#include "proc.h"

#include "common.h"
int insn_mul(struct proc *proc, insn_t insn)	ATTRIBUTE(nonnull);
int insn_mulw(struct proc *proc, insn_t insn)	ATTRIBUTE(nonnull);
int insn_mulh(struct proc *proc, insn_t insn)	ATTRIBUTE(nonnull);
int insn_mulhu(struct proc *proc, insn_t insn)	ATTRIBUTE(nonnull);
int insn_mulhsu(struct proc *proc, insn_t insn)	ATTRIBUTE(nonnull);
int insn_div(struct proc *proc, insn_t insn)	ATTRIBUTE(nonnull);
int insn_divu(struct proc *proc, insn_t insn)	ATTRIBUTE(nonnull);
int insn_divw(struct proc *proc, insn_t insn)	ATTRIBUTE(nonnull);
int insn_divuw(struct proc *proc, insn_t insn)	ATTRIBUTE(nonnull);
int insn_rem(struct proc *proc, insn_t insn)	ATTRIBUTE(nonnull);
int insn_remu(struct proc *proc, insn_t insn)	ATTRIBUTE(nonnull);
int insn_remw(struct proc *proc, insn_t insn)	ATTRIBUTE(nonnull);
int insn_remuw(struct proc *proc, insn_t insn)	ATTRIBUTE(nonnull);

#endif // RVRUN_RV64M_H

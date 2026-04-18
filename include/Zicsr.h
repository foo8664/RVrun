/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Zicsr instructions
 *
 *  Copyright (C) 2026 by Diego Evaristo <di.diegoevaristo@gmail.com>
 */

#ifndef RVRUN_ZICSR_H
#define RVRUN_ZICSR_H

#include "common.h"
#include "riscv.h"
#include "proc.h"

/*
 * Calls a specialized function to it's csr field. The specialized functions
 * must have the following prototype:
 * 	int func(enum ABI_REG rd, enum ABI_REG rs1, struct proc *proc, csr_t csr)
 * proc will be nonnull, and the function must return 0 in case of success.
 */
int insn_csrrw(struct proc *proc, insn_t insn)	ATTRIBUTE(nonnull);
int insn_csrrs(struct proc *proc, insn_t insn)	ATTRIBUTE(nonnull);
int insn_csrrc(struct proc *proc, insn_t insn)	ATTRIBUTE(nonnull);
int insn_csrrwi(struct proc *proc, insn_t insn)	ATTRIBUTE(nonnull);
int insn_csrrsi(struct proc *proc, insn_t insn)	ATTRIBUTE(nonnull);
int insn_csrrci(struct proc *proc, insn_t insn)	ATTRIBUTE(nonnull);

#endif // RVRUN_ZICSR_H

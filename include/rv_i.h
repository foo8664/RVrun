/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Rv64I instructions
 *
 *  Copyright (C) 2026 by Diego Oliveira Evaristo <di.diegoevaristo@gmail.com>
 */

#ifndef RISCV_RV64I_H
#define RISCV_RV64I_H

#include "riscv.h"
#include "proc.h"

/*
 * These are the functions returned by insn_decode(), they simulate the
 * instruction with their name on the process `proc`. `insn` should be their
 * 32bit verbatim representation
 */

// R-type instructions
int insn_add(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_slt(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_sltu(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_and(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_or(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_xor(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_sll(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_srl(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_sra(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_sub(struct proc *proc, insn_t insn)	__attribute__((nonnull));

// I-type instructions
int insn_andi(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_ori(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_xori(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_addi(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_slti(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_sltiu(struct proc *proc, insn_t insn)	__attribute__((nonnull));


#endif // RISCV_RV64I_H

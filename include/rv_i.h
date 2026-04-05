/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Rv64I instructions
 *
 *  Copyright (C) 2026 by Diego Evaristo <di.diegoevaristo@gmail.com>
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

// Register-Register instructions
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
int insn_addw(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_subw(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_sllw(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_srlw(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_sraw(struct proc *proc, insn_t insn)	__attribute__((nonnull));

// Register-Immediate instructions
int insn_andi(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_ori(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_xori(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_addi(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_slli(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_srli(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_srai(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_slti(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_sltiu(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_addiw(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_slliw(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_srliw(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_sraiw(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_lui(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_auipc(struct proc *proc, insn_t insn)	__attribute__((nonnull));

// Jump instructions
int insn_jal(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_jalr(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_beq(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_bne(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_blt(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_bltu(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_bge(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_bgeu(struct proc *proc, insn_t insn)	__attribute__((nonnull));

// Store/Load Instructions
int insn_lb(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_lh(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_lw(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_lbu(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_lhu(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_lwu(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_ld(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_sb(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_sh(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_sw(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_sd(struct proc *proc, insn_t insn)	__attribute__((nonnull));

// Miscallenous
int insn_ecall(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_ebreak(struct proc *proc, insn_t insn)	__attribute__((nonnull));
int insn_fence(struct proc *proc, insn_t insn)	__attribute__((nonnull));

#endif // RISCV_RV64I_H

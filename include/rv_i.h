#ifndef RISCV_RV64I_H
#define RISCV_RV64I_H

#include "riscv.h"
#include "proc.h"

/*
 * These are the functions returned by insn_decode(), they simulate the
 * instruction with their name on the process `proc`. `insn` should be their
 * 32bit verbatim representation
 */
int insn_add(struct proc *proc, insn_t insn) __attribute__((nonnull));
int insn_slt(struct proc *proc, insn_t insn) __attribute__((nonnull));
int insn_sltu(struct proc *proc, insn_t insn) __attribute__((nonnull));
int insn_and(struct proc *proc, insn_t insn) __attribute__((nonnull));
int insn_or(struct proc *proc, insn_t insn) __attribute__((nonnull));
int insn_xor(struct proc *proc, insn_t insn) __attribute__((nonnull));
int insn_sll(struct proc *proc, insn_t insn) __attribute__((nonnull));
int insn_srl(struct proc *proc, insn_t insn) __attribute__((nonnull));
int insn_sra(struct proc *proc, insn_t insn) __attribute__((nonnull));
int insn_sub(struct proc *proc, insn_t insn) __attribute__((nonnull));

#endif // RISCV_RV64I_H

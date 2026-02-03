#ifndef INSN_H
#define INSN_H

#include "opcodes.h"
#define IS_INSN(insn, name) (((insn) & MATCH_##name) == (MATCH_##name))

#include "riscv.h"
#include "loader.h"
/*
 * fetches the next instruction from a process and returns it's size in bytes,
 * will return -1 and set errno if it fails, analogous to memload() but checks
 * for the MEM_EXEC flag, and `addr` is `proc->pc`.
 */
int insn_fetch(struct proc *proc, insn_t *insn) __attribute__((nonnull));

/*
 * Decodes an instruction and returns the function that simulates it, will
 * return NULL and set errno to ENOSYS if the instruction is not supported
 */
int (*insn_decode(insn_t insn))(struct proc *, insn_t);

/*
 * These are the functions returned by insn_decode(), these currently don't do
 * anything, and are there just for testing.
 */
int insn_add(struct proc *proc, insn_t insn) __attribute__((nonnull));
int insn_and(struct proc *proc, insn_t insn) __attribute__((nonnull));

#endif // INSN_H

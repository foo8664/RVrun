/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Process loading and emulation
 *
 *  Copyright (C) 2026 by Diego Oliveira Evaristo <di.diegoevaristo@gmail.com>
 */

#ifndef LOADER_H
#define LOADER_H

#include "riscv.h"
#include "memory.h"

#include <stdbool.h>
struct exitinfo {
	int status;
	bool exited;
};

// Process structure
struct proc {
	reg_t regs[32]; // reg[N] is register xN
	reg_t pc;

	struct memory mem;
	struct exitinfo exitinfo;
};

// Free's a process allocated by loadproc
void freeproc(struct proc *proc) __attribute__((nonnull, cold));
// Loades a process from an ELF file
struct proc *loadproc(const char *path)
	__attribute__((nonnull, cold, malloc(freeproc, 1)));


// set and get a process's registers
static inline void mvreg(struct proc *proc, enum ABI_REG reg, reg_t val)
	__attribute__((nonnull));
static inline reg_t getreg(const struct proc *proc, enum ABI_REG reg)
	__attribute__((nonnull));

#include <assert.h>
static inline void mvreg(struct proc *proc, enum ABI_REG reg, reg_t val)
{
	assert(reg < 32);

	if (reg != 0)
		proc->regs[reg] = val;
}

static inline reg_t getreg(const struct proc *proc, enum ABI_REG reg)
{
	assert(reg < 32);
	if (reg == 0)
		return 0;
	return proc->regs[reg];
}

#endif /* LOADER_H */

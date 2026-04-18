/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Process loading and emulation
 *
 *  Copyright (C) 2026 by Diego Evaristo <di.diegoevaristo@gmail.com>
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

#include <stddef.h>
struct fdinfo {
	size_t fdmax;
	int *fds;
};

// Process structure
struct proc {
	// Normal registers
	reg_t regs[32]; // reg[N] is register xN
	reg_t pc;

	// CSR registers
	uint32_t fcsr;

	// Extra info
	struct memory mem;
	struct exitinfo exitinfo;
	struct fdinfo fdinfo;
};

#include "common.h"

// Free's a process allocated by loadproc
void freeproc(struct proc *proc) ATTRIBUTE(nonnull, cold);

// Loades a process from an ELF file
struct proc *loadproc(const char *path) ATTRIBUTE(nonnull, cold, malloc(freeproc, 1));

// Loads a processes argv and envp. argv should not contain RvRun's arguments
int load_argv_and_envp(struct proc *proc, char **argv, char **envp) ATTRIBUTE(nonnull, cold);

// set and get a process's registers
static inline void mvreg(struct proc *proc, enum ABI_REG reg, reg_t val) ATTRIBUTE(nonnull);
static inline reg_t getreg(const struct proc *proc, enum ABI_REG reg) ATTRIBUTE(nonnull);

#include <assert.h>
static inline void mvreg(struct proc *proc, enum ABI_REG reg, reg_t val)
{
	assert(reg >= 0 && reg < 32);
	proc->regs[reg] = (reg ? val : 0);
}

static inline reg_t getreg(const struct proc *proc, enum ABI_REG reg)
{
	assert(reg >= 0 && reg < 32);
	return (reg ? proc->regs[reg] : 0);
}

#endif /* LOADER_H */

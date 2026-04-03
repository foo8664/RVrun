/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Process loading and emulation
 *
 *  Copyright (C) 2026 by Diego Oliveira <di.diegoevaristo@gmail.com>
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
	reg_t regs[32]; // reg[N] is register xN
	reg_t pc;

	struct memory mem;
	struct exitinfo exitinfo;
	struct fdinfo fdinfo;
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

#include "debug.h"
static inline void mvreg(struct proc *proc, enum ABI_REG reg, reg_t val)
{
	if (reg > 32) {
		err_log("Invalid Register x%d", reg);
		panic("Tried to access inexistent register");
	}
	proc->regs[reg] = (reg ? val : 0);
}

static inline reg_t getreg(const struct proc *proc, enum ABI_REG reg)
{
	if (reg > 32) {
		err_log("Invalid Register x%d", reg);
		panic("Tried to access inexistent register");
	}
	return (reg ? proc->regs[reg] : 0);
}

#endif /* LOADER_H */

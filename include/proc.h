#ifndef LOADER_H
#define LOADER_H

#include "riscv.h"
#include "memory.h"

// Process structure
struct proc {
	reg_t regs[32]; // reg[N] is register xN
	reg_t pc;
	struct memory mem;
};

// Free's a process allocated by loadproc
void freeproc(struct proc *proc) __attribute__((nonnull, cold));
// Loades a process from an ELF file
struct proc *loadproc(const char *path)
	__attribute__((nonnull, cold, malloc(freeproc, 1)));

#endif /* LOADER_H */

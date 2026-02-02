#ifndef INSN_H
#define INSN_H

#include "riscv.h"
int insn_fetch(struct proc *proc, insn_t *insn) __attribute__((nonnull));

#endif // INSN_H

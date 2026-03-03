/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Main file, loads, executes, and exits
 *
 *  Copyright (C) 2026 by Diego Oliveira Evaristo <di.diegoevaristo@gmail.com>
 */

#include <assert.h>
#include "riscv.h"
#include "debug.h"
#include "memory.h"
#include "proc.h"
#include "insn.h"

int main(void)
{
	struct proc *proc;
	insn_func_t insn_func;
	insn_t insn;
	int ret;
	int i;

	if (!(proc = loadproc("test.elf")))
		return 1;

	ret = insn_fetch(proc, &insn);
	assert(ret == 4);
	dbg_log("insn 0x%.8x at addr 0x%lx", insn, proc->pc);
	assert(insn_func = insn_decode(insn));

	assert(insn_func(proc, insn) == 0);
	proc->pc += (unsigned )ret;

	for (i = 0; i < 32; ++i)
		dbg_log("proc->x%d = %ld", i, (ireg_t)proc->regs[i]);

	freeproc(proc);
	return 0;
}

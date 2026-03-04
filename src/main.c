/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Main file, loads, executes, and exits
 *
 *  Copyright (C) 2026 by Diego Oliveira Evaristo <di.diegoevaristo@gmail.com>
 */

#include <errno.h>
#include <string.h>
#include "riscv.h"
#include "debug.h"
#include "memory.h"
#include "proc.h"
#include "insn.h"

// Von Neumann fetch-decode-exec cycle, returns non-zero at error
int exec_cycle(struct proc *proc) __attribute__((nonnull, cold));

int main(void)
{
	struct proc *proc;
	int ret;

	if (!(proc = loadproc("test.elf")))
		return 1;

	ret = exec_cycle(proc);
	if (ret != 0)
		err_log("Interrupting fetch-decode-exec cycle");
	else
		ret = proc->exitinfo.status;

	freeproc(proc);
	return ret;
}

int exec_cycle(struct proc *proc)
{
	insn_func_t insn_func;
	insn_t insn;
	int pc_skip;

	errno = 0;
	while (!proc->exitinfo.exited) {
		pc_skip = insn_fetch(proc, &insn);
		if (pc_skip == -1) {
			err_log("Can't fetch insn at 0x%lx: %s", proc->pc,
				strerror(errno));
			return 1;
		}

		insn_func = insn_decode(insn);
		if (!insn_func) {
			err_log("Can't decode 0x%x at pc 0x%lx", insn, proc->pc);
			return 1;
		}

		if (insn_func(proc, insn) != 0) {
			err_log("Can't execute insn 0x%x at pc 0x%lx: %s",
				insn, proc->pc, strerror(errno));
			return 1;
		}

		proc->pc += (rvaddr_t)pc_skip;
	}

	return 0;
}

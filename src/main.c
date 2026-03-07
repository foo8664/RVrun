/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Main file, loads, executes, and exits
 *
 *  Copyright (C) 2026 by Diego Oliveira Evaristo <di.diegoevaristo@gmail.com>
 */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include "riscv.h"
#include "debug.h"
#include "memory.h"
#include "proc.h"
#include "insn.h"
#include "config.h"

// Von Neumann fetch-decode-exec cycle, returns non-zero at error
static int exec_cycle(struct proc *proc) __attribute__((nonnull, cold));
// Parses options and sets globalcfg from config.h
static void setcfg(int argc, char **argv) __attribute__((nonnull, cold));

int main(int argc, char **argv)
{
	struct proc *proc;
	int ret;

	setdbgcfg();
	setcfg(argc, argv);
	if (optind >= argc)
		panic("Needs a file to execute");

	if (!(proc = loadproc(argv[optind])))
		return 1;

	ret = exec_cycle(proc);
	if (ret != 0)
		err_log("Interrupting fetch-decode-exec cycle");
	else
		ret = proc->exitinfo.status;

	freeproc(proc);
	cleancfg(&globalcfg, NULL);
	return ret;
}

static int exec_cycle(struct proc *proc)
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

static void setcfg(int argc, char **argv)
{
	static struct optpair opts[] = {{
		.rvopt = {.type = CFG_STDIN},
		.opt = {.has_arg = ARG_MANDATORY, .name = "stdin"}
	},				{
		.rvopt = {.type = CFG_STDOUT},
		.opt = {.has_arg = ARG_MANDATORY, .name = "stdout"}
	},				{
		.rvopt = {.type = CFG_STDERR},
		.opt = {.has_arg = ARG_MANDATORY, .name = "stderr"}
	},				{
		.rvopt = {.type = CFG_LOGFILE},
		.opt = {.has_arg = ARG_MANDATORY, .name = "log-file"}
	},				{
		.rvopt = {.type = CFG_LOGLEVEL},
		.opt = {.has_arg = ARG_MANDATORY, .name = "log-level"}
	}};

	static struct rvconfig cfg = {
		.optstring = "",
		.size = 5,
		.opts = opts,
	};

	opterr = 1;
	parsecfg(&cfg, argc, argv);
	set_globalcfg(&cfg);
	setdbgcfg();
}

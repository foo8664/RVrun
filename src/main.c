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

	if (!(proc = loadproc("test.elf")))
		return 1;

	ret = insn_fetch(proc, &insn);
	assert(ret == 4);
	dbg_log("insn 0x%.8x at addr 0x%lx", insn, proc->pc);
	assert(insn_func = insn_decode(insn));

	assert(insn_func(proc, insn) == 0);
	proc->pc += (unsigned )ret;

	freeproc(proc);
	return 0;
}

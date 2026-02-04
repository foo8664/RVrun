#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <assert.h>
#include "riscv.h"
#include "memory.h"
#include "loader.h"
#include "insn.h"

#include "debug.h"

int main(void)
{
	struct proc *proc;
	insn_func_t insn_func;
	insn_t insn;
	int ret;

	warn_log("Test warning, %s", "Test arg");
	panic("Testing panic");

	if (!(proc = loadproc("test.elf")))
		return 1;

	ret = insn_fetch(proc, &insn);
	assert(ret == 4);
	printf("insn 0x%.8x at addr 0x%lx\n", insn, proc->pc);
	assert(insn_func = insn_decode(insn));

	assert(insn_func(proc, insn) == 0);
	proc->pc += (unsigned )ret;

	freeproc(proc);
	return 0;
}

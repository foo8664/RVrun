#include <stdio.h>
#include "loader.h"

int main(void)
{
	struct proc *proc;

	if (!(proc = loadproc("test.elf")))
		return 1;

	printf("proc->pc=0x%x\n", proc->pc);
	for (int i = 0; i < 32; ++i)
		printf("proc->x%d=%x\n", i, proc->regs[i]);

	freeproc(proc);
	return 0;
}

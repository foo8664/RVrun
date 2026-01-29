#include <stdio.h>
#include "loader.h"

int main(void)
{
	struct proc *proc;
	struct memseg *seg;
	unsigned i;
	unsigned j;

	if (!(proc = loadproc("test.elf")))
		return 1;

	printf("proc->pc=0x%x\n", proc->pc);
	for (i = 0; i < 32; ++i)
		printf("proc->x%d=%x\n", i, proc->regs[i]);

	for (seg = proc->mem.segments, i = 0; seg; seg = seg->next, ++i) {
		printf("segment(%d)->start = 0x%x\n", i, seg->start);
		printf("segment(%d)->end = 0x%x\n", i, seg->end);
		printf("segment(%d)->flags = 0x%hhx\n", i, seg->flags);
		printf("segment(%d)->mem = %p\n", i, seg->mem);

		/* Pipe to file */
		fprintf(stderr, "segment(%d):", i);
		for (j = 0; j < seg->end - seg->start - 1; ++j)
			fputc(seg->mem[j], stderr);
	}

	freeproc(proc);
	return 0;
}

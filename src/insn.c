#include <errno.h>
#include "riscv.h"
#include "memory.h"
#include "loader.h"
#include "insn.h"

int insn_fetch(struct proc *proc, insn_t *insn)
{
	struct memseg *seg;

	seg = is_memseg(proc->mem, proc->pc, proc->pc + sizeof(insn) + 1);
	if (!seg) {
		errno = EINVAL;
		return -1;
	} else if (!(seg->flags & MEM_READ) || !(seg->flags & MEM_EXEC)) {
		errno = EPERM;
		return -1;
	}

	*insn = (insn_t)
		(((insn_t)seg->mem[proc->pc - seg->start]) 		|
		((insn_t)seg->mem[proc->pc - seg->start + 1] << 8) 	|
		((insn_t)seg->mem[proc->pc - seg->start + 2] << 16) 	|
		((insn_t)seg->mem[proc->pc - seg->start + 3] << 24));
	return 4;
}

#include <errno.h>
#include <stddef.h>
#include "riscv.h"
#include "rv_i.h"
#include "proc.h"
#include "debug.h"
#include "insn.h"

int insn_fetch(struct proc *proc, insn_t *insn)
{
	struct memseg *seg;

	seg = is_memseg(proc->mem, proc->pc, proc->pc + sizeof(*insn) + 1);
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

#define ADD_INSN(name, fname) if (IS_INSN(insn, name)) return (fname)
int (*insn_decode(insn_t insn))(struct proc *, insn_t)
{

	ADD_INSN(ADD, insn_add);
	ADD_INSN(SLT, insn_slt);
	ADD_INSN(SLTU, insn_sltu);
	ADD_INSN(AND, insn_and);
	ADD_INSN(OR, insn_or);
	ADD_INSN(XOR, insn_xor);
	ADD_INSN(SLL, insn_sll);
	ADD_INSN(SRL, insn_srl);
	ADD_INSN(SRA, insn_sra);
	ADD_INSN(SUB, insn_sub);

	errno = ENOSYS;
	return NULL;
}
#undef ADD_INSN

#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <assert.h>
#include "riscv.h"
#include "memory.h"
#include "loader.h"

int main(void)
{
	struct proc *proc;
	uint8_t byte;
	uint16_t hword;
	uint32_t word;
	uint32_t insn;
	uint64_t qword;

	if (!(proc = loadproc("test.elf")))
		return 1;

	// Making sure it works
	proc->regs[REG_SP]--;
	byte = 5;
	assert(memstore(proc->mem, proc->regs[REG_SP], &byte) == 0);
	byte = 0;
	assert(memload(proc->mem, proc->regs[REG_SP], &byte) == 0);
	assert(byte == 5);
	proc->regs[REG_SP]++;

	assert(memload(proc->mem, proc->pc, &insn) == 0);
	printf("pc register (0x%lx) points at instruction 0x%x\n",
		proc->pc, insn);

	proc->regs[REG_SP] -= sizeof(qword);
	qword = 0x0123456789abcdef;
	assert(memstore(proc->mem, proc->regs[REG_SP], &qword) == 0);
	qword = 0;
	assert(memload(proc->mem, proc->regs[REG_SP], &qword) == 0);
	assert(qword == 0x0123456789abcdef);
	proc->regs[REG_SP] += sizeof(qword);

	// Ensuring it fails gracefully
	errno = 0;
	assert(memstore(proc->mem, proc->pc, &insn) == -1);
	assert(errno == EPERM);

	errno = 0;
	assert(memload(proc->mem, (rvaddr_t)NULL, &byte) == -1);
	assert(errno == EINVAL);

	errno = 0;
	assert(memload(proc->mem, proc->regs[REG_SP] - sizeof(byte) + 1, &byte)
			== -1 && errno == EINVAL);
	errno = 0;
	assert(memload(proc->mem, proc->regs[REG_SP] - sizeof(hword) + 1,
			&hword) == -1 && errno == EINVAL);
	errno = 0;
	assert(memload(proc->mem, proc->regs[REG_SP] - sizeof(word) + 1, &word)
			== -1 && errno == EINVAL);
	errno = 0;
	assert(memload(proc->mem, proc->regs[REG_SP] - sizeof(qword) + 1,
			&qword) == -1 && errno == EINVAL);

	freeproc(proc);
	return 0;
}

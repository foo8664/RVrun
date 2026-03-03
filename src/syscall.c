#include <stdbool.h>
#include "proc.h"
#include "syscall.h"

// Delegate the process freeing to the execution loop, allows cleaner exits
int rvsys_exit(struct proc *proc)
{
	proc->exitinfo.exited = true;
	proc->exitinfo.status = (int)proc->regs[REG_A0];
	return 0;
}

// RvRun does not support threads, these do essentialy the same
int rvsys_exit_group(struct proc *proc)
{
	proc->exitinfo.exited = true;
	proc->exitinfo.status = (int)proc->regs[REG_A0];
	return 0;
}

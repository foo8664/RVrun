#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include "memory.h"
#include "proc.h"
#include "syscall.h"
#include "debug.h"

// To call system calls
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>


static void *getbuff(struct proc *proc, rvaddr_t start, size_t n,
		     enum memflags pmask, enum memflags pmatch)
		   __attribute__((nonnull));

static char *getstr(struct proc *proc, rvaddr_t start, enum memflags pmask,
		    enum memflags pmatch) __attribute__((nonnull, malloc(free)));

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

int rvsys_write(struct proc *proc)
{
	ssize_t ret;
	size_t n;
	char *buff;
	int fd;

	if (proc->regs[REG_A0] > proc->fdinfo.fdmax) {
		err_log("rvsys_write(): Bad file descriptor %d",
			(int)proc->regs[REG_A0]);
		proc->regs[REG_A0] = (reg_t)-1;
		return 0;
	}

	fd = (int)proc->fdinfo.fds[proc->regs[REG_A0]];
	n = (size_t)proc->regs[REG_A2];
	buff = getbuff(proc, proc->regs[REG_A1], n, MEM_READ, MEM_READ);

	if (buff == NULL)
		err_log("rvsys_write(): getbuff(proc, 0x%lx, %zu, %d, %d) returned NULL",
			proc->regs[REG_A1], n, MEM_READ, MEM_READ);

	dbg_log("Emulating sys_write(%d (real fd %d), 0x%lx (real ptr %p), %zu)",
		fd, (int)proc->regs[REG_A0], proc->regs[REG_A1], buff, n);
	ret = syscall(SYS_write, fd, buff, n);
	dbg_log("Syscall returned %zi", (ssize_t)ret);

	proc->regs[REG_A0] = (reg_t)ret;
	return 0;
}

int rvsys_read(struct proc *proc)
{
	ssize_t ret;
	size_t n;
	char *buff;
	int fd;

	if (proc->regs[REG_A0] > proc->fdinfo.fdmax) {
		err_log("rvsys_read(): Bad file descriptor %d",
			(int)proc->regs[REG_A0]);
		proc->regs[REG_A0] = (reg_t)-1;
		return 0;
	}

	fd = (int)proc->fdinfo.fds[proc->regs[REG_A0]];
	n = (size_t)proc->regs[REG_A2];
	buff = getbuff(proc, proc->regs[REG_A1], n, MEM_WRITE, MEM_WRITE);

	if (buff == NULL)
		err_log("rvsys_read(): getbuff(proc, 0x%lx, %zu, %d, %d) returned NULL",
			proc->regs[REG_A1], n, MEM_READ, MEM_READ);

	dbg_log("Emulating sys_read(%d, 0x%lx (real ptr %p), %zu)", fd,
		proc->regs[REG_A1], buff, n);
	ret = syscall(SYS_read, fd, buff, n);
	dbg_log("Syscall returned %zi", (ssize_t)ret);

	proc->regs[REG_A0] = (reg_t)ret;
	return 0;
}

int rvsys_openat(struct proc *proc)
{
	size_t i;
	long ret;
	char *pathname;
	mode_t mode;
	int flags;
	int dfd;

	pathname = getstr(proc, (rvaddr_t)proc->regs[REG_A1], MEM_READ, MEM_READ);
	if (!pathname) {
		err_log("Could not get pathname at 0x%lx", proc->regs[REG_A1]);
		return -1;
	}

	dfd = (int)proc->regs[REG_A0];
	flags = (int)proc->regs[REG_A2];
	mode = (mode_t)proc->regs[REG_A3];

	if (dfd != AT_FDCWD && pathname[0] != '/') {
		if (dfd < 0 || (size_t)dfd > proc->fdinfo.fdmax) {
			err_log("rvsys_openat(): Invalid dfd argument %d", dfd);
			dfd = -1;
		} else {
			dfd = proc->fdinfo.fds[dfd];
		}
	}

	dbg_log("Emulating sys_openat(%d (real fd %d) 0x%lx (real ptr %p: \"%s\"), %d, 0x%x)",
		(int)proc->regs[REG_A0], dfd, proc->regs[REG_A1], pathname,
		pathname, flags, mode);
	ret = syscall(SYS_openat, dfd, pathname, flags, mode);
	dbg_log("Syscall returned %ld", ret);

	if (ret > 0 && (size_t)ret > proc->fdinfo.fdmax) {
		err_log("Syscall returned fd too high (%ld), max is %zu", ret,
			proc->fdinfo.fdmax);
		free(pathname);
		close((int)ret);
		proc->regs[REG_A0] = (reg_t)-1;
		return 0;
	}

	for (i = 0; i < proc->fdinfo.fdmax && proc->fdinfo.fds[i] != -1; ++i)
		;
	proc->fdinfo.fds[i] = (int)ret;
	proc->regs[REG_A0] = (reg_t)i;

	free(pathname);
	return 0;
}

int rvsys_close(struct proc *proc)
{
	long ret;
	int fd;

	fd = (int)proc->regs[REG_A0];
	if ((size_t)fd > proc->fdinfo.fdmax) {
		err_log("rvsys_close(): Bad file descriptor %d", fd);
		proc->regs[REG_A0] = (reg_t)-1;
		return 0;
	}

	dbg_log("Emulating sys_close(%d (real fd %d))", fd, proc->fdinfo.fds[fd]);
	ret = syscall(SYS_close, proc->fdinfo.fds[fd]);
	proc->fdinfo.fds[fd] = -1;
	dbg_log("Syscall returned %ld", ret);

	proc->regs[REG_A0] = (reg_t)ret;
	return 0;
}

static void *getbuff(struct proc *proc, rvaddr_t start, size_t n,
		   enum memflags pmask, enum memflags pmatch)
{
	struct memseg *seg;

	seg = is_memseg(proc->mem, start, start + n);
	if (!seg)
		return NULL;
	if ((seg->flags & pmask) != pmatch) {
		errno = EPERM;
		return NULL;
	}

	return (char *)(seg->mem + start - seg->start);
}

static char *getstr(struct proc *proc, rvaddr_t start, enum memflags pmask,
		    enum memflags pmatch)
{
	struct memseg *seg;
	char *str;
	char *tmp;
	rvaddr_t addr;
	size_t i;
	size_t allocn;

	seg = is_memseg(proc->mem, start, start + 1);
	if (!seg)
		return NULL;
	if ((seg->flags & pmask) != pmatch) {
		errno = EPERM;
		return NULL;
	}

	if (!(str = malloc((allocn = 32) * sizeof(*str))))
		return NULL;

	for (addr = start, i = 0; addr < seg->end; ++addr, ++i) {
		if (i >= allocn) {
			if (!(tmp = realloc(str, (allocn *= 2) * sizeof(*str)))) {
				free(str);
				return NULL;
			}
			str = tmp;
		}

		if ((str[i] = (char)seg->mem[addr  - seg->start]) == '\0')
			break;
	}

	if (!(tmp = realloc(str, (i + 1) * sizeof(*str)))) {
		free(str);
		return NULL;
	}

	return tmp;
}

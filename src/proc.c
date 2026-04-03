/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Process loading and simulating
 *
 *  Copyright (C) 2026 by Diego Oliveira <di.diegoevaristo@gmail.com>
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include "config.h"

// For resource initialization, copy emulator's soft limits
#include <sys/types.h>
#include <sys/resource.h>
#include <unistd.h>

#include <elf.h>
#include "proc.h"

#include <errno.h>
#include "debug.h"


static int elfparse(FILE *file, struct proc *proc, const char *path)
					__attribute__((nonnull, cold));
static int loadseg(FILE *fp, Elf64_Phdr elfph, struct proc *proc)
					__attribute__((nonnull, cold));
static int loadstack(struct proc *proc)	__attribute__((nonnull, cold));
static int loadfds(struct proc *proc)	__attribute__((nonnull, cold));

struct proc *loadproc(const char *path)
{
	FILE *fp = NULL;
	struct proc *proc = NULL;

	if (!(fp = fopen(path, "rb")))
		goto err_out;
	if (!(proc = calloc(1, sizeof(*proc))))
		goto err_out;
	if (elfparse(fp, proc, path))
		goto err_out;
	if (loadstack(proc))
		goto err_out;
	if (loadfds(proc))
		goto err_out;

	proc->exitinfo.exited = false;

	fclose(fp);
	return proc;

err_out:
	if (fp)
		fclose(fp);
	if (proc)
		free(proc);
	return NULL;
}

void freeproc(struct proc *proc)
{
	size_t i;

	if (proc->fdinfo.fds) {
		for (i = 3; i < proc->fdinfo.fdmax; ++i)
			if (proc->fdinfo.fds[i] != -1)
				close(proc->fdinfo.fds[i]);
		free(proc->fdinfo.fds);
	}

	freemem(&proc->mem);
	free(proc);
}

static int elfparse(FILE *file, struct proc *proc, const char *path)
{
	Elf64_Ehdr elfh;
	Elf64_Phdr elfph;
	int err;

	if (fread(&elfh, sizeof(elfh), 1, file) != 1)
		return -1;

	// Checking if ELF file is correct
	if (memcmp(elfh.e_ident, ELFMAG, 4) != 0) {
		err_log("%s: wrong magic number for ELF", path);
		return 1;
	} else if (elfh.e_ident[EI_CLASS] != ELFCLASS64) {
		err_log("%s: not 64bit ELF", path);
		return 1;
	} else if (elfh.e_ident[EI_DATA] != ELFDATA2LSB) {
		err_log("%s: no two compliment and little indian", path);
		return 1;
	} else if (elfh.e_ident[EI_VERSION] == EV_NONE) {
		err_log("%s: invalid ELF version", path);
		return 1;
	} else if (elfh.e_ident[EI_OSABI] != ELFOSABI_SYSV &&
		   elfh.e_ident[EI_OSABI] == ELFOSABI_LINUX) {
		err_log("%s: wrong ABI", path);
		return 1;
	} else if (elfh.e_type != ET_EXEC) {
		err_log("%s: file is not executable (ELF field)", path);
		return 1;
	// EM_RISCV is not in elf(5), but is in RISC-V ABI spec page 23
	} else if (elfh.e_machine != EM_RISCV) {
		err_log("%s: not a RISC-V file", path);
		return 1;
	} else if (elfh.e_version == EV_NONE) {
		err_log("%s: invalid file version", path);
		return 1;
	}

	for (uint16_t i = 0; i < elfh.e_phnum; ++i) {
		if (fseek(file, (long int)(elfh.e_phoff +
		    (Elf64_Off)(i * elfh.e_phentsize)), SEEK_SET) != 0) {
			err_log("elfparse: fseek error: %s", strerror(errno));
			return 1;
		}

		if (fread(&elfph, sizeof(elfph), 1, file) != 1) {
			err_log("elfparse: fread error: %s", strerror(errno));
			return 1;
		}
		if ((err = loadseg(file, elfph, proc)) != 0) {
			err_log("%s: elfparse: Could not load segment", path);
			return err;
		}
	}

	proc->pc = elfh.e_entry;
	return 0;
}

static int loadseg(FILE *fp, Elf64_Phdr elfph, struct proc *proc)
{
	struct memseg *seg;
	uint8_t flags = 0;

	if (elfph.p_type != PT_LOAD)
		return 0;

	// TODO: check elfph.p_align correctness
	flags |= (elfph.p_flags & PF_R) ? MEM_READ : 0;
	flags |= (elfph.p_flags & PF_W) ? MEM_WRITE : 0;
	flags |= (elfph.p_flags & PF_X) ? MEM_EXEC : 0;

	dbg_log("Loading file contents from offset 0x%lx to 0x%lx into address"
		" 0x%lx to 0x%lx, flags=0x%hhx", elfph.p_offset,
		elfph.p_offset + elfph.p_filesz, elfph.p_vaddr,
		elfph.p_vaddr + elfph.p_memsz, flags);

	if (!(seg = addseg(&proc->mem, elfph.p_vaddr, elfph.p_vaddr +
	    elfph.p_memsz + 1, flags))) {
		err_log("loadseg(): Allocation error: %s", strerror(errno));
		return 1;
	}

	if (elfph.p_filesz > elfph.p_memsz) {
		err_log("loadseg(): file bigger than memsz");
		freemem(&proc->mem);
		return 1;
	}

	if (fseek(fp, (long int)elfph.p_offset, SEEK_SET) == -1) {
		err_log("loadseg(): fseek error: %s", strerror(errno));
		freemem(&proc->mem);
		return 1;
	}

	if (fread(seg->mem, 1, elfph.p_filesz, fp) != elfph.p_filesz) {
		err_log("loadseg(): fread error: %s", strerror(errno));
		freemem(&proc->mem);
		return 1;
	}

	return 0;
}

static int loadstack(struct proc *proc)
{
	struct rlimit slimit;
	rvaddr_t start;

	assert(sizeof(slimit.rlim_cur) <= sizeof(start));
	assert(sizeof(random()) <= sizeof(start));

	if (getrlimit(RLIMIT_STACK, &slimit) == -1) {
		err_log("loadstack(): getrlimit(): %s", strerror(errno));
		return 1;
	}

	if (slimit.rlim_cur == RLIM_INFINITY) {
		slimit.rlim_cur = 2 * (1024 * 1024); // 2Mb stack
		warn_log("loadstack(): Stack size soft limit is \"infinite\", "
			 "reducing to %d", (int)slimit.rlim_cur);
	}

	srandom((unsigned)time(NULL));
	do {
		start = (rvaddr_t)random();
		if (start + slimit.rlim_cur < start)
			continue;
	} while (is_memseg(proc->mem, start, start + slimit.rlim_cur + 1));

	info_log("Loading stack of %lu bytes on addr 0x%lx",
		 slimit.rlim_cur, start);

	if (!addseg(&proc->mem, start, start + slimit.rlim_cur + 1,
	    MEM_READ | MEM_WRITE)) {
		err_log("loadstack(): Allocation error: %s", strerror(errno));
		return 1;
	}

	proc->regs[REG_SP] = start + slimit.rlim_cur;
	return 0;
}

static int loadfds(struct proc *proc)
{
	struct rlimit fdlimit;
	struct rvopt *opt;

	// Tries to copy emulator's soft limit, defaults to 512 in failure
	proc->fdinfo.fdmax = 512;
	if (getrlimit(RLIMIT_NOFILE, &fdlimit) == -1) {
		warn_log("loadfds(): Could not query max fd soft limit, defaulting to %zu. Error: %s", proc->fdinfo.fdmax,
			 strerror(errno));

	} else if (fdlimit.rlim_cur == RLIM_INFINITY) {
		warn_log("loadfds(): Soft limit of file descriptors is \"infinity\", reducing to %zu", proc->fdinfo.fdmax);
	} else {
		proc->fdinfo.fdmax = fdlimit.rlim_cur;
	}

	if (!(proc->fdinfo.fds = malloc(proc->fdinfo.fdmax * sizeof(
					*proc->fdinfo.fds)))) {
		err_log("loadfds(): malloc(): %s", strerror(errno));
		return -1;
	}

	memset(proc->fdinfo.fds, -1, proc->fdinfo.fdmax * sizeof(*proc->fdinfo.fds));

	// Setting stdin, stdout, and stderr of proc. Defaults to emulator's
	if ((opt = getcfgopt(&globalcfg, CFG_STDIN)) && opt->set)
		proc->fdinfo.fds[STDIN_FILENO] = opt->u.integer;
	else
		proc->fdinfo.fds[STDIN_FILENO] = STDIN_FILENO;
	if ((opt = getcfgopt(&globalcfg, CFG_STDOUT)) && opt->set)
		proc->fdinfo.fds[STDOUT_FILENO] = opt->u.integer;
	else
		proc->fdinfo.fds[STDOUT_FILENO] = STDOUT_FILENO;
	if ((opt = getcfgopt(&globalcfg, CFG_STDERR)) && opt->set)
		proc->fdinfo.fds[STDERR_FILENO] = opt->u.integer;
	else
		proc->fdinfo.fds[STDERR_FILENO] = STDERR_FILENO;

	return 0;
}

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

// For stack initialization, copy emulator's soft limit
#include <sys/types.h>
#include <sys/resource.h>

#include <elf.h>
#include "proc.h"

#include <errno.h>
#include "debug.h"


static int elfparse(FILE *file, struct proc *proc, const char *path)
	__attribute__((nonnull, cold));
static int loadseg(FILE *fp, Elf64_Phdr elfph, struct proc *proc)
	__attribute__((nonnull, cold));
static int loadstack(struct proc *proc)
	__attribute__((nonnull, cold));

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

	if (slimit.rlim_cur == RLIM_INFINITY)
		slimit.rlim_cur = 2 * (1024 * 1024); // 2Mb stack

	srandom((unsigned)time(NULL));
	do {
		start = (rvaddr_t)random();
		if (start + slimit.rlim_cur < start)
			continue;
	} while (is_memseg(proc->mem, start, start + slimit.rlim_cur + 1));

	if (!addseg(&proc->mem, start, start + slimit.rlim_cur + 1,
	    MEM_READ | MEM_WRITE)) {
		err_log("loadstack(): Allocation error: %s", strerror(errno));
		return 1;
	}

	proc->regs[REG_SP] = start + slimit.rlim_cur;
	return 0;
}

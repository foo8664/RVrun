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
#include "loader.h"

enum LOAD_ERR {
	ELF_NOT_EXEC=1,
	ELF_NOT_RISCV,
	ELF_NOT_SYSV,
	ELF_NOT_MAGIC,
	ELF_NOT_VERSION,
	ELF_NOT_FVERSION,
	ELF_NOT_64BIT,
	ELF_NOT_LITTLE,
	ELF_SEGMENT_ALLOCFAIL,
	ELF_SEGMENT_MEMTOOSMALL,
	ELF_SEGMENT_CANTOFFSET,
	ELF_SEGMENT_CANTREAD,
	PROC_CANNOT_QUERYSTACKSZ,
	PROC_CANNOT_ALLOCSTACK,
};

static const char elfmag[] = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};

static int elfparse(FILE *file, struct proc *proc)
	__attribute__((nonnull, cold));
static int loadseg(FILE *fp, Elf64_Phdr elfph, struct proc *proc)
	__attribute__((nonnull, cold));
static int loadstack(struct proc *proc)
	__attribute__((nonnull, cold));
static void loader_err(const char *path, enum LOAD_ERR e)
	__attribute__((nonnull, cold));

struct proc *loadproc(const char *path)
{
	FILE *fp = NULL;
	struct proc *proc = NULL;
	enum LOAD_ERR err;

	if (!(fp = fopen(path, "rb")))
		goto err_out;
	if (!(proc = calloc(1, sizeof(*proc))))
		goto err_out;
	if ((err = elfparse(fp, proc)) != 0) {
		loader_err(path, err);
		goto err_out;
	}

	if ((err = loadstack(proc)) != 0) {
		loader_err(path, err);
		goto err_out;
	}

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

static int elfparse(FILE *file, struct proc *proc)
{
	Elf64_Ehdr elfh;
	Elf64_Phdr elfph;
	enum LOAD_ERR err;

	if (fread(&elfh, sizeof(elfh), 1, file) != 1)
		return -1;

	// Checking if ELF file is correct
	if (memcmp(elfh.e_ident, elfmag, sizeof(elfmag)) != 0)
		return ELF_NOT_MAGIC;
	else if (elfh.e_ident[EI_CLASS] != ELFCLASS64)
		return ELF_NOT_64BIT;
	else if (elfh.e_ident[EI_DATA] != ELFDATA2LSB)
		return ELF_NOT_LITTLE;
	else if (elfh.e_ident[EI_VERSION] == EV_NONE)
		return ELF_NOT_VERSION;
	else if (elfh.e_ident[EI_OSABI] != ELFOSABI_SYSV)
		return ELF_NOT_SYSV;
	else if (elfh.e_type != ET_EXEC)
		return ELF_NOT_EXEC;
	// EM_RISCV is not in elf(5), but is in RISC-V ABI spec page 23
	else if (elfh.e_machine != EM_RISCV)
		return ELF_NOT_RISCV;
	else if (elfh.e_version == EV_NONE)
		return ELF_NOT_FVERSION;

	for (uint16_t i = 0; i < elfh.e_phnum; ++i) {
		if (fseek(file, (long int)(elfh.e_phoff +
		   (Elf64_Off)(i * elfh.e_phentsize)), SEEK_SET) != 0)
			return ELF_SEGMENT_CANTOFFSET;
		if (fread(&elfph, sizeof(elfph), 1, file) != 1)
			return ELF_SEGMENT_CANTOFFSET;
		if ((err = loadseg(file, elfph, proc)) != 0)
			return err;
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
	    elfph.p_memsz + 1, flags)))
		return ELF_SEGMENT_ALLOCFAIL;

	if (elfph.p_filesz > elfph.p_memsz) {
		freemem(&proc->mem);
		return ELF_SEGMENT_MEMTOOSMALL;
	}

	if (fseek(fp, (long int)elfph.p_offset, SEEK_SET) == -1) {
		freemem(&proc->mem);
		return ELF_SEGMENT_CANTOFFSET;
	}

	if (fread(seg->mem, 1, elfph.p_filesz, fp) != elfph.p_filesz) {
		freemem(&proc->mem);
		return ELF_SEGMENT_CANTREAD;
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
		perror("loadstack(): getrlimit()");
		return PROC_CANNOT_QUERYSTACKSZ;
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
	   MEM_READ | MEM_WRITE))
		return PROC_CANNOT_ALLOCSTACK;

	proc->regs[REG_SP] = start + slimit.rlim_cur;
	return 0;
}

static void loader_err(const char *path, enum LOAD_ERR e)
{
	char *msg = "";

	switch (e) {
	case ELF_NOT_VERSION:
		msg = "Invalid ELF version";
		break;
	case ELF_NOT_FVERSION:
		msg = "Invalid file version";
		break;
	case ELF_NOT_RISCV:
		msg = "File is not for RISC-V";
		break;
	case ELF_NOT_EXEC:
		msg = "File is not executable";
		break;
	case ELF_NOT_MAGIC:
		msg = "Can't find ELF magic numbers";
		break;
	case ELF_NOT_64BIT:
		msg = "RISC-V ISA in use is not 64bit";
		break;
	case ELF_NOT_LITTLE:
		msg = "Cannot emulate non-little-endian code";
		break;
	case ELF_NOT_SYSV:
		msg = "Cannot emulate files that don't follow the system V ABI";
		break;
	case ELF_SEGMENT_CANTOFFSET:
		msg = "Cannot offset into file";
		break;
	case ELF_SEGMENT_CANTREAD:
		msg = "Cannot read file";
		break;
	case ELF_SEGMENT_ALLOCFAIL:
		msg = "Failed to allocate segment";
		break;
	case ELF_SEGMENT_MEMTOOSMALL:
		msg = "Memory is too small for segment";
		break;
	case PROC_CANNOT_QUERYSTACKSZ:
		msg = "Cannot query stack size";
		break;
	case PROC_CANNOT_ALLOCSTACK:
		msg = "Cannot alocate stack for the process";
		break;
	default:
		msg = "Unknown error";
		break;
	}

	fprintf(stderr, "%s: %s\n", path, msg);
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <elf.h>
#include "loader.h"

enum LOAD_ERR {
	ELF_NOT_EXEC=1,
	ELF_NOT_RISCV,
	ELF_NOT_SYSV,
	ELF_NOT_MAGIC,
	ELF_NOT_VERSION,
	ELF_NOT_FVERSION,
	ELF_NOT_32BIT,
	ELF_NOT_LITTLE,
};

static const char elfmag[] = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};

static int elfparse(FILE *file, struct proc *proc) __attribute__((nonnull,cold));
static void loaderr(const char *path, enum LOAD_ERR e) __attribute__((nonnull));

struct proc *loadproc(const char *path)
{
	FILE *fp = NULL;
	struct proc *proc = NULL;
	enum LOAD_ERR err;

	if (!(fp = fopen(path, "rb")))
		goto err_out;
	if (!(proc = malloc(sizeof(*proc))))
		goto err_out;
	if ((err = elfparse(fp, proc)) != 0) {
		loaderr(path, err);
		goto err_out;
	}

	memset(proc->regs, 0, sizeof(proc->regs));
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
	free(proc);
}

static int elfparse(FILE *file, struct proc *proc)
{
	Elf32_Ehdr elfh;

	if (fread(&elfh, sizeof(elfh), 1, file) != 1)
		return -1;

	// Checking if ELF file is correct
	if (memcmp(elfh.e_ident, elfmag, sizeof(elfmag)) != 0)
		return ELF_NOT_MAGIC;
	else if (elfh.e_ident[EI_CLASS] != ELFCLASS32)
		return ELF_NOT_32BIT;
	else if (elfh.e_ident[EI_DATA] != ELFDATA2LSB)
		return ELF_NOT_LITTLE;
	else if (elfh.e_ident[EI_VERSION] == EV_NONE)
		return ELF_NOT_VERSION;
	else if (elfh.e_ident[EI_OSABI] != ELFOSABI_SYSV)
		return ELF_NOT_SYSV;
	else if (elfh.e_type != ET_EXEC)
		return ELF_NOT_EXEC;
	// EM_RISCV is not in elf(5), but is in RISC-V ISA spec page 23
	else if (elfh.e_machine != EM_RISCV)
		return ELF_NOT_RISCV;
	else if (elfh.e_version == EV_NONE)
		return ELF_NOT_FVERSION;

	proc->pc = elfh.e_entry;
	return 0;
}

static void loaderr(const char *path, enum LOAD_ERR e)
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
	case ELF_NOT_32BIT:
		msg = "RISC-V ISA in use is not 32bit";
		break;
	case ELF_NOT_LITTLE:
		msg = "Cannot emulate non-little-endian code";
		break;
	case ELF_NOT_SYSV:
		msg = "Cannot emulate files that don't follow the system V ABI";
		break;
	default:
		msg = "Unknown error";
		break;
	}

	fprintf(stderr, "%s: %s\n", path, msg);
}

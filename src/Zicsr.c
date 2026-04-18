/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Zicsr instructions
 *
 *  Copyright (C) 2026 by Diego Evaristo <di.diegoevaristo@gmail.com>
 */

#include "common.h"
#include "riscv.h"
#include "opcodes.h" // CSR_* definitions
#include "debug.h"
#include "proc.h"
#include "rv_f.h"

static inline void csr_getfields(insn_t insn, enum ABI_REG *rd, enum ABI_REG *rs1,
				csr_t *csr) ATTRIBUTE(nonnull);


static inline void csr_getfields(insn_t insn, enum ABI_REG *rd, enum ABI_REG *rs1,
				csr_t *csr)
{
	*rd =  (enum ABI_REG)((insn & 0xf80)	>> 7);
	*rs1 = (enum ABI_REG)((insn & 0xf8000)	>> 15);
	*csr = (csr_t)((insn & 0xfff00000)	>> 20);
}

int insn_csrrw(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	csr_t csr;

	csr_getfields(insn, &rd, &rs1, &csr);

	switch (csr) {
	case CSR_FCSR:
	case CSR_FFLAGS:
	case CSR_FRM:
		return fcsr_csrrw(proc, rd, rs1, csr);
	default:
		err_log("0x%lx: csrrw: Unsupported csr of 0x%hx",
			proc->pc, csr);
		return -1;
	}
}

int insn_csrrs(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	csr_t csr;

	csr_getfields(insn, &rd, &rs1, &csr);

	switch (csr) {
	case CSR_FCSR:
	case CSR_FFLAGS:
	case CSR_FRM:
		return fcsr_csrrs(proc, rd, rs1, csr);
	default:
		err_log("0x%lx: csrrs: Unsupported csr of 0x%hx",
			proc->pc, csr);
		return -1;
	}
}

int insn_csrrc(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	csr_t csr;

	csr_getfields(insn, &rd, &rs1, &csr);

	switch (csr) {
	case CSR_FCSR:
	case CSR_FFLAGS:
	case CSR_FRM:
		return fcsr_csrrc(proc, rd, rs1, csr);
	default:
		err_log("0x%lx: csrrc: Unsupported csr of 0x%hx",
			proc->pc, csr);
		return -1;
	}
}

int insn_csrrwi(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	csr_t csr;

	csr_getfields(insn, &rd, &rs1, &csr);

	switch (csr) {
	case CSR_FCSR:
		return fcsr_csrrwi(proc, rd, rs1, csr);
	default:
		err_log("0x%lx: csrrwi: Unsupported csr of 0x%hx",
			proc->pc, csr);
		return -1;
	}
}

int insn_csrrsi(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	csr_t csr;

	csr_getfields(insn, &rd, &rs1, &csr);

	switch (csr) {
	case CSR_FCSR:
	case CSR_FFLAGS:
	case CSR_FRM:
		return fcsr_csrrsi(proc, rd, rs1, csr);
	default:
		err_log("0x%lx: csrrsi: Unsupported csr of 0x%hx",
			proc->pc, csr);
		return -1;
	}
}

int insn_csrrci(struct proc *proc, insn_t insn)
{
	enum ABI_REG rd;
	enum ABI_REG rs1;
	csr_t csr;

	csr_getfields(insn, &rd, &rs1, &csr);

	switch (csr) {
	case CSR_FCSR:
	case CSR_FFLAGS:
	case CSR_FRM:
		return fcsr_csrrci(proc, rd, rs1, csr);
	default:
		err_log("0x%lx: csrrci: Unsupported csr of 0x%hx",
			proc->pc, csr);
		return -1;
	}
}

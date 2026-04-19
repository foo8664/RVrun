/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * RISC-V F extension
 *
 *  Copyright (C) 2026 by Diego Evaristo <di.diegoevaristo@gmail.com>
 */

#include <stdint.h>
#include <fenv.h>
#include <math.h>
#include "riscv.h"
#include "common.h"
#include "debug.h"
#include "proc.h"
#include "opcodes.h" // CSR_.* definitions
#include "rv_f.h"

#include <xmmintrin.h> // Disable FTZ (Flush-Set-Zero)
#include <pmmintrin.h> // Disable DAZ (Dernomals-Are-Zero)

#define SETBITS(bits, mask) ((bits) | (mask))
#define CLEARBITS(bits, mask) ((bits) & ~(mask))

// Returns 1 if dym, 0 if not (and then propagates)
static int propagate_frm(uint32_t fcsr);

static int propagate_frm(uint32_t fcsr)
{
	switch (GET_FRM(fcsr)) {
	case RISCV_FRND_RNE:
		if (fesetround(FE_TONEAREST) != 0)
			panic("fsetround() failed");
		return 0;
	case RISCV_FRND_RTZ:
		if (fesetround(FE_TOWARDZERO) != 0)
			panic("fsetround() failed");
		return 0;
	case RISCV_FRND_RDN:
		if (fesetround(FE_DOWNWARD) != 0)
			panic("fsetround() failed");
		return 0;
	case RISCV_FRND_RUP:
		if (fesetround(FE_UPWARD) != 0)
			panic("fsetround() failed");
		return 0;
	case RISCV_FRND_DYN:
		return 1;
	default:
		return 0;
	}
}

// Must be ran before any floating point operations
void riscv_float_setup(struct proc *proc)
{
	if (fesetenv(FE_DFL_ENV) != 0)
		panic("fsetenv() failed");
	if (feclearexcept(FE_ALL_EXCEPT) != 0)
		panic("fclearexcept() failed");
	proc->fcsr = 0;
	propagate_frm(proc->fcsr);

	// GCC complains about (apparently) unfixible sign conversions
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_OFF);
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_OFF);
#pragma GCC diagnostic pop

	dbg_log("Disabled FTZ and DAZ for IEE-754-2008 compliance, and "
		"initialized floating point environment");
}

int fcsr_csrrw(struct proc *proc, enum ABI_REG rd, enum ABI_REG rs1, csr_t csr)
{
	switch (csr) {
	case CSR_FCSR:
		mvreg(proc, rd, zextend32to64(proc->fcsr & 0xff));
		proc->fcsr = (uint32_t)(getreg(proc, rs1) & 0xff);
		return propagate_frm(proc->fcsr);
	case CSR_FRM:
		mvreg(proc, rd, zextend8to64(GET_FRM(proc->fcsr)));
		WRITE_FRM(proc->fcsr, getreg(proc, rs1));
		return propagate_frm(proc->fcsr);
	case CSR_FFLAGS:
		mvreg(proc, rd, zextend8to64(GET_FFLAGS(proc->fcsr)));
		WRITE_FFLAGS(proc->fcsr, getreg(proc, rs1));
		return 0;
	}

	err_log("0x%lx: Calling fcsr_csrrw() with wrong csr argument 0x%hx",
		proc->pc, csr);
	return -1;
}

int fcsr_csrrs(struct proc *proc, enum ABI_REG rd, enum ABI_REG rs1, csr_t csr)
{
	switch (csr) {
	case CSR_FCSR:
		mvreg(proc, rd, zextend32to64(proc->fcsr & 0xff));
		if (rs1) {
			proc->fcsr = SETBITS(proc->fcsr, (uint32_t)(getreg(proc, rs1) & 0xff));
			return propagate_frm(proc->fcsr);
		}
		return 0;
	case CSR_FRM:
		mvreg(proc, rd, zextend8to64(GET_FRM(proc->fcsr)));
		if (rs1) {
			WRITE_FRM(proc->fcsr, SETBITS(GET_FRM(proc->fcsr),
				  (uint32_t)getreg(proc, rs1)));
			return propagate_frm(proc->fcsr);
		}
		return 0;
	case CSR_FFLAGS:
		mvreg(proc, rd, zextend8to64(GET_FFLAGS(proc->fcsr)));
		if (rs1) {
			WRITE_FFLAGS(proc->fcsr, SETBITS(GET_FFLAGS(proc->fcsr),
				     (uint32_t)getreg(proc, rs1)));
		}
		return 0;
	}

	err_log("0x%lx: Calling fcsr_csrrs() with wrong csr argument 0x%hx",
		proc->pc, csr);
	return -1;
}

int fcsr_csrrc(struct proc *proc, enum ABI_REG rd, enum ABI_REG rs1, csr_t csr)
{
	switch (csr) {
	case CSR_FCSR:
		mvreg(proc, rd, zextend32to64(proc->fcsr & 0xff));
		if (rs1) {
			proc->fcsr = CLEARBITS(proc->fcsr, (uint32_t)(getreg(proc, rs1) & 0xff));
			return propagate_frm(proc->fcsr);
		}
		return 0;
	case CSR_FRM:
		mvreg(proc, rd, zextend8to64(GET_FRM(proc->fcsr)));
		if (rs1) {
			WRITE_FRM(proc->fcsr, CLEARBITS(GET_FRM(proc->fcsr),
				  (uint32_t)getreg(proc, rs1)));
			return propagate_frm(proc->fcsr);
		}
		return 0;
	case CSR_FFLAGS:
		mvreg(proc, rd, zextend8to64(GET_FFLAGS(proc->fcsr)));
		if (rs1) {
			WRITE_FFLAGS(proc->fcsr, CLEARBITS(GET_FFLAGS(proc->fcsr),
				     (uint32_t)getreg(proc, rs1)));
		}
		return 0;
	}

	err_log("0x%lx: Calling fcsr_csrrc() with wrong csr argument 0x%hx",
		proc->pc, csr);
	return -1;
}

int fcsr_csrrwi(struct proc *proc, enum ABI_REG rd, enum ABI_REG rs1, csr_t csr)
{
	uint32_t imm = (uint32_t)rs1;

	switch (csr) {
	case CSR_FCSR:
		mvreg(proc, rd, zextend32to64(proc->fcsr & 0xff));
		proc->fcsr = imm;
		return propagate_frm(proc->fcsr);
	case CSR_FRM:
		mvreg(proc, rd, zextend8to64(GET_FRM(proc->fcsr)));
		WRITE_FRM(proc->fcsr, GET_FRM(imm));
		return propagate_frm(proc->fcsr);
	case CSR_FFLAGS:
		mvreg(proc, rd, zextend8to64(GET_FFLAGS(proc->fcsr)));
		WRITE_FFLAGS(proc->fcsr, GET_FFLAGS(imm));
		return 0;
	}

	err_log("0x%lx: Calling fcsr_csrrwi() with wrong csr argument 0x%hx",
		proc->pc, csr);
	return -1;
}

int fcsr_csrrsi(struct proc *proc, enum ABI_REG rd, enum ABI_REG rs1, csr_t csr)
{
	uint32_t imm = (uint32_t)rs1;

	switch (csr) {
	case CSR_FCSR:
		mvreg(proc, rd, zextend32to64(proc->fcsr & 0xff));
		if (imm) {
			proc->fcsr = SETBITS(proc->fcsr, imm);
			return propagate_frm(proc->fcsr);
		}
		return 0;
	case CSR_FRM:
		mvreg(proc, rd, zextend8to64(GET_FRM(proc->fcsr)));
		if (imm) {
			WRITE_FRM(proc->fcsr, SETBITS(proc->fcsr, GET_FRM(imm)));
			return propagate_frm(proc->fcsr);
		}
		return 0;
	case CSR_FFLAGS:
		mvreg(proc, rd, zextend8to64(GET_FFLAGS(proc->fcsr)));
		if (imm)
			WRITE_FFLAGS(proc->fcsr, SETBITS(proc->fcsr, GET_FFLAGS(imm)));
		return 0;
	}

	err_log("0x%lx: Calling fcsr_csrrsi() with wrong csr argument 0x%hx",
		proc->pc, csr);
	return -1;
}

int fcsr_csrrci(struct proc *proc, enum ABI_REG rd, enum ABI_REG rs1, csr_t csr)
{
	uint32_t imm = (uint32_t)rs1;

	switch (csr) {
	case CSR_FCSR:
		mvreg(proc, rd, zextend32to64(proc->fcsr & 0xff));
		if (imm) {
			proc->fcsr = CLEARBITS(proc->fcsr, imm);
			return propagate_frm(proc->fcsr);
		}
		return 0;
	case CSR_FRM:
		mvreg(proc, rd, zextend8to64(GET_FRM(proc->fcsr)));
		if (imm) {
			WRITE_FRM(proc->fcsr, CLEARBITS(proc->fcsr,
				  (uint32_t)GET_FRM(imm)));
			return propagate_frm(proc->fcsr);
		}
		return 0;
	case CSR_FFLAGS:
		mvreg(proc, rd, zextend8to64(GET_FFLAGS(proc->fcsr)));
		if (imm)
			WRITE_FFLAGS(proc->fcsr, CLEARBITS(proc->fcsr,
				     (uint32_t)GET_FFLAGS(imm)));
		return 0;
	}

	err_log("0x%lx: Calling fcsr_csrrci() with wrong csr argument 0x%hx",
		proc->pc, csr);
	return -1;
}

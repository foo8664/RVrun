/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * RISC-V F extension
 *
 *  Copyright (C) 2026 by Diego Evaristo <di.diegoevaristo@gmail.com>
 */

#include <stdbool.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
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

// Returns 1 if dyn, 0 if not (and then propagates for everything but RMM)
static int propagate_frm(uint32_t fcsr);

// Sets guest's fflags based on host floating-point exceptions.
static void propagate_fflags(uint32_t *fcsr);

// Gets width/rm field in an instruction
static inline uint8_t insn2rm(insn_t insn) ATTRIBUTE(const);

// Canonical NaN for single precision
static inline float canonical_nan(void) ATTRIBUTE(const);

// Equals -0.0 for single precision
static inline bool eq_minus_zero(float f) ATTRIBUTE(const);

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

static void propagate_fflags(uint32_t *fcsr)
{
	int host_excepts = fetestexcept(FE_ALL_EXCEPT);
	int guest_excepts = 0;

	guest_excepts |= (host_excepts & FE_INEXACT)	? RISCV_FEXCEPT_NX : 0;
	guest_excepts |= (host_excepts & FE_INVALID)	? RISCV_FEXCEPT_NV : 0;
	guest_excepts |= (host_excepts & FE_DIVBYZERO)	? RISCV_FEXCEPT_DZ : 0;
	guest_excepts |= (host_excepts & FE_OVERFLOW)	? RISCV_FEXCEPT_OF : 0;
	guest_excepts |= (host_excepts & FE_UNDERFLOW)	? RISCV_FEXCEPT_UF : 0;
	WRITE_FFLAGS(*fcsr, SETBITS(GET_FFLAGS(*fcsr), guest_excepts));
}

static inline uint8_t insn2rm(insn_t insn)
{
	return (uint8_t)((insn & 0x7000) >> 12);
}

static inline float canonical_nan(void)
{
	union {
		uint32_t hex;
		float f;
	} nan = {.hex = 0x7fc00000};

	return nan.f;
}

static inline bool eq_minus_zero(float f)
{
	uint32_t minus_zero = 0x80000000;

	_Static_assert(sizeof(minus_zero) == sizeof(f), "sizeof float is not 4");
	return memcmp(&f, &minus_zero, sizeof(minus_zero)) == 0;
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

int insn_flw(struct proc *proc, insn_t insn)
{
	uint8_t rd;
	uint8_t rs1;
	uint64_t imm;
	uint32_t val;
	freg_t fval;

	rd = insn2rd(insn);
	rs1 = insn2rs1(insn);
	imm = (insn & 0xfff00000) >> 20;
	imm = extend16to64((uint16_t)(imm | ((insn & (0x80000000)) ? 0xf000 : 0)));

	if (memload(proc->mem, getreg(proc, rs1) + imm, &val) == -1) {
		err_log("0x%lx: flw: Cannot load from 0x%lx", proc->pc, getreg(proc, rs1) + imm);
		return -1;
	}

	_Static_assert(sizeof(fval) == sizeof(val), "sizeof(freg_t) is not 4");
	memcpy(&fval, &val, sizeof(fval));
	mvfreg(proc, rd, fval);
	return 0.;
}

int insn_fsw(struct proc *proc, insn_t insn)
{
	uint8_t rs1;
	uint8_t rs2;
	uint64_t imm = 0;
	uint32_t val;
	freg_t fval;

	rs1 = insn2rs1(insn);
	rs2 = insn2rs2(insn);
	imm |= (insn & 0xf80) >> 7;
	imm |= (insn & 0xfe000000) >> 20;
	imm = extend16to64((uint16_t)(imm | ((insn & (0x80000000)) ? 0xf000 : 0)));

	_Static_assert(sizeof(val) == sizeof(fval), "sizeof(freg_t) is not 4");
	fval = getfreg(proc, rs2);
	memcpy(&val, &fval, sizeof(val));

	if (memstore(proc->mem, getreg(proc, rs1) + imm, &val) == -1) {
		err_log("0x%lx: fsw: Cannost store at 0x%lx", proc->pc, getreg(proc, rs1) + imm);
		return -1;
	}

	return 0;
}

int insn_fadd_s(struct proc *proc, insn_t insn)
{
	uint8_t rd;
	uint8_t rs1;
	uint8_t rs2;
	uint8_t rm;
	float res;

	rd = insn2rd(insn);
	rs1 = insn2rs1(insn);
	rs2 = insn2rs2(insn);
	rm = insn2rm(insn);

	feclearexcept(FE_ALL_EXCEPT);
	if (rm == RISCV_FRND_RMM ||
	   (rm == RISCV_FRND_DYN && GET_FRM(proc->fcsr) == RISCV_FRND_RMM)) {
		res = getfreg(proc, rs1) + getfreg(proc, rs2);
		propagate_fflags(&proc->fcsr);
		res = roundf(res);
	} else if (rm == RISCV_FRND_DYN) {
		res = getfreg(proc, rs1) + getfreg(proc, rs2);
		propagate_fflags(&proc->fcsr);
	} else {
		propagate_frm((uint32_t)rm  << 5);
		res = getfreg(proc, rs1) + getfreg(proc, rs2);
		propagate_fflags(&proc->fcsr);
		propagate_frm(proc->fcsr);
	}

	if (isnan(res))
		res = canonical_nan();

	mvfreg(proc, rd, res);
	return 0;
}

int insn_fsub_s(struct proc *proc, insn_t insn)
{
	uint8_t rd;
	uint8_t rs1;
	uint8_t rs2;
	uint8_t rm;
	float res;

	rd = insn2rd(insn);
	rs1 = insn2rs1(insn);
	rs2 = insn2rs2(insn);
	rm = insn2rm(insn);

	feclearexcept(FE_ALL_EXCEPT);
	if (rm == RISCV_FRND_RMM ||
	   (rm == RISCV_FRND_DYN && GET_FRM(proc->fcsr) == RISCV_FRND_RMM)) {
		res = getfreg(proc, rs1) - getfreg(proc, rs2);
		propagate_fflags(&proc->fcsr);
		res = roundf(res);
	} else if (rm == RISCV_FRND_DYN) {
		res = getfreg(proc, rs1) - getfreg(proc, rs2);
		propagate_fflags(&proc->fcsr);
	} else {
		propagate_frm((uint32_t)rm  << 5);
		res = getfreg(proc, rs1) - getfreg(proc, rs2);
		propagate_fflags(&proc->fcsr);
		propagate_frm(proc->fcsr);
	}

	if (isnan(res))
		res = canonical_nan();

	mvfreg(proc, rd, res);
	return 0;
}

int insn_fmul_s(struct proc *proc, insn_t insn)
{
	uint8_t rd;
	uint8_t rs1;
	uint8_t rs2;
	uint8_t rm;
	float res;

	rd = insn2rd(insn);
	rs1 = insn2rs1(insn);
	rs2 = insn2rs2(insn);
	rm = insn2rm(insn);

	feclearexcept(FE_ALL_EXCEPT);
	if (rm == RISCV_FRND_RMM ||
	   (rm == RISCV_FRND_DYN && GET_FRM(proc->fcsr) == RISCV_FRND_RMM)) {
		res = getfreg(proc, rs1) * getfreg(proc, rs2);
		propagate_fflags(&proc->fcsr);
		res = roundf(res);
	} else if (rm == RISCV_FRND_DYN) {
		res = getfreg(proc, rs1) * getfreg(proc, rs2);
		propagate_fflags(&proc->fcsr);
	} else {
		propagate_frm((uint32_t)rm  << 5);
		res = getfreg(proc, rs1) * getfreg(proc, rs2);
		propagate_fflags(&proc->fcsr);
		propagate_frm(proc->fcsr);
	}

	if (isnan(res))
		res = canonical_nan();

	mvfreg(proc, rd, res);
	return 0;
}

int insn_fdiv_s(struct proc *proc, insn_t insn)
{
	uint8_t rd;
	uint8_t rs1;
	uint8_t rs2;
	uint8_t rm;
	float res;

	rd = insn2rd(insn);
	rs1 = insn2rs1(insn);
	rs2 = insn2rs2(insn);
	rm = insn2rm(insn);

	feclearexcept(FE_ALL_EXCEPT);
	if (rm == RISCV_FRND_RMM ||
	   (rm == RISCV_FRND_DYN && GET_FRM(proc->fcsr) == RISCV_FRND_RMM)) {
		res = getfreg(proc, rs1) / getfreg(proc, rs2);
		propagate_fflags(&proc->fcsr);
		res = roundf(res);
	} else if (rm == RISCV_FRND_DYN) {
		res = getfreg(proc, rs1) / getfreg(proc, rs2);
		propagate_fflags(&proc->fcsr);
	} else {
		propagate_frm((uint32_t)rm  << 5);
		res = getfreg(proc, rs1) / getfreg(proc, rs2);
		propagate_fflags(&proc->fcsr);
		propagate_frm(proc->fcsr);
	}

	if (isnan(res))
		res = canonical_nan();

	mvfreg(proc, rd, res);
	return 0;
}

int insn_fsqrt_s(struct proc *proc, insn_t insn)
{
	uint8_t rd;
	uint8_t rs1;
	uint8_t rm;
	float res;

	rd = insn2rd(insn);
	rs1 = insn2rs1(insn);
	rm = insn2rm(insn);

	feclearexcept(FE_ALL_EXCEPT);
	if (rm == RISCV_FRND_RMM ||
	   (rm == RISCV_FRND_DYN && GET_FRM(proc->fcsr) == RISCV_FRND_RMM)) {
		res = sqrtf(getfreg(proc, rs1));
		propagate_fflags(&proc->fcsr);
		res = roundf(res);
	} else if (rm == RISCV_FRND_DYN) {
		res = sqrtf(getfreg(proc, rs1));
		propagate_fflags(&proc->fcsr);
	} else {
		propagate_frm((uint32_t)rm  << 5);
		res = sqrtf(getfreg(proc, rs1));
		propagate_fflags(&proc->fcsr);
		propagate_frm(proc->fcsr);
	}

	if (isnan(res))
		res = canonical_nan();

	return 0;
}

int insn_fmin_s(struct proc *proc, insn_t insn)
{
	uint8_t rd;
	uint8_t rs1;
	uint8_t rs2;
	float res;

	rd = insn2rd(insn);
	rs1 = insn2rs1(insn);
	rs2 = insn2rs2(insn);

	// Double NaN (Result is NaN)
	if (isnan(getfreg(proc, rs1)) && isnan(getfreg(proc, rs2))) {
		res = canonical_nan();
		WRITE_FFLAGS(proc->fcsr, GET_FFLAGS(proc->fcsr) | RISCV_FEXCEPT_NV);
	// One NaN (Result is non-NaN)
	} else if (isnan(getfreg(proc, rs1)) || isnan(getfreg(proc, rs2))) {
		res = isnan(getfreg(proc, rs1)) ? getfreg(proc, rs2) : getfreg(proc, rs1);
		WRITE_FFLAGS(proc->fcsr, GET_FFLAGS(proc->fcsr) | RISCV_FEXCEPT_NV);
	// -0.0 < 0.0 for FMIN and FMAX
	} else if (eq_minus_zero(getfreg(proc, rs1)) && getfreg(proc, rs2) == 0.0) {
		res = getfreg(proc, rs1);
	} else if (getfreg(proc, rs1) == 0.0 && eq_minus_zero(getfreg(proc, rs2))) {
		res = getfreg(proc, rs2);
	} else {
		res = min(getfreg(proc, rs1), getfreg(proc, rs2));
	}

	if (isnan(res))
		res = canonical_nan();

	mvfreg(proc, rd, res);
	return 0;
}

int insn_fmax_s(struct proc *proc, insn_t insn)
{
	uint8_t rd;
	uint8_t rs1;
	uint8_t rs2;
	float res;

	rd = insn2rd(insn);
	rs1 = insn2rs1(insn);
	rs2 = insn2rs2(insn);

	// Double NaN (Result is NaN)
	if (isnan(getfreg(proc, rs1)) && isnan(getfreg(proc, rs2))) {
		res = canonical_nan();
		WRITE_FFLAGS(proc->fcsr, GET_FFLAGS(proc->fcsr) | RISCV_FEXCEPT_NV);
	// One NaN (Result is non-NaN)
	} else if (isnan(getfreg(proc, rs1)) || isnan(getfreg(proc, rs2))) {
		res = isnan(getfreg(proc, rs1)) ? getfreg(proc, rs2) : getfreg(proc, rs1);
		WRITE_FFLAGS(proc->fcsr, GET_FFLAGS(proc->fcsr) | RISCV_FEXCEPT_NV);
	// -0.0 < 0.0 for FMIN and FMAX
	} else if (getfreg(proc, rs1) == 0.0f && eq_minus_zero(getfreg(proc, rs2))) {
		res = getfreg(proc, rs1);
	} else if (eq_minus_zero(getfreg(proc, rs1)) && getfreg(proc, rs2) == 0.0f) {
		res = getfreg(proc, rs2);
	} else {
		res = max(getfreg(proc, rs1), getfreg(proc, rs2));
	}

	if (isnan(res))
		res = canonical_nan();

	mvfreg(proc, rd, res);
	return 0;
}

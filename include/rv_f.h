/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * RISC-V F extension
 *
 *  Copyright (C) 2026 by Diego Evaristo <di.diegoevaristo@gmail.com>
 */

#ifndef RVRUN_RV_F_H
#define RVRUN_RV_F_H

// F extension's rounding modes (frm values)
#define RISCV_FRND_RNE 0
#define RISCV_FRND_RTZ 1
#define RISCV_FRND_RDN 2
#define RISCV_FRND_RUP 3
#define RISCV_FRND_RMM 4
#define RISCV_FRND_DYN 5

// F extension's floating point exceptions (fflags fields)
#define RISCV_FEXCEPT_NV 0x10
#define RISCV_FEXCEPT_DZ 0x8
#define RISCV_FEXCEPT_OF 0x4
#define RISCV_FEXCEPT_UF 0x2
#define RISCV_FEXCEPT_NX 0x1

// Extracts frm and fflags bits from fcsr
#define GET_FRM(fcsr) (((uint8_t)(fcsr) & 0xe0) >> 5)
#define GET_FFLAGS(fcsr) ((uint8_t)(fcsr) & 0x1f)

// Writes frm and fflags bits on fcsr
#define WRITE_FRM(fcsr, val) ((fcsr) |= (uint32_t)((val) & 0xe0))
#define WRITE_FFLAGS(fcsr, val) ((fcsr) |= (uint32_t)((val) & 0x1f))


int fcsr_csrrw(struct proc *proc, enum ABI_REG rd, enum ABI_REG rs1, csr_t csr)
	ATTRIBUTE(nonnull);
int fcsr_csrrs(struct proc *proc, enum ABI_REG rd, enum ABI_REG rs1, csr_t csr)
	ATTRIBUTE(nonnull);
int fcsr_csrrc(struct proc *proc, enum ABI_REG rd, enum ABI_REG rs1, csr_t csr)
	ATTRIBUTE(nonnull);
int fcsr_csrrwi(struct proc *proc, enum ABI_REG rd, enum ABI_REG rs1, csr_t csr)
	ATTRIBUTE(nonnull);
int fcsr_csrrsi(struct proc *proc, enum ABI_REG rd, enum ABI_REG rs1, csr_t csr)
	ATTRIBUTE(nonnull);
int fcsr_csrrci(struct proc *proc, enum ABI_REG rd, enum ABI_REG rs1, csr_t csr)
	ATTRIBUTE(nonnull);

// Must be ran before any floating point operations
void riscv_float_setup(struct proc *proc) ATTRIBUTE(nonnull);

// Floating point instructions
int insn_flw(struct proc *proc, insn_t insn) ATTRIBUTE(nonnull);
int insn_fsw(struct proc *proc, insn_t insn) ATTRIBUTE(nonnull);
int insn_fadd_s(struct proc *proc, insn_t insn) ATTRIBUTE(nonnull);
int insn_fsub_s(struct proc *proc, insn_t insn) ATTRIBUTE(nonnull);
int insn_fmul_s(struct proc *proc, insn_t insn) ATTRIBUTE(nonnull);
int insn_fdiv_s(struct proc *proc, insn_t insn) ATTRIBUTE(nonnull);
int insn_fsqrt_s(struct proc *proc, insn_t insn) ATTRIBUTE(nonnull);
int insn_fmin_s(struct proc *proc, insn_t insn) ATTRIBUTE(nonnull);
int insn_fmax_s(struct proc *proc, insn_t insn) ATTRIBUTE(nonnull);
int insn_fmadd_s(struct proc *proc, insn_t insn) ATTRIBUTE(nonnull);
int insn_fmsub_s(struct proc *proc, insn_t insn) ATTRIBUTE(nonnull);
int insn_fnmadd_s(struct proc *proc, insn_t insn) ATTRIBUTE(nonnull);
int insn_fnmsub_s(struct proc *proc, insn_t insn) ATTRIBUTE(nonnull);
int insn_fcvt_w_s(struct proc *proc, insn_t insn) ATTRIBUTE(nonnull);
int insn_fcvt_l_s(struct proc *proc, insn_t insn) ATTRIBUTE(nonnull);
int insn_fcvt_wu_s(struct proc *proc, insn_t insn) ATTRIBUTE(nonnull);
int insn_fcvt_lu_s(struct proc *proc, insn_t insn) ATTRIBUTE(nonnull);
int insn_fcvt_s_w(struct proc *proc, insn_t insn) ATTRIBUTE(nonnull);
int insn_fcvt_s_wu(struct proc *proc, insn_t insn) ATTRIBUTE(nonnull);
int insn_fcvt_s_l(struct proc *proc, insn_t insn) ATTRIBUTE(nonnull);
int insn_fcvt_s_lu(struct proc *proc, insn_t insn) ATTRIBUTE(nonnull);

#endif // RVRUN_RV_F_H

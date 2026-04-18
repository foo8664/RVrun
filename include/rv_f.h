/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * RISC-V F extension
 *
 *  Copyright (C) 2026 by Diego Evaristo <di.diegoevaristo@gmail.com>
 */

#ifndef RVRUN_RV_F_H
#define RVRUN_RV_F_H

#define RISCV_FRND_RNE 0
#define RISCV_FRND_RTZ 1
#define RISCV_FRND_RDN 2
#define RISCV_FRND_RUP 3
#define RISCV_FRND_RMM 4
#define RISCV_FRND_DYN 5

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

#endif // RVRUN_RV_F_H

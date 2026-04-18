/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Instruction fetching and decoding
 *
 *  Copyright (C) 2026 by Diego Evaristo <di.diegoevaristo@gmail.com>
 */

#include <errno.h>
#include <stddef.h>
#include "riscv.h"
#include "proc.h"
#include "debug.h"
#include "insn.h"

#include "rv_i.h"
#include "rv_m.h"

int insn_fetch(struct proc *proc, insn_t *insn)
{
	struct memseg *seg;

	seg = is_memseg(proc->mem, proc->pc, proc->pc + sizeof(*insn) + 1);
	if (UNLIKELY(!seg)) {
		errno = EINVAL;
		return -1;
	} else if (UNLIKELY(!(seg->flags & MEM_READ) || !(seg->flags & MEM_EXEC))) {
		errno = EPERM;
		return -1;
	}

	*insn = (insn_t)
		(((insn_t)seg->mem[proc->pc - seg->start]) 		|
		((insn_t)seg->mem[proc->pc - seg->start + 1] << 8) 	|
		((insn_t)seg->mem[proc->pc - seg->start + 2] << 16) 	|
		((insn_t)seg->mem[proc->pc - seg->start + 3] << 24));
	return 4;
}

#define ADD_INSN(name, fname) if (IS_INSN(insn, name)) return (fname)
int (*insn_decode(insn_t insn))(struct proc *, insn_t)
{
	// Rv64I Instructions:
	ADD_INSN(ADD,	insn_add);
	ADD_INSN(SLT,	insn_slt);
	ADD_INSN(SLTU,	insn_sltu);
	ADD_INSN(AND,	insn_and);
	ADD_INSN(OR,	insn_or);
	ADD_INSN(XOR,	insn_xor);
	ADD_INSN(SLL,	insn_sll);
	ADD_INSN(SRL,	insn_srl);
	ADD_INSN(SRA,	insn_sra);
	ADD_INSN(SUB,	insn_sub);
	ADD_INSN(ADDW,	insn_addw);
	ADD_INSN(SUBW,	insn_subw);
	ADD_INSN(SLLW,	insn_sllw);
	ADD_INSN(SRLW,	insn_srlw);
	ADD_INSN(SRLW,	insn_srlw);
	ADD_INSN(ANDI,	insn_andi);
	ADD_INSN(ORI, 	insn_ori);
	ADD_INSN(XORI,	insn_xori);
	ADD_INSN(SLLI,	insn_slli);
	ADD_INSN(SRLI,	insn_srli);
	ADD_INSN(SRAI,	insn_srai);
	ADD_INSN(ADDI,	insn_addi);
	ADD_INSN(SLTI,	insn_slti);
	ADD_INSN(SLTIU,	insn_sltiu);
	ADD_INSN(ADDIW,	insn_addiw);
	ADD_INSN(SLLIW,	insn_slliw);
	ADD_INSN(SRLIW,	insn_srliw);
	ADD_INSN(SRAIW,	insn_sraiw);
	ADD_INSN(LUI,	insn_lui);
	ADD_INSN(AUIPC,	insn_auipc);
	ADD_INSN(JAL,	insn_jal);
	ADD_INSN(JALR,	insn_jalr);
	ADD_INSN(BEQ,	insn_beq);
	ADD_INSN(BNE,	insn_bne);
	ADD_INSN(BLT,	insn_blt);
	ADD_INSN(BLTU,	insn_bltu);
	ADD_INSN(BGE,	insn_bge);
	ADD_INSN(BGEU,	insn_bgeu);
	ADD_INSN(LB,	insn_lb);
	ADD_INSN(LH,	insn_lh);
	ADD_INSN(LW,	insn_lw);
	ADD_INSN(LD,	insn_ld);
	ADD_INSN(LBU,	insn_lbu);
	ADD_INSN(LHU,	insn_lhu);
	ADD_INSN(LWU,	insn_lwu);
	ADD_INSN(SB,	insn_sb);
	ADD_INSN(SH,	insn_sh);
	ADD_INSN(SW,	insn_sw);
	ADD_INSN(SD,	insn_sd);
	ADD_INSN(ECALL, insn_ecall);
	ADD_INSN(EBREAK,insn_ebreak);
	ADD_INSN(FENCE, insn_fence);

	// Rv64M Instructions
	ADD_INSN(MUL,	insn_mul);
	ADD_INSN(MULW,	insn_mulw);
	ADD_INSN(MULH,	insn_mulh);
	ADD_INSN(MULHU,	insn_mulhu);
	ADD_INSN(MULHSU,insn_mulhsu);
	ADD_INSN(DIV,	insn_div);
	ADD_INSN(DIVU,	insn_divu);
	ADD_INSN(DIVW,	insn_divw);
	ADD_INSN(DIVUW,	insn_divuw);
	ADD_INSN(REM,	insn_rem);
	ADD_INSN(REMU,	insn_remu);
	ADD_INSN(REMW,	insn_remw);
	ADD_INSN(REMUW,	insn_remuw);

	errno = ENOSYS;
	return NULL;
}
#undef ADD_INSN

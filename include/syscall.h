/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * <one line to give the program's name and a brief idea of what it does.>
 *
 *  Copyright (C) 2026 by Diego Oliveira Evaristo <di.diegoevaristo@gmail.com>
 */

#ifndef RVRUN_SYSCALL_H
#define RVRUN_SYSCALL_H

int rvsys_exit(struct proc *proc)	__attribute__((nonnull));
int rvsys_exit_group(struct proc *proc)	__attribute__((nonnull));

#endif // RVRUN_SYSCALL_H

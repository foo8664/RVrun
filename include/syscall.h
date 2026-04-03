/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Emulates Linux system calls
 *
 *  Copyright (C) 2026 by Diego Oliveira <di.diegoevaristo@gmail.com>
 */

#ifndef RVRUN_SYSCALL_H
#define RVRUN_SYSCALL_H

int rvsys_exit(struct proc *proc)	__attribute__((nonnull));
int rvsys_exit_group(struct proc *proc)	__attribute__((nonnull));
int rvsys_write(struct proc *proc)	__attribute__((nonnull));
int rvsys_read(struct proc *proc)	__attribute__((nonnull));
int rvsys_openat(struct proc *proc)	__attribute__((nonnull));
int rvsys_close(struct proc *proc)	__attribute__((nonnull));

#endif // RVRUN_SYSCALL_H

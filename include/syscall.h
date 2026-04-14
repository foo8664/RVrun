/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Emulates Linux system calls
 *
 *  Copyright (C) 2026 by Diego Evaristo <di.diegoevaristo@gmail.com>
 */

#ifndef RVRUN_SYSCALL_H
#define RVRUN_SYSCALL_H

#include "common.h"
int rvsys_exit(struct proc *proc)	ATTRIBUTE(nonnull);
int rvsys_exit_group(struct proc *proc)	ATTRIBUTE(nonnull);
int rvsys_write(struct proc *proc)	ATTRIBUTE(nonnull);
int rvsys_read(struct proc *proc)	ATTRIBUTE(nonnull);
int rvsys_openat(struct proc *proc)	ATTRIBUTE(nonnull);
int rvsys_close(struct proc *proc)	ATTRIBUTE(nonnull);

#endif // RVRUN_SYSCALL_H

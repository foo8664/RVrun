/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Common macros used in the code
 *
 *  Copyright (C) 2026 by Diego Evaristo <di.diegoevaristo@gmail.com>
 */

#ifndef RVRUN_COMMON_H
#define RVRUN_COMMON_H

// Warn if using on untested architecture
#ifndef __x86_64__
# warning "RvRun was only tested in x86-64 architecture"
#endif

// Array size
#define ARRAY_SIZE(arr) (sizeof(arr) / (sizeof(*(arr))))

// Branch prediction optimizations
#if defined(__clang__) || defined(__GNUC__)
# define LIKELY(x)	(__builtin_expect(!!(x), 1))
# define UNLIKELY(x)	(__builtin_expect(!!(x), 0))
#else
# define LIKELY(x)	(x)
# define UNLIKELY(x)	(x)
#endif

// Using __attribute__ portably
#if defined(__clang__) || defined(__GNUC__)
# define ATTRIBUTE(...) __attribute__((__VA_ARGS__))
#else
# define ATTRIBUTE(...)
#endif

// Cache optimizations
#define PREFETCH_READ 0
#define PREFETCH_WRITE 1
#define PREFETCH_SHARED_READ 2
#define PREFETCH_NOTMP 0
#define PREFETCH_LOWTMP 1
#define PREFETCH_MODERATETMP 2
#define PREFETCH_HIGHTMP 3
#if defined(__clang__) || defined(__GNUC__)
# define PREFETCH(addr, ...) (__builtin_prefetch(addr, __VA_ARGS__))
#else
# define PREFETCH(addr, ...)
#endif

#endif // RVRUN_COMMON_H

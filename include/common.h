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

// max-min
#define min(a, b) ((a) <= (b) ? (a) : (b))
#define max(a, b) ((a) >= (b) ? (a) : (b))

// Branch prediction optimizations
#ifdef __GNUC__
# define LIKELY(x)	(__builtin_expect(!!(x), 1))
# define UNLIKELY(x)	(__builtin_expect(!!(x), 0))
#else
# define LIKELY(x)	(x)
# define UNLIKELY(x)	(x)
#endif

/*
 * Using __attribute__ portably. Clang does support the __attribute__(()) feature,
 * but many of the ones used in the code are simply not supported.
 */
#if defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER__)
# define ATTRIBUTE(...) __attribute__((__VA_ARGS__))
#else
# define ATTRIBUTE(...)
#endif

#endif // RVRUN_COMMON_H

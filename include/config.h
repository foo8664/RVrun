/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Runtime config, parses argv.
 *
 *  Copyright (C) 2026 by Diego Oliveira Evaristo <di.diegoevaristo@gmail.com>
 */

#ifndef RISCV_CONFIG_H
#define RISCV_CONFIG_H

enum RVOPT {
	SET_STDIN=1, // Just for testing
};

#ifdef POSIXLY_CORRECT
# undef POSIXLY_CORRECT
#endif

// Constants for .has_arg on struct option from getopt.h
#define NO_ARG 0
#define ARG_MANDATORY 1
#define ARG_OPTIONAL 2

#include <getopt.h>
#include <stdbool.h>
struct rvopt {
	union {
		unsigned long ulong;
		unsigned uint;
		int integer;
		long ilong;
		char *str;
		void *ptr;
	} u;

	enum RVOPT type;
	bool set;
	bool arg;
};

#include <stddef.h>
struct rvconfig {
	const char *optstring;

	size_t size;
	struct optpair {
		struct rvopt rvopt;
		struct option opt;
	} *opts;
};

typedef void (*configclean_t(struct optpair *));
/*
 * Cleans a config created by a call parsecfg(), does not free either cfg
 * or cfg->opts, as they may not be dynamically allocated. Calls cleanup()
 * on every element of cfg->opts
 */
void cleancfg(struct rvconfig *cfg, configclean_t cleanup)
	__attribute__((nonnull(1)));

// Parses argv and creates a config to be used by the program
void parsecfg(struct rvconfig *cfg, int argc, char **argv)
	__attribute__((nonnull));

// Returns the pointer to a specific option in a config
struct rvopt *getcfgopt(struct rvconfig *cfg, enum RVOPT type)
	__attribute__((nonnull));

// Sets a config as the global one
void set_globalcfg(struct rvconfig *cfg) __attribute__((nonnull));

// Access the program's global config as the globalcfg "variable"
struct rvconfig *get_globalcfg(void);
#define globalcfg (*get_globalcfg())

#endif // RISCV_CONFIG_H

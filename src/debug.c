/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Functions for logging and debugging
 *
 *  Copyright (C) 2026 by Diego Oliveira Evaristo <di.diegoevaristo@gmail.com>
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "config.h"
#include "debug.h"

static FILE *logfile = NULL;
static enum LOGLEVEL loglevel = -1;

void setdbgcfg()
{
	struct rvopt *opt;

	if (!get_globalcfg()) {
		logfile = stderr;
		loglevel = INFO_LOGLEVEL;
		return;
	}

	if ((opt = getcfgopt(&globalcfg, CFG_LOGFILE)) && opt->set)
		logfile = (FILE *)opt->u.ptr;
	else
		logfile = stderr;

	if ((opt = getcfgopt(&globalcfg, CFG_LOGLEVEL)) && opt->set)
		loglevel = (enum LOGLEVEL)opt->u.uint;
	else
		loglevel = INFO_LOGLEVEL;
}

void dbg_log(const char *restrict fmt, ...)
{
	va_list args;

	if (loglevel > DBG_LOGLEVEL)
		return;

	fputs("[DEBUG]: ", logfile);

	va_start(args, fmt);
	vfprintf(logfile, fmt, args);
	fputc('\n', logfile);
	va_end(args);
}

void info_log(const char *restrict fmt, ...)
{
	va_list args;

	if (loglevel > INFO_LOGLEVEL)
		return;

	fputs("[INFO]: ", logfile);

	va_start(args, fmt);
	vfprintf(logfile, fmt, args);
	fputc('\n', logfile);
	va_end(args);
}

void warn_log(const char *restrict fmt, ...)
{
	va_list args;

	if (loglevel > WARN_LOGLEVEL)
		return;

	fputs("[WARNING]: ", logfile);

	va_start(args, fmt);
	vfprintf(logfile, fmt, args);
	fputc('\n', logfile);
	va_end(args);
}

void err_log(const char *restrict fmt, ...)
{
	va_list args;

	if (loglevel > ERR_LOGLEVEL)
		return;

	fputs("[WARNING]: ", logfile);

	va_start(args, fmt);
	vfprintf(logfile, fmt, args);
	fputc('\n', logfile);
	va_end(args);
}

void rvrunpanic(const char *restrict file, const char *restrict func, int line,
		const char *restrict reason)
{
	fprintf(logfile, "[PANIC]: %s:%s():%d: %s\n", file, func, line, reason);
	abort();
}

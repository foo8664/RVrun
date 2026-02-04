#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "debug.h"

void dbg_log(const char *restrict fmt, ...)
{
	va_list args;

	fputs("[DEBUG]: ", stderr);

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	fputc('\n', stderr);
	va_end(args);
}

void info_log(const char *restrict fmt, ...)
{
	va_list args;

	fputs("[INFO]: ", stderr);

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	fputc('\n', stderr);
	va_end(args);
}

void warn_log(const char *restrict fmt, ...)
{
	va_list args;

	fputs("[WARNING]: ", stderr);

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	fputc('\n', stderr);
	va_end(args);
}

void err_log(const char *restrict fmt, ...)
{
	va_list args;

	fputs("[WARNING]: ", stderr);

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	fputc('\n', stderr);
	va_end(args);
}

void rvrunpanic(const char *restrict file, const char *restrict func, int line,
		const char *restrict reason)
{
	fprintf(stderr, "[PANIC]: %s:%s():%d: %s\n", file, func, line, reason);
	abort();
}

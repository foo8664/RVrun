#ifndef DEBUG_H
#define DEBUG_H

/*
 * Logging functions, work like fprintf(stderr, fmt, ...) but display a message
 * such as "[DEBUG]", "[INFO]", etc. Should be preferred as log-file, filtering,
 * and other features might be added later
 */
void dbg_log(const char *restrict fmt, ...)
	__attribute__((nonnull, format(printf, 1, 2)));
void info_log(const char *restrict fmt, ...)
	__attribute__((nonnull, format(printf, 1, 2)));
void warn_log(const char *restrict fmt, ...)
	__attribute__((nonnull, format(printf, 1, 2)));
void err_log(const char *restrict fmt, ...)
	__attribute__((nonnull, format(printf, 1, 2)));

/*
 * Panics, sends a message to stderr with a reason, and the exact file,
 * function, and line, in which the panic happened.
 */
void rvrunpanic(const char *restrict file, const char *restrict func, int line,
		const char *restrict reason) __attribute__((nonnull, noreturn));
#define panic(reason) (rvrunpanic(__FILE__, __func__, __LINE__, reason))

#endif // DEBUG_H

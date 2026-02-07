#ifndef MEMORY_H
#define MEMORY_H

enum memflags {
	MEM_READ=0x1,
	MEM_WRITE=0x2,
	MEM_EXEC=0x4,
};

#define MEMFLAG_INVALID(f) ((!((f) & MEM_READ) && !((f) & MEM_WRITE) && \
			!((f) & MEM_EXEC)) || ((f) & ~(MEM_READ | 	\
			MEM_WRITE | MEM_EXEC)))
#include "riscv.h"
#include <stdint.h>

// Memory segments, map addresses in range [start, end[
struct memseg {
	struct memseg *next;
	unsigned char *mem;
	rvaddr_t start;
	rvaddr_t end;
	uint8_t flags;
};

// Memory structure, linked list of segments
struct memory {
	struct memseg *segments;
};

// Adds a memory segment
struct memseg *addseg(struct memory *mem, rvaddr_t start, rvaddr_t end,
		      uint8_t flags) __attribute__((nonnull));

// Frees a memory segment
void freeseg(struct memory *mem, struct memseg *seg) __attribute__((nonnull));

// Frees all of the memory
void freemem(struct memory *mem);

// Returns the segment that maps addresses in range [start, end[
struct memseg *is_memseg(struct memory mem, rvaddr_t start, rvaddr_t end)
	__attribute__((nonnull));

/*
 * Loads `size` bits at address `addr` in `out`, returns 0 if the load suceeds,
 * and -1 if it fails, setting errno before returning.
 * Size must be either 8, 16, 32, or 64. You can use the `memload()` macro to
 * call this function with the correct `size` parameter and a uintN_t * pointer
 */
int memloadN(struct memory mem, rvaddr_t addr, uint8_t size, void *out)
	__attribute__((nonnull));

/*
 * Stores `size` bits from `in` in address `addr`, returns 0 if the store
 * suceeds, and -1 if it fails, setting errno before returning.
 * Size must be either 8, 16, 32, or 64. You can use the `memstore()` macro to
 * call this function with the correct `size` paramater and a uintN_t * pointer
 */
int memstoreN(struct memory mem, rvaddr_t addr, uint8_t size, const void *in)
	__attribute__((nonnull, access(read_only, 4)));

#define memload(mem, addr, ptr) (_Generic(ptr,				\
		uint8_t *:  memloadN(mem, addr, 8,  (void *)ptr),	\
		uint16_t *: memloadN(mem, addr, 16, (void *)ptr),	\
		uint32_t *: memloadN(mem, addr, 32, (void *)ptr),	\
		uint64_t *: memloadN(mem, addr, 64, (void *)ptr),	\
		default   : abort()))

#define memstore(mem, addr, ptr) (_Generic(ptr,\
		uint8_t *:  memstoreN(mem, addr, 8,  (const void *)ptr),\
		uint16_t *: memstoreN(mem, addr, 16, (const void *)ptr),\
		uint32_t *: memstoreN(mem, addr, 32, (const void *)ptr),\
		uint64_t *: memstoreN(mem, addr, 64, (const void *)ptr),\
		default   : abort()))

#endif // MEMORY_H

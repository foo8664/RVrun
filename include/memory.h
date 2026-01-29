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

/* Memory segments, node of `mem`, flags field if memflags ORed together */
struct memseg {
	struct memseg *next;
	char *mem;
	rvaddr_t start;
	rvaddr_t end;
	uint8_t flags;
};

/* Memory structure, linked list of segments */
struct memory {
	struct memseg *segments;
};

/* Adds a memory segment */
struct memseg *addseg(struct memory *mem, rvaddr_t start, rvaddr_t end,
		      uint8_t flags) __attribute__((nonnull));

/* Frees a memory segment */
void freeseg(struct memory *mem, struct memseg *seg) __attribute__((nonnull));
/* Frees all of the memory */
void freemem(struct memory *mem);

#endif /* MEMORY_H */

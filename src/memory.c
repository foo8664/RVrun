#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include "riscv.h"
#include "memory.h"

static inline void memload8(struct memseg *seg, rvaddr_t addr, uint8_t *in)
	__attribute__((nonnull));
static inline void memload16(struct memseg *seg, rvaddr_t addr, uint16_t *in)
	__attribute__((nonnull));
static inline void memload32(struct memseg *seg, rvaddr_t addr, uint32_t *in)
	__attribute__((nonnull));
static inline void memload64(struct memseg *seg, rvaddr_t addr, uint64_t *in)
	__attribute__((nonnull));
static inline void memstore8(struct memseg *seg, rvaddr_t addr, uint8_t in)
	__attribute__((nonnull));
static inline void memstore16(struct memseg *seg, rvaddr_t addr, uint16_t in)
	__attribute__((nonnull));
static inline void memstore32(struct memseg *seg, rvaddr_t addr, uint32_t in)
	__attribute__((nonnull));
static inline void memstore64(struct memseg *seg, rvaddr_t addr, uint64_t in)
	__attribute__((nonnull));

struct memseg *addseg(struct memory *mem, rvaddr_t start, rvaddr_t end,
		      uint8_t flags)
{
	struct memseg *seg;
	struct memseg *before;

	if (MEMFLAG_INVALID(flags))
		return NULL;
	if (is_memseg(*mem, start, end))
		return NULL;
	if (!(seg = malloc(sizeof(*seg))))
		return NULL;
	if (!(seg->mem = malloc(end - start - 1))) {
		free(seg);
		return NULL;
	}

	seg->start = start;
	seg->end = end;
	seg->flags = flags;
	seg->next = NULL;

	if (!mem->segments) {
		mem->segments = seg;
		return seg;
	}

	before = mem->segments;
	while (before->next)
		before = before->next;
	before->next = seg;
	return seg;
}

void freeseg(struct memory *mem, struct memseg *seg)
{
	struct memseg *before;

	if (seg == mem->segments) {
		if (!seg->next)
			goto free_seg;
		mem->segments = seg->next;
		goto free_seg;
	}

	before = mem->segments;
	while (before && before->next != seg)
		before = before->next;
	if (!before)
		return;

	before->next = seg->next;
free_seg:
	free(seg->mem);
	free(seg);
}

void freemem(struct memory *mem)
{
	struct memseg *seg;
	struct memseg *next;
	seg = mem->segments;

	while (seg) {
		next = seg->next;
		freeseg(mem, seg);
		seg = next;
	}
}

struct memseg *is_memseg(struct memory mem, rvaddr_t start, rvaddr_t end)
{
	struct memseg *seg;

	if (start > end)
		return NULL;

	for (seg = mem.segments; seg; seg = seg->next)
		if (start >= seg->start && end <= seg->end)
			return seg;
	return NULL;
}

int memloadN(struct memory mem, rvaddr_t addr, uint8_t size, void *out)
{
	struct memseg *seg;

	assert(size == 8 || size == 16 || size == 32 || size == 64);

	seg = is_memseg(mem, addr, addr + (size / 8) + 1);
	if (!seg) {
		errno = EINVAL;
		return -1;
	} else if (!(seg->flags & MEM_READ)) {
		errno = EPERM;
		return -1;
	}

	switch (size) {
	case 8:
		memload8(seg, addr, (uint8_t *)out);
		break;
	case 16:
		memload16(seg, addr, (uint16_t *)out);
		break;
	case 32:
		memload32(seg, addr, (uint32_t *)out);
		break;
	case 64:
		memload64(seg, addr, (uint64_t *)out);
		break;
	}

	return 0;
}

int memstoreN(struct memory mem, rvaddr_t addr, uint8_t size, const void *in)
{
	struct memseg *seg;

	assert(size == 8 || size == 16 || size == 32 || size == 64);

	seg = is_memseg(mem, addr, addr + (size / 8) + 1);
	if (!seg) {
		errno = EINVAL;
		return -1;
	} else if (!(seg->flags & MEM_WRITE)) {
		errno = EPERM;
		return -1;
	}

	switch (size) {
	case 8:
		memstore8(seg, addr, *((const uint8_t *)in));
		break;
	case 16:
		memstore16(seg, addr, *((const uint16_t *)in));
		break;
	case 32:
		memstore32(seg, addr, *((const uint32_t *)in));
		break;
	case 64:
		memstore64(seg, addr, *((const uint64_t *)in));
		break;
	}

	return 0;
}

static inline void memstore8(struct memseg *seg, rvaddr_t addr, uint8_t in)
{
	seg->mem[addr - seg->start] = in;
}

static inline void memstore16(struct memseg *seg, rvaddr_t addr, uint16_t in)
{
	seg->mem[addr - seg->start] = (uint8_t)(in & 0xff);
	seg->mem[addr - seg->start + 1] = (uint8_t)((in & 0xff00) >> 8);
}

static inline void memstore32(struct memseg *seg, rvaddr_t addr, uint32_t in)
{
	seg->mem[addr - seg->start] = (uint8_t)(in & 0xff);
	seg->mem[addr - seg->start + 1] = (uint8_t)((in & 0xff00) >> 8);
	seg->mem[addr - seg->start + 2] = (uint8_t)((in & 0xff0000) >> 16);
	seg->mem[addr - seg->start + 3] = (uint8_t)((in & 0xff000000) >> 24);
}

static inline void memstore64(struct memseg *seg, rvaddr_t addr, uint64_t in)
{
	seg->mem[addr - seg->start] = (uint8_t)(in & 0xff);
	seg->mem[addr - seg->start + 1] = (uint8_t)((in & 0xff00) >> 8);
	seg->mem[addr - seg->start + 2] = (uint8_t)((in & 0xff0000) >> 16);
	seg->mem[addr - seg->start + 3] = (uint8_t)((in & 0xff000000) >> 24);
	seg->mem[addr - seg->start + 4] = (uint8_t)((in & 0xff00000000) >> 32);
	seg->mem[addr - seg->start + 5] = (uint8_t)((in & 0xff0000000000)
							>> 40);
	seg->mem[addr - seg->start + 6] = (uint8_t)((in & 0xff000000000000)
							>> 48);
	seg->mem[addr - seg->start + 7] = (uint8_t)((in & 0xff00000000000000)
							>> 56);
}

static inline void memload8(struct memseg *seg, rvaddr_t addr, uint8_t *out)
{
	*out = (uint8_t)seg->mem[addr - seg->start];
}

static inline void memload16(struct memseg *seg, rvaddr_t addr, uint16_t *out)
{
	*out = 	(uint16_t)
		(((uint16_t)seg->mem[addr - seg->start]) |
		((uint16_t)seg->mem[addr - seg->start + 1] << 8));
}

static inline void memload32(struct memseg *seg, rvaddr_t addr, uint32_t *out)
{
	*out = 	(uint32_t)
		(((uint32_t)seg->mem[addr - seg->start]) 		|
		((uint32_t)seg->mem[addr - seg->start + 1] << 8) 	|
		((uint32_t)seg->mem[addr - seg->start + 2] << 16) 	|
		((uint32_t)seg->mem[addr - seg->start + 3] << 24));
}

static inline void memload64(struct memseg *seg, rvaddr_t addr, uint64_t *out)
{
	*out = 	(uint64_t)
		(((uint64_t)seg->mem[addr - seg->start]) 		|
		((uint64_t)seg->mem[addr - seg->start + 1] << 8) 	|
		((uint64_t)seg->mem[addr - seg->start + 2] << 16) 	|
		((uint64_t)seg->mem[addr - seg->start + 3] << 24)	|
		((uint64_t)seg->mem[addr - seg->start + 4] << 32) 	|
		((uint64_t)seg->mem[addr - seg->start + 5] << 40) 	|
		((uint64_t)seg->mem[addr - seg->start + 6] << 48) 	|
		((uint64_t)seg->mem[addr - seg->start + 7] << 56));
}

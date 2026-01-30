#include <stdlib.h>
#include <stdbool.h>
#include "riscv.h"
#include "memory.h"

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

bool is_memseg(struct memory mem, rvaddr_t start, rvaddr_t end)
{
	struct memseg *seg;

	if (start > end)
		return false;

	for (seg = mem.segments; seg; seg = seg->next)
		if (start >= seg->start && end <= seg->end)
			return true;
	return false;
}

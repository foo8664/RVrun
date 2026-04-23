# SPDX-License-Identifier: GPL-2.0-only
#
#  Makefile for RvRun
#
#   Copyright (C) 2026 by Diego Evaristo <di.diegoevaristo@gmail.com>
#
PREFIX ?= /usr/local

src = src
headers = include

CC ?= cc
CFLAGS = -Wall -Werror -Wextra -Wconversion
CFLAGS += -std=c11 -pedantic-errors -D_GNU_SOURCE
CFLAGS += -I$(headers)
CFLAGS += -O2 -mtune=native
CFLAGS += -DRVRUN_VERSION_MAJOR=1 -DRVRUN_VERSION_MINOR=1
LDFLAGS = -lm

VPATH = $(src):$(headers)
objs = main.o debug.o config.o memory.o proc.o syscall.o insn.o riscv.o rv_i.o \
       rv_m.o rv_f.o Zicsr.o

rvrun: $(objs)
	$(CC) $(LDFLAGS) $(CFLAGS) $(objs) -o rvrun

# Copied and slightly modified from the GNU Make manual section 4.14
%.d: %.c $(headers)/opcodes.h
	@set -e; rm -f $@; 					\
	$(CC) -MM $(CFLAGS) $< > $@.$$$$;			\
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;	\
	rm -f $@.$$$$;
-include $(objs:.o=.d)

.PHONY: clean install uninstall
clean:
	-rm -f *.o *.d* 2>/dev/null || true

install:
	install -m755 rvrun $(PREFIX)/bin/rvrun

uninstall:
	-@rm -i $(PREFIX)/bin/rvrun || true

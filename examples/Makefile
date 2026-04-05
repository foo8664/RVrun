# SPDX-License-Identifier: GPL-2.0-only
#
#  Makefile for RvRun's example programs
#
#   Copyright (C) 2026 by Diego Oliveira <di.diegoevaristo@gmail.com>
#

# Might want to edit these:
AS = riscv64-elf-linux-gnu-as
LD = riscv64-elf-linux-gnu-ld

%.elf: common.o
	$(AS) $(subst .elf,.asm,$@) -o $(subst .elf,.o,$@)
	$(LD) common.o $(subst .elf,.o,$@) -o $@

common.o: common.asm
	$(AS) common.asm -o common.o

.PHONY: clean all
clean:
	-rm *.o *.elf 2>/dev/null || true
all: common.o
	./make_all.sh

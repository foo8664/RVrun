# Might want to edit these:
AS = riscv64-elf-linux-gnu-as
LD = riscv64-elf-linux-gnu-ld

%.elf: common.o
	$(AS) $(subst .elf,.asm,$@) -o $(subst .elf,.o,$@)
	$(LD) common.o $(subst .elf,.o,$@) -o $@

common.o: common.asm
	$(AS) common.asm -o common.o

.PHONY: clean
clean:
	-rm *.o *.elf 2>/dev/null || true

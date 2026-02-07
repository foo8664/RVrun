src = src
headers = include

CC = gcc
CFLAGS = -Wall -Werror -Wextra -Wconversion
CFLAGS += -std=c11 -pedantic-errors -D_GNU_SOURCE
CFLAGS += -I$(headers)
CFLAGS += -O2

VPATH = $(src):$(headers)
objs = main.o debug.o memory.o proc.o insn.o

rvrun: $(objs)
	$(CC) $(CFLAGS) $(objs) -o rvrun

$(headers)/opcodes.h:
	@set -e;						\
	git clone https://github.com/riscv/riscv-opcodes.git;	\
	cd riscv-opcodes;					\
	make EXTENSIONS='rv_i' encoding.out.h;			\
	cd ..;							\
	mv riscv-opcodes/encoding.out.h include/opcodes.h;	\
	rm -rf riscv-opcodes

# Copied and slightly modified from the GNU Make manual section 4.14
%.d: %.c $(headers)/opcodes.h
	@set -e; rm -f $@; 					\
	$(CC) -MM $(CFLAGS) $< > $@.$$$$;			\
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;	\
	rm -f $@.$$$$;
include $(objs:.o=.d)

.PHONY: clean
clean:
	-rm 2>/dev/null rvrun *.o *.d $(headers)/opcodes.h || true

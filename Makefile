src = src
headers = include

CC = gcc
CFLAGS = -Wall -Werror -Wextra -Wconversion
CFLAGS += -std=c99 -pedantic-errors
CFLAGS += -I$(headers)
CFLAGS += -O2

VPATH=$(src):$(headers)
objs = main.o

rvrun: $(objs)
	$(CC) $(CFLAGS) $(objs) -o rvrun

# Copied and slightly modified from the GNU Make manual section 4.14
%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$;
include $(objs:.o=.d)

.PHONY: clean
clean:
	-rm 2>/dev/null rvrun *.o *.d || true

CC = clang++
SRCS = cmdloop.c pattern_parser.c
OBJS = $(subst .c,.o,$(SRCS))
LIBS = -lreadline
INCDIRS=-I/usr/local/include -Iinclude
LIBDIR=/usr/local/lib
CFLAGS = -std=c11 $(WARNFLAGS) -g -pg $(INCDIRS) -O0

all: cmdloop

%.o: %.c
	$(CC) -c -o $@ -x c $< $(CFLAGS)

cmdloop: $(OBJS)
	$(CC) $(CFLAGS) -L$(LIBDIR) $(LIBS) -o $@ $^

clean:
	rm cmdloop

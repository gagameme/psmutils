CC := gcc
CFLAGS := -g -Wall
CPPFLAGS :=
LDLIBS :=

DESTDIR :=
prefix := /usr/local
exec_prefix := $(prefix)
bindir := $(exec_prefix)/bin/
srcdir := $(dir $(word 1,$(MAKEFILE_LIST)))

DEST := psmedit
SRC  := main.c \
        operate.c \
        mc.c mcb.c \
        write.c print.c extract.c swap.c title.c delete.c newdata.c

OBJS := $(patsubst %.c,%.o,$(SRC))


all: $(DEST)

.PHONY: all install clean

$(DEST): $(OBJS)
	$(CC) -o $@ $(CFLAGS) $^ $(LDLIBS)

install: $(DEST)
	mkdir -p $(DESTDIR)$(bindir)
	cp $? $(DESTDIR)$(bindir)

clean:
	$(RM) $(OBJS) $(DEST)


CC := gcc
CFLAGS := -g -Wall `pkg-config --cflags libusb-1.0`
LDLIBS := `pkg-config --libs libusb-1.0`

DESTDIR :=
prefix := /usr/local
exec_prefix := $(prefix)
bindir := $(exec_prefix)/bin/
srcdir := $(dir $(word 1,$(MAKEFILE_LIST)))

DEST := mcactl
SRC  := main.c operate.c mcaio.c

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


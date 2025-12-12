# Makefile for Unix-like toolchains.

srcdir = .
CC = cc
CFLAGS = -g
LDFLAGS =
CPPFLAGS =
LIBS =

# If we have VPATH we can do out-of-source build.
VPATH = $(srcdir)

# Set V=1 to show recipes.
Q = $(V_$(V))
V_ = @

all: zeno-spec

objects = \
	tools/zeno-spec/main.o

zeno-spec: $(objects)
	@echo "LD $@"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(objects) $(LIBS)

.c.o:
	@echo "CC $@"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

clean:
	@echo "CLEAN zeno-spec *.o"
	$(Q)rm -f zeno-spec $(objects)

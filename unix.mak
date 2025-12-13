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

#
# Top-level targets
#

all: zeno-spec

clean:
	rm -f zeno-spec $(zeno_spec_objects)

-include src/*.d

#
# zeno-spec executable
#

zeno_spec_objects = \
	src/encoding.o \
	src/io.o \
	src/lex.o \
	src/main.o \
	src/token.o

objects = $(zeno_spec_objects)

zeno-spec: $(zeno_spec_objects)
	@echo "LD $@"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(zeno_spec_objects) $(LIBS)

.c.o:
	@echo "CC $@"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) -c $(CFLAGS) $(CPPFLAGS) -I$(srcdir) -o $@ $<

#
# Tests
#

NOT = $(srcdir)/tools/not

# TODO: an actual test framework
test: zeno-spec
	./zeno-spec $(srcdir)/tests/syntax/valid/bom.zn > /dev/null
	./zeno-spec $(srcdir)/tests/syntax/valid/cr.zn > /dev/null
	./zeno-spec $(srcdir)/tests/syntax/valid/crlf.zn > /dev/null
	./zeno-spec $(srcdir)/tests/syntax/valid/tab.zn > /dev/null
	./zeno-spec $(srcdir)/tests/syntax/valid/tokens.zn > /dev/null
	$(NOT) ./zeno-spec $(srcdir)/tests/syntax/invalid/bad_utf8.zn > /dev/null
	$(NOT) ./zeno-spec $(srcdir)/tests/syntax/invalid/bad_utf8_in_block_comment.zn > /dev/null
	$(NOT) ./zeno-spec $(srcdir)/tests/syntax/invalid/bad_utf8_in_line_comment.zn > /dev/null
	$(NOT) ./zeno-spec $(srcdir)/tests/syntax/invalid/binary_literal_no_value.zn > /dev/null
	$(NOT) ./zeno-spec $(srcdir)/tests/syntax/invalid/binary_literal_trailing_junk.zn > /dev/null
	$(NOT) ./zeno-spec $(srcdir)/tests/syntax/invalid/binary_literal_trailing_underscore.zn > /dev/null
	$(NOT) ./zeno-spec $(srcdir)/tests/syntax/invalid/decimal_literal_leading_zero.zn > /dev/null
	$(NOT) ./zeno-spec $(srcdir)/tests/syntax/invalid/decimal_literal_trailing_junk.zn > /dev/null
	$(NOT) ./zeno-spec $(srcdir)/tests/syntax/invalid/decimal_literal_trailing_underscore.zn > /dev/null
	$(NOT) ./zeno-spec $(srcdir)/tests/syntax/invalid/hex_literal_no_value.zn > /dev/null
	$(NOT) ./zeno-spec $(srcdir)/tests/syntax/invalid/hex_literal_trailing_junk.zn > /dev/null
	$(NOT) ./zeno-spec $(srcdir)/tests/syntax/invalid/hex_literal_trailing_underscore.zn > /dev/null
	$(NOT) ./zeno-spec $(srcdir)/tests/syntax/invalid/unclosed_block_comment.zn > /dev/null

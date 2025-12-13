# Makefile for Unix-like toolchains.

srcdir = .

# Executable suffix
E =
# Object suffix
O = .o

# C compiler options.
CC = cc
CFLAGS = -g
LDFLAGS =
CPPFLAGS =
LIBS =

# Yacc options.
YACC = byacc
YFLAGS = -v

# If we have VPATH we can do out-of-source build.
VPATH = $(srcdir)

# Set V=1 to show recipes.
Q = $(V_$(V))
V_ = @

#
# zeno-spec executable options
#

zeno_spec_objects = \
	src/ast$(O) \
	src/base$(O) \
	src/encoding$(O) \
	src/format$(O) \
	src/io$(O) \
	src/lex$(O) \
	src/main$(O) \
	src/parse.tab$(O) \
	src/token$(O)

zeno_spec_exe = zeno-spec$(E)

#
# Top-level targets
#

all: $(zeno_spec_exe)

clean:
	rm -f $(zeno_spec_exe) $(zeno_spec_objects)
	rm -f src/parse.output src/parse.tab.c
	rm -f src/*.d

-include src/*.d

#
# zeno-spec executable targets
#

$(zeno_spec_exe): $(zeno_spec_objects)
	@echo "LD $@"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(zeno_spec_objects) $(LIBS)

.c$(O):
	@echo "CC $@: $<"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) -c $(CFLAGS) $(CPPFLAGS) -I$(srcdir) -o $@ $<

src/parse.tab.c: src/parse.y
	@echo "YACC $@: $?"
	$(Q)mkdir -p $(@D)
	$(Q)LC_ALL=C $(YACC) $(YFLAGS) -b src/parse $?

#
# Tests
#

# TODO: an actual test framework
test: test-lex

test-lex: test-lex-valid test-lex-invalid

CHECK_LEX_VALID = ./$(zeno_spec_exe) --check-lex $(srcdir)/tests/lex/valid
CHECK_LEX_INVALID = ./$(zeno_spec_exe) --check-lex-invalid $(srcdir)/tests/lex/invalid

test-lex-valid: zeno-spec$(E)
	$(CHECK_LEX_VALID)/bom.zn
	$(CHECK_LEX_VALID)/cr.zn
	$(CHECK_LEX_VALID)/crlf.zn
	$(CHECK_LEX_VALID)/tab.zn
	$(CHECK_LEX_VALID)/tokens.zn

test-lex-invalid: zeno-spec$(E)
	$(CHECK_LEX_INVALID)/bad_utf8.zn
	$(CHECK_LEX_INVALID)/bad_utf8_in_block_comment.zn
	$(CHECK_LEX_INVALID)/bad_utf8_in_line_comment.zn
	$(CHECK_LEX_INVALID)/binary_literal_no_value.zn
	$(CHECK_LEX_INVALID)/binary_literal_trailing_junk.zn
	$(CHECK_LEX_INVALID)/binary_literal_trailing_underscore.zn
	$(CHECK_LEX_INVALID)/decimal_literal_leading_zero.zn
	$(CHECK_LEX_INVALID)/decimal_literal_trailing_junk.zn
	$(CHECK_LEX_INVALID)/decimal_literal_trailing_underscore.zn
	$(CHECK_LEX_INVALID)/hex_literal_no_value.zn
	$(CHECK_LEX_INVALID)/hex_literal_trailing_junk.zn
	$(CHECK_LEX_INVALID)/hex_literal_trailing_underscore.zn
	$(CHECK_LEX_INVALID)/unclosed_block_comment.zn

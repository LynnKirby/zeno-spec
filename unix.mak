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
	src/driver/diagnostics$(O) \
	src/driver/lex_actions$(O) \
	src/driver/main$(O) \
	src/driver/parse_actions$(O) \
	src/lang/ast$(O) \
	src/lang/lex$(O) \
	src/lang/parse.tab$(O) \
	src/lang/token$(O) \
	src/support/arena$(O) \
	src/support/bigint$(O) \
	src/support/encoding$(O) \
	src/support/format$(O) \
	src/support/io$(O) \
	src/support/malloc$(O)

zeno_spec_exe = zeno-spec$(E)

#
# Top-level targets
#

all: $(zeno_spec_exe)

clean:
	rm -f $(zeno_spec_exe) $(zeno_spec_objects)
	rm -f src/lang/parse.output src/lang/parse.tab.c
	rm -f src/driver/*.d src/lang/*.d src/support/*.d

-include src/driver/*.d
-include src/lang/*.d
-include src/support/*.d

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

src/lang/parse.tab.c: src/lang/parse.y
	@echo "YACC $@: $?"
	$(Q)mkdir -p $(@D)
	$(Q)LC_ALL=C $(YACC) $(YFLAGS) -b src/lang/parse $?

#
# Tests
#

# TODO: an actual test framework
test: test-lex

test-lex: test-lex-valid test-lex-invalid

CHECK_LEX_VALID = $(Q)./$(zeno_spec_exe) --check-lex $(srcdir)/tests/lex/valid
CHECK_LEX_INVALID = $(Q)./$(zeno_spec_exe) --check-lex-invalid $(srcdir)/tests/lex/invalid

test-lex-valid: zeno-spec$(E)
	$(CHECK_LEX_VALID)/bom.zn
	$(CHECK_LEX_VALID)/cr.zn
	$(CHECK_LEX_VALID)/crlf.zn
	$(CHECK_LEX_VALID)/tab.zn
	$(CHECK_LEX_VALID)/tokens.zn
	@echo "PASS lex-valid"

test-lex-invalid: zeno-spec$(E)
	$(CHECK_LEX_INVALID)/bad_utf8.zn
	$(CHECK_LEX_INVALID)/bad_utf8_in_block_comment.zn
	$(CHECK_LEX_INVALID)/bad_utf8_in_line_comment.zn
	$(CHECK_LEX_INVALID)/binary_literal_extra_underscore.zn
	$(CHECK_LEX_INVALID)/binary_literal_leading_underscore.zn
	$(CHECK_LEX_INVALID)/binary_literal_no_value.zn
	$(CHECK_LEX_INVALID)/binary_literal_trailing_junk.zn
	$(CHECK_LEX_INVALID)/binary_literal_trailing_underscore.zn
	$(CHECK_LEX_INVALID)/decimal_literal_extra_underscore.zn
	$(CHECK_LEX_INVALID)/decimal_literal_leading_zero.zn
	$(CHECK_LEX_INVALID)/decimal_literal_trailing_junk.zn
	$(CHECK_LEX_INVALID)/decimal_literal_trailing_underscore.zn
	$(CHECK_LEX_INVALID)/hex_literal_extra_underscore.zn
	$(CHECK_LEX_INVALID)/hex_literal_leading_underscore.zn
	$(CHECK_LEX_INVALID)/hex_literal_no_value.zn
	$(CHECK_LEX_INVALID)/hex_literal_trailing_junk.zn
	$(CHECK_LEX_INVALID)/hex_literal_trailing_underscore.zn
	$(CHECK_LEX_INVALID)/unclosed_block_comment.zn
	@echo "PASS lex-invalid"

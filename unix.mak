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
# Executable config
#

lib_objects = \
	src/ast/context$(O) \
	src/ast/dump$(O) \
	src/ast/nodes$(O) \
	src/ast/source$(O) \
	src/ast/string$(O) \
	src/driver/diagnostics$(O) \
	src/driver/lex_actions$(O) \
	src/driver/parse_actions$(O) \
	src/parsing/lex$(O) \
	src/parsing/parse.tab$(O) \
	src/sema/decl_map$(O) \
	src/sema/type_checking$(O) \
	src/support/arena$(O) \
	src/support/bigint$(O) \
	src/support/encoding$(O) \
	src/support/format$(O) \
	src/support/hash_map$(O) \
	src/support/io$(O) \
	src/support/malloc$(O) \
	src/support/string_ref$(O) \
	src/parsing/token$(O)

zeno_spec_objects = \
	$(lib_objects) \
	src/driver/main$(O)

zeno_spec_exe = zeno-spec$(E)

lex_fuzz_objects = $(lib_objects) src/parsing/lex_fuzz$(O)
lex_fuzz_exe = lex_fuzz$(E)

hash_map_test_objects = $(lib_objects) src/support/hash_map_test$(O)
hash_map_test_exe = hash_map_test$(E)

#
# Top-level targets
#

all: $(zeno_spec_exe) $(hash_map_test)

clean:
	@echo "CLEAN"
	$(Q)rm -f $(lib_objects)
	$(Q)rm -f $(zeno_spec_exe) src/driver/main$(O)
	$(Q)rm -f $(lex_fuzz_exe) src/parsing/lex_fuzz$(O)
	$(Q)rm -f $(hash_map_test_exe) src/support/hash_map_test$(O)
	$(Q)rm -f src/parsing/parse.output src/parsing/parse.tab.c

-include src/ast/*.d
-include src/driver/*.d
-include src/lang/*.d
-include src/support/*.d
-include src/parsing/*.d

#
# Compile targets
#

.c$(O):
	@echo "CC $@: $<"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) -c $(CFLAGS) $(CPPFLAGS) -I$(srcdir) -o $@ $<

#
# zeno-spec executable
#

$(zeno_spec_exe): $(zeno_spec_objects)
	@echo "LD $@"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(zeno_spec_objects) $(LIBS)

src/parsing/parse.tab.c: src/parsing/parse.y
	@echo "YACC $@: $?"
	$(Q)mkdir -p $(@D)
	$(Q)LC_ALL=C $(YACC) $(YFLAGS) -b src/parsing/parse $?

#
# Fuzz executables
#

fuzz: $(lex_fuzz_exe)

$(lex_fuzz_exe): $(lex_fuzz_objects)
	@echo "LD $@"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(lex_fuzz_objects) $(LIBS)

#
# Test executables
#

$(hash_map_test_exe): $(hash_map_test_objects)
	@echo "LD $@"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(hash_map_test_objects) $(LIBS)

#
# Tests
#

# TODO: an actual test framework
test: test-lex test-types test-hash-map

test-lex: test-lex-valid test-lex-invalid

CHECK_LEX_VALID = $(Q)./$(zeno_spec_exe) --check-lex $(srcdir)/tests/lex/valid
CHECK_LEX_INVALID = $(Q)./$(zeno_spec_exe) --check-lex-invalid $(srcdir)/tests/lex/invalid
CHECK_TYPES_INVALID = $(Q)./$(zeno_spec_exe) --check-types-invalid $(srcdir)/tests/binding/invalid

test-lex-valid: $(zeno_spec_exe)
	@echo "TEST lex-valid"
	$(CHECK_LEX_VALID)/bom.zn
	$(CHECK_LEX_VALID)/cr.zn
	$(CHECK_LEX_VALID)/crlf.zn
	$(CHECK_LEX_VALID)/tab.zn
	$(CHECK_LEX_VALID)/tokens.zn

test-lex-invalid: $(zeno_spec_exe)
	@echo "TEST lex-invalid"
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

test-types: test-types-invalid

test-types-invalid: $(zeno_spec_exe)
	@echo "TEST types-invalid"
	$(CHECK_TYPES_INVALID)/undefined_return_type.zn

test-hash-map: $(hash_map_test_exe)
	@echo "TEST hash-map"
	$(Q)./$(hash_map_test_exe)

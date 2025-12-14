#include "src/lang/lex.h"
#include "src/support/encoding.h"
#include "src/lang/limits.h"

#include <assert.h>
#include <string.h>

void Lexer_init(Lexer* lexer, ByteStringRef source, LexerConfig const* config) {
    /* Must be nul-terminated. */
    assert(source.size > 0);
    assert(source.data[source.size - 1] == 0);

    lexer->done = false;
    lexer->cursor = (uint8_t const*)source.data;
    lexer->limit = lexer->cursor + source.size - 1;
    lexer->cursor_pos.line = 1;
    lexer->cursor_pos.column = 1;
    lexer->total_characters = 0;

    if (config == NULL) {
        lexer->config.tab_stop = 8;
    } else {
        lexer->config = *config;
    }

    /* Skip byte order mark. */
    if (
        lexer->cursor[0] == 0xEF
        && lexer->cursor[1] == 0xBB
        && lexer->cursor[2] == 0xBF
    ) {
        lexer->cursor += 3;
        lexer->total_characters += 1;
    }
}

void Lexer_destroy(Lexer* lexer) {
    (void)lexer; /* unused */
    /* Currently no-op. */
}

static void sync_token_pos_to_cursor(Lexer* lexer, LexResult* result) {
    result->u.token.pos = lexer->cursor_pos;
}

static void set_token_kind(LexResult* result, TokenKind kind) {
    result->u.token.kind = kind;
}

static void set_token_integer(LexResult* result, BigInt integer) {
    result->u.token.value.integer = integer;
}

static void set_token_string(LexResult* result, StringRef string) {
    result->u.token.value.string = string;
}

static void set_error(Lexer* lexer, LexResult* result, LexErrorKind error) {
    SourcePos pos;

    pos = result->u.token.pos;

    result->is_token = false;
    result->u.error.pos = pos;
    result->u.error.kind = error;

    lexer->done = true;
}

static void increment_line(Lexer* lexer, LexResult* result) {
    lexer->cursor_pos.line += 1;
    lexer->cursor_pos.column = 1;
    lexer->characters_in_line = 0;

    if (lexer->cursor_pos.line > MAX_LINES_PER_FILE) {
        sync_token_pos_to_cursor(lexer, result);
        set_error(lexer, result, LexErrorKind_LineLimitExceeded);
        longjmp(lexer->exit_jmp_buf, 1);
    }
}

static void handle_invisible_character(Lexer* lexer, LexResult* result) {
    lexer->characters_in_line += 1;
    lexer->total_characters += 1;

    if (lexer->characters_in_line > MAX_CHARACTERS_PER_LINE) {
        sync_token_pos_to_cursor(lexer, result);
        set_error(lexer, result, LexErrorKind_ColumnLimitExceeded);
        longjmp(lexer->exit_jmp_buf, 1);
    }

    if (lexer->total_characters > MAX_CHARACTERS_PER_FILE) {
        sync_token_pos_to_cursor(lexer, result);
        set_error(lexer, result, LexErrorKind_CharacterLimitExceeded);
        longjmp(lexer->exit_jmp_buf, 1);
    }
}

static void handle_normal_character(Lexer* lexer, LexResult* result) {
    lexer->cursor_pos.column += 1;
    handle_invisible_character(lexer, result);
}

static void handle_tab(Lexer* lexer, LexResult* result) {
    lexer->cursor_pos.column +=
        lexer->config.tab_stop
        - (lexer->cursor_pos.column % lexer->config.tab_stop);
    handle_invisible_character(lexer, result);
}

static void handle_crlf_line_terminator(Lexer* lexer, LexResult* result) {
    handle_invisible_character(lexer, result);
    handle_invisible_character(lexer, result);
    increment_line(lexer, result);
}

static void handle_line_terminator(Lexer* lexer, LexResult* result) {
    handle_invisible_character(lexer, result);
    increment_line(lexer, result);
}

static int is_id_start(int32_t ch) {
    return (ch >= 'a' && ch <= 'z')
        || (ch >= 'A' && ch <= 'Z')
        || (ch == '_');
}

static int is_id_continue(int32_t ch) {
    return is_id_start(ch)
        || (ch >= '0' && ch <= '9');
}

static int is_digit(int32_t ch, int base) {
    if (ch >= '0' && ch <= '9') {
        return (ch - '0') < base;
    } else if (ch >= 'a' && ch <= 'z') {
        return (ch - 'a' + 10) < base;
    } else if (ch >= 'A' && ch <= 'Z') {
        return (ch - 'A' + 10) < base;
    }
    return false;
}

static TokenKind id_or_keyword(uint8_t const* data, size_t size) {
    #define X(name, str)                                                 \
        if ((sizeof(str) - 1) == size && memcmp(data, str, size) == 0) { \
            return TokenKind_##name;                                     \
        }
    KEYWORD_KIND_LIST(X)
    #undef X
    return TokenKind_Identifier;
}

static int lex_line_comment(Lexer* lexer, LexResult* result) {
    uint8_t const* cursor;
    (void)result; /* unused */

    assert(lexer->cursor[0] == '/');
    assert(lexer->cursor[1] == '/');
    lexer->cursor += 2;
    handle_normal_character(lexer, result);
    handle_normal_character(lexer, result);

    cursor = lexer->cursor;

    for (;;) {
        int32_t ch;
        ch = utf8_decode(&cursor, lexer->limit);
        if (ch == '\r' || ch == '\n' || ch == U_EOF) {
            break;
        }
        if (ch == U_BAD) {
            sync_token_pos_to_cursor(lexer, result);
            set_error(lexer, result, LexErrorKind_BadEncoding);
            return false;
        }
        lexer->cursor = cursor;
        handle_normal_character(lexer, result);
    }

    return true;
}

static int lex_block_comment(Lexer* lexer, LexResult* result) {
    uint8_t const* cursor;
    unsigned nesting;

    assert(lexer->cursor[0] == '/');
    assert(lexer->cursor[1] == '*');
    lexer->cursor += 2;
    handle_normal_character(lexer, result);
    handle_normal_character(lexer, result);

    cursor = lexer->cursor;
    nesting = 1;

    while (nesting > 0) {
        int32_t ch;
        ch = utf8_decode(&cursor, lexer->limit);
        switch (ch) {
        case U_EOF:
            sync_token_pos_to_cursor(lexer, result);
            set_error(lexer, result, LexErrorKind_UnclosedBlockComment);
            return false;
        case U_BAD:
            sync_token_pos_to_cursor(lexer, result);
            set_error(lexer, result, LexErrorKind_BadEncoding);
            return false;
        case '\r':
            if (cursor[0] == '\n') {
                cursor += 1;
                handle_crlf_line_terminator(lexer, result);
                break;
            }
            /* fallthrough */
        case '\n':
            handle_line_terminator(lexer, result);
            break;
        case '\t':
            handle_tab(lexer, result);
            break;
        case '*':
            if (cursor[0] == '/') {
                nesting -= 1;
                cursor += 1;
                handle_normal_character(lexer, result);
                handle_normal_character(lexer, result);
            }
            break;
        case '/':
            if (cursor[0] == '*') {
                nesting += 1;
                cursor += 1;
                handle_normal_character(lexer, result);
                handle_normal_character(lexer, result);
            }
            break;
        default:
            handle_normal_character(lexer, result);
            break;
        }
        lexer->cursor = cursor;
    }

    return true;
}

static void lex_number_literal(Lexer* lexer, LexResult* result) {
    int base = 10;
    uint8_t const* digits_start;

    set_token_kind(result, TokenKind_IntLiteral);

    if (lexer->cursor[0] == '0') {
        if (lexer->cursor[1] == 'x' || lexer->cursor[1] == 'X') {
            lexer->cursor += 2;
            handle_normal_character(lexer, result);
            handle_normal_character(lexer, result);
            base = 16;
        } else if (lexer->cursor[1] == 'b' || lexer->cursor[1] == 'B') {
            base = 2;
            lexer->cursor += 2;
            handle_normal_character(lexer, result);
            handle_normal_character(lexer, result);
        } else {
            lexer->cursor += 1;
            set_token_integer(result, BigInt_from_int(0));
            handle_normal_character(lexer, result);
            if (is_id_continue(lexer->cursor[0])) {
                set_error(lexer, result, LexErrorKind_DecimalLeadingZero);
            }
            return;
        }

        /* Base prefix but no value (example: `0x`) */
        if (!is_digit(lexer->cursor[0], base)) {
            set_error(lexer, result, LexErrorKind_BadIntLiteral);
            return;
        }
    }

    digits_start = lexer->cursor;

    for (;;) {
        if (is_digit(lexer->cursor[0], base)) {
            lexer->cursor += 1;
            handle_normal_character(lexer, result);
            continue;
        }

        if (lexer->cursor[0] == '_') {
            /* Leading underscore. */
            if (digits_start == lexer->cursor) {
                set_error(lexer, result, LexErrorKind_BadIntLiteral);
                return;
            }

            /* Extra underscore. */
            if (lexer->cursor[1] == '_') {
                set_error(lexer, result, LexErrorKind_BadIntLiteral);
                return;
            }

            lexer->cursor += 1;
            handle_normal_character(lexer, result);
            continue;
        }

        /* Junk that's not a digit or underscore. */
        if (is_id_continue(lexer->cursor[0])) {
            set_error(lexer, result, LexErrorKind_BadIntLiteral);
            return;
        }

        break;
    }

    /* Trailing underscore. */
    if (lexer->cursor[-1] == '_') {
        set_error(lexer, result, LexErrorKind_BadIntLiteral);
        return;
    }

    {
        ByteStringRef digits;
        digits.data = (char const*)digits_start;
        digits.size = lexer->cursor - digits_start;
        set_token_integer(result, BigInt_parse(digits, base));
    }
}

static int next(Lexer* lexer, LexResult* result) {
    if (lexer->done) {
        return false;
    }

loop:
    result->is_token = true;
    sync_token_pos_to_cursor(lexer, result);

    if (is_id_start(lexer->cursor[0])) {
        uint8_t const* start;
        TokenKind kind;
        start = lexer->cursor;
        while (is_id_continue(lexer->cursor[0])) {
            lexer->cursor += 1;
            handle_normal_character(lexer, result);
        }
        kind = id_or_keyword(start, lexer->cursor - start);
        set_token_kind(result, kind);
        if (kind == TokenKind_Identifier) {
            StringRef string;
            string.data = start;
            string.size = lexer->cursor - start;
            set_token_string(result, string);
        }
        return true;
    }

    if (is_digit(lexer->cursor[0], 10)) {
        lex_number_literal(lexer, result);
        return true;
    }

    switch (lexer->cursor[0]) {
    case ' ':
        lexer->cursor += 1;
        handle_normal_character(lexer, result);
        goto loop;

    case '\t':
        lexer->cursor += 1;
        handle_tab(lexer, result);
        goto loop;

    case '\r':
        if (lexer->cursor[1] == '\n') {
            lexer->cursor += 1;
            handle_crlf_line_terminator(lexer, result);
            goto loop;
        }
        /* fallthrough */
    case '\n':
        lexer->cursor += 1;
        handle_line_terminator(lexer, result);
        goto loop;

    case 0:
        /* Check if EOF or embedded nul. */
        if (lexer->cursor == lexer->limit) {
            lexer->done = true;
            set_token_kind(result, TokenKind_EndOfFile);
            return true;
        }
        break;

    case '.':
        lexer->cursor += 1;
        handle_normal_character(lexer, result);
        if (lexer->cursor[0] == '.') {
            if (lexer->cursor[1] == '.') {
                lexer->cursor += 2;
                handle_normal_character(lexer, result);
                handle_normal_character(lexer, result);
                set_token_kind(result, TokenKind_ClosedRange);
                return true;
            }
            if (lexer->cursor[1] == '<') {
                lexer->cursor += 2;
                handle_normal_character(lexer, result);
                handle_normal_character(lexer, result);
                set_token_kind(result, TokenKind_HalfOpenRange);
                return true;
            }
        }
        set_token_kind(result, TokenKind_Period);
        return true;

    case '=':
        lexer->cursor += 1;
        handle_normal_character(lexer, result);
        switch (lexer->cursor[0]) {
        case '=':
            lexer->cursor += 1;
            handle_normal_character(lexer, result);
            set_token_kind(result, TokenKind_EqualEqual);
            break;
        case '>':
            lexer->cursor += 1;
            handle_normal_character(lexer, result);
            set_token_kind(result, TokenKind_FatArrow);
            break;
        default:
            set_token_kind(result, TokenKind_Equal);
            break;
        }
        return true;

    case '-':
        lexer->cursor += 1;
        handle_normal_character(lexer, result);
        switch (lexer->cursor[0]) {
        case '=':
            lexer->cursor += 1;
            handle_normal_character(lexer, result);
            set_token_kind(result, TokenKind_MinusEqual);
            break;
        case '>':
            lexer->cursor += 1;
            handle_normal_character(lexer, result);
            set_token_kind(result, TokenKind_ThinArrow);
            break;
        default:
            set_token_kind(result, TokenKind_Minus);
            break;
        }
        return true;

    case '/':
        switch (lexer->cursor[1]) {
        case '*':
            if (lex_block_comment(lexer, result)) {
                goto loop;
            }
            break;
        case '/':
            if (lex_line_comment(lexer, result)) {
                goto loop;
            }
            break;
        case '=':
            lexer->cursor += 2;
            handle_normal_character(lexer, result);
            handle_normal_character(lexer, result);
            set_token_kind(result, TokenKind_SlashEqual);
            break;
        default:
            lexer->cursor += 1;
            handle_normal_character(lexer, result);
            set_token_kind(result, TokenKind_Slash);
            break;
        }
        return true;

    #define SYMBOL_X(ch, name)                        \
        case ch:                                      \
            lexer->cursor += 1;                       \
            handle_normal_character(lexer, result);   \
            set_token_kind(result, TokenKind_##name); \
            return true;

    #define SYMBOL_XE(ch, name)                                  \
        case ch:                                                 \
            lexer->cursor += 1;                                  \
            handle_normal_character(lexer, result);              \
            if (lexer->cursor[0] == '=') {                       \
                lexer->cursor += 1;                              \
                handle_normal_character(lexer, result);          \
                set_token_kind(result, TokenKind_##name##Equal); \
            } else {                                             \
                set_token_kind(result, TokenKind_##name);        \
            }                                                    \
            return true;

    #define SYMBOL_XXE(ch, name)                                           \
        case ch:                                                           \
            lexer->cursor += 1;                                            \
            handle_normal_character(lexer, result);                        \
            if (lexer->cursor[0] == ch) {                                  \
                lexer->cursor += 1;                                        \
                handle_normal_character(lexer, result);                    \
                if (lexer->cursor[0] == '=') {                             \
                    lexer->cursor += 1;                                    \
                    handle_normal_character(lexer, result);                \
                    set_token_kind(result, TokenKind_##name##name##Equal); \
                } else {                                                   \
                    set_token_kind(result, TokenKind_##name##name);        \
                }                                                          \
            } else if (lexer->cursor[0] == '=') {                          \
                lexer->cursor += 1;                                        \
                handle_normal_character(lexer, result);                    \
                set_token_kind(result, TokenKind_##name##Equal);           \
            } else {                                                       \
                set_token_kind(result, TokenKind_##name);                  \
            }                                                              \
            return true;

    SYMBOL_X('(', LeftParen)
    SYMBOL_X(')', RightParen)
    SYMBOL_X('{', LeftCurly)
    SYMBOL_X('}', RightCurly)
    SYMBOL_X('[', LeftSquare)
    SYMBOL_X(']', RightSquare)
    SYMBOL_X(',', Comma)
    SYMBOL_X(':', Colon)
    SYMBOL_X(';', Semicolon)
    SYMBOL_X('~', Tilde)
    SYMBOL_X('@', At)

    SYMBOL_XE('!', Exclaim)
    SYMBOL_XE('+', Plus)
    SYMBOL_XE('*', Star)
    SYMBOL_XE('%', Percent)

    SYMBOL_XXE('<', Less)
    SYMBOL_XXE('>', Greater)
    SYMBOL_XXE('&', Amp)
    SYMBOL_XXE('|', Bar)
    SYMBOL_XXE('^', Caret)

    #undef SYMBOL_X
    #undef SYMBOL_XE
    #undef SYMBOL_XXE
    }

    {
        int32_t ch;
        ch = utf8_decode(&lexer->cursor, lexer->limit);
        if (ch == U_BAD) {
            set_error(lexer, result, LexErrorKind_BadEncoding);
        } else {
            set_error(lexer, result, LexErrorKind_UnexpectedCharacter);
            result->u.error.value.character = ch;
        }
    }

    return true;
}

int Lexer_next(Lexer* lexer, LexResult* result) {
    if (setjmp(lexer->exit_jmp_buf)) {
        return true;
    }
    return next(lexer, result);
}

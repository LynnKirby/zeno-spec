#include "src/syntax/lex.h"
#include "src/lang/limits.h"
#include "src/support/encoding.h"
#include "src/support/malloc.h"

#include <assert.h>
#include <string.h>

typedef struct LexContext {
    AstContext* ast;
    LexerConfig config;

    uint8_t const* cursor;
    uint8_t const* limit;

    SourcePos token_pos;
    SourcePos cursor_pos;

    uint32_t characters_in_line;
    uint32_t total_characters;

    jmp_buf exit_jmp_buf;

    /* Success result */
    Token* tokens_data;
    size_t tokens_size;
    size_t tokens_capacity;

    /* Error result */
    LexError error;
} LexContext;

static void sync_token_pos_to_cursor(LexContext* context) {
    context->token_pos = context->cursor_pos;
}

static void push_token(LexContext* context, TokenKind kind) {
    Token* token;
    context->tokens_data = ensure_array_capacity(
        sizeof(Token),
        context->tokens_data,
        &context->tokens_size,
        &context->tokens_capacity,
        1
    );
    token = &context->tokens_data[context->tokens_size];
    context->tokens_size += 1;
    token->kind = kind;
    token->pos = context->token_pos;
}

static void push_integer_token(
    LexContext* context, TokenKind kind, BigInt integer
) {
    push_token(context, kind);
    context->tokens_data[context->tokens_size - 1].value.integer = integer;
}

static void push_string_token(
    LexContext* context, TokenKind kind, StringRef string
) {
    push_token(context, kind);
    context->tokens_data[context->tokens_size - 1].value.string =
        AstContext_add_string(context->ast, string);
}

static void exit_with_character_error(
    LexContext* context, LexErrorKind error, int32_t character
) {
    context->error.kind = error;
    context->error.pos = context->token_pos;
    context->error.value.character = character;

    longjmp(context->exit_jmp_buf, 1);
}

static void exit_with_error(LexContext* context, LexErrorKind error) {
    context->error.kind = error;
    context->error.pos = context->token_pos;

    longjmp(context->exit_jmp_buf, 1);
}

static void increment_line(LexContext* context) {
    context->cursor_pos.line += 1;
    context->cursor_pos.column = 1;
    context->characters_in_line = 0;

    if (context->cursor_pos.line > MAX_LINES_PER_FILE) {
        sync_token_pos_to_cursor(context);
        exit_with_error(context, LexErrorKind_LineLimitExceeded);
    }
}

static void handle_invisible_character(LexContext* context) {
    context->characters_in_line += 1;
    context->total_characters += 1;

    if (context->characters_in_line > MAX_CHARACTERS_PER_LINE) {
        sync_token_pos_to_cursor(context);
        exit_with_error(context, LexErrorKind_ColumnLimitExceeded);
    }

    if (context->total_characters > MAX_CHARACTERS_PER_FILE) {
        sync_token_pos_to_cursor(context);
        exit_with_error(context, LexErrorKind_CharacterLimitExceeded);
    }
}

static void handle_normal_character(LexContext* context) {
    context->cursor_pos.column += 1;
    handle_invisible_character(context);
}

static void handle_tab(LexContext* context) {
    context->cursor_pos.column +=
        context->config.tab_stop
        - (context->cursor_pos.column % context->config.tab_stop);
    handle_invisible_character(context);
}

static void handle_crlf_line_terminator(LexContext* context) {
    handle_invisible_character(context);
    handle_invisible_character(context);
    increment_line(context);
}

static void handle_line_terminator(LexContext* context) {
    handle_invisible_character(context);
    increment_line(context);
}

/*
 * Classification
 */

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

/*
 * Comments
 */

static int lex_line_comment(LexContext* context) {
    uint8_t const* cursor;

    assert(context->cursor[0] == '/');
    assert(context->cursor[1] == '/');
    context->cursor += 2;
    handle_normal_character(context);
    handle_normal_character(context);

    cursor = context->cursor;

    for (;;) {
        int32_t ch;
        ch = utf8_decode(&cursor, context->limit);
        if (ch == '\r' || ch == '\n' || ch == U_EOF) {
            break;
        }
        if (ch == U_BAD) {
            sync_token_pos_to_cursor(context);
            exit_with_error(context, LexErrorKind_BadEncoding);
            return false;
        }
        context->cursor = cursor;
        handle_normal_character(context);
    }

    return true;
}

static int lex_block_comment(LexContext* context) {
    uint8_t const* cursor;
    unsigned nesting;

    assert(context->cursor[0] == '/');
    assert(context->cursor[1] == '*');
    context->cursor += 2;
    handle_normal_character(context);
    handle_normal_character(context);

    cursor = context->cursor;
    nesting = 1;

    while (nesting > 0) {
        int32_t ch;
        ch = utf8_decode(&cursor, context->limit);
        switch (ch) {
        case U_EOF:
            sync_token_pos_to_cursor(context);
            exit_with_error(context, LexErrorKind_UnclosedBlockComment);
            return false;
        case U_BAD:
            sync_token_pos_to_cursor(context);
            exit_with_error(context, LexErrorKind_BadEncoding);
            return false;
        case '\r':
            if (cursor[0] == '\n') {
                cursor += 1;
                handle_crlf_line_terminator(context);
                break;
            }
            /* fallthrough */
        case '\n':
            handle_line_terminator(context);
            break;
        case '\t':
            handle_tab(context);
            break;
        case '*':
            if (cursor[0] == '/') {
                nesting -= 1;
                cursor += 1;
                handle_normal_character(context);
                handle_normal_character(context);
            }
            break;
        case '/':
            if (cursor[0] == '*') {
                nesting += 1;
                cursor += 1;
                handle_normal_character(context);
                handle_normal_character(context);
            }
            break;
        default:
            handle_normal_character(context);
            break;
        }
        context->cursor = cursor;
    }

    return true;
}

/*
 * Number literal
 */

static void lex_number_literal(LexContext* context) {
    int base = 10;
    uint8_t const* digits_start;

    if (context->cursor[0] == '0') {
        if (context->cursor[1] == 'x' || context->cursor[1] == 'X') {
            context->cursor += 2;
            handle_normal_character(context);
            handle_normal_character(context);
            base = 16;
        } else if (context->cursor[1] == 'b' || context->cursor[1] == 'B') {
            base = 2;
            context->cursor += 2;
            handle_normal_character(context);
            handle_normal_character(context);
        } else {
            context->cursor += 1;
            push_integer_token(
                context, TokenKind_IntLiteral, BigInt_from_int(0)
            );
            handle_normal_character(context);
            if (is_id_continue(context->cursor[0])) {
                exit_with_error(context, LexErrorKind_DecimalLeadingZero);
            }
            return;
        }

        /* Base prefix but no value (example: `0x`) */
        if (!is_digit(context->cursor[0], base)) {
            exit_with_error(context, LexErrorKind_BadIntLiteral);
            return;
        }
    }

    digits_start = context->cursor;

    for (;;) {
        if (is_digit(context->cursor[0], base)) {
            context->cursor += 1;
            handle_normal_character(context);
            continue;
        }

        if (context->cursor[0] == '_') {
            /* Leading underscore. */
            if (digits_start == context->cursor) {
                exit_with_error(context, LexErrorKind_BadIntLiteral);
                return;
            }

            /* Extra underscore. */
            if (context->cursor[1] == '_') {
                exit_with_error(context, LexErrorKind_BadIntLiteral);
                return;
            }

            context->cursor += 1;
            handle_normal_character(context);
            continue;
        }

        /* Junk that's not a digit or underscore. */
        if (is_id_continue(context->cursor[0])) {
            exit_with_error(context, LexErrorKind_BadIntLiteral);
            return;
        }

        break;
    }

    /* Trailing underscore. */
    if (context->cursor[-1] == '_') {
        exit_with_error(context, LexErrorKind_BadIntLiteral);
        return;
    }

    {
        ByteStringRef digits;
        digits.data = (char const*)digits_start;
        digits.size = context->cursor - digits_start;
        push_integer_token(
            context, TokenKind_IntLiteral, BigInt_parse(digits, base)
        );
    }
}

/*
 * Basic
 */

static void lex_bytes_inner(LexContext* context) {
loop:
    sync_token_pos_to_cursor(context);

    if (is_id_start(context->cursor[0])) {
        uint8_t const* start;
        TokenKind kind;
        start = context->cursor;
        while (is_id_continue(context->cursor[0])) {
            context->cursor += 1;
            handle_normal_character(context);
        }
        kind = id_or_keyword(start, context->cursor - start);
        if (kind == TokenKind_Identifier) {
            StringRef string;
            string.data = start;
            string.size = context->cursor - start;
            push_string_token(context, TokenKind_Identifier, string);
        } else {
            push_token(context, kind);
        }
        goto loop;
    }

    if (is_digit(context->cursor[0], 10)) {
        lex_number_literal(context);
        goto loop;
    }

    switch (context->cursor[0]) {
    case ' ':
        context->cursor += 1;
        handle_normal_character(context);
        goto loop;

    case '\t':
        context->cursor += 1;
        handle_tab(context);
        goto loop;

    case '\r':
        if (context->cursor[1] == '\n') {
            context->cursor += 1;
            handle_crlf_line_terminator(context);
            goto loop;
        }
        /* fallthrough */
    case '\n':
        context->cursor += 1;
        handle_line_terminator(context);
        goto loop;

    case 0:
        /* Check if EOF or embedded nul. */
        if (context->cursor == context->limit) {
            push_token(context, TokenKind_EndOfFile);
            return;
        }
        break;

    case '.':
        context->cursor += 1;
        handle_normal_character(context);
        if (context->cursor[0] == '.') {
            if (context->cursor[1] == '.') {
                context->cursor += 2;
                handle_normal_character(context);
                handle_normal_character(context);
                push_token(context, TokenKind_ClosedRange);
                goto loop;
            }
            if (context->cursor[1] == '<') {
                context->cursor += 2;
                handle_normal_character(context);
                handle_normal_character(context);
                push_token(context, TokenKind_HalfOpenRange);
                goto loop;
            }
        }
        push_token(context, TokenKind_Period);
        goto loop;

    case '=':
        context->cursor += 1;
        handle_normal_character(context);
        switch (context->cursor[0]) {
        case '=':
            context->cursor += 1;
            handle_normal_character(context);
            push_token(context, TokenKind_EqualEqual);
            break;
        case '>':
            context->cursor += 1;
            handle_normal_character(context);
            push_token(context, TokenKind_FatArrow);
            break;
        default:
            push_token(context, TokenKind_Equal);
            break;
        }
        goto loop;

    case '-':
        context->cursor += 1;
        handle_normal_character(context);
        switch (context->cursor[0]) {
        case '=':
            context->cursor += 1;
            handle_normal_character(context);
            push_token(context, TokenKind_MinusEqual);
            break;
        case '>':
            context->cursor += 1;
            handle_normal_character(context);
            push_token(context, TokenKind_ThinArrow);
            break;
        default:
            push_token(context, TokenKind_Minus);
            break;
        }
        goto loop;

    case '/':
        switch (context->cursor[1]) {
        case '*':
            lex_block_comment(context);
            break;
        case '/':
            lex_line_comment(context);
            break;
        case '=':
            context->cursor += 2;
            handle_normal_character(context);
            handle_normal_character(context);
            push_token(context, TokenKind_SlashEqual);
            break;
        default:
            context->cursor += 1;
            handle_normal_character(context);
            push_token(context, TokenKind_Slash);
            break;
        }
        goto loop;

    #define SYMBOL_X(ch, name)                     \
        case ch:                                   \
            context->cursor += 1;                  \
            handle_normal_character(context);      \
            push_token(context, TokenKind_##name); \
            goto loop;

    #define SYMBOL_XE(ch, name)                               \
        case ch:                                              \
            context->cursor += 1;                             \
            handle_normal_character(context);                 \
            if (context->cursor[0] == '=') {                  \
                context->cursor += 1;                         \
                handle_normal_character(context);             \
                push_token(context, TokenKind_##name##Equal); \
            } else {                                          \
                push_token(context, TokenKind_##name);        \
            }                                                 \
            goto loop;

    #define SYMBOL_XXE(ch, name)                                        \
        case ch:                                                        \
            context->cursor += 1;                                       \
            handle_normal_character(context);                           \
            if (context->cursor[0] == ch) {                             \
                context->cursor += 1;                                   \
                handle_normal_character(context);                       \
                if (context->cursor[0] == '=') {                        \
                    context->cursor += 1;                               \
                    handle_normal_character(context);                   \
                    push_token(context, TokenKind_##name##name##Equal); \
                } else {                                                \
                    push_token(context, TokenKind_##name##name);        \
                }                                                       \
            } else if (context->cursor[0] == '=') {                     \
                context->cursor += 1;                                   \
                handle_normal_character(context);                       \
                push_token(context, TokenKind_##name##Equal);           \
            } else {                                                    \
                push_token(context, TokenKind_##name);                  \
            }                                                           \
            goto loop;

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
        ch = utf8_decode(&context->cursor, context->limit);
        if (ch == U_BAD) {
            exit_with_error(context, LexErrorKind_BadEncoding);
        } else {
            exit_with_character_error(
                context, LexErrorKind_UnexpectedCharacter, ch
            );
        }
    }
}

/*
 * Init
 */

void lex_source(
    LexResult* result,
    AstContext* ast,
    SourceFile const* source,
    LexerConfig const* config
) {
    LexContext context;

    context.ast = ast;
    context.cursor = SourceFile_data(source);
    context.limit = context.cursor + SourceFile_size(source);

    context.cursor_pos.line = 1;
    context.cursor_pos.column = 1;
    context.total_characters = 0;

    context.tokens_data = NULL;
    context.tokens_size = 0;
    context.tokens_capacity = 0;

    if (config == NULL) {
        context.config.tab_stop = 8;
    } else {
        context.config = *config;
    }

    /* Skip byte order mark. */
    if (
        context.cursor[0] == 0xEF
        && context.cursor[1] == 0xBB
        && context.cursor[2] == 0xBF
    ) {
        context.cursor += 3;
        context.total_characters += 1;
    }

    if (setjmp(context.exit_jmp_buf) == 0) {
        lex_bytes_inner(&context);
        result->is_tokens = true;
        result->u.tokens.data = context.tokens_data;
        result->u.tokens.size = context.tokens_size;
    } else {
        xfree(context.tokens_data);
        result->is_tokens = false;
        result->u.error = context.error;
    }
}

/*
 * Zeno parser in Yacc form.
 * Requires byacc or Bison.
 */

%start File

%define api.pure full
%locations
%lex-param {ParseContext* context}
%parse-param {ParseContext* context}

%code top {
    #include "src/syntax/parse.h"

    typedef struct ParseContext {
        TokenList const* tokens;
        size_t token_index;
        AstContext* ast;
        ParseResult* result;
    } ParseContext;

    /* Our location type is the token index. */
    #define YYLTYPE uint32_t

    /* Set the location of non-terminals to the first token. */
    #define YYLLOC_DEFAULT(loc, rhs, n) \
        do {                            \
            (loc) = YYRHSLOC(rhs, 1);   \
        } while (0)
}

%code provides {
    /* Give a nicer name to this builtin type. */
    typedef YYSTYPE YaccValue;

    static void yyerror(
        uint32_t* loc,
        ParseContext* context,
        char const* message
    );

    static int yylex(
        YaccValue* value,
        uint32_t* loc,
        ParseContext* context
    );

    static void success(ParseContext* context, Item* item);
    static void expected(
        ParseContext* context, SyntaxCategory category, uint32_t token_index
    );

    #define SUCCESS(result) success(context, (result))

    #define EXPECTED(category, index)                              \
        do {                                                       \
            expected(context, SyntaxCategory_##category, (index)); \
            YYABORT;                                               \
        } while (0)
}

%union {
    Item* item;
    Expr* expr;
    StringRef string;
    BigInt integer;
}

%type <item> Item
%type <item> FunctionItem

%type <expr> Block
%type <expr> Stmt
%type <expr> ReturnStmt

%type <expr> Expr
%type <expr> PrimaryExpr

%type <expr> Type

%type <expr> ReturnType

/* Set to 0 so that EndOfFile == YYEOF */
%token EndOfFile 0

%token <string> Identifier
%token <integer> IntLiteral

%token LeftParen
%token RightParen
%token LeftCurly
%token RightCurly
%token LeftSquare
%token RightSquare

%token Period
%token Comma
%token Colon
%token Semicolon

%token Equal
%token EqualEqual
%token Exclaim
%token ExclaimEqual
%token Plus
%token PlusEqual
%token Minus
%token MinusEqual
%token Star
%token StarEqual
%token Slash
%token SlashEqual
%token Percent
%token PercentEqual
%token Less
%token LessEqual
%token LessLess
%token LessLessEqual
%token Greater
%token GreaterEqual
%token GreaterGreater
%token GreaterGreaterEqual

%token Tilde
%token Amp
%token AmpEqual
%token AmpAmp
%token AmpAmpEqual
%token Bar
%token BarEqual
%token BarBar
%token BarBarEqual
%token Caret
%token CaretEqual
%token CaretCaret
%token CaretCaretEqual

%token ThinArrow
%token FatArrow

%token At
%token ClosedRange
%token HalfOpenRange

%token Class
%token Def
%token Else
%token False
%token For
%token If
%token Import
%token In
%token Interface
%token Let
%token Mut
%token Out
%token Return
%token True
%token Var
%token While

%%

/*
 * Items
 */

File: Item { SUCCESS($1); }

Item:
      FunctionItem { $$ = $1; }
    | error { EXPECTED(Item, @$); }

/*
 * Statements
 */

Block: LeftCurly Stmt Semicolon RightCurly { $$ = $2; }

Stmt:
      ReturnStmt { $$ = $1; }
    | error { EXPECTED(Stmt, @$); }

ReturnStmt:
    Return Expr
    { $$ = (Expr*)ReturnExpr_new(context->ast, $2); }

/*
 * Expressions
 */

Expr:
      PrimaryExpr { $$ = $1; }
    | error { EXPECTED(Expr, @$); }

PrimaryExpr:
      IntLiteral { $$ = (Expr*)IntLiteralExpr_new(context->ast, $1); }
    | LeftParen Expr RightParen { $$ = $2; }

/*
 * Type expressions
 */

Type:
      Identifier { $$ = (Expr*)IdentifierExpr_new(context->ast, $1); }
    | error { EXPECTED(Type, @$); }

/*
 * Function-related
 */

ReturnType:
    ThinArrow Type { $$ = $2; }

FunctionItem:
    Def Identifier FunctionItemParams ReturnType Block
    { $$ = (Item*)FunctionItem_new(context->ast, $2, $4, $5); }

FunctionItemParams:
    LeftParen RightParen

%%

#include "src/support/io.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void success(ParseContext* context, Item* item) {
    context->result->kind = ParseResultKind_Success;
    context->result->u.syntax = item;
}

static void expected(
    ParseContext* context, SyntaxCategory category, uint32_t token_index
) {
    ParseError* error;
    Token* token;

    context->result->kind = ParseResultKind_ParseError;

    token = &context->tokens->data[token_index];
    error = &context->result->u.parse_error;

    error->expected_category = category;
    error->actual_token_pos = token->pos;
    error->actual_token_kind = token->kind;
}

static void yyerror(
    uint32_t* loc, ParseContext* context, char const* message
) {
    (void)loc; /* unused */
    context->result->kind = ParseResultKind_YaccError;
    context->result->u.yacc_error.data = message;
    context->result->u.yacc_error.size = strlen(message);
}

static int yylex(YaccValue* value, uint32_t* loc, ParseContext* context) {
    Token const* token;

    (void)loc; /* unused */

    *loc = context->token_index;

    assert(context->token_index < context->tokens->size);
    token = &context->tokens->data[context->token_index];
    context->token_index += 1;

    switch (token->kind) {
    case TokenKind_Identifier:
        value->string = token->value.string;
        break;
    case TokenKind_IntLiteral:
        value->integer = token->value.integer;
        break;
    default:
        break;
    }

    /* Translate TokenKind to Yacc token value. */
    switch (token->kind) {
    #define X(name, str) case TokenKind_##name: return name;
    TOKEN_KIND_LIST(X)
    #undef X
    default:
        assert(0 && "unreachable");
        return YYEOF;
    }
}

void parse(ParseResult* result, AstContext* context, TokenList const* tokens) {
    ParseContext parse_context;
    assert(tokens->size > 0);
    assert(tokens->data[tokens->size - 1].kind == TokenKind_EndOfFile);
    parse_context.tokens = tokens;
    parse_context.token_index = 0;
    parse_context.result = result;
    parse_context.ast = context;
    yyparse(&parse_context);
}

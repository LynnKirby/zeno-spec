# Concrete syntax

A source file is a sequence of Unicode characters.
The *lexical syntax* converts a source file into a list of tokens.
The *hierarchical syntax* converts a list of tokens into a syntax tree.

## Source text {#sec:source-text}

Characters in a source file are limited to (TODO: exclude control, bidi, etc).

## Lexical syntax {#sec:lex}

The lexical syntax is defined with a parsing expression grammar [@PEG].

The tokens produced are:

- The string terminals in the Symbol rule.
- The string terminals in the Keyword rule.
- The IDENTIFIER rule.
- The INT_LITERAL rule.

Note: The "." metacharacter is limited to the characters defined in @sec:source-text.

```peg
SourceFile <- <U+FEFF>? (Trivia / Token)*

Trivia <-
      Whitespace
    / LineTerminator
    / LineComment
    / BlockComment

Token <-
      Symbol
    / Keyword
    / IDENTIFIER
    / INT_LITERAL

Whitespace <- <U+0009 Tab> / <U+0020 Space>

LineTerminator <-
      <U+000D Carriage Return> <U+000A Line Feed>
    / <U+000D Carriage Return>
    / <U+000A Line Feed>

LineComment <- "//" (!LineTerminator .)*

BlockComment <- "/*" (BlockComment / .)* "*/"

Symbol <-
      "("
    / ")"
    / "{"
    / "}"
    / "["
    / "]"
    / "..."
    / "..<"
    / "."
    / ","
    / ":"
    / ";"
    / "=="
    / "=>"
    / "="
    / "!="
    / "!"
    / "+="
    / "+"
    / "->"
    / "-="
    / "-"
    / "*="
    / "*"
    / "/="
    / "/" !"*"
    / "<<="
    / "<<"
    / "<="
    / "<"
    / ">>="
    / ">>"
    / ">="
    / ">"
    / "^^="
    / "^^"
    / "^="
    / "^"
    / "@"

Keyword <-
    (
          "def"
        / "class"
        / "else"
        / "false"
        / "for"
        / "if"
        / "import"
        / "interface"
        / "in"
        / "let"
        / "mut"
        / "out"
        / "return"
        / "true"
        / "var"
        / "while"
    ) !IdContinue

IDENTIFIER <- IdStart IdContinue*
   IdStart <- [a-zA-Z_]
IdContinue <- IdStart / [0-9]

INT_LITERAL <-
      DecimalLiteral
    / HexLiteral
    / BinaryLiteral

DecimalLiteral <-
      "0" !IdContinue
    / [1-9] ("_" [0-9])* !IdContinue

HexLiteral <- ("0x" / "0X") [0-9a-fA-F] ("_" [0-9a-fA-F])* !IdContinue

BinaryLiteral <- ("0b" / "0B") [01] ("_" [01])* !IdContinue
```

## Hierarchical syntax {#sec:parse}

The hierarchical syntax is defined with a context-free grammar.

```grammar
File ::= Item

Item ::= FunctionItem

Block ::= "{" Stmt ";" "}"

Stmt ::= ReturnStmt

ReturnStmt ::= "return" Expr

Expr ::=
      PrimaryExpr

PrimaryExpr ::=
      INT_LITERAL
    | "(" Expr ")"

FunctionItem ::= "def" IDENTIFIER FunctionItemParams Block
FunctionItemParams ::= "(" ")" "->" Type

Type ::= IDENTIFIER
```

## Limits {#sec:syntax-limits}

Source files are divided into lines by the LineTerminator rule.
The LineTerminator characters are included in the line as its last characters.
The last line has no LineTerminator characters.

A source file shall have at most $2^{22} - 1$ (4194303) characters.

A source file shall have at most $2^{20} - 1$ (1048575) lines.

A source file shall have at most $2^{12} - 1$ (4095) characters per line.

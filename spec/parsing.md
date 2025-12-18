# Parsing

A source file is a sequence of Unicode characters.
The *lexical grammar* converts source file into a list of tokens.
The *hierarchical grammar* converts a list of tokens into abstract syntax.

## Source file

Characters in a source file are limited to (TODO: exclude control, bidi, etc).

## Lexical grammar

The lexical syntax is defined with a parsing expression grammar [@PEG].

The tokens produced are:

- The string terminals in the `symbol` rule.
- The string terminals in the `keyword` rule.
- The `Identifier` rule.
- The `IntLiteral` rule.

Note: The "." metacharacter is limited to the characters defined in @sec:source-file.

```peg
source_file <- <U+FEFF>? (trivia / token)*

trivia <-
      whitespace
    / line_terminator
    / line_comment
    / block_comment

token <-
      symbol
    / keyword
    / NAME
    / INT_LITERAL

whitespace <- <U+0009 Tab> / <U+0020 Space>

line_terminator <-
      <U+000D Carriage Return> <U+000A Line Feed>
    / <U+000D Carriage Return>
    / <U+000A Line Feed>

line_comment <- "//" (!line_terminator .)*

block_comment <- "/*" (block_comment / .)* "*/"

symbol <-
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

keyword <-
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
    ) !name_continue

Identifier <- id_start id_continue*
id_start <- [a-zA-Z_]
id_continue <- id_start / [0-9]

IntLiteral <-
      decimal_literal
    / hex_literal
    / binary_literal

decimal_literal <-
      "0" !id_continue
    / [1-9] ("_" [0-9])* !id_continue

hex_literal <- ("0x" / "0X") [0-9a-fA-F] ("_" [0-9a-fA-F])* !id_continue

binary_literal <- ("0b" / "0B") [01] ("_" [01])* !id_continue
```

## Hierarchical grammar

The hierarchical syntax is defined with an attribute grammar [@Knu68].

### Items

```
file ->
    item

item ->
    function_item
```

### Statements

```
block ->
    "{" stmt ";" "}" => stmt

stmt ->
    return_stmt

return_stmt ->
    "return" expr => ReturnExpr(expr)
```

### Expressions

```
expr ->
    e:primary_expr => e

primary_expr ->
    x:Identifier => NameExpr(x)
    i:IntLiteral => IntLiteralExpr(i)
    "(" expr ")" => expr
```

### Types

Types are syntactically expressions.

```
type ->
    expr => expr
```

### Functions

```
return_type ->
    "->" type => type

function_item ->
    "def" name:Identifier type:function_item_signature body:block
        => ExtFunctionItem { name, type, body }

function_item_signature ->
    function_item_params return_type
        => FunctionTypeExpr { return_type }

function_item_params ->
    "(" ")"
```

## Limits

Source files are divided into lines by the `line_terminator` rule.
The characters in a `line_terminator` are included in the line as its last characters.
The last line has no `line_terminator` characters.

A source file shall have at most $2^{22} - 1$ (4&#x202F;194&#x202F;303) characters.

A source file shall have at most $2^{20} - 1$ (1&#x202F;048&#x202F;575) lines.

A source file shall have at most $2^{12} - 1$ (4095) characters per line.

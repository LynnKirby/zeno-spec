# Abstract syntax

```
Item ::=
    FunctionItem

FunctionItem ::= {
    name : Identifier
    type : FunctionTypeExpr
    body : Expr
}

Expr ::=
    IntLiteralExpr(IntLiteral)
    ReturnExpr(Expr)
    NameExpr(Identifier)
    NeverExpr
    VoidExpr
    Int32Expr
    FunctionTypeExpr

Type ::=
    Never
    Void
    Int32
    FunctionType
    TypeType

FunctionType ::= {
    return : Type
}

FunctionTypeExpr ::= {
    return : Expr
}
```

```
is_value(IntLiteralExpr(_))
is_value(VoidExpr)
is_value(Int32Expr)

is_value(e.return)
----------------------------
is_value(f:FunctionTypeExpr)
```

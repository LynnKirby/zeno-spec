# Type checking

```
TypeCtx ::= {
    decls : Identifier -> Decl
    return_type : Type?
}

Decl ::=
    ConstDecl(Expr, Type)
```

```
# DeclWf-Const
is_value(e)
---------------
ConstDecl(e) wf

# DeclWf-Var
-------------
VarDecl(_) wf
```

## Type lifting

`as_type` is a partial function that maps type values to proper types.

```
as_type(VoidExpr) = Void
as_type(Int32Expr) = Int32

as_type(f:FunctionTypeExpr) =
    FunctionType {
        return_type = as_type(f.return_type)
    }
```

## Constant evaluation

```
# ConstEval-Value
is_value(e)
----------------------------
TypeCtx |- const_eval(e) = e

# ConstEval-Name
TypeCtx(x) = ConstDecl(e)
--------------------------------------
TypeCtx |- const_eval(NameExpr(x)) = e

# ConstEval-FunctionType
TypeCtx |- const_eval(f.return_type) = return_type'
f' = FunctionTypeExpr {
    return_type = return_type'
}
---------------------------------------------------
TypeCtx |- const_eval(f:FunctionTypeExpr) = f'
```

## Function typing

```
# TypeItem-Function
TypeCtx |- const_eval(f.type) = f_type'
as_type(f_type'.return) = return_type
TypeCtx' = TypeCtx[return_type |-> return_type]
TypeCtx |- f.body : body_type
----------------------------------------------------
TypeCtx |- f:FunctionItem ok
```

## Expression typing

```
# TypeExpr-IntLiteral
------------------------------------
TypeCtx |- IntLiteralExpr(_) : Int32

# TypeExpr-Return
TypeCtx.return_type = type
TypeCtx |- e : type
--------------------------------
TypeCtx |- ReturnExpr(e) : Never

# TypeExpr-ConstName
TypeCtx(x) = ConstDecl(e, type)
-------------------------------
TypeCtx |- NameExpr(x) : type

# TypeExpr-Type
is_type_expr(e)
-----------------------
TypeCtx |- e : TypeType
```

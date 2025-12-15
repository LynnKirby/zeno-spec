# Symbol binding {#sec:binding}

Symbol binding replaces identifiers with unique symbols.
It has two phases:

1. *Symbol generation* associates a unique symbol with each declaration and records metadata about the declaration in a global table.

2. *Symbol resolution* replaces use of a declaration with its symbol, taking into account lexical scope and shadowing.

## Symbol generation {#sec:symbol-generation}

```
# SymGenItem-Function
X not in D
D[X |-> x]; e ~~> D'; e'
-------------------------------------------------
D; def x() -> t { e } ~~> D'; def X() -> t { e' }
```

```
# SymGenExpr-Return
D; e ~~> D'; e'
-----------------------------
D; return e ~~> D'; return e'
```

```
# SymGenExpr-Literal
literal(e)
-------------
D; e ~~> D; e
```

```
# SymGenExpr-Identifier
-------------
D; x ~~> D; x
```

## Symbol resolution {#sec:symbol-resolution}

```
# SymResItem-Function
E_X = E[D.identifier(X) |-> X]
D, E_X |- t ~~> T
D, E_X |- e ~~> e'
--------------------------------------------------
D, E |- def X() -> t { e } ~~> def X() -> T { e' }
```

```
# SymResExpr-Return
D, E |- e ~~> e'
------------------------------
D, E |- return e ~~> return e'
```

```
# SymResExpr-Literal
literal(e)
---------------
D, E |- e ~~> e
```

```
# SymResExpr-Identifier
E(x) = X
---------------
D, E |- x ~~> X
```

# Evaluation

# Runtime syntax

```
Expr ::=
    ...
    BreakReturn

is_value(...)
is_value(BreakReturn)
```

## Evaluation context

```
E ::=
    ReturnExpr(E)
    []
```

## Expression evaluation

```
# EvalExpr-Context
mem, e ~~> mem', e'
-------------------------
mem, E[e] ~~> mem', E[e']

# EvalExpr-Return
is_value(e)
mem' = mem[$return := e]
-----------------------------------
mem, return e ~~> BreakReturn, mem'
```

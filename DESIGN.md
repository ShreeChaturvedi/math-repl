# REPL Calculator Design

## Overview

The calculator is a three-stage interpreter:

1. **Tokenizer**: converts source text into tokens.
2. **Parser**: builds an AST with precedence rules.
3. **Evaluator**: walks the AST against a mutable state.

Each step is isolated so tests can target them independently.

## Grammar (Simplified)

```
assignment  := ternary ("=" assignment)?
ternary     := equality ("?" ternary ":" ternary)?
equality    := relational (("==" | "!=") relational)*
relational  := additive (("<" | "<=" | ">" | ">=") additive)*
additive    := term (("+" | "-") term)*
term        := power (("*" | "/" | "%") power)*
power       := unary ("^" power)?
unary       := ("+" | "-") unary | primary
primary     := NUMBER | IDENT | IDENT "(" args ")" | "(" assignment ")"
args        := assignment ("," assignment)*
```

The ternary operator is right-associative and sits between equality and
assignment precedence.

## AST Nodes

- **Number**: literal numeric value.
- **Variable**: identifier reference.
- **Unary**: prefix plus/minus.
- **Binary**: arithmetic, assignment, and comparisons.
- **Function Call**: name + argument list.
- **Ternary**: condition, then-branch, else-branch.

`std::unique_ptr` is used to give each node exclusive ownership of its children.

## Evaluation Rules

- Variables are stored in `State::vars`.
- Built-in constants and the last result (`_`) are read-only.
- Built-in functions are resolved before user-defined functions.
- User-defined functions have their own local scope; assignments inside a
  function only affect the local scope.
- Function definitions are only allowed at the top level.

## Error Handling

Parsing and evaluation throw typed exceptions (`ParseError`, `EvalError`) that
are caught in the REPL loop to produce readable error output.

## Operator Precedence

Highest to lowest:

1. `+` `-` (unary)
2. `^`
3. `*` `/` `%`
4. `+` `-` (binary)
5. `<` `<=` `>` `>=`
6. `==` `!=`
7. `?:`
8. `=`

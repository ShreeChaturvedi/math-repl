<p align="center">
  <img src="docs/branding/readme-light.svg#gh-light-mode-only" width="860" alt="Math REPL">
  <img src="docs/branding/readme-dark.svg#gh-dark-mode-only" width="860" alt="Math REPL">
</p>

---

[![ci](https://github.com/ShreeChaturvedi/math-repl/actions/workflows/ci.yml/badge.svg)](https://github.com/ShreeChaturvedi/math-repl/actions/workflows/ci.yml)
[![license](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![c++](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://isocpp.org/)

Math REPL is a small C++23 expression language with a REPL loop, user-defined
functions, and an intentionally readable implementation. This README is more
educational than the other repos: it walks through how the tokenizer, parser,
and evaluator fit together with diagrams and concrete examples.

## Quick Start

```bash
cmake -S . -B build
cmake --build build
./build/src/repl
```

Or with presets:

```bash
cmake --preset dev
cmake --build --preset dev
./build/src/repl
```

## Usage

```text
> x = 2
2
> square(n) = n * n
Defined square(n)
> square(5)
25
> 3 > 2 ? 10 : 20
10
> pi
3.14159
> _ * 3
9.42478
```

### Commands

- `help`     Show help and syntax hints
- `vars`     List user variables
- `fns`      List user functions
- `consts`   List built-in constants
- `builtins` List built-in functions
- `load <file>` Run a script file
- `reset`    Clear variables and functions
- `history`  Show recent inputs (interactive sessions only)
- `clear`    Clear the screen
- `exit` / `quit` Exit the REPL

## Built-in Functions

Unary: `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `sinh`, `cosh`, `tanh`,
`asinh`, `acosh`, `atanh`, `sqrt`, `cbrt`, `exp`, `ln`, `log`, `log2`, `abs`,
`floor`, `ceil`, `round`, `trunc`.

Binary: `pow`, `fmod`, `atan2`.

## Constants

`pi, e, tau`

## My Approach

<p align="center">
  <img src="docs/diagrams/pipeline-light.svg#gh-light-mode-only" width="860" alt="Tokenizer → Parser → Evaluator">
  <img src="docs/diagrams/pipeline-dark.svg#gh-dark-mode-only" width="860" alt="Tokenizer → Parser → Evaluator">
</p>

### 1) Tokenization

The tokenizer scans characters and emits tokens such as numbers, identifiers,
operators, and punctuation. For example:

```
Input:  x > 0 ? x : -x
Tokens: Identifier(x), Greater, Number(0), Question, Identifier(x), Colon, Minus, Identifier(x)
```

This step is deliberately small and strict: unknown characters produce a
`ParseError` immediately.

### 2) Parsing

Parsing turns tokens into an AST using operator precedence and associativity.
Assignment is right-associative, as is exponentiation. Ternary sits between
comparisons and assignment.

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

<p align="center">
  <img src="docs/diagrams/ast-light.svg#gh-light-mode-only" width="860" alt="AST for ternary expression">
  <img src="docs/diagrams/ast-dark.svg#gh-dark-mode-only" width="860" alt="AST for ternary expression">
</p>

### 3) Evaluation

Evaluation walks the AST and produces a `double` result. Variables and user
functions live in `State`. Built-in functions are resolved before user-defined
functions, and constants plus `_` (last result) are read-only.

### Functions and Scope

- `f(x) = x * x` defines a function.
- Calls create a local scope for parameters.
- Assignments inside a function only affect that local scope.
- Recursion works because user functions are stored in shared state.

### Error Handling

Parsing and evaluation throw typed exceptions (`ParseError`, `EvalError`). The
REPL catches them and prints readable error messages without exiting.

## Build and Test

### Requirements

- C++23 compiler (Clang 16+, GCC 13+, MSVC 19.37+)
- CMake 3.20+
- Ninja (recommended)

### Configure + Build

```bash
cmake -S . -B build -DREPL_BUILD_TESTS=ON
cmake --build build
```

### Run Tests

```bash
ctest --test-dir build --output-on-failure
```

## Design Notes

See `DESIGN.md` for the grammar, AST, and evaluation strategy. The evaluator uses
`double` everywhere and comparisons are direct (`==` is exact).

## Third-Party

- Catch2 (BSL-1.0) for testing.
- linenoise (BSD) for line editing and history (disabled on Windows builds).

## License

MIT. See `LICENSE`.

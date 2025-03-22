<p align="center">
  <img src="docs/branding/readme-light.svg#gh-light-mode-only" width="860" alt="REPL Calculator">
  <img src="docs/branding/readme-dark.svg#gh-dark-mode-only" width="860" alt="REPL Calculator">
</p>

---

[![ci](https://github.com/ShreeChaturvedi/repl/actions/workflows/ci.yml/badge.svg)](https://github.com/ShreeChaturvedi/repl/actions/workflows/ci.yml)
[![license](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![c++](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://isocpp.org/)

A focused C++23 REPL calculator with a small expression language, user-defined
functions, and a clean three-phase pipeline (tokenize, parse, evaluate).

## Highlights

- Expression language with precedence, ternary, and comparisons.
- Built-in math library plus constants (pi, e, tau).
- User-defined functions with arity checks and recursion support.
- Command history, script loading, and compact CLI helpers.
- Catch2 tests for tokenizer, parser, evaluator, and integration.

## Quick Start

```bash
cmake -S . -B build
cmake --build build
./build/repl
```

Or with presets:

```bash
cmake --preset dev
cmake --build --preset dev
./build/repl
```

## REPL Usage

```text
> x = 2
2
> square(n) = n * n
Defined square(n)
> square(5)
25
> x > 10 ? 1 : 0
0
> _ * 3
75
```

### Commands

- `help`     Show help and syntax hints
- `vars`     List user variables
- `fns`      List user functions
- `consts`   List built-in constants
- `builtins` List built-in functions
- `load <file>` Run a script file
- `reset`    Clear variables and functions
- `history`  Show recent inputs
- `clear`    Clear the screen
- `exit` / `quit` Exit the REPL

## Built-in Functions

| Name | Arity | Description |
| --- | :---: | --- |
| sin, cos, tan | 1 | Trigonometric (radians) |
| asin, acos, atan | 1 | Inverse trigonometric |
| sinh, cosh, tanh | 1 | Hyperbolic |
| asinh, acosh, atanh | 1 | Inverse hyperbolic |
| sqrt, cbrt | 1 | Roots |
| exp | 1 | Exponential |
| ln, log, log2 | 1 | Logarithms |
| abs | 1 | Absolute value |
| floor, ceil, round, trunc | 1 | Rounding |
| pow | 2 | Power |
| fmod | 2 | Floating-point modulo |
| atan2 | 2 | Quadrant-aware arctangent |

## Constants

| Name | Value |
| --- | --- |
| pi | 3.141592653589793 |
| e | 2.718281828459045 |
| tau | 6.283185307179586 |

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
- linenoise (BSD) for line editing and history.

## License

MIT. See `LICENSE`.

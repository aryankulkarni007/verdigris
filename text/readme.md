# Koz

A value-oriented scripting language that transpiles to C, designed as an
abstraction layer over a C game engine for an RTS game.

## Identity

Koz is value-oriented. There are no objects with persistent identity —
only values transformed into new values. Time is explicit: state at T is
input, state at T+1 is output. Functions are transformations, not
procedures.

## Core properties

- Transpiles to C — no runtime, no GC
- 100% deterministic — no hidden state, no implicit ordering
- No manual memory — the translation layer owns all allocation
- Pure by default — effects are explicitly marked
- First-class C interop and engine binding support

## Design

- Static type system with algebraic data types (sum + product)
- Exhaustive pattern matching
- Structural polymorphism — implicit interface satisfaction
- Explicit mutability — `mut` for locals only, never on inputs
- No exceptions — error types via `Result(T, E)`
- Returns new state rather than mutating — translator optimises to
  in-place mutation where safe (linear use detection)

```text
 .
├──  build.sh
├──  include
│   ├──  arena.h
│   ├──  ast.h
│   ├──  lexer.h
│   ├──  main.h
│   ├──  parser.h
│   └──  token.h
├──  koz.dSYM
│   └──  Contents
│       ├──  Info.plist
│       └──  Resources
│           ├──  DWARF
│           │   └── 󰡯 koz
│           └──  Relocations
│               └──  aarch64
│                   └──  koz.yml
├── 󰣞 src
│   ├──  arena.c
│   ├──  ast.c
│   ├──  ast_print.c
│   ├──  lexer.c
│   ├──  main.c
│   ├──  parser_expr.c
│   ├──  parser_stmt.c
│   ├──  test_suite.c
│   └──  token.c
├──  tests
│   └──  test_ast_print.c
└──  text
    ├──  draft1.koz
    ├──  draft2.koz
    ├──  readme.md
    └──  todo.md
```

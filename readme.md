# rime programming language

- (X) file handling
- ( ) lexer
  - (X) base
  - ( ) extended
- ( ) parser + ast
- ( ) error handler
- ( ) type checker
- ( ) your own IR
- ( ) llvm backend
- ( ) multi-file support
- ( ) treesitter grammar
- ( ) lsp
- ( ) build tool

# Rime — Language Design Notes (Early Draft)

rime is a systems-oriented language focused on zero-cost abstractions, deterministic behavior, and high performance, while avoiding explicit memory management in user code. The language is being built from first principles with an emphasis on clarity, composability, and aesthetic syntax.

---

## Compilation Model

Rime is designed as a compiled language. Although early iterations may target C as a backend, the long-term goal is a self-contained compiler to avoid inheriting constraints from C’s design.

---

## Frontend Architecture

The frontend is structured in three distinct stages:

```
source → tokens → CST → AST
```

### Lexer

The lexer performs a single pass over the source file (loaded entirely into memory) and produces a sequence of tokens.

Each token consists of:

- a `TokenKind`
- a `Span` (`start`, `end` byte offsets)

Spans are byte-based and do not store line/column information directly. Line mappings are maintained separately for diagnostics.

---

### Lossless Token Stream

The lexer preserves all source information through a **lossless token model**.

- Whitespace and comments are not emitted as regular tokens
- Instead, they are captured as **trivia**
- Trivia is attached to tokens (typically as leading trivia)

This allows:

- exact reconstruction of source code
- high-quality diagnostics
- future tooling such as formatters

---

### Token Design

Token kinds represent **syntax**, not semantics.

Included:

- identifiers (`IDENT`)
- literals (`INT`, `FLOAT`, `STRING`, `TRUE`, `FALSE`)
- keywords (`fn`, `if`, `match`, etc.)
- operators and delimiters (`+`, `=`, `(`, `{`, etc.)

Not included:

- type names such as `int`, `bool`, `Point`

All type names lex as `IDENT`. Their meaning is resolved later during semantic analysis.

---

### Trivia

Trivia represents non-semantic source elements:

- whitespace
- newlines
- comments (line, block, documentation)

Trivia is stored separately from tokens but attached to them, preserving full source fidelity without complicating parsing.

---

### Parsing Strategy (CST)

The parser constructs a **Concrete Syntax Tree (CST)**.

Key properties:

- lossless representation of syntax
- preserves all tokens and structure
- supports invalid or incomplete code

The parser is designed to be **error-tolerant**:

- missing tokens are inserted into the tree when expected
- parsing continues rather than aborting
- errors are recorded but do not halt construction

This avoids heavy reliance on synchronization points and enables better diagnostics.

---

### AST Lowering

The CST is later lowered into a more compact and semantic AST.

- redundant syntax is removed
- structure is normalized
- semantic analysis operates on this layer

---

## Type System Direction

Types are treated as identifiers at the lexical level.

This allows:

- uniform handling of built-in and user-defined types
- simpler and more extensible syntax
- delayed semantic resolution

Example:

```
int x;
Point y;
```

Both `int` and `Point` are lexed as `IDENT`.

---

## Core Infrastructure

The compiler is being built alongside a small foundational library, including:

- a resizable vector (dynamic array)
- an aligned arena allocator
- span and source mapping utilities
- token and trivia representations
- a diagnostic system

This library is intended to remain minimal but robust, forming the backbone of all later stages.

---

## Design Philosophy

Rime aims to:

- preserve information as long as possible (lossless stages)
- separate syntax from semantics cleanly
- favor explicit structure over implicit behavior
- enable strong tooling and diagnostics from the outset

The language is being developed iteratively, with correctness and architectural clarity prioritized over rapid feature expansion.

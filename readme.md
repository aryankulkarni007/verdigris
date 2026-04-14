# agenda

## Phase 0: Foundation (Current)

Goal: Rock-solid infrastructure before parsing

- (X) Arena allocator
- (X) String interning (in progress - see evaluation below)
- (X) Vec macros (dynamic arrays for token streams, CST children)
- (X) Source spans (line/col tracking from lexer)
- ( ) Diagnostic system (error/warning collection and printing)
- ( ) Symbol table scaffold (scope management, name->declaration mapping)

## Phase 1: CST Construction

Goal: Parse source into concrete syntax tree preserving all trivia

- ( ) CST node definitions - One struct per syntactic category
- ( ) CST parser - Recursive descent over token stream
- ( ) CST pretty printer - Verify round-trip fidelity

## Phase 2: AST Lowering

Goal: Desugar CST to semantic AST

- ( ) AST node definitions - Semantic structure, no syntax noise
- ( ) Lowering passes - Pipeline >>, ! propagation, pattern desugaring
- ( ) AST printer - Debug representation

## Phase 3: Name Resolution

Goal: Connect every identifier to its declaration

- ( ) Symbol table with scopes - Block scoping, function parameters
- ( ) Import/module resolution - Basic single-file first
- ( ) Error reporting - "undefined variable 'x'"

## Phase 4: Type Checking

Goal: Validate and infer types

- ( ) Type representation - Primitives, structs, enums, generics
- ( ) Unification - Hindley-Milner style inference
- ( ) Interface checking - Structural satisfaction
- ( ) Pattern exhaustiveness - All cases covered

## Phase 5: HIR Lowering

Goal: Fully resolved intermediate representation

- ( ) Monomorphization - Expand generic instantiations
- ( ) Pattern lowering - Decision trees
- ( ) Pool operations explicit - Allocations visible

## Phase 6: C Codegen

Goal: Emit debuggable C

- ( ) Type translation - Verdigris types → C types
- ( ) Runtime library - libverdi with pools, vec, string
- ( ) C emitter - Pretty-printed C with #line directives

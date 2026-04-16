#pragma once

#include "diagnostic.h"
#include "lexer.h"
#include "span.h"
#include "token.h"

/*\
 * T is a type
 * L is a literal
 * E is an expr
 * LE is both
 * S is a stmt
 * D is a decl
\*/

#define CST_T(X)                                                               \
  /* ── module ─────────────────────────────────────────────────────────── */  \
  X(CST_MODULE) /* root node: contains all top-level declarations       */     \
  /* ── declarations ───────────────────────────────────────────────────── */  \
  X(CST_FUNCTION_DECL)  /* fn add(a: int, b: int) -> int { a + b }      */     \
  X(CST_STRUCT_DECL)    /* struct Point { x: int, y: int }              */     \
  X(CST_ENUM_DECL)      /* enum Option[T] { Some(T), None }             */     \
  X(CST_INTERFACE_DECL) /* interface Printable { format(self) -> string }*/    \
  X(CST_IMPL_DECL)      /* Point::format(self) -> string { ... }        */     \
  X(CST_VAR_DECL)       /* let x = 42;  or  mut y = 10;                 */     \
  X(CST_IMPORT_DECL)    /* import "std/vec"                             */     \
  /* ── statements ─────────────────────────────────────────────────────── */  \
  X(CST_BLOCK)         /* { stmt; stmt; }                              */      \
  X(CST_EXPR_STMT)     /* expression used as statement: foo();          */     \
  X(CST_IF_STMT)       /* if x == 1 { } else { }                       */      \
  X(CST_WHILE_STMT)    /* while x < 10 { }                             */      \
  X(CST_FOR_STMT)      /* for i in 0..10 { }                           */      \
  X(CST_LOOP_STMT)     /* loop { break; }                              */      \
  X(CST_MATCH_STMT)    /* match n { 0 => 1, _ => 0 } (statement pos)   */      \
  X(CST_RETURN_STMT)   /* return x;                                    */      \
  X(CST_BREAK_STMT)    /* break;                                       */      \
  X(CST_CONTINUE_STMT) /* continue;                                    */      \
  X(CST_DEFER_STMT)    /* defer cleanup();                             */      \
  /* ── expressions ────────────────────────────────────────────────────── */  \
  X(CST_IDENT)         /* variable or function name: x, foo             */     \
  X(CST_INT_LIT)       /* integer literal: 42, 0, 1000                  */     \
  X(CST_FLOAT_LIT)     /* float literal: 3.14, 1.5                      */     \
  X(CST_STRING_LIT)    /* string literal: "hello"                       */     \
  X(CST_BOOL_LIT)      /* boolean literal: true, false                  */     \
  X(CST_NULL_LIT)      /* null literal                                  */     \
  X(CST_BINARY_EXPR)   /* a + b, x == y, etc.                          */      \
  X(CST_UNARY_EXPR)    /* -x, !flag, &value                            */      \
  X(CST_CALL_EXPR)     /* foo(1, 2)                                    */      \
  X(CST_PIPELINE_EXPR) /* data >> filter >> map                        */      \
  X(CST_MATCH_EXPR)    /* match n { 0 => 1 } as expression             */      \
  X(CST_IF_EXPR)       /* if true { 1 } else { 0 }                     */      \
  X(CST_FIELD_ACCESS)  /* point.x, self.value                          */      \
  X(CST_INDEX_EXPR)    /* array[0], vec[i]                             */      \
  X(CST_RANGE_EXPR)    /* 0..10, 0..=10                                */      \
  X(CST_CAST_EXPR)     /* x as float                                   */      \
  X(CST_ERROR_PROP)    /* open()!                                      */      \
  X(CST_CATCH_EXPR)    /* open() catch |e| default                     */      \
  X(CST_STRUCT_LIT)    /* Point { x: 0, y: 0 }                         */      \
  X(CST_ARRAY_LIT)     /* [1, 2, 3, 4]                                 */      \
  X(CST_TUPLE_LIT)     /* (1, "hello", true)                           */      \
  /* ── patterns (for match arms) ─────────────────────────────────────── */   \
  X(CST_PATTERN_LIT)    /* 0, "hello", true, null                       */     \
  X(CST_PATTERN_IDENT)  /* x (binding)                                  */     \
  X(CST_PATTERN_ENUM)   /* Some(x), None                                */     \
  X(CST_PATTERN_STRUCT) /* Point { x, y }                               */     \
  X(CST_PATTERN_TUPLE)  /* (x, y, z)                                    */     \
  X(CST_PATTERN_WILD)   /* _                                            */     \
  X(CST_PATTERN_GUARD)  /* n if n > 0                                   */     \
  /* ── types ─────────────────────────────────────────────────────────── */   \
  X(CST_TYPE_PRIMITIVE) /* int, float, bool, string                     */     \
  X(CST_TYPE_NAMED)     /* Point, Option[T] (name + optional args)      */     \
  X(CST_TYPE_TUPLE)     /* (int, string)                                */     \
  X(CST_TYPE_ARRAY)     /* [int; 4]                                     */     \
  X(CST_TYPE_SLICE)     /* &[int]                                       */     \
  X(CST_TYPE_UNION)     /* int | IOError                                */     \
  X(CST_TYPE_OPTIONAL)  /* ?int                                         */     \
  /* ── parameters ────────────────────────────────────────────────────── */   \
  X(CST_PARAM) /* a: int, mut self, other: Point               */              \
  /* ── match arm ─────────────────────────────────────────────────────── */   \
  X(CST_MATCH_ARM) /* pattern => expression                         */

typedef enum {
#define AS_ENUM(name) name,
  CST_T(AS_ENUM)
#undef AS_ENUM
} CST_T;

/// the base node for all nodes
#define CST_NODE_BASE                                                          \
  CST_T kind;                                                                  \
  SourceSpan span

/* literals */
// clang-format off
/// keyword or variable name
typedef struct { CST_NODE_BASE; Token *token; InternID value; } IDENT_L_CSTN;  // identifier
typedef struct { CST_NODE_BASE; Token *token; int64_t value;} INT_L_CSTN;  // int literal
typedef struct { CST_NODE_BASE; Token *token; double value;} FLT_L_CSTN;   // float literal
typedef struct { CST_NODE_BASE; Token *token; InternID value; } STR_L_CSTN;// string literal
typedef struct { CST_NODE_BASE; Token *token; bool value; } BOOL_L_CSTN;   // bool literal
typedef struct { CST_NODE_BASE; Token *token; } NULL_L_CSTN;               // null literal
// clang-format on

/* types */

// Forward declare cst_t union
typedef union cst_t cst_t;

/// int, float, string etc
typedef struct {
  CST_NODE_BASE;
  Token *token;
} PRIM_T_CSTN;

/// i.e. a struct or enum etc
typedef struct {
  CST_NODE_BASE;
  Token *name;
  VecHeader(cst_t *) generic_args;
} NAMED_T_CSTN;

/// (type, type, type)
typedef struct {
  CST_NODE_BASE;
  VecHeader(cst_t *) types;
} TUPLE_T_CSTN;

typedef struct {
  CST_NODE_BASE;
  cst_t *lmt_t;
  Token *size; // TK_INT token
} ARRAY_T_CSTN;

typedef struct {
  CST_NODE_BASE;
  cst_t *lmt_t;
} SLICE_T_CSTN;

/// type | type | ...
typedef struct {
  CST_NODE_BASE;
  VecHeader(cst_t *) types;
} UNION_T_CSTN;

/// (inner)?
typedef struct {
  CST_NODE_BASE;
  cst_t *inner;
} OPT_T_CSTN;

union cst_t {
  CST_NODE_BASE;
  PRIM_T_CSTN _primitive;
  NAMED_T_CSTN _named;
  TUPLE_T_CSTN _tuple;
  ARRAY_T_CSTN _array;
  SLICE_T_CSTN _slice;
  UNION_T_CSTN _union;
  OPT_T_CSTN _optional;
};

/* exprs */
typedef union cst_e cst_e;

/// (op)(expr)
typedef struct {
  CST_NODE_BASE;
  Token *op; // -, !, &, *
  cst_e *expr;
} UNARY_E_CSTN;

/// (left)(op)(right)
typedef struct {
  CST_NODE_BASE;
  cst_e *left;
  Token *op; // +, -, *, /, ==, !=, >, <, &&, ||, etc
  cst_e *right;
} BINARY_E_CSTN;

/// (target).(field)
typedef struct {
  CST_NODE_BASE;
  cst_e *target; // expr before dot
  Token *dot;    // target(dot)field
  Token *field;  // ident after dot
} FIELD_E_CSTN;

/// (target)[(index)]
typedef struct {
  CST_NODE_BASE;
  cst_e *target; // -> target[index]
  Token *obrack; // target->[index]
  cst_e *index;  // target[->index]
  Token *cbrack; // target[index->]
} INDEX_E_CSTN;

/// (callee)(<args>)
typedef struct {
  CST_NODE_BASE;
  cst_e *callee;           // ->callee(<args>)
  Token *oparen;           // callee->(<args>)
  VecHeader(cst_e *) args; // callee(<->args>)
  Token *cparen;           // callee(<args>->)
} CALL_E_CSTN;

/// (start)..(end) | (start)..=(end)
typedef struct {
  CST_NODE_BASE;
  cst_e *start;
  Token *op; // .. | ..=
  cst_e *end;
} RANGE_E_CSTN;

/// (target) as (type)
typedef struct {
  CST_NODE_BASE;
  cst_e *target;
  Token *as;
  cst_t *type;
} CAST_E_CSTN;

/// (expr)!
typedef struct {
  CST_NODE_BASE;
  cst_e *expr;
  Token *bang;
} EPROP_E_CSTN;

/// (expr) catch |(err)| (handler)
typedef struct {
  CST_NODE_BASE;
  cst_e *expr; //  expr that might fail
  Token *catch;
  Token *opipe;
  Token *err_var; // identifier
  Token *cpipe;
  cst_e *handler;
} CATCH_E_CSTN;

/// (stage1) >> (stage2) >> ...
typedef struct {
  CST_NODE_BASE;
  VecHeader(cst_e *) stages; // delimited by >>
} PLINE_E_CSTN;

/* complex literal exprs */

/// fields in struct_lit_expr
typedef struct {
  Token *name;
  Token *colon;
  cst_e *value;
} FieldInit;

/// struct (ident) { <fields> }
typedef struct {
  CST_NODE_BASE;
  cst_t *type; // optional: inference possible
  Token *obrace;
  VecHeader(FieldInit *) fields;
  Token *cbrace;
} STRUCT_LE_CSTN;

/// [lmt, lmt, lmt]
typedef struct {
  CST_NODE_BASE;
  Token *obrack;
  VecHeader(cst_e *) lmnts; // , delimited
  Token *cbrack;
} ARRAY_LE_CSTN;

/// (lmt, lmt, ...)
typedef struct {
  CST_NODE_BASE;
  Token *oparen;
  VecHeader(cst_e *) lmnts; // , delimited
  Token *cparen;
} TUPLE_LE_CSTN;

/// if (cond) (block) else? (block?)
typedef struct {
  CST_NODE_BASE;
  Token *_if;
  cst_e *cond;
  cst_e *then;
  Token *_else;
  cst_e *else_then; // optional
} IF_E_CSTN;

/// (pattern) => (expr or block)
typedef union cst_s cst_s;
typedef union cst_p cst_p;

/// (expr);
typedef struct {
  CST_NODE_BASE;
  cst_e *expr;
  Token *semi;
} EXPR_STMT_S_CSTN;

/// return (expr);
typedef struct {
  CST_NODE_BASE;
  Token *ret;
  cst_e *expr;
  Token *semi;
} RETURN_S_CSTN;

/// break;
typedef struct {
  CST_NODE_BASE;
  Token *brk;
  Token *semi;
} BREAK_S_CSTN;

/// continue;
typedef struct {
  CST_NODE_BASE;
  Token *cont;
  Token *semi;
} CONT_S_CSTN;

/// defer (expr);
typedef struct {
  CST_NODE_BASE;
  Token *defer;
  cst_e *expr; // call or a block
  Token *semi;
} DEFER_S_CSTN;

/// if (cond) {  } else ? { }?
typedef struct {
  CST_NODE_BASE;
  Token *_if;
  cst_e *cond;
  cst_s *then;      // block stmt
  Token *_else;     // optional
  cst_s *else_then; // optional
} IF_S_CSTN;

/// while (cond) { }
typedef struct {
  CST_NODE_BASE;
  Token *_while;
  cst_e *cond;
  cst_s *body; // block stmt
} WHILE_S_CSTN;

/// loop { }
typedef struct {
  CST_NODE_BASE;
  Token *loop;
  cst_s *body; // block stmt
} LOOP_S_CSTN;

/// for (index) in (iterable) { }
typedef struct {
  CST_NODE_BASE;
  Token *_for;
  cst_p *pattern; // loop variable
  Token *_in;     // in keyword
  cst_e *iter;    // expr producing iterable
  cst_s *body;    // block stmt
} FOR_S_CSTN;

typedef struct MatchArm MatchArm;

typedef struct {
  CST_NODE_BASE;
  Token *match;
  cst_e *target;
  Token *obrace;
  VecHeader(MatchArm *) arms;
  Token *cbrace;
} MATCH_S_CSTN;

/// { stmt; stmt; stmt; tail? }
typedef struct {
  CST_NODE_BASE;
  Token *obrace;
  VecHeader(cst_s *) stmts;
  cst_e *tail;
  Token *cbrace;
} BLOCK_CSTN;

struct MatchArm {
  SourceSpan span;
  cst_p *pattern;
  Token *fat_arrow;
  cst_e *expr;  // for match expressions
  cst_s *block; // for match statements
};

/// match (cond) { arm, arm, ... }
typedef struct {
  CST_NODE_BASE;
  Token *match;
  cst_e *target; // expr being matched
  Token *obrace;
  VecHeader(MatchArm *) arms;
  Token *cbrace;
} MATCH_E_CSTN;

union cst_e {
  CST_NODE_BASE;
  // literals
  IDENT_L_CSTN ident_lit;
  INT_L_CSTN int_lit;
  FLT_L_CSTN float_lit;
  STR_L_CSTN string_lit;
  BOOL_L_CSTN bool_lit;
  NULL_L_CSTN null_lit;
  // exprs
  UNARY_E_CSTN unary;
  BINARY_E_CSTN binary;
  FIELD_E_CSTN field;
  INDEX_E_CSTN index;
  CALL_E_CSTN call;
  RANGE_E_CSTN range;
  CAST_E_CSTN cast;
  EPROP_E_CSTN error_prop;
  CATCH_E_CSTN catch;
  PLINE_E_CSTN pipeline;
  STRUCT_LE_CSTN struct_lit;
  ARRAY_LE_CSTN array_lit;
  TUPLE_LE_CSTN tuple_lit;
  BLOCK_CSTN block;
  IF_E_CSTN if_expr;
  MATCH_E_CSTN match_expr;
};

union cst_s {
  CST_NODE_BASE;
  EXPR_STMT_S_CSTN expr_stmt;
  RETURN_S_CSTN return_stmt;
  BREAK_S_CSTN break_stmt;
  CONT_S_CSTN continue_stmt;
  DEFER_S_CSTN defer_stmt;
  IF_S_CSTN if_stmt;
  WHILE_S_CSTN while_stmt;
  LOOP_S_CSTN loop_stmt;
  FOR_S_CSTN for_stmt;
  MATCH_S_CSTN match_stmt;
  BLOCK_CSTN block;
};

// Wildcard: _
typedef struct {
  CST_NODE_BASE;
  Token *underscore;
} WILD_P_CSTN;

// identifier: x (binds value to name)
typedef struct {
  CST_NODE_BASE;
  Token *name;
  InternID id;
} IDENT_P_CSTN;

// Literal: 0, true, "hello"
typedef struct {
  CST_NODE_BASE;
  cst_e *literal;
} LIT_P_CSTN;

// Enum variant: Some(x), None
typedef struct {
  CST_NODE_BASE;
  Token *variant;          // Some, None
  VecHeader(cst_p *) args; // Pterns for payload
} ENUM_P_CSTN;

// Struct: Point { x, y } or Point { x: x_P, y: y_pat }
typedef struct {
  Token *name;
  Token *colon; // optional: x: P
  cst_p *Ptern; // pattern for this field
} FIELD_P;

typedef struct {
  CST_NODE_BASE;
  Token *type_name; // Point
  Token *obrace;
  VecHeader(FIELD_P *) fields;
  Token *cbrace;
} STRUCT_P_CSTN;

// Tuple: (x, y, z)
typedef struct {
  CST_NODE_BASE;
  Token *oparen;
  VecHeader(cst_p *) elements;
  Token *cparen;
} TUPLE_P_CSTN;

// Ptern with guard: n if n > 0
typedef struct {
  CST_NODE_BASE;
  cst_p *inner;
  Token *_if;
  cst_e *guard;
} GUARD_P_CSTN;

union cst_p {
  CST_NODE_BASE;
  WILD_P_CSTN wild;
  IDENT_P_CSTN ident;
  LIT_P_CSTN literal;
  ENUM_P_CSTN enum_pat;
  STRUCT_P_CSTN struct_pat;
  TUPLE_P_CSTN tuple;
  GUARD_P_CSTN guard;
};

typedef union cst_d cst_d;

// fn parameter
typedef struct {
  CST_NODE_BASE;
  Token *mut; // optional
  Token *name;
  Token *colon;
  cst_t *type;
} PARAM_D_CSTN;

// var decl: let/mut pattern = init;
typedef struct {
  CST_NODE_BASE;
  Token *let_or_mut;
  cst_p *pattern;
  Token *eq;
  cst_e *init;
  Token *semi;
} VAR_D_CSTN;

// function decl
typedef struct {
  CST_NODE_BASE;
  Token *name;
  Token *oparen;
  VecHeader(PARAM_D_CSTN *) params;
  Token *cparen;
  Token *arrow;    // optional (void return)
  cst_t *ret_type; // optional
  cst_s *body;     // optional (forward decl has none)
} FUNC_D_CSTN;

// struct decl
typedef struct {
  Token *name;
  Token *colon;
  cst_t *type;
} STRUCT_FIELD;

typedef struct {
  CST_NODE_BASE;
  Token *name;
  VecHeader(cst_t *) generic_params; // optional [T, U]
  Token *obrace;
  VecHeader(STRUCT_FIELD *) fields;
  VecHeader(FUNC_D_CSTN *) methods; // method signatures
  Token *cbrace;
} STRUCT_D_CSTN;

// enum decl
typedef struct {
  Token *name;
  VecHeader(cst_t *) args; // optional payload types
} ENUM_VARIANT;

typedef struct {
  CST_NODE_BASE;
  Token *name;
  VecHeader(cst_t *) generic_params;
  Token *obrace;
  VecHeader(ENUM_VARIANT *) variants;
  Token *cbrace;
} ENUM_D_CSTN;

// interface decl
typedef struct {
  CST_NODE_BASE;
  Token *name;
  VecHeader(cst_t *) generic_params;
  Token *obrace;
  VecHeader(FUNC_D_CSTN *) methods; // signatures only
  Token *cbrace;
} INTERFACE_D_CSTN;

// implementation (method definitions outside struct)
typedef struct {
  CST_NODE_BASE;
  Token *type_name;
  Token *ccolon;
  Token *method_name;
  FUNC_D_CSTN *func; // full function definition
} IMPL_D_CSTN;

// import declaration
typedef struct {
  CST_NODE_BASE;
  Token *import;
  Token *path;  // string literal token
  Token *as;    // optional
  Token *alias; // optional
  Token *semi;
} IMPORT_D_CSTN;

// module (root)
typedef struct {
  CST_NODE_BASE;
  VecHeader(cst_d *) decls;
} MODULE_D_CSTN;

union cst_d {
  CST_NODE_BASE;
  VAR_D_CSTN var;
  FUNC_D_CSTN func;
  STRUCT_D_CSTN struct_decl;
  ENUM_D_CSTN enum_decl;
  INTERFACE_D_CSTN interface;
  IMPL_D_CSTN impl;
  IMPORT_D_CSTN import;
  MODULE_D_CSTN module;
};

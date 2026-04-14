#ifndef CST_H
#define CST_H

#include "diagnostic.h"
#include "lexer.h"
#include "span.h"
#include "token.h"

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
  X(CST_BLOCK_EXPR)    /* { stmt; expr } (yields value)                */      \
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

typedef struct CSTNode CSTNode;
typedef struct CSTExpr CSTExpr;
typedef struct CSTStmt CSTStmt;
typedef struct CSTDecl CSTDecl;
typedef struct CSTType CSTType;
typedef struct CSTPattern CSTPattern;

struct CSTNode {
  CST_T kind;
  SourceSpan span;
};

typedef struct {
  TStream tokens;
  size_t pos; // current token index
  DiagBag *diags;
  Arena *ast_arena;
  bool panic_mode; // error recovery flag
} Parser;

// clang-format off
static inline Token *p_current(Parser *p) { return &p->tokens.data[p->pos]; }
static inline Token *p_peek(Parser *p) { return &p->tokens.data[p->pos + 1]; }
static inline Token *p_advance(Parser *p) { return &p->tokens.data[p->pos++]; }
static inline bool p_check(Parser *p, TK_T type) { return p_current(p)->type == type; }
static inline bool p_match(Parser *p, TK_T type) { if (p_check(p, type)) { p_advance(p); return true; } return false; }
static Token *p_expect(Parser *p, TK_T type, const char *expected);
// clang-format on

#endif // CST_H

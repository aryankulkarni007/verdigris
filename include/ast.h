#ifndef AST_H
#define AST_H

#include "arena.h"
#include "token.h"
#include <stdbool.h>
#include <stddef.h>

// DONE: I need to represent everything in the following enums in the unions in
// the Nodes. I also need helper nodes for big nodes like blocks and func_decls

// forward decl of nodes
typedef struct Type Type;
typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct Decl Decl;

// --- X-MACROS ---

// the following has all everything that is representable in the language
#define TYPE_KINDS(X)                                                          \
  X(T_PRIMITIVE, "Primitive") /* int, float, bool, string, char */             \
  X(T_NAME, "Name")           /* User defined types (Player, Team) */          \
  X(T_ARRAY, "Array")         /* [T] */                                        \
  X(T_GENERIC, "Generic")     /* Result(T, E), Option(T) */                    \
  X(T_FUNCTION, "Function")   /* (args) -> return_type */                      \
  X(T_UNIT, "Unit")           /* () */

#define EXPR_KINDS(X)                                                          \
  X(E_INT_LIT, "IntLit")                                                       \
  X(E_FLOAT_LIT, "FloatLit")                                                   \
  X(E_STR_LIT, "StrLit")                                                       \
  X(E_CHAR_LIT, "CharLit")                                                     \
  X(E_BOOL_LIT, "BoolLit") /* true, false */                                   \
  X(E_IDENT, "Ident")      /* variable/constant names */                       \
  X(E_NONE, "None")        /* none keyword */                                  \
  X(E_BINARY, "Binary")    /* +, -, **, &&, ==, etc. */                        \
  X(E_UNARY, "Unary")      /* !, -, ~, ? */                                    \
  X(E_CALL, "Call")        /* func(args) */                                    \
  X(E_METHOD, "Method")    /* x.display() */                                   \
  X(E_ACCESS, "Access")    /* p.x */                                           \
  X(E_INDEX, "Index")      /* items[0] */                                      \
  X(E_STRUCT, "Struct")    /* Player { x = 0 } */                              \
  X(E_ARRAY, "Array")      /* [1, 2, 3] */                                     \
  X(E_RANGE, "Range")      /* 0..10, 0..=10 */                                 \
  X(E_IF, "If")            /* if as expression */                              \
  X(E_MATCH, "Match")      /* match as expression */                           \
  X(E_BLOCK, "Block")      /* { ... } resulting in a value */

#define STMT_KINDS(X)                                                          \
  X(S_LET, "Let")            /* let x = ... (optionally let mut) */            \
  X(S_ASSIGN, "Assign")      /* x = y */                                       \
  X(S_OP_ASSIGN, "OpAssign") /* +=, -=, *=, etc. */                            \
  X(S_EXPR, "ExprStmt")      /* expression followed by newline/semicolon */    \
  X(S_WHILE, "While")                                                          \
  X(S_FOR, "For")                                                              \
  X(S_LOOP, "Loop")                                                            \
  X(S_RETURN, "Return")                                                        \
  X(S_BREAK, "Break")                                                          \
  X(S_BLOCK, "Block")                                                          \
  X(S_CONTINUE, "Continue")                                                    \
  X(S_IMPORT, "Import")                                                        \
  X(S_MODULE, "Module")

#define DECL_KINDS(X)                                                          \
  X(D_STRUCT, "StructDecl")                                                    \
  X(D_ENUM, "EnumDecl")                                                        \
  X(D_FUNC, "FuncDecl")  /* covers free functions and methods */               \
  X(D_IMPL, "ImplDecl")  /* @ Player { ... } */                                \
  X(D_EXTERN, "Extern")  /* extern { ... } */                                  \
  X(D_TYPE, "TypeAlias") /* type aliases */

// --- ENUM GENERATION ---

typedef enum {
#define AS_ENUM(kind, name) kind,
  TYPE_KINDS(AS_ENUM)
#undef AS_ENUM
} TypeKind;

typedef enum {
#define AS_ENUM(kind, name) kind,
  EXPR_KINDS(AS_ENUM)
#undef AS_ENUM
} ExprKind;

typedef enum {
#define AS_ENUM(kind, name) kind,
  STMT_KINDS(AS_ENUM)
#undef AS_ENUM
} StmtKind;

typedef enum {
#define AS_ENUM(kind, name) kind,
  DECL_KINDS(AS_ENUM)
#undef AS_ENUM
} DeclKind;

// --- NODES ---

// Represents a single field in a struct: x: float
typedef struct {
  char *name;
  struct Type *type;
} Field;

// Represents an enum variant: PlayerMoved(Player) or RoundOver
typedef struct {
  char *name;
  struct Type *payload; // NULL if the variant has no data
} Variant;

// Represents a function parameter: (p, amount)
typedef struct {
  char *name;
  struct Type *type;
} Param;

// Represents one arm of a match statement: Pattern => Body
typedef struct {
  struct Expr *pattern; // e.g., PlayerMoved(p)
  struct Expr *guard;   // Optional 'if' condition (null if unused)
  struct Expr *body;    // The expression to execute
} MatchArm;

typedef struct {
  struct Expr *condition;
  struct Stmt *then_block; // optional
  struct Stmt *else_block; // NULL if no else
} IfExpr;

typedef struct {
  struct Expr *target; // The variables being matched
  MatchArm *arms;
  size_t arm_count;
} MatchExpr;

typedef struct {
  char *name;
  Param *params;
  size_t param_count;
  struct Type *return_type;
  struct Stmt *body;
} FuncDecl;

typedef struct {
  struct Type *target_type;
  struct Decl **methods; // array of D_FUNC
  size_t method_count;
} ImplDecl;

typedef struct {
  struct Stmt **statements;
  size_t stmt_count;
  struct Expr *expr_final; // the tail expression that returns a value
} Block;

struct Type {
  TypeKind kind;
  Token token; // for line / col reporting
  union {
    TType primitive;      // int, float,...
    char *name;           // T_NAME
    struct Type *element; // T_ARRAY
    struct {
      char *base;
      struct Type **params;
      size_t param_count;
    } generic; // T_GENERIC
    struct {
      struct Type **params;
      size_t param_count;
      struct Type *return_type;
    } function; // T_FUNCTION
  } as;
};

struct Expr {
  ExprKind kind;
  Token token; // for line / col reporting
  union {
    long int_val;
    double float_val;
    char *str_val;
    char char_val;
    char *ident_name;
    bool bool_val;

    // clang-format off
    struct { struct Expr *left; struct Expr *right; } binary;
    struct { struct Expr *operand; } unary;
    struct { struct Expr *callee; struct Expr **args; size_t arg_count; } call;
    struct { struct Expr *site; char *name; } access; // p.x
    struct { struct Expr *site; struct Expr *index; } index;  // items[0]
    // clang-format on

    struct {
      char *struct_name;
      struct {
        char *name;
        struct Expr *value;
      } *fields;
      size_t field_count;
    } _struct;

    struct {
      struct Expr **elements;
      size_t count;
    } array_init;

    struct {
      struct Expr *start;
      struct Expr *end;
      bool is_inclusive;
    } range;

    struct {
      struct Expr *site;
      char *name;
      struct Expr **args;
      size_t arg_count;
    } method;

    IfExpr if_expr;
    MatchExpr match_expr;
    Block block;
  } as;
};

struct Stmt {
  StmtKind kind;
  Token token;
  union {
    struct {
      char *name;
      bool is_mut;
      Type *type_annotation; // optional
      Expr *init;
    } let_binding;

    struct {
      Expr *target; // for x.y = ...
      Expr *value;
    } assign;

    struct {
      Token op; // Store the += or -= token here
      Expr *target;
      Expr *value;
    } op_assign;

    Block block; // reuse the block helper for S_BLOCK

    struct {
      Expr *condition;
      struct Stmt *body; // A S_BLOCK
    } _while;

    struct {
      char *iterator;
      Expr *iterable;
      struct Stmt *body;
    } _for;

    struct {
      struct Stmt *body;
    } _loop;

    struct Expr *expression; // S_EXPR
    struct Expr *return_value;
  } as;
};

struct Decl {
  DeclKind kind;
  Token token;
  union {
    // clang-format off
    struct { char *name; Field *fields; size_t field_count; } _struct;
    struct { char *name; Variant *variants; size_t variant_count; } _enum;
    struct { char *alias_name; Type *target; } type_alias;

    FuncDecl function;
    ImplDecl impl;
    struct { struct Decl **decls; size_t count; } extern_block;
    // clang-format on
  } as;
};

// this module represents one .koz file's AST
typedef struct {
  struct Decl **declarations;
  size_t count;
} Module;

// --- UTILITY PROTOTYPES ---

const char *type_kind_to_str(TypeKind k);
const char *expr_kind_to_str(ExprKind k);
const char *stmt_kind_to_str(StmtKind k);
const char *decl_kind_to_str(DeclKind k);

// --- VISUALIZER ---

/// Prints the entire module starting from top-level declarations.
void ast_print_module(Module *m);

/**
 * Individual printers if you need to debug a single node.
 * 'indent' is the current depth, 'last_mask' tracks vertical lines,
 * and 'is_last' determines if we draw a └── or ├──.
 */
void ast_print_decl(Decl *d, int indent, bool *last_mask, bool is_last);
void ast_print_stmt(Stmt *s, int indent, bool *last_mask, bool is_last);
void ast_print_expr(Expr *e, int indent, bool *last_mask, bool is_last);
void ast_print_type(Type *t); // Types are usually printed inline

// Type Builders
Type *ast_type_primitive(Arena *arena, Token token, TType primitive);
Type *ast_type_name(Arena *arena, Token token, char *name);
Type *ast_type_array(Arena *arena, Token token, Type *element);

// Expression Builders
Expr *ast_expr_int(Arena *arena, Token token, long value);
Expr *ast_expr_float(Arena *arena, Token token, double value);
Expr *ast_expr_string(Arena *arena, Token token, char *value);
Expr *ast_expr_char(Arena *arena, Token token, char value);
Expr *ast_expr_bool(Arena *arena, Token token, bool value);
Expr *ast_expr_none(Arena *arena, Token token);
Expr *ast_expr_ident(Arena *arena, Token token, char *name);
Expr *ast_expr_binary(Arena *arena, Token op_token, Expr *left, Expr *right);
Expr *ast_expr_unary(Arena *arena, Token op_token, Expr *operand);

Expr *ast_expr_access(Arena *arena, Token op_token, Expr *left, Token ident);
Expr *ast_expr_index(Arena *arena, Token bracket_token, Expr *site,
                     Expr *index);
Expr *ast_expr_call(Arena *arena, Token token, Expr *callee, Expr **args,
                    size_t arg_count);
Expr *ast_expr_block(Arena *arena, Token token, Stmt **stmts, size_t stmt_count,
                     Expr *tail);

Expr *ast_expr_array(Arena *arena, Token token, Expr **elements, size_t count);

// struct literal builder
Expr *ast_expr_struct(Arena *arena, Token token, char *struct_name,
                      size_t field_count, char **names, Expr **values);

// TODO: range builder
// Expr *ast_expr_range(Arena *arena, Token token, Expr *start, Expr *end, bool
// is_inclusive);

Expr *ast_expr_if(Arena *arena, Token token, Expr *condition, Stmt *then_expr,
                  Stmt *else_expr);

// TODO: match expression builder
// Expr *ast_expr_match(Arena *arena, Token token, Expr *target, MatchArm *arms,
// size_t arm_count);

// Statement Builders
Stmt *ast_stmt_let(Arena *arena, Token token, char *name, bool is_mut,
                   Type *type_annotation, Expr *init);
Stmt *ast_stmt_block(Arena *arena, Token token, Stmt **stmts, size_t stmt_count,
                     Expr *tail);
Stmt *ast_stmt_break(Arena *arena, Token token);
Stmt *ast_stmt_continue(Arena *arena, Token token);
Stmt *ast_stmt_return(Arena *arena, Token token, Expr *value);
Stmt *ast_stmt_expr(Arena *arena, Token token, Expr *expression);
Stmt *ast_stmt_while(Arena *arena, Token token, Expr *condition, Stmt *block);
Stmt *ast_stmt_loop(Arena *arena, Token token, Stmt *body);
Stmt *ast_stmt_for(Arena *arena, Token token, char *iterator, Expr *iterable,
                   Stmt *body);

Stmt *ast_stmt_assign(Arena *arena, Token token, Expr *target, Expr *value);
Stmt *ast_stmt_op_assign(Arena *arena, Token op_token, Expr *target,
                         Expr *value);

// TODO: import statement builder
// Stmt *ast_stmt_import(Arena *arena, Token token, char *path);

// Declaration Builders
Decl *ast_decl_extern(Arena *arena, Token token, Decl **decls,
                      size_t decl_count);
Decl *ast_decl_impl(Arena *arena, Token token, Type *target_type,
                    Decl **methods, size_t method_count);
Decl *ast_decl_func(Arena *arena, Token token, char *name, Param *params,
                    size_t param_count, Type *return_type, Stmt *body);
Decl *ast_decl_struct(Arena *arena, Token token, char *name, Field *fields,
                      size_t field_count);
Decl *ast_decl_enum(Arena *arena, Token token, char *name, Variant *variants,
                    size_t variant_count);

// TODO: type alias builder
// Decl *ast_decl_type_alias(Arena *arena, Token token, char *alias_name, Type
// *target);

#endif // AST_H

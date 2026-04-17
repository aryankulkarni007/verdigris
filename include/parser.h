#ifndef PARSER_H
#define PARSER_H

#include "arena.h"
#include "cst.h"
#include "diagnostic.h"
#include "lexer.h"

typedef struct {
  TStream tokens;
  size_t pos;
  const char *file_path;
  const char *source;
  DiagBag *diags;
  Arena *cst_arena;
  InternTable *intern;
  bool panic_mode;
} Parser;

#define PRECEDENCE(X)                                                          \
  X(ASSIGN)         /* = += -= *= /= %= */                                     \
  X(PIPELINE)       /* >> */                                                   \
  X(LOGIC_OR)       /* || */                                                   \
  X(LOGIC_AND)      /* && */                                                   \
  X(EQUALITY)       /* == != */                                                \
  X(COMPARISON)     /* < > <= >= */                                            \
  X(RANGE)          /* .. ..= */                                               \
  X(ADDITIVE)       /* + - */                                                  \
  X(MUTLIPLICATIVE) /* *  % */                                                 \
  X(CAST)           /* as */                                                   \
  X(UNARY)          /* - ! & * */                                              \
  X(ERROR_PROP)     /* ! */                                                    \
  X(INDEXING)       /* () [] . */                                              \
  X(PRIMARY)        /* i.e in parse_primary */

typedef enum {
#define AS_ENUM(name) name,
  PRECEDENCE(AS_ENUM)
#undef AS_ENUM
} PRECENDENCE;

void parser_init(Parser *p, TStream tokens, Arena *cst_arena, DiagBag *diags,
                 InternTable *intern, const char *file_path,
                 const char *source);

cst_d *parse(Parser *p);

cst_e *parse_expr(Parser *p);
cst_s *parse_stmt(Parser *p);
cst_d *parse_decl(Parser *p);
cst_d *parse_module(Parser *p);

static cst_e *parse_primary(Parser *p);

#endif

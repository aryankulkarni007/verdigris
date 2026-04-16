#pragma once

#include "cst.h"
#include "diagnostic.h"
#include "lexer.h"
#include "token.h"

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

cst_d *parse(Parser *p, TStream tokens, Arena *cst_arena, DiagBag *diags);

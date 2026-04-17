#include "../include/parser.h"
#include "../include/cst_builder.h"
#include "../include/error_codes.h"
#include <stdlib.h>

static Token *p_current(Parser *p) { return &p->tokens.data[p->pos]; }

static Token *p_peek(Parser *p) {
  if (p->pos + 1 >= p->tokens.len)
    return &p->tokens.data[p->pos];
  return &p->tokens.data[p->pos + 1];
}

static Token *p_advance(Parser *p) { return &p->tokens.data[p->pos++]; }

static bool p_check(Parser *p, TK_T type) { return p_current(p)->type == type; }

static bool p_match(Parser *p, TK_T type) {
  if (p_check(p, type)) {
    p_advance(p);
    return true;
  }
  return false;
}

static Token *p_expect(Parser *p, TK_T type, const char *expected) {
  if (p_check(p, type)) {
    return p_advance(p);
  }

  SourceSpan span = span_from_token(p_current(p));
  diag_error(p->diags, span, E_EXPECTED_TOKEN, "expected %s, found '%s'",
             expected, tk_name(p_current(p)->type));
  p->panic_mode = true;
  return NULL;
}

void parser_init(Parser *p, TStream tokens, Arena *cst_arena, DiagBag *diags,
                 InternTable *intern, const char *file_path,
                 const char *source) {
  p->tokens = tokens;
  p->pos = 0;
  p->file_path = file_path;
  p->source = source;
  p->diags = diags;
  p->cst_arena = cst_arena;
  p->intern = intern;
  p->panic_mode = false;
}

cst_d *parse(Parser *p) { return parse_module(p); }

cst_d *parse_module(Parser *p) { return (cst_d *)parse_expr(p); }

static const char *token_lexeme(Parser *p, Token *t) {
  size_t len = t->span.end - t->span.start;
  char *buf = arena_alloc(p->cst_arena, len + 1);
  snprintf(buf, len + 1, "%.*s", (int)len, p->source + t->span.start);
  return buf;
}

static cst_e *parse_primary(Parser *p) {
  Token *token = p_current(p);

  switch (token->type) {
  case TK_IDENT: {
    Token *t = p_advance(p);
    InternID id = t->id;

    if (id.key == intern_cstr(p->intern, "true").key) {
      return cst_new_bool(p->cst_arena, t, true);
    }
    if (id.key == intern_cstr(p->intern, "false").key) {
      return cst_new_bool(p->cst_arena, t, false);
    }
    if (id.key == intern_cstr(p->intern, "null").key) {
      return cst_new_null(p->cst_arena, t);
    }

    return cst_new_ident(p->cst_arena, t);
  }
  case TK_INT: {
    p_advance(p);
    int64_t value = strtoll(token_lexeme(p, token), NULL, 10);
    return cst_new_int(p->cst_arena, token, value);
  }
  case TK_FLOAT: {
    p_advance(p);
    double value = strtod(token_lexeme(p, token), NULL);
    return cst_new_float(p->cst_arena, token, value);
  }
  case TK_STRING: {
    p_advance(p);
    return cst_new_str(p->cst_arena, token);
  }
  case TK_OPAREN: {
    p_advance(p); // (
    cst_e *expr = parse_expr(p);
    p_expect(p, TK_CPAREN, "')'");
    return expr;
  }
  default: {
    SourceSpan span = span_from_token(token);
    diag_error(p->diags, span, E_UNEXPECTED_TOKEN, "unexpected token '%s'",
               tk_name(p_current(p)->type));
    p->panic_mode = true;
    return NULL;
  }
  }
}

/// (expr) (op) (expr)
cst_e *parse_binary(Parser *p) {
  cst_e *left = parse_expr(p);
  while (true) {
    Token *op = p_current(p);
  }
}

cst_e *parse_unary(Parser *p) {
  Token *op = p_current(p);
  if (op->type == TK_MINUS || op->type == TK_BANG || op->type == TK_AMP ||
      op->type == TK_STAR) {
    p_advance(p);
    cst_e *expr = parse_unary(p);
    return cst_new_unary(p->cst_arena, op, expr);
  }
  return parse_primary(p);
}

cst_e *parse_expr(Parser *p) { return parse_unary(p); }

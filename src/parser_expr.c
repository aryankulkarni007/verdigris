#include "../include/parser.h"
#include <stdio.h>
#include <stdlib.h>

// this parser_new follows my design principle for State Machine's like lexer
// and parser -> (parser | lexer)_new() inits the state and then the
// corresponding entry point into the state machine takes a reference to the
// arena. so we compartmentalise arena_init and only call arena_alloc outside of
// the main func. that may change soon tho
void parser_new(Parser *p, Token *tokens, size_t count) {
  p->tokens = tokens;
  p->pos = 0;
  p->count = count;
}

Expr *parse_infix(Parser *p, Arena *a, Expr *left, Token op) {
  switch (op.ttype) {
  case TOKEN_PLUS:
  case TOKEN_MINUS:
  case TOKEN_STAR:
  case TOKEN_SLASH:
  case TOKEN_PERCENT:
  case TOKEN_STARSTAR:
  case TOKEN_EQ:
  case TOKEN_NEQ:
  case TOKEN_LT:
  case TOKEN_GT:
  case TOKEN_LEQ:
  case TOKEN_GEQ:
  case TOKEN_AND:
  case TOKEN_OR:
  case TOKEN_BAND:
  case TOKEN_BOR:
  case TOKEN_BXOR:
  case TOKEN_LSHIFT:
  case TOKEN_RSHIFT: {
    Precedence prec = get_precedence(op.ttype);
    Precedence right_prec = (op.ttype == TOKEN_STARSTAR) ? prec - 1 : prec;
    Expr *right = parse_expr(p, a, right_prec);
    return ast_expr_binary(a, op, left, right);
  }

  case TOKEN_DOT: {
    Token ident = CURRENT(p);
    ADVANCE(p);
    return ast_expr_access(a, op, left, ident);
  }

  case TOKEN_LPAREN: {
    Expr *local_args[256];
    size_t arg_count = 0;

    if (CURRENT(p).ttype != TOKEN_RPAREN) {
      local_args[arg_count++] = parse_expr(p, a, PREC_NONE);
      while (CURRENT(p).ttype == TOKEN_COMMA) {
        ADVANCE(p);
        local_args[arg_count++] = parse_expr(p, a, PREC_NONE);
      }
    }

    EXPECT(p, TOKEN_RPAREN, "expected ')' after arguments");

    Expr **args = NULL;
    if (arg_count > 0) {
      args = arena_allocate(a, arg_count * sizeof(Expr *));
      for (size_t i = 0; i < arg_count; i++)
        args[i] = local_args[i];
    }

    return ast_expr_call(a, op, left, args, arg_count);
  }

  case TOKEN_LBRACK: {
    Expr *index_expr = parse_expr(p, a, PREC_NONE);
    EXPECT(p, TOKEN_RBRACK, "expected ']' after index expression");
    return ast_expr_index(a, op, left, index_expr);
  }

  case TOKEN_QUESTION:
    return ast_expr_unary(a, op, left);

  default:
    fprintf(stderr, "error at %zu:%zu: unknown infix operator '%s'\n", op.line,
            op.column, op.token);
    exit(1);
  }
}

Expr *parse_expr(Parser *p, Arena *a, Precedence min_prec) {
  Expr *left = parse_primary(p, a);
  // DEBUG : CURRENT WAS PEEK BEFORE
  while (get_precedence(CURRENT(p).ttype) > min_prec) {
    Token op = CURRENT(p);
    ADVANCE(p);
    left = parse_infix(p, a, left, op);
  }
  return left;
}

Expr *parse_literal(Parser *p, Arena *a) {
  Token t = CURRENT(p);
  ADVANCE(p);

  switch (t.ttype) {
  case TOKEN_INT_LIT:
    return ast_expr_int(a, t, atoll(t.token));
  case TOKEN_FLOAT_LIT:
    return ast_expr_float(a, t, atof(t.token));
  case TOKEN_STRING_LIT:
    return ast_expr_string(a, t, t.token);
  case TOKEN_CHAR_LIT:
    return ast_expr_char(a, t, t.token[0]);
  case TOKEN_TRUE:
    return ast_expr_bool(a, t, true);
  case TOKEN_FALSE:
    return ast_expr_bool(a, t, false);
  case TOKEN_NONE:
    return ast_expr_none(a, t);
  default:
    fprintf(
        stderr,
        "error at %zu:%zu: parse_literal called on non-literal token '%s'\n",
        t.line, t.column, t.token);
    exit(1);
  }
}

Expr *parse_name(Parser *p, Arena *a) {
  Token tok = CURRENT(p);
  ADVANCE(p);

  switch (tok.ttype) {
  case TOKEN_IDENT:
    return ast_expr_ident(a, tok, tok.token);
  case TOKEN_UNDERSCORE:
    return ast_expr_ident(a, tok, "_");
  default:
    fprintf(stderr,
            "error at %zu:%zu: parse_name called on non-name token '%s'\n",
            tok.line, tok.column, tok.token);
    exit(1);
  }
}

Expr *parse_grouped_expr(Parser *p, Arena *a) {
  ADVANCE(p);
  Expr *inner = parse_expr(p, a, PREC_NONE);
  EXPECT(p, TOKEN_RPAREN, "expected ')' after expression");
  return inner;
}

Expr *parse_block_expr(Parser *p, Arena *a) {
  Token start = CURRENT(p);
  ADVANCE(p);

  Stmt *local_stmts[1024];
  size_t stmt_count = 0;
  Expr *tail = NULL;

  while (CURRENT(p).ttype != TOKEN_RBRACE && CURRENT(p).ttype != TOKEN_EOF) {
    if (stmt_count >= 1024) {
      fprintf(stderr, "error at %zu:%zu: block exceeds 1024 statements\n",
              start.line, start.column);
      exit(1);
    }
    Stmt *stmt = parse_stmt(p, a);
    local_stmts[stmt_count++] = stmt;
  }

  if (CURRENT(p).ttype != TOKEN_RBRACE) {
    tail = parse_expr(p, a, PREC_NONE);
  }

  EXPECT(p, TOKEN_RBRACE, "expected '}' after block expression");

  Stmt **stmts = NULL;
  if (stmt_count > 0) {
    stmts = arena_allocate(a, stmt_count * sizeof(Stmt *));
    for (size_t i = 0; i < stmt_count; i++)
      stmts[i] = local_stmts[i];
  }

  return ast_expr_block(a, start, stmts, stmt_count, tail);
}

Stmt *parse_continue_stmt(Parser *p, Arena *a) {
  Token token = CURRENT(p);
  ADVANCE(p);
  return ast_stmt_continue(a, token);
}

Stmt *parse_return_stmt(Parser *p, Arena *a) {
  Token token = CURRENT(p);
  ADVANCE(p);

  Expr *value = NULL;
  if (CURRENT(p).ttype != TOKEN_RBRACE && CURRENT(p).ttype != TOKEN_EOF) {
    value = parse_expr(p, a, PREC_NONE);
  }
  return ast_stmt_return(a, token, value);
}

Stmt *parse_expr_stmt(Parser *p, Arena *a) {
  Token start = CURRENT(p);
  Expr *e = parse_expr(p, a, PREC_NONE);
  return ast_stmt_expr(a, start, e);
}

Expr *parse_primary(Parser *p, Arena *a) {
  Token token = CURRENT(p);

  switch (token.ttype) {
  case TOKEN_INT_LIT:
  case TOKEN_FLOAT_LIT:
  case TOKEN_STRING_LIT:
  case TOKEN_CHAR_LIT:
  case TOKEN_TRUE:
  case TOKEN_FALSE:
  case TOKEN_NONE:
    return parse_literal(p, a);

  case TOKEN_IDENT:
  case TOKEN_UNDERSCORE:
    return parse_name(p, a);

  case TOKEN_LPAREN:
    return parse_grouped_expr(p, a);

  case TOKEN_LBRACE:
    fprintf(stderr, "error at %zu:%zu: block expressions not yet implemented\n",
            token.line, token.column);
    exit(1);

  case TOKEN_LBRACK:
    fprintf(stderr, "error at %zu:%zu: array literals not yet implemented\n",
            token.line, token.column);
    exit(1);

  case TOKEN_IF:
    fprintf(stderr, "error at %zu:%zu: if expressions not yet implemented\n",
            token.line, token.column);
    exit(1);

  case TOKEN_MATCH:
    fprintf(stderr, "error at %zu:%zu: match expressions not yet implemented\n",
            token.line, token.column);
    exit(1);

  case TOKEN_SOME:
    fprintf(stderr, "error at %zu:%zu: constructor calls not yet implemented\n",
            token.line, token.column);
    exit(1);

  case TOKEN_MINUS:
  case TOKEN_NOT:
  case TOKEN_BNOT:
    fprintf(stderr, "error at %zu:%zu: unary expressions not yet implemented\n",
            token.line, token.column);
    exit(1);

  default:
    fprintf(stderr, "error at %zu:%zu: unexpected token '%s' in expression\n",
            token.line, token.column, token.token);
    exit(1);
  }
}

Precedence get_precedence(TType type) {
  switch (type) {
  case TOKEN_OR:
    return PREC_LOGICAL_OR;
  case TOKEN_AND:
    return PREC_LOGICAL_AND;
  case TOKEN_EQ:
  case TOKEN_NEQ:
    return PREC_EQUALITY;
  case TOKEN_LT:
  case TOKEN_GT:
  case TOKEN_LEQ:
  case TOKEN_GEQ:
    return PREC_COMPARE;
  case TOKEN_BOR:
    return PREC_BIT_OR;
  case TOKEN_BXOR:
    return PREC_BIT_XOR;
  case TOKEN_BAND:
    return PREC_BIT_AND;
  case TOKEN_LSHIFT:
  case TOKEN_RSHIFT:
    return PREC_SHIFT;
  case TOKEN_PLUS:
  case TOKEN_MINUS:
    return PREC_ADD;
  case TOKEN_STAR:
  case TOKEN_SLASH:
  case TOKEN_PERCENT:
    return PREC_MUL;
  case TOKEN_STARSTAR:
    return PREC_POWER;
  case TOKEN_DOT:
  case TOKEN_LPAREN:
  case TOKEN_LBRACK:
  case TOKEN_QUESTION:
    return PREC_POSTFIX;
  default:
    return PREC_NONE;
  }
}

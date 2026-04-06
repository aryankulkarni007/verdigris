#include "../include/parser.h"
#include <stdio.h>
#include <stdlib.h>

void parser_new(Parser *p, Token *tokens, size_t count) {
  p->tokens = tokens;
  p->pos = 0;
  p->count = count;
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
    // should be unreachable
    fprintf(stderr, "error: parse_literal called on a non-literal token\n");
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
    return ast_expr_ident(
        a, tok, "_"); // or ast_expr_underscore if you want a separate node

  default:
    fprintf(stderr, "error: parse_name called on non-name token\n");
    exit(1);
  }
}

Expr *parse_grouped_expr(Parser *p, Arena *a) {
  ADVANCE(p);
  Expr *inner = parse_expr(p, a, PREC_NONE);
  EXPECT(p, TOKEN_RPAREN, "expected ')' after expression");
  return inner;
}

// WARNING: draft function
Expr *parse_block_expr(Parser *p, Arena *a) {
  Token start = CURRENT(p); // the '{' token
  ADVANCE(p);

  // Stack array for statements (max 1024 per block)
  Stmt *local_stmts[1024];
  size_t stmt_count = 0;
  Expr *tail = NULL;

  // Parse statements until we hit '}' or EOF
  while (CURRENT(p).ttype != TOKEN_RBRACE && CURRENT(p).ttype != TOKEN_EOF) {
    if (stmt_count >= 1024) {
      fprintf(stderr, "error: block exceeds 1024 statements\n");
      exit(1);
    }

    Stmt *stmt = parse_stmt(p, a);
    local_stmts[stmt_count++] = stmt;
  }

  // Check for trailing expression (only if not already at '}')
  if (CURRENT(p).ttype != TOKEN_RBRACE) {
    tail = parse_expr(p, a, PREC_NONE);
  }

  EXPECT(p, TOKEN_RBRACE, "expected '}' after block expression");

  // Allocate exact array in arena and copy pointers
  Stmt **stmts = arena_allocate(a, stmt_count * sizeof(Stmt *));
  for (size_t i = 0; i < stmt_count; i++) {
    stmts[i] = local_stmts[i];
  }

  // Allocate the Expr node
  Expr *node = arena_allocate(a, sizeof(Expr));
  node->kind = E_BLOCK;
  node->token = start;
  node->as.block.statements = stmts;
  node->as.block.stmt_count = stmt_count;
  node->as.block.expr_final = tail;

  return node;
}
Stmt *parse_stmt(Parser *p, Arena *a) {
  switch (CURRENT(p).ttype) {
  case TOKEN_LET:
    return parse_let_stmt(p, a);
  case TOKEN_WHILE:
    return parse_while_stmt(p, a);
  case TOKEN_FOR:
    return parse_for_stmt(p, a);
  case TOKEN_LOOP:
    return parse_loop_stmt(p, a);
  case TOKEN_RETURN:
    return parse_return_stmt(p, a);
  case TOKEN_BREAK:
    return parse_break_stmt(p, a);
  case TOKEN_CONTINUE:
    return parse_continue_stmt(p, a);
  case TOKEN_LBRACE:
    return parse_block_stmt(p, a); // S_BLOCK, different from block expr
  default:
    // Expression statement
    Expr *e = parse_expr(p, a, PREC_NONE);
    return ast_stmt_expr(a, CURRENT(p), e); // you need this builder
  }
}

Expr *parse_primary(Parser *p, Arena *a) {
  Token token = CURRENT(p);

  switch (token.ttype) {
  // Literals
  case TOKEN_INT_LIT:
  case TOKEN_FLOAT_LIT:
  case TOKEN_STRING_LIT:
  case TOKEN_CHAR_LIT:
  case TOKEN_TRUE:
  case TOKEN_FALSE:
  case TOKEN_NONE:
    return parse_literal(p, a);
    break;

  // Names
  case TOKEN_IDENT:
  case TOKEN_UNDERSCORE:
    return parse_name(p, a);
    break;

  // Grouping & collections
  case TOKEN_LPAREN:
    return parse_grouped_expr(p, a);
    break;

  case TOKEN_LBRACE:
    return parse_block_expr(p, a);
    break;

  case TOKEN_LBRACK:
    return parse_array_expr(p, a);
    break;

  // Control flow expressions
  case TOKEN_IF:
    return parse_if_expr(p, a);
    break;

  case TOKEN_MATCH:
    return parse_match_expr(p, a);
    break;

  // Unary operators
  case TOKEN_MINUS:
  case TOKEN_NOT:
  case TOKEN_BNOT:
    return parse_unary_expr(p, a);
    break;

  default:
    // error
    break;
  }
}

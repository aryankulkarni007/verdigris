#include "../include/ast.h"
#include "../include/parser.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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
    // S_BLOCK, different from E_BLOCK, but only because one is a Stmt*
    // and the other is Expr*, under the hood they are the same struct
    return parse_block_stmt(p, a);
  default: {
    Token token = CURRENT(p);
    Expr *e = parse_expr(p, a, PREC_NONE);
    return ast_stmt_expr(a, token, e);
  }
  }
}

Stmt *parse_for_stmt(Parser *p, Arena *a) {
  Token _for = CURRENT(p);
  ADVANCE(p);

  if (CURRENT(p).ttype != TOKEN_IDENT) {
    fprintf(stderr, "error at %zu:%zu: expected identifier after 'for'\n",
            CURRENT(p).line, CURRENT(p).column);
    exit(1);
  }
  Token ident_token = CURRENT(p);
  char *iterator = ident_token.token;
  ADVANCE(p);

  EXPECT(p, TOKEN_IN, "expected 'in' after for loop iterator");

  Expr *iterable = parse_expr(p, a, PREC_NONE);

  Stmt *body = parse_block_stmt(p, a);

  return ast_stmt_for(a, _for, iterator, iterable, body);
}

Stmt *parse_let_stmt(Parser *p, Arena *a) {
  Token let = CURRENT(p);
  ADVANCE(p);
  bool is_mut = false;
  if (CURRENT(p).ttype == TOKEN_MUT) {
    is_mut = true;
    ADVANCE(p);
  }
  if (CURRENT(p).ttype != TOKEN_IDENT) {
    fprintf(stderr, "error at %zu:%zu: expected identifier after let\n",
            CURRENT(p).line, CURRENT(p).column);
    exit(1);
  }
  Token ident = CURRENT(p);
  char *name = ident.token;
  ADVANCE(p);

  Type *_type = NULL;
  if (CURRENT(p).ttype == TOKEN_COLON) {
    ADVANCE(p);
    _type = parse_type(p, a);
  }

  EXPECT(p, TOKEN_ASSIGN, "expected '=' after let binding");
  Expr *init = parse_expr(p, a, PREC_NONE);
  return ast_stmt_let(a, let, name, is_mut, _type, init);
}

Stmt *parse_while_stmt(Parser *p, Arena *a) {
  Token _while = CURRENT(p);
  ADVANCE(p);
  Expr *condition = parse_expr(p, a, PREC_NONE);
  EXPECT(p, TOKEN_LBRACE, "expected '{' after while condition");
  Stmt *block = parse_block_stmt(p, a);
  EXPECT(p, TOKEN_RBRACE, "expected '}' after while block");
  return ast_stmt_while(a, _while, condition, block);
}

Stmt *parse_loop_stmt(Parser *p, Arena *a) {
  Token loop_tok = CURRENT(p);
  ADVANCE(p);
  Stmt *body = parse_block_stmt(p, a);
  return ast_stmt_loop(a, loop_tok, body);
}

Stmt *parse_block_stmt(Parser *p, Arena *a) {
  Token start = CURRENT(p); // the '{' token
  ADVANCE(p);

  // stack array for statements (max 1024 per block)
  Stmt *local_stmts[1024];
  size_t stmt_count = 0;
  // Parse statements until we hit '}' or EOF
  while (CURRENT(p).ttype != TOKEN_RBRACE && CURRENT(p).ttype != TOKEN_EOF) {
    if (stmt_count >= 1024) {
      fprintf(stderr, "error at %zu:%zu: block exceeds 1024 statements\n",
              start.line, start.column);
      exit(1);
    }

    Stmt *stmt = parse_stmt(p, a);
    local_stmts[stmt_count++] = stmt;
  }

  // Check for trailing expression (only if not already at '}')
  // WARNING: but never do this for a block smtm - they don't have tail
  // expressions
  // if (CURRENT(p).ttype != TOKEN_RBRACE) {
  //   tail = parse_expr(p, a, PREC_NONE);
  // }

  EXPECT(p, TOKEN_RBRACE, "expected '}' after block expression");

  Stmt **stmts = NULL;
  if (stmt_count > 0) {
    stmts = arena_allocate(a, stmt_count * sizeof(Stmt *));
    for (size_t i = 0; i < stmt_count; i++)
      stmts[i] = local_stmts[i];
  }
  return ast_stmt_block(a, start, stmts, stmt_count, NULL);
}

Stmt *parse_break_stmt(Parser *p, Arena *a) {
  Token tok = CURRENT(p);
  ADVANCE(p);
  return ast_stmt_break(a, tok);
}

Type *parse_type(Parser *p, Arena *a) {
  Token token = CURRENT(p);

  switch (token.ttype) {
  case TOKEN_TINT:
  case TOKEN_TFLOAT:
  case TOKEN_TBOOL:
  case TOKEN_TSTRING:
  case TOKEN_TCHAR:
    ADVANCE(p);
    return ast_type_primitive(a, token, token.ttype);

  case TOKEN_IDENT:
    ADVANCE(p);
    return ast_type_name(a, token, token.token);

  case TOKEN_LBRACK:
    // Array type [T]
    ADVANCE(p); // consume '['
    Type *element = parse_type(p, a);
    EXPECT(p, TOKEN_RBRACK, "expected ']' after array element type");
    return ast_type_array(a, token, element);

  default:
    fprintf(stderr, "error at %zu:%zu: expected type, got '%s'\n", token.line,
            token.column, token.token);
    exit(1);
  }
}

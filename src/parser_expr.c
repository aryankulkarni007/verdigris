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
    // Binary operator
    Precedence prec = get_precedence(op.ttype);
    // For right-associative operators (like **), use prec - 1
    Precedence right_prec = (op.ttype == TOKEN_STARSTAR) ? prec - 1 : prec;
    Expr *right = parse_expr(p, a, right_prec);
    return ast_expr_binary(a, op, left, right);
  }

  case TOKEN_DOT: {
    Token ident = CURRENT(p);
    ADVANCE(p);
    return ast_expr_access(a, op, left, ident);
  } break;

  case TOKEN_LPAREN: {
    Expr *local_args[256];
    size_t arg_count = 0;

    if (CURRENT(p).ttype != TOKEN_RPAREN) {
      // first argument
      local_args[arg_count++] = parse_expr(p, a, PREC_NONE);

      // additional arguments after commas
      while (CURRENT(p).ttype == TOKEN_COMMA) {
        ADVANCE(p); // consume comma
        local_args[arg_count++] = parse_expr(p, a, PREC_NONE);
      }
    }

    EXPECT(p, TOKEN_RPAREN, "expected ')' after arguments");

    // copy to arena
    Expr **args = arena_allocate(a, arg_count * sizeof(Expr *));
    for (size_t i = 0; i < arg_count; i++) {
      args[i] = local_args[i];
    }

    return ast_expr_call(a, op, left, args, arg_count);
  } break;

  case TOKEN_LBRACK: {
    Expr *index_expr = parse_expr(p, a, PREC_NONE);
    EXPECT(p, TOKEN_RBRACK, "expected ']' after index expression");
    return ast_expr_index(a, op, left, index_expr);
  } break;

  case TOKEN_QUESTION:
    // error propagation
    return ast_expr_unary(a, op, left); // postfix '?' wraps the left

  default:
    fprintf(stderr, "error: unknown infix operator\n");
    exit(1);
  }
}

Expr *parse_expr(Parser *p, Arena *a, Precedence min_prec) {
  // primary expression
  Expr *left = parse_primary(p, a);

  // while the next operator has precedence >= min_prec
  while (get_precedence(PEEK(p).ttype) >= min_prec) {
    Token op = ADVANCE(p); // consume the operator
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

Expr *parse_block_expr(Parser *p, Arena *a) {
  Token start = CURRENT(p); // the '{' token
  ADVANCE(p);

  // stack array for statements (max 1024 per block)
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

  return ast_expr_block(a, start, stmts, stmt_count, tail);
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

Stmt *parse_break_stmt(Parser *p, Arena *a) {
  Token token = CURRENT(p);
  ADVANCE(p);
  return ast_stmt_break(a, token);
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

  if (CURRENT(p).ttype != TOKEN_RBRACE && CURRENT(p).ttype != TOKEN_EOF)
    value = parse_expr(p, a, PREC_NONE);

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

  // names
  case TOKEN_IDENT:
  case TOKEN_UNDERSCORE:
    return parse_name(p, a);

  // grouping
  case TOKEN_LPAREN:
    return parse_grouped_expr(p, a);

  // block expression
  case TOKEN_LBRACE:
    return parse_block_expr(p, a);

  // array literal
  case TOKEN_LBRACK:
    // TODO: implement parse_array_expr
    fprintf(stderr, "error: array literals not implemented yet\n");
    exit(1);

  // control flow expressions
  case TOKEN_IF:
    // TODO: implement parse_if_expr
    fprintf(stderr, "error: if expressions not implemented yet\n");
    exit(1);

  case TOKEN_MATCH:
    // TODO: implement parse_match_expr
    fprintf(stderr, "error: match expressions not implemented yet\n");
    exit(1);

  // constructor
  case TOKEN_SOME:
    // TODO: implement parse_constructor_call
    fprintf(stderr, "error: constructor calls not implemented yet\n");
    exit(1);

  // unary operators
  case TOKEN_MINUS:
  case TOKEN_NOT:
  case TOKEN_BNOT:
    // TODO: implement parse_unary_expr
    fprintf(stderr, "error: unary expressions not implemented yet\n");
    exit(1);

  default:
    fprintf(stderr, "error: unexpected token '%s' at %zu:%zu\n", token.token,
            token.line, token.column);
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

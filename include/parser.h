#ifndef PARSER_H
#define PARSER_H

#include "arena.h"
#include "ast.h"
#include "token.h"
#include <stdbool.h>
#include <stddef.h>

typedef enum {
  PREC_NONE, // 0 - Starting point for parse_expr().
             // Consumes everything until EOF or low-precedence operator.
             // Use when: parse_expr(arena, p, PREC_NONE)

  PREC_LOGICAL_OR, // 1 - || (lowest binary operator)
                   // Right side uses PREC_LOGICAL_OR (left-associative)

  PREC_LOGICAL_AND, // 2 - &&
                    // Right side uses PREC_LOGICAL_AND

  PREC_EQUALITY, // 3 - == !=
                 // Right side uses PREC_EQUALITY

  PREC_COMPARE, // 4 - < > <= >=
                // Right side uses PREC_COMPARE

  PREC_BIT_OR, // 5 - |
               // Right side uses PREC_BIT_OR

  PREC_BIT_XOR, // 6 - ^
                // Right side uses PREC_BIT_XOR

  PREC_BIT_AND, // 7 - &
                // Right side uses PREC_BIT_AND

  PREC_SHIFT, // 8 - << >>
              // Right side uses PREC_SHIFT

  PREC_ADD, // 9 - + -
            // Right side uses PREC_ADD

  PREC_MUL, // 10 - * / %
            // Right side uses PREC_MUL

  PREC_POWER, // 11 - ** (right-associative)
              // Right side uses PREC_POWER - 1 (because right binds tighter)
              // Example: 2 ** 3 ** 4 = 2 ** (3 ** 4)

  PREC_POSTFIX, // 12 - . [ ( ? (highest)
} Precedence;

Precedence get_precedence(TType type);

typedef struct {
  Token *tokens;
  size_t pos;
  size_t count;
} Parser;

// helpers
#define CURRENT(p) ((p)->tokens[(p)->pos])
#define PEEK(p) ((p)->tokens[(p)->pos + 1])
#define PEEK_NEXT(p) ((p)->tokens[(p)->pos + 2])
#define ADVANCE(p) ((p)->tokens[(p)->pos++])
#define IS_AT_END(p) ((p)->tokens[(p)->pos].ttype == TOKEN_EOF)
#define EXPECT(p, type, msg)                                                   \
  if (CURRENT(p).ttype != (type)) {                                            \
    Token tok = CURRENT(p);                                                    \
    fprintf(stderr, "error at %zu:%zu: %s, got '%s'\n", tok.line, tok.column,  \
            (msg), tok.token);                                                 \
    exit(1);                                                                   \
  }                                                                            \
  ADVANCE(p)

// main functions
void parser_new(Parser *parser, Token *stream, size_t count);
Module *parse(Arena *arena, Parser *p);

Expr *parse_expr(Parser *p, Arena *arena, Precedence min_prec);
Expr *parse_primary(Parser *parser, Arena *arena);
Expr *parse_infix(Parser *p, Arena *a, Expr *left, Token op);

// parse expr helper
Expr *parse_literal(Parser *parser, Arena *arena);
Expr *parse_name(Parser *parser, Arena *arena);
Expr *parse_grouped_expr(Parser *parser, Arena *arena);
Expr *parse_block_expr(Parser *parser, Arena *arena);
Expr *parse_array_expr(Parser *parser, Arena *arena);
Expr *parse_if_expr(Parser *parser, Arena *arena);
Expr *parse_match_expr(Parser *parser, Arena *arena);
Expr *parse_unary_expr(Parser *parser, Arena *arena);

// TODO: parse stmt helper
Stmt *parse_stmt(Parser *p, Arena *a);
Stmt *parse_let_stmt(Parser *p, Arena *a);
Stmt *parse_while_stmt(Parser *p, Arena *a);
Stmt *parse_for_stmt(Parser *p, Arena *a);
Stmt *parse_loop_stmt(Parser *p, Arena *a);
Stmt *parse_return_stmt(Parser *p, Arena *a);
Stmt *parse_break_stmt(Parser *p, Arena *a);
Stmt *parse_continue_stmt(Parser *p, Arena *a);
Stmt *parse_block_stmt(Parser *p, Arena *a);
Stmt *ast_stmt_expr(Arena *a, Token token, Expr *e); // you need this builder
// S_BLOCK, different from block e;

// Token current(Parser *p);
// Token peek(Parser *p);
// Token advance(Parser *p);
// void expect(Parser *p, TType type, const char *msg);

#endif // PARSER_H

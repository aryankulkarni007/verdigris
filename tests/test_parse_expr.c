#include "../include/lexer.h"
#include "../include/parser.h"
#include <stdio.h>
#include <string.h>

// Helper to tokenize a source string
static Token *tokenize_source(Arena *arena, const char *source,
                              size_t *token_count) {
  Source src = {.buffer = (char *)source, .file_size = strlen(source)};
  Lexer lexer;
  lexer_new(&lexer, src);
  lex(arena, &lexer);

  Token *stream = (Token *)arena->start;
  size_t count = 0;
  while (stream[count].ttype != TOKEN_EOF)
    count++;
  *token_count = count;
  return stream;
}

// Test helper: parse expression and return
static Expr *parse_expr_from_string(Arena *arena, const char *source) {
  size_t token_count;
  Token *tokens = tokenize_source(arena, source, &token_count);

  Parser parser;
  parser_new(&parser, tokens, token_count);

  return parse_expr(&parser, arena, PREC_NONE);
}

void test_integer_literal(void) {
  Arena arena;
  char buf[4096];
  arena_init(&arena, buf, sizeof(buf));

  Expr *expr = parse_expr_from_string(&arena, "42");

  if (expr->kind == E_INT_LIT && expr->as.int_val == 42) {
    printf("test_integer_literal passed\n");
  } else {
    printf("test_integer_literal failed\n");
  }
}

void test_binary_addition(void) {
  Arena arena;
  char buf[4096];
  arena_init(&arena, buf, sizeof(buf));

  Expr *expr = parse_expr_from_string(&arena, "1 + 2");

  if (expr->kind == E_BINARY && expr->token.ttype == TOKEN_PLUS &&
      expr->as.binary.left->kind == E_INT_LIT &&
      expr->as.binary.left->as.int_val == 1 &&
      expr->as.binary.right->kind == E_INT_LIT &&
      expr->as.binary.right->as.int_val == 2) {
    printf("test_binary_addition passed\n");
  } else {
    printf("test_binary_addition failed\n");
  }
}

void run_parser_tests(void) {
  printf("\nStarting Koz Parser Tests...\n");
  printf("=============================\n\n");

  test_integer_literal();
  test_binary_addition();
  // Add more tests

  printf("\n=============================\n");
  printf("Parser Tests Complete.\n");
}

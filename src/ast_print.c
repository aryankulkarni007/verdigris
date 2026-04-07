#include "../include/lexer.h"
#include "../include/parser.h"
#include <stdio.h>
#include <string.h>

// Helper: parse source into a module (for testing expressions, wrap in dummy
// function)
static Module *parse_source_to_module(Arena *arena, const char *source) {
  // Tokenize
  Source src = {.buffer = (char *)source, .file_size = strlen(source)};

  // Token arena
  Arena token_arena;
  char token_buf[4096];
  arena_init(&token_arena, token_buf, sizeof(token_buf));

  Lexer lexer;
  lexer_new(&lexer, src);
  lex(&token_arena, &lexer);

  Token *stream = token_arena.start;
  size_t token_count = 0;
  while (stream[token_count].ttype != TOKEN_EOF)
    token_count++;

  // Parse
  Parser parser;
  parser_new(&parser, stream, token_count);

  // For expressions, wrap in a function body for testing
  // Or just parse expression directly and print
  return NULL; // TODO
}

// Simple test: parse expression and print
void test_parse_expr(const char *name, const char *source) {
  printf("\n=== Test: %s ===\n", name);
  printf("Source: %s\n", source);

  // Token arena
  Arena token_arena;
  char token_buf[4096];
  arena_init(&token_arena, token_buf, sizeof(token_buf));

  // Parse source
  Source src = {.buffer = (char *)source, .file_size = strlen(source)};
  Lexer lexer;
  lexer_new(&lexer, src);
  lex(&token_arena, &lexer);

  Token *stream = token_arena.start;
  size_t token_count = 0;
  while (stream[token_count].ttype != TOKEN_EOF)
    token_count++;

  // AST arena
  Arena ast_arena;
  char ast_buf[16384];
  arena_init(&ast_arena, ast_buf, sizeof(ast_buf));

  Parser parser;
  parser_new(&parser, stream, token_count);

  Expr *expr = parse_expr(&parser, &ast_arena, PREC_NONE);

  // Wrap expression in a dummy statement and module for printing
  Stmt *expr_stmt = ast_stmt_expr(&ast_arena, expr->token, expr);
  Stmt *block_stmts[] = {expr_stmt};
  Block block = {
      .statements = block_stmts, .stmt_count = 1, .expr_final = NULL};
  Stmt *func_body =
      ast_stmt_block(&ast_arena, expr->token, block_stmts, 1, NULL);

  Decl *func =
      ast_decl_func(&ast_arena, expr->token, "test", NULL, 0, NULL, func_body);
  Decl *decls[] = {func};
  Module mod = {.declarations = decls, .count = 1};

  ast_print_module(&mod);
}

void run_parser_tests(void) {
  printf("\n========================================\n");
  printf("Koz Parser Tests \n");
  printf("========================================\n");

  test_parse_expr("Integer Literal", "42");
  test_parse_expr("Binary Addition", "1 + 2");
  test_parse_expr("Precedence", "1 + 2 * 3");
  test_parse_expr("Parentheses", "(1 + 2) * 3");
  test_parse_expr("Function Call", "foo(1, 2)");
  test_parse_expr("Index Access", "items[0]");
  test_parse_expr("Field Access", "player.x");
  test_parse_expr("Block Expression", "{ 5 + 3 }");
  test_parse_expr("Nested Block", "{ let x = 5; x + 2 }");

  printf("\n========================================\n");
  printf("Parser Tests Complete\n");
  printf("========================================\n");
}

#include "../include/arena.h"
#include "../include/cst.h"
#include "../include/diagnostic.h"
#include "../include/intern.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/pretty.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  Arena token_arena;
  Arena trivia_arena;
  Arena string_arena;
  Arena cst_arena;
  Arena diag_arena;
  InternTable table;
  DiagBag diags;
} TestContext;

static TestContext g_ctx;

static void test_init(void) {
  g_ctx.token_arena = arena_init(1 << 20);
  g_ctx.trivia_arena = arena_init(1 << 20);
  g_ctx.string_arena = arena_init(1 << 20);
  g_ctx.cst_arena = arena_init(1 << 20);
  g_ctx.diag_arena = arena_init(1 << 20);
  g_ctx.table = intern_init(&g_ctx.string_arena);
  g_ctx.diags = diag_init(&g_ctx.diag_arena);
}

static cst_e *parse_source(const char *buffer) {
  Source source = {
      .buffer = buffer,
      .file_path = "<test>",
      .file_size = strlen(buffer),
  };

  Lexer lexer;
  lexer_init(&lexer, &g_ctx.token_arena, &g_ctx.string_arena,
             &g_ctx.trivia_arena, &source, &g_ctx.table);
  TStream stream = lex(&lexer);

  Parser p;
  parser_init(&p, stream, &g_ctx.cst_arena, &g_ctx.diags, &g_ctx.table,
              source.file_path, source.buffer);

  return parse_expr(&p);
}

static void test_parse_expr(const char *name, const char *source,
                            cst_e *(*parser_fn)(Parser *)) {
  printf("\n=== Testing %s ===\n", name);

  cst_e *expr = parse_source(source);

  cst_pretty_init(stdout, &g_ctx.table);
  printf("Source: %s\n", source);
  printf("Parsed: ");
  print_expr(expr);
  printf("\n");
}

// ============================================================================
// Tests
// ============================================================================

void test_parse_int(void) {
  const char *source = "42";
  cst_e *expr = parse_source(source);

  cst_pretty_init(stdout, &g_ctx.table);
  printf("Source: %s\n", source);
  printf("Parsed: ");
  print_expr(expr);
  printf("\n");

  assert(expr->kind == CST_INT_LIT);
  assert(((INT_L_CSTN *)expr)->value == 42);
  printf("✓ Parse integer\n");
}

void test_parse_ident(void) {
  const char *source = "x";
  cst_e *expr = parse_source(source);

  cst_pretty_init(stdout, &g_ctx.table);
  printf("Source: %s\n", source);
  printf("Parsed: ");
  print_expr(expr);
  printf("\n");

  assert(expr->kind == CST_IDENT);
  printf("✓ Parse identifier\n");
}

void test_parse_float(void) {
  const char *source = "3.14";
  cst_e *expr = parse_source(source);

  cst_pretty_init(stdout, &g_ctx.table);
  printf("Source: %s\n", source);
  printf("Parsed: ");
  print_expr(expr);
  printf("\n");

  assert(expr->kind == CST_FLOAT_LIT);
  assert(((FLT_L_CSTN *)expr)->value == 3.14);
  printf("✓ Parse float\n");
}

void test_parse_string(void) {
  const char *source = "\"hello\"";
  cst_e *expr = parse_source(source);

  cst_pretty_init(stdout, &g_ctx.table);
  printf("Source: %s\n", source);
  printf("Parsed: ");
  print_expr(expr);
  printf("\n");

  assert(expr->kind == CST_STRING_LIT);
  printf("✓ Parse string\n");
}

void test_parse_bool(void) {
  const char *source = "true";
  cst_e *expr = parse_source(source);

  cst_pretty_init(stdout, &g_ctx.table);
  printf("Source: %s\n", source);
  printf("Parsed: ");
  print_expr(expr);
  printf("\n");

  assert(expr->kind == CST_BOOL_LIT);
  assert(((BOOL_L_CSTN *)expr)->value == true);
  printf("✓ Parse bool (true)\n");

  source = "false";
  expr = parse_source(source);
  assert(expr->kind == CST_BOOL_LIT);
  assert(((BOOL_L_CSTN *)expr)->value == false);
  printf("✓ Parse bool (false)\n");
}

void test_parse_null(void) {
  const char *source = "null";
  cst_e *expr = parse_source(source);

  cst_pretty_init(stdout, &g_ctx.table);
  printf("Source: %s\n", source);
  printf("Parsed: ");
  print_expr(expr);
  printf("\n");

  assert(expr->kind == CST_NULL_LIT);
  printf("✓ Parse null\n");
}

void test_parse_group(void) {
  const char *source = "(42)";
  cst_e *expr = parse_source(source);

  cst_pretty_init(stdout, &g_ctx.table);
  printf("Source: %s\n", source);
  printf("Parsed: ");
  print_expr(expr);
  printf("\n");

  assert(expr->kind == CST_INT_LIT);
  assert(((INT_L_CSTN *)expr)->value == 42);
  printf("✓ Parse grouped expression\n");
}

void test_parse_unary_neg(void) {
  const char *source = "-42";
  cst_e *expr = parse_source(source);

  cst_pretty_init(stdout, &g_ctx.table);
  printf("Source: %s\n", source);
  printf("Parsed: ");
  print_expr(expr);
  printf("\n");

  assert(expr->kind == CST_UNARY_EXPR);
  UNARY_E_CSTN *u = (UNARY_E_CSTN *)expr;
  assert(u->op->type == TK_MINUS);
  assert(u->expr->kind == CST_INT_LIT);
  assert(((INT_L_CSTN *)u->expr)->value == 42);
  printf("✓ Parse unary negation\n");
}

void test_parse_unary_not(void) {
  const char *source = "!true";
  cst_e *expr = parse_source(source);

  cst_pretty_init(stdout, &g_ctx.table);
  printf("Source: %s\n", source);
  printf("Parsed: ");
  print_expr(expr);
  printf("\n");

  assert(expr->kind == CST_UNARY_EXPR);
  UNARY_E_CSTN *u = (UNARY_E_CSTN *)expr;
  assert(u->op->type == TK_BANG);
  assert(u->expr->kind == CST_BOOL_LIT);
  assert(((BOOL_L_CSTN *)u->expr)->value == true);
  printf("✓ Parse unary not\n");
}

void test_parse_unary_group(void) {
  const char *source = "-(42)";
  cst_e *expr = parse_source(source);

  cst_pretty_init(stdout, &g_ctx.table);
  printf("Source: %s\n", source);
  printf("Parsed: ");
  print_expr(expr);
  printf("\n");

  assert(expr->kind == CST_UNARY_EXPR);
  UNARY_E_CSTN *u = (UNARY_E_CSTN *)expr;
  assert(u->op->type == TK_MINUS);
  assert(u->expr->kind == CST_INT_LIT);
  assert(((INT_L_CSTN *)u->expr)->value == 42);
  printf("✓ Parse unary with group\n");
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
  printf("========================================\n");
  printf("         Parser Tests\n");
  printf("========================================\n");

  test_init();

  // Literals
  test_parse_int();
  test_parse_float();
  test_parse_string();
  test_parse_bool();
  test_parse_null();
  test_parse_ident();

  // Grouping and Unary
  test_parse_group();
  test_parse_unary_neg();
  test_parse_unary_not();
  test_parse_unary_group();

  // TODO: Add binary, postfix, etc.

  printf("\nAll parser tests passed!\n");
  return 0;
}

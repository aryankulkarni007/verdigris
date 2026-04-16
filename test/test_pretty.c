// test/test_pretty.c
#include "../include/arena.h"
#include "../include/cst.h"
#include "../include/cst_builder.h"
#include "../include/intern.h"
#include "../include/pretty.h"
#include "../include/token.h"
#include <assert.h>
#include <stdio.h>

// Helper to create tokens for testing
static Token make_token(TK_T type, const char *lexeme, InternTable *intern) {
  return (Token){
      .span = {.start = 0, .end = strlen(lexeme)},
      .type = type,
      .line = 1,
      .line_start_pos = 0,
      .path = "<test>",
      .id = intern_cstr(intern, lexeme),
  };
}

void test_pretty_literals(void) {
  printf("\n=== Testing Literals ===\n\n");

  Arena arena = arena_init(1LL << 20);
  InternTable intern = intern_init(&arena);

  cst_pretty_init(stdout, &intern);

  // Integer
  Token int_tok = make_token(TK_INT, "42", &intern);
  cst_e *int_lit = cst_new_int(&arena, &int_tok, 42);
  printf("Int: ");
  print_expr(int_lit);
  printf("\n");

  // Float
  Token float_tok = make_token(TK_FLOAT, "3.14", &intern);
  cst_e *float_lit = cst_new_float(&arena, &float_tok, 3.14);
  printf("Float: ");
  print_expr(float_lit);
  printf("\n");

  // String
  Token str_tok = make_token(TK_STRING, "\"hello\"", &intern);
  cst_e *str_lit = cst_new_str(&arena, &str_tok);
  printf("String: ");
  print_expr(str_lit);
  printf("\n");

  // Bool
  Token true_tok = make_token(TK_TRUE, "true", &intern);
  cst_e *bool_lit = cst_new_bool(&arena, &true_tok, true);
  printf("Bool: ");
  print_expr(bool_lit);
  printf("\n");

  // Null
  Token null_tok = make_token(TK_NULL, "null", &intern);
  cst_e *null_lit = cst_new_null(&arena, &null_tok);
  printf("Null: ");
  print_expr(null_lit);
  printf("\n");

  // Identifier
  Token ident_tok = make_token(TK_IDENT, "x", &intern);
  cst_e *ident = cst_new_ident(&arena, &ident_tok);
  printf("Ident: ");
  print_expr(ident);
  printf("\n");
}

void test_pretty_unary(void) {
  printf("\n=== Testing Unary Expressions ===\n\n");

  Arena arena = arena_init(1LL << 20);
  InternTable intern = intern_init(&arena);
  cst_pretty_init(stdout, &intern);

  Token x_tok = make_token(TK_IDENT, "x", &intern);
  Token neg_tok = make_token(TK_MINUS, "-", &intern);
  Token not_tok = make_token(TK_BANG, "!", &intern);

  cst_e *x = cst_new_ident(&arena, &x_tok);
  cst_e *neg_x = cst_new_unary(&arena, &neg_tok, x);
  cst_e *not_x = cst_new_unary(&arena, &not_tok, x);

  printf("-x: ");
  print_expr(neg_x);
  printf("\n");

  printf("!x: ");
  print_expr(not_x);
  printf("\n");
}

void test_pretty_binary(void) {
  printf("\n=== Testing Binary Expressions ===\n\n");

  Arena arena = arena_init(1LL << 20);
  InternTable intern = intern_init(&arena);
  cst_pretty_init(stdout, &intern);

  Token one_tok = make_token(TK_INT, "1", &intern);
  Token two_tok = make_token(TK_INT, "2", &intern);
  Token three_tok = make_token(TK_INT, "3", &intern);
  Token plus_tok = make_token(TK_PLUS, "+", &intern);
  Token mul_tok = make_token(TK_STAR, "*", &intern);

  cst_e *one = cst_new_int(&arena, &one_tok, 1);
  cst_e *two = cst_new_int(&arena, &two_tok, 2);
  cst_e *three = cst_new_int(&arena, &three_tok, 3);

  cst_e *add = cst_new_binary(&arena, one, &plus_tok, two);
  cst_e *mul = cst_new_binary(&arena, add, &mul_tok, three);

  printf("1 + 2 * 3: ");
  print_expr(mul);
  printf("\n");
}

void test_pretty_types(void) {
  printf("\n=== Testing Types ===\n\n");

  Arena arena = arena_init(1LL << 20);
  InternTable intern = intern_init(&arena);
  cst_pretty_init(stdout, &intern);

  // Primitive
  Token int_tok = make_token(TK_INT, "int", &intern);
  cst_t *prim = cst_new_prim_type(&arena, &int_tok);
  printf("Primitive: ");
  print_type(prim);
  printf("\n");

  // Optional
  Token question_tok = make_token(TK_QUESTION, "?", &intern);
  cst_t *opt = cst_new_opt_type(&arena, &question_tok, prim);
  printf("Optional: ");
  print_type(opt);
  printf("\n");

  // Array
  Token obrack_tok = make_token(TK_OBRACK, "[", &intern);
  Token cbrack_tok = make_token(TK_CBRACK, "]", &intern);
  Token size_tok = make_token(TK_INT, "4", &intern);
  cst_t *arr =
      cst_new_array_type(&arena, &obrack_tok, prim, &size_tok, &cbrack_tok);
  printf("Array: ");
  print_type(arr);
  printf("\n");
}

void test_pretty_struct_lit(void) {
  printf("\n=== Testing Struct Literal ===\n\n");

  Arena arena = arena_init(1LL << 20);
  InternTable intern = intern_init(&arena);
  cst_pretty_init(stdout, &intern);

  Token point_tok = make_token(TK_IDENT, "Point", &intern);
  Token obrace_tok = make_token(TK_OBRACE, "{", &intern);
  Token cbrace_tok = make_token(TK_CBRACE, "}", &intern);
  Token x_tok = make_token(TK_IDENT, "x", &intern);
  Token y_tok = make_token(TK_IDENT, "y", &intern);
  Token colon_tok = make_token(TK_COL, ":", &intern);
  Token zero_tok = make_token(TK_INT, "0", &intern);

  cst_t *point_type = cst_new_named_type(&arena, &point_tok);
  cst_e *struct_lit =
      cst_new_struct_lit(&arena, point_type, &obrace_tok, &cbrace_tok);
  STRUCT_LE_CSTN *s = (STRUCT_LE_CSTN *)struct_lit;

  FieldInit *fx = arena_alloc(&arena, sizeof(FieldInit));
  fx->name = &x_tok;
  fx->colon = &colon_tok;
  fx->value = cst_new_int(&arena, &zero_tok, 0);
  vec_push_struct(&arena, s->fields, fx);

  FieldInit *fy = arena_alloc(&arena, sizeof(FieldInit));
  fy->name = &y_tok;
  fy->colon = &colon_tok;
  fy->value = cst_new_int(&arena, &zero_tok, 0);
  vec_push_struct(&arena, s->fields, fy);

  printf("Struct literal: ");
  print_expr(struct_lit);
  printf("\n");
}

void test_pretty_block(void) {
  printf("\n=== Testing Block ===\n\n");

  Arena arena = arena_init(1LL << 20);
  InternTable intern = intern_init(&arena);
  cst_pretty_init(stdout, &intern);

  Token obrace_tok = make_token(TK_OBRACE, "{", &intern);
  Token cbrace_tok = make_token(TK_CBRACE, "}", &intern);
  Token x_tok = make_token(TK_IDENT, "x", &intern);
  Token semi_tok = make_token(TK_SEMI, ";", &intern);
  Token one_tok = make_token(TK_INT, "1", &intern);

  cst_s *block = cst_new_block_stmt(&arena, &obrace_tok, &cbrace_tok);
  BLOCK_CSTN *b = (BLOCK_CSTN *)block;

  cst_e *x = cst_new_ident(&arena, &x_tok);
  cst_s *stmt = cst_new_expr_stmt(&arena, x, &semi_tok);
  vec_push_struct(&arena, b->stmts, stmt);

  b->tail = cst_new_int(&arena, &one_tok, 1);

  printf("Block:\n");
  print_stmt(block);
  printf("\n");
}

void test_pretty_if_expr(void) {
  printf("\n=== Testing If Expression ===\n\n");

  Arena arena = arena_init(1LL << 20);
  InternTable intern = intern_init(&arena);
  cst_pretty_init(stdout, &intern);

  Token if_tok = make_token(TK_IF, "if", &intern);
  Token else_tok = make_token(TK_ELSE, "else", &intern);
  Token x_tok = make_token(TK_IDENT, "x", &intern);
  Token one_tok = make_token(TK_INT, "1", &intern);
  Token zero_tok = make_token(TK_INT, "0", &intern);
  Token obrace_tok = make_token(TK_OBRACE, "{", &intern);
  Token cbrace_tok = make_token(TK_CBRACE, "}", &intern);

  cst_e *cond = cst_new_ident(&arena, &x_tok);
  cst_e *then_val = cst_new_int(&arena, &one_tok, 1);
  cst_e *else_val = cst_new_int(&arena, &zero_tok, 0);

  // Wrap in blocks
  cst_e *then_block = cst_new_block(&arena, &obrace_tok, &cbrace_tok);
  ((BLOCK_CSTN *)then_block)->tail = then_val;

  cst_e *else_block = cst_new_block(&arena, &obrace_tok, &cbrace_tok);
  ((BLOCK_CSTN *)else_block)->tail = else_val;

  cst_e *if_expr =
      cst_new_if_expr(&arena, &if_tok, cond, then_block, &else_tok, else_block);

  printf("If expression: ");
  print_expr(if_expr);
  printf("\n");
}

void test_pretty_match(void) {
  printf("\n=== Testing Match Expression ===\n\n");

  Arena arena = arena_init(1LL << 20);
  InternTable intern = intern_init(&arena);
  cst_pretty_init(stdout, &intern);

  Token match_tok = make_token(TK_MATCH, "match", &intern);
  Token obrace_tok = make_token(TK_OBRACE, "{", &intern);
  Token cbrace_tok = make_token(TK_CBRACE, "}", &intern);
  Token n_tok = make_token(TK_IDENT, "n", &intern);
  Token zero_tok = make_token(TK_INT, "0", &intern);
  Token one_tok = make_token(TK_INT, "1", &intern);
  Token wild_tok = make_token(TK_UND, "_", &intern);
  Token fat_arrow_tok = make_token(TK_FATARROW, "=>", &intern);

  cst_e *target = cst_new_ident(&arena, &n_tok);
  cst_e *match_expr =
      cst_new_match_expr(&arena, &match_tok, target, &obrace_tok, &cbrace_tok);
  MATCH_E_CSTN *m = (MATCH_E_CSTN *)match_expr;

  // Arm 1: 0 => 1
  cst_p *pat_zero = cst_new_lit_pat(&arena, cst_new_int(&arena, &zero_tok, 0));
  MatchArm *arm1 = cst_new_match_arm(&arena, pat_zero, &fat_arrow_tok);
  arm1->expr = cst_new_int(&arena, &one_tok, 1);
  vec_push_struct(&arena, m->arms, arm1);

  // Arm 2: _ => 0
  cst_p *pat_wild = cst_new_wild_pat(&arena, &wild_tok);
  MatchArm *arm2 = cst_new_match_arm(&arena, pat_wild, &fat_arrow_tok);
  arm2->expr = cst_new_int(&arena, &zero_tok, 0);
  vec_push_struct(&arena, m->arms, arm2);

  printf("Match expression:\n");
  print_expr(match_expr);
  printf("\n");
}

int main(void) {
  printf("========================================\n");
  printf("       CST Pretty Printer Tests\n");
  printf("========================================\n");

  test_pretty_literals();
  test_pretty_unary();
  test_pretty_binary();
  test_pretty_types();
  test_pretty_struct_lit();
  test_pretty_block();
  test_pretty_if_expr();
  test_pretty_match();

  printf("\nAll pretty printer tests complete!\n");
  return 0;
}

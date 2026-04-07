#include "../include/ast.h"
#include <stdio.h>
#include <string.h>

// Helper to mock tokens for the visualizer
static Token mock_token(const char *lexeme) {
  Token t = {.line = 1, .column = 1};
  strncpy(t.token, lexeme, sizeof(t.token) - 1);
  t.token[sizeof(t.token) - 1] = '\0';
  return t;
}

void run_ast_tests(void) {
  printf("AST Visualizer Test...\n");
  printf("==================================\n\n");

  // 1. Build Expression: 5 + y
  Expr e_five = {.kind = E_INT_LIT, .as.int_val = 5};
  Expr e_y = {.kind = E_IDENT, .as.ident_name = "y"};
  Expr e_plus = {.kind = E_BINARY,
                 .token = mock_token("+"),
                 .as.binary.left = &e_five,
                 .as.binary.right = &e_y};

  // 2. Build Statement: let mut x = 5 + y
  Stmt s_let = {
      .kind = S_LET,
      .as.let_binding = {.name = "x", .is_mut = true, .init = &e_plus}};

  // 3. Build Block logic
  Stmt *block_stmts[] = {&s_let};
  Block b = {.statements = block_stmts, .stmt_count = 1, .expr_final = NULL};

  // 4. Build Function: fn main() { ... }
  Stmt s_main_body = {.kind = S_BLOCK, .as.block = b};

  Decl d_func = {.kind = D_FUNC,
                 .as.function = {.name = "main", .body = &s_main_body}};

  // 5. Build a Struct: struct Player { hp: int }
  Field f_hp = {.name = "hp", .type = NULL};
  Decl d_struct = {
      .kind = D_STRUCT,
      .as._struct = {.name = "Player", .fields = &f_hp, .field_count = 1}};

  // 6. Module Assembly
  Decl *decls[] = {&d_func, &d_struct};
  Module mod = {.declarations = decls, .count = 2};

  // This calls the printer logic in src/ast_print.c
  ast_print_module(&mod);

  printf("\n==================================\n");
  printf("AST Print Test Complete.\n");
}

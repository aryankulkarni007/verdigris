#include "../include/ast.h"
#include <stdio.h>
#include <string.h>

// Helper to mock tokens since the printer uses e->token.token
Token mock_token(TType type, const char *lexeme) {
  Token t = {.ttype = type, .line = 1, .column = 1};
  // Ensure we don't overflow the fixed buffer in Token
  strncpy(t.token, lexeme, sizeof(t.token) - 1);
  t.token[sizeof(t.token) - 1] = '\0';
  return t;
}

int main() {
  printf("Starting Koz AST Visualizer Test...\n\n");

  // --- 1. Expressions: 5 + y ---
  Expr e_five = {.kind = E_INT_LIT, .as.int_val = 5};
  Expr e_y = {.kind = E_IDENT, .as.ident_name = "y"};

  Expr e_plus = {.kind = E_BINARY,
                 .token = mock_token(TOKEN_PLUS, "+"),
                 .as.binary.left = &e_five,
                 .as.binary.right = &e_y};

  // --- 2. Statement: let x = 5 + y ---
  // Note: in ast.h, let_binding is inside the 'as' union of Stmt
  Stmt s_let = {.kind = S_LET,
                .as.let_binding = {.name = "x",
                                   .is_mut = false,
                                   .type_annotation = NULL, // infer
                                   .init = &e_plus}};

  // --- 3. Block: { let x = 5 + y } ---
  Stmt *block_stmts[] = {&s_let};
  Block b = {.statements = block_stmts, .stmt_count = 1, .expr_final = NULL};

  // Correctly using S_BLOCK (stmt) to wrap the block logic
  Stmt s_block = {.kind = S_BLOCK, .as.block = b};

  // --- 4. Declaration: fn main() { ... } ---
  Decl d_func = {.kind = D_FUNC,
                 .as.function = {.name = "main",
                                 .params = NULL,
                                 .param_count = 0,
                                 .return_type = NULL,
                                 .body = &s_block}};

  // --- 5. Struct: Player { hp: int } ---
  // FIX: The type is 'Field', not 'StructField'
  Field fields[] = {
      {.name = "hp", .type = NULL} // NULL = infer/primitive mock
  };

  Decl d_struct = {
      .kind = D_STRUCT,
      .as._struct = {.name = "Player", .fields = fields, .field_count = 1}};

  // --- 6. Module Assembly ---
  Decl *decls[] = {&d_func, &d_struct};
  Module mod = {.declarations = decls, .count = 2};

  // Print the result
  ast_print_module(&mod);

  return 0;
}

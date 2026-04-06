#include "../include/ast.h"
#include "../include/arena.h"

Type *ast_type_primitive(Arena *arena, Token token, TType primitive) {
  Type *node = arena_allocate(arena, sizeof(Type));
  node->kind = T_PRIMITIVE;
  node->token = token;
  node->as.primitive = primitive;
  return node;
}

Type *ast_type_name(Arena *arena, Token token, char *name) {
  Type *node = arena_allocate(arena, sizeof(Type));
  node->kind = T_NAME;
  node->token = token;
  node->as.name = name;
  return node;
}

Type *ast_type_array(Arena *arena, Token token, Type *element) {
  Type *node = arena_allocate(arena, sizeof(Type));
  node->kind = T_ARRAY;
  node->token = token;
  node->as.element = element;
  return node;
}

Expr *ast_expr_int(Arena *arena, Token token, long value) {
  Expr *node = arena_allocate(arena, sizeof(Expr));
  node->kind = E_INT_LIT;
  node->token = token;
  node->as.int_val = value;
  return node;
}

Expr *ast_expr_float(Arena *arena, Token token, double value) {
  Expr *node = arena_allocate(arena, sizeof(Expr));
  node->kind = E_FLOAT_LIT;
  node->token = token;
  node->as.float_val = value;
  return node;
}

Expr *ast_expr_string(Arena *arena, Token token, char *value) {
  Expr *node = arena_allocate(arena, sizeof(Expr));
  node->kind = E_STR_LIT;
  node->token = token;
  node->as.str_val = value;
  return node;
}

Expr *ast_expr_bool(Arena *arena, Token token, bool value) {
  Expr *node = arena_allocate(arena, sizeof(Expr));
  node->kind = E_BOOL_LIT;
  node->token = token;
  node->as.bool_val = value;
  return node;
}

Expr *ast_expr_ident(Arena *arena, Token token, char *name) {
  Expr *node = arena_allocate(arena, sizeof(Expr));
  node->kind = E_IDENT;
  node->token = token;
  node->as.ident_name = name;
  return node;
}

Expr *ast_expr_binary(Arena *arena, Token op_token, Expr *left, Expr *right) {
  Expr *node = arena_allocate(arena, sizeof(Expr));
  node->kind = E_BINARY;
  node->token = op_token;
  node->as.binary.left = left;
  node->as.binary.right = right;
  return node;
}

Expr *ast_expr_unary(Arena *arena, Token op_token, Expr *operand) {
  Expr *node = arena_allocate(arena, sizeof(Expr));
  node->kind = E_UNARY;
  node->token = op_token;
  node->as.unary.operand = operand;
  return node;
}

Stmt *ast_stmt_let(Arena *arena, Token token, char *name, bool is_mut,
                   Type *type_annotation, Expr *init) {
  Stmt *node = arena_allocate(arena, sizeof(Stmt));
  node->kind = S_LET;
  node->token = token;
  node->as.let_binding.name = name;
  node->as.let_binding.is_mut = is_mut;
  node->as.let_binding.type_annotation = type_annotation;
  node->as.let_binding.init = init;
  return node;
}

Stmt *ast_stmt_block(Arena *arena, Token token, Stmt **stmts, size_t stmt_count,
                     Expr *tail) {
  Stmt *node = arena_allocate(arena, sizeof(Stmt));
  node->kind = S_BLOCK;
  node->token = token;
  node->as.block.statements = stmts;
  node->as.block.stmt_count = stmt_count;
  node->as.block.expr_final = tail;
  return node;
}

Decl *ast_decl_struct(Arena *arena, Token token, char *name, Field *fields,
                      size_t field_count) {
  Decl *node = arena_allocate(arena, sizeof(Decl));
  node->kind = D_STRUCT;
  node->token = token;
  node->as._struct.name = name;
  node->as._struct.fields = fields;
  node->as._struct.field_count = field_count;
  return node;
}

Decl *ast_decl_func(Arena *arena, Token token, char *name, Param *params,
                    size_t param_count, Type *return_type, Stmt *body) {
  Decl *node = arena_allocate(arena, sizeof(Decl));
  node->kind = D_FUNC;
  node->token = token;
  node->as.function.name = name;
  node->as.function.params = params;
  node->as.function.param_count = param_count;
  node->as.function.return_type = return_type;
  node->as.function.body = body;
  return node;
}

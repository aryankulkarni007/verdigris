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
  node->as.name = arena_strdup(arena, name);
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
  node->as.str_val = arena_strdup(arena, value);
  return node;
}

Expr *ast_expr_char(Arena *arena, Token token, char value) {
  Expr *node = arena_allocate(arena, sizeof(Expr));
  node->kind = E_CHAR_LIT;
  node->token = token;
  node->as.char_val = value;
  return node;
}

Expr *ast_expr_bool(Arena *arena, Token token, bool value) {
  Expr *node = arena_allocate(arena, sizeof(Expr));
  node->kind = E_BOOL_LIT;
  node->token = token;
  node->as.bool_val = value;
  return node;
}

Expr *ast_expr_none(Arena *arena, Token token) {
  Expr *node = arena_allocate(arena, sizeof(Expr));
  node->kind = E_NONE;
  node->token = token;
  return node;
}

Expr *ast_expr_ident(Arena *arena, Token token, char *name) {
  Expr *node = arena_allocate(arena, sizeof(Expr));
  node->kind = E_IDENT;
  node->token = token;
  node->as.ident_name = arena_strdup(arena, name);
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

Expr *ast_expr_access(Arena *arena, Token op_token, Expr *left, Token ident) {
  Expr *node = arena_allocate(arena, sizeof(Expr));
  node->kind = E_ACCESS;
  node->token = op_token;
  node->as.access.site = left;
  node->as.access.name = arena_strdup(arena, ident.token);
  return node;
}

Expr *ast_expr_index(Arena *arena, Token token, Expr *site, Expr *index) {
  Expr *node = arena_allocate(arena, sizeof(Expr));
  node->kind = E_INDEX;
  node->token = token;
  node->as.index.site = site;
  node->as.index.index = index;
  return node;
}

Expr *ast_expr_call(Arena *arena, Token token, Expr *callee, Expr **args,
                    size_t arg_count) {
  Expr *node = arena_allocate(arena, sizeof(Expr));
  node->kind = E_CALL;
  node->token = token;
  node->as.call.callee = callee;
  node->as.call.args = args;
  node->as.call.arg_count = arg_count;
  return node;
}

Expr *ast_expr_block(Arena *arena, Token token, Stmt **stmts, size_t stmt_count,
                     Expr *tail) {
  Expr *node = arena_allocate(arena, sizeof(Expr));
  node->kind = E_BLOCK;
  node->token = token;
  node->as.block.statements = stmts;
  node->as.block.stmt_count = stmt_count;
  node->as.block.expr_final = tail;
  return node;
}

Expr *ast_expr_if(Arena *arena, Token token, Expr *condition, Stmt *then_expr,
                  Stmt *else_expr) {
  Expr *node = arena_allocate(arena, sizeof(Expr));
  node->kind = E_IF;
  node->token = token;
  node->as.if_expr.condition = condition;
  node->as.if_expr.then_block = then_expr;
  node->as.if_expr.else_block = else_expr;
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

Expr *ast_expr_array(Arena *arena, Token token, Expr **elements, size_t count) {
  Expr *node = arena_allocate(arena, sizeof(Expr));
  node->kind = E_ARRAY;
  node->token = token;
  node->as._array.elements = elements;
  node->as._array.count = count;
  return node;
}

Expr *ast_expr_struct(Arena *arena, Token token, char *struct_name,
                      size_t field_count, char **names, Expr **values) {
  Expr *node = arena_allocate(arena, sizeof(Expr));
  node->kind = E_STRUCT;
  node->token = token;
  node->as._struct.struct_name = arena_strdup(arena, struct_name);
  node->as._struct.field_count = field_count;
  node->as._struct.fields = NULL;

  if (field_count > 0) {
    node->as._struct.fields =
        arena_allocate(arena, field_count * sizeof(*node->as._struct.fields));
    for (size_t i = 0; i < field_count; i++) {
      node->as._struct.fields[i].name = names[i];
      node->as._struct.fields[i].value = values[i];
    }
  }

  return node;
}

Expr *ast_expr_range(Arena *arena, Token token, Expr *start, Expr *end,
                     bool is_inclusive) {
  Expr *node = arena_allocate(arena, sizeof(Expr));
  node->kind = E_RANGE;
  node->token = token;
  node->as.range.start = start;
  node->as.range.end = end;
  node->as.range.is_inclusive = is_inclusive;
  return node;
}

Expr *ast_expr_match(Arena *arena, Token token, Expr *target, MatchArm *arms,
                     size_t arm_count) {
  Expr *node = arena_allocate(arena, sizeof(Expr));
  node->kind = E_MATCH;
  node->token = token;
  node->as._match.target = target;
  node->as._match.arms = arms;
  node->as._match.arm_count = arm_count;
  return node;
}

Stmt *ast_stmt_break(Arena *arena, Token token) {
  Stmt *node = arena_allocate(arena, sizeof(Stmt));
  node->kind = S_BREAK;
  node->token = token;
  return node;
}

Stmt *ast_stmt_continue(Arena *arena, Token token) {
  Stmt *node = arena_allocate(arena, sizeof(Stmt));
  node->kind = S_CONTINUE;
  node->token = token;
  return node;
}

Stmt *ast_stmt_return(Arena *arena, Token token, Expr *value) {
  Stmt *node = arena_allocate(arena, sizeof(Stmt));
  node->kind = S_RETURN;
  node->token = token;
  node->as.return_value = value; // NULL if no value
  return node;
}

Stmt *ast_stmt_expr(Arena *arena, Token token, Expr *expression) {
  Stmt *node = arena_allocate(arena, sizeof(Stmt));
  node->kind = S_EXPR;
  node->token = token;
  node->as.expression = expression;
  return node;
}

Stmt *ast_stmt_while(Arena *arena, Token token, Expr *condition, Stmt *block) {
  Stmt *node = arena_allocate(arena, sizeof(Stmt));
  node->kind = S_WHILE;
  node->token = token;
  node->as._while.condition = condition;
  node->as._while.body = block;
  return node;
}

Stmt *ast_stmt_loop(Arena *arena, Token token, Stmt *body) {
  Stmt *node = arena_allocate(arena, sizeof(Stmt));
  node->kind = S_LOOP;
  node->token = token;
  node->as._loop.body = body;
  return node;
}

Stmt *ast_stmt_for(Arena *arena, Token token, char *iterator, Expr *iterable,
                   Stmt *body) {
  Stmt *node = arena_allocate(arena, sizeof(Stmt));
  node->kind = S_FOR;
  node->token = token;
  node->as._for.iterator = iterator;
  node->as._for.iterable = iterable;
  node->as._for.body = body;
  return node;
}

Stmt *ast_stmt_assign(Arena *arena, Token token, Expr *target, Expr *value) {
  Stmt *node = arena_allocate(arena, sizeof(Stmt));
  node->kind = S_ASSIGN;
  node->token = token;
  node->as.assign.target = target;
  node->as.assign.value = value;
  return node;
}

Stmt *ast_stmt_op_assign(Arena *arena, Token op_token, Expr *target,
                         Expr *value) {
  Stmt *node = arena_allocate(arena, sizeof(Stmt));
  node->kind = S_OP_ASSIGN;
  node->token = op_token;
  node->as.op_assign.target = target; // could have been a massive bug
  node->as.op_assign.value = value;   //
  return node;
}

Decl *ast_decl_impl(Arena *arena, Token token, Type *target_type,
                    Decl **methods, size_t method_count) {
  Decl *node = arena_allocate(arena, sizeof(Decl));
  node->kind = D_IMPL;
  node->token = token;
  node->as.impl.target_type = target_type;
  node->as.impl.methods = methods;
  node->as.impl.method_count = method_count;
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
  node->as.function.name = arena_strdup(arena, name);
  node->as.function.params = params;
  node->as.function.param_count = param_count;
  node->as.function.return_type = return_type;
  node->as.function.body = body;
  return node;
}

Decl *ast_decl_enum(Arena *arena, Token token, char *name, Variant *variants,
                    size_t variant_count) {
  Decl *node = arena_allocate(arena, sizeof(Decl));
  node->kind = D_ENUM;
  node->token = token;
  node->as._enum.name = arena_strdup(arena, name);
  node->as._enum.variants = variants;
  node->as._enum.variant_count = variant_count;
  return node;
}

Decl *ast_decl_extern(Arena *arena, Token token, Decl **decls,
                      size_t decl_count) {
  Decl *node = arena_allocate(arena, sizeof(Decl));
  node->kind = D_EXTERN;
  node->token = token;
  node->as.extern_block.decls = decls;
  node->as.extern_block.count = decl_count;
  return node;
}

Decl *ast_decl_type_alias(Arena *arena, Token token, char *alias_name,
                          Type *target) {
  Decl *node = arena_allocate(arena, sizeof(Decl));
  node->kind = D_TYPE;
  node->token = token;
  node->as.type_alias.alias_name = arena_strdup(arena, alias_name);
  node->as.type_alias.target = target;
  return node;
}

Pattern *ast_pat_wildcard(Arena *arena, Token token) {
  Pattern *node = arena_allocate(arena, sizeof(Pattern));
  node->kind = P_WILDCARD;
  node->token = token;
  return node;
}

Pattern *ast_pat_ident(Arena *arena, Token token, char *name) {
  Pattern *node = arena_allocate(arena, sizeof(Pattern));
  node->kind = P_IDENT;
  node->token = token;
  node->as.name = arena_strdup(arena, name);
  return node;
}

Pattern *ast_pat_literal(Arena *arena, Token token, Expr *literal) {
  Pattern *node = arena_allocate(arena, sizeof(Pattern));
  node->kind = P_LITERAL;
  node->token = token;
  node->as.literal = literal;
  return node;
}
Pattern *ast_pat_enum(Arena *arena, Token token, char *variant,
                      Pattern *inner) {
  Pattern *node = arena_allocate(arena, sizeof(Pattern));
  node->kind = P_ENUM;
  node->token = token;
  node->as._enum.variant = arena_strdup(arena, variant);
  node->as._enum.inner = inner;
  return node;
}

Pattern *ast_pat_struct(Arena *arena, Token token, char *name, char **fields,
                        size_t field_count) {
  Pattern *node = arena_allocate(arena, sizeof(Pattern));
  node->kind = P_STRUCT;
  node->token = token;
  node->as._struct.name = arena_strdup(arena, name);
  node->as._struct.field_count = field_count;
  node->as._struct.fields = NULL;

  if (field_count > 0) {
    node->as._struct.fields =
        arena_allocate(arena, field_count * sizeof(char *));
    for (size_t i = 0; i < field_count; i++) {
      node->as._struct.fields[i] = arena_strdup(arena, fields[i]);
    }
  }

  return node;
}

Pattern *ast_pat_range(Arena *arena, Token token, Expr *start, Expr *end,
                       bool is_inclusive) {
  Pattern *node = arena_allocate(arena, sizeof(Pattern));
  node->kind = P_RANGE;
  node->token = token;
  node->as.range.start = start;
  node->as.range.end = end;
  node->as.range.is_inclusive = is_inclusive;
  return node;
}

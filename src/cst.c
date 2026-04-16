#include "../include/cst.h"
#include "../include/cst_builder.h"
#include "../include/span.h"
#include "../include/vector.h"

/* literal constructors  */

cst_e *cst_new_ident(Arena *a, Token *t) {
  IDENT_L_CSTN *node = arena_alloc(a, sizeof(IDENT_L_CSTN));
  node->kind = CST_IDENT;
  node->span = span_from_token(t);
  node->token = t;
  node->value = t->id;
  return (cst_e *)node;
}

cst_e *cst_new_int(Arena *a, Token *t, int64_t value) {
  INT_L_CSTN *node = arena_alloc(a, sizeof(INT_L_CSTN));
  node->kind = CST_INT_LIT;
  node->span = span_from_token(t);
  node->token = t;
  node->value = value;
  return (cst_e *)node;
}

cst_e *cst_new_float(Arena *a, Token *t, double value) {
  FLT_L_CSTN *node = arena_alloc(a, sizeof(FLT_L_CSTN));
  node->kind = CST_FLOAT_LIT;
  node->span = span_from_token(t);
  node->token = t;
  node->value = value;
  return (cst_e *)node;
}

cst_e *cst_new_str(Arena *a, Token *t) {
  STR_L_CSTN *node = arena_alloc(a, sizeof(STR_L_CSTN));
  node->kind = CST_STRING_LIT;
  node->span = span_from_token(t);
  node->token = t;
  node->value = t->id;
  return (cst_e *)node;
}

cst_e *cst_new_bool(Arena *a, Token *t, bool value) {
  BOOL_L_CSTN *node = arena_alloc(a, sizeof(BOOL_L_CSTN));
  node->kind = CST_BOOL_LIT;
  node->span = span_from_token(t);
  node->token = t;
  node->value = value;
  return (cst_e *)node;
}

cst_e *cst_new_null(Arena *a, Token *t) {
  NULL_L_CSTN *node = arena_alloc(a, sizeof(NULL_L_CSTN));
  node->kind = CST_NULL_LIT;
  node->span = span_from_token(t);
  node->token = t;
  return (cst_e *)node;
}

/* type constructors */

cst_t *cst_new_prim_type(Arena *a, Token *t) {
  PRIM_T_CSTN *node = arena_alloc(a, sizeof(PRIM_T_CSTN));
  node->kind = CST_TYPE_PRIMITIVE;
  node->span = span_from_token(t);
  node->token = t;
  return (cst_t *)node;
}

cst_t *cst_new_named_type(Arena *a, Token *name) {
  NAMED_T_CSTN *node = arena_alloc(a, sizeof(NAMED_T_CSTN));
  node->kind = CST_TYPE_NAMED;
  node->span = span_from_token(name);
  node->name = name;
  vec_init(a, node->generic_args, cst_t *, 0);
  return (cst_t *)node;
}

cst_t *cst_new_tuple_type(Arena *a, Token *oparen, Token *cparen) {

  TUPLE_T_CSTN *node = arena_alloc(a, sizeof(TUPLE_T_CSTN));
  node->kind = CST_TYPE_TUPLE;
  node->span = span_merge(span_from_token(oparen), span_from_token(cparen));
  vec_init(a, node->types, cst_t *, 0);
  return (cst_t *)node;
}

cst_t *cst_new_array_type(Arena *a, Token *obrack, cst_t *elem_type,
                          Token *size, Token *cbrack) {

  ARRAY_T_CSTN *node = arena_alloc(a, sizeof(ARRAY_T_CSTN));
  node->kind = CST_TYPE_ARRAY;
  node->span = span_merge(span_from_token(obrack), span_from_token(cbrack));
  node->lmt_t = elem_type;
  node->size = size;
  return (cst_t *)node;
}

cst_t *cst_new_slice_type(Arena *a, Token *amp, cst_t *elem_type,
                          Token *cbrack) {
  SLICE_T_CSTN *node = arena_alloc(a, sizeof(SLICE_T_CSTN));
  node->kind = CST_TYPE_SLICE;
  node->span = span_merge(span_from_token(amp), span_from_token(cbrack));
  node->lmt_t = elem_type;
  return (cst_t *)node;
}

/// TODO: mutate the span after parsing because the number lmts in the union
/// cannot be known at constructor time, only at parser runtime
cst_t *cst_new_union_type(Arena *a, Token *pipe) {
  UNION_T_CSTN *node = arena_alloc(a, sizeof(UNION_T_CSTN));
  node->kind = CST_TYPE_UNION;
  node->span = span_from_token(pipe); // default to the just the first pipe
  vec_init(a, node->types, cst_t *, 0);
  return (cst_t *)node;
}

cst_t *cst_new_opt_type(Arena *a, Token *question, cst_t *inner) {
  OPT_T_CSTN *node = arena_alloc(a, sizeof(OPT_T_CSTN));
  node->kind = CST_TYPE_OPTIONAL;
  node->span = span_merge(inner->span, span_from_token(question));
  node->inner = inner;
  return (cst_t *)node;
}

/* expr constructors */

cst_e *cst_new_unary(Arena *a, Token *op, cst_e *expr) {
  UNARY_E_CSTN *node = arena_alloc(a, sizeof(UNARY_E_CSTN));
  node->kind = CST_UNARY_EXPR;
  node->span = span_merge(expr->span, span_from_token(op));
  node->op = op;
  node->expr = expr;
  return (cst_e *)node;
}

cst_e *cst_new_binary(Arena *a, cst_e *left, Token *op, cst_e *right) {
  BINARY_E_CSTN *node = arena_alloc(a, sizeof(BINARY_E_CSTN));
  node->kind = CST_BINARY_EXPR;
  node->span = span_merge(left->span, right->span);
  node->left = left;
  node->op = op;
  node->right = right;
  return (cst_e *)node;
}

cst_e *cst_new_field(Arena *a, cst_e *target, Token *dot, Token *field) {
  FIELD_E_CSTN *node = arena_alloc(a, sizeof(FIELD_E_CSTN));
  node->kind = CST_FIELD_ACCESS;
  node->span = span_merge(target->span, span_from_token(field));
  node->target = target;
  node->dot = dot;
  node->field = field;
  return (cst_e *)node;
}

cst_e *cst_new_index(Arena *a, cst_e *target, Token *obrack, cst_e *index,
                     Token *cbrack) {
  INDEX_E_CSTN *node = arena_alloc(a, sizeof(INDEX_E_CSTN));
  node->kind = CST_INDEX_EXPR;
  node->span = span_merge(target->span, span_from_token(cbrack));
  node->target = target;
  node->obrack = obrack;
  node->index = index;
  node->cbrack = cbrack;
  return (cst_e *)node;
}

cst_e *cst_new_call(Arena *a, cst_e *callee, Token *oparen, Token *cparen) {
  CALL_E_CSTN *node = arena_alloc(a, sizeof(CALL_E_CSTN));
  node->kind = CST_CALL_EXPR;
  node->span = span_merge(callee->span, span_from_token(cparen));
  node->callee = callee;
  node->oparen = oparen;
  vec_init(a, node->args, cst_e *, 0);
  node->cparen = cparen;
  return (cst_e *)node;
}

cst_e *cst_new_range(Arena *a, cst_e *start, Token *op, cst_e *end) {
  RANGE_E_CSTN *node = arena_alloc(a, sizeof(RANGE_E_CSTN));
  node->kind = CST_RANGE_EXPR;
  node->span = span_merge(start->span, end->span);
  node->start = start;
  node->op = op;
  node->end = end;
  return (cst_e *)end;
}

cst_e *cst_new_cast(Arena *a, cst_e *target, Token *as, cst_t *type) {
  CAST_E_CSTN *node = arena_alloc(a, sizeof(CAST_E_CSTN));
  node->kind = CST_CAST_EXPR;
  node->span = span_merge(target->span, type->span);
  node->target = target;
  node->as = as;
  node->type = type;
  return (cst_e *)node;
}

cst_e *cst_new_error_prop(Arena *a, cst_e *expr, Token *bang) {
  EPROP_E_CSTN *node = arena_alloc(a, sizeof(EPROP_E_CSTN));
  node->kind = CST_ERROR_PROP;
  node->span = span_merge(expr->span, span_from_token(bang));
  node->expr = expr;
  node->bang = bang;
  return (cst_e *)node;
}

/// TODO: mutate span to include handler
cst_e *cst_new_catch(Arena *a, cst_e *expr, Token *catch, Token *opipe,
                     Token *err_var, Token *cpipe) {
  CATCH_E_CSTN *node = arena_alloc(a, sizeof(CATCH_E_CSTN));
  node->kind = CST_CATCH_EXPR;
  node->span =
      span_merge(expr->span, span_from_token(cpipe)); // update after parsing
  node->expr = expr;
  node->catch = catch;
  node->opipe = opipe;
  node->err_var = err_var;
  node->cpipe = cpipe;
  node->handler = NULL; // TODO: update after parsing
  return (cst_e *)node;
}

/// TODO: same situation as union. compute span at parser runtime
cst_e *cst_new_pipeline(Arena *a, Token *pipe) {
  PLINE_E_CSTN *node = arena_alloc(a, sizeof(PLINE_E_CSTN));
  node->kind = CST_PIPELINE_EXPR;
  node->span = span_from_token(pipe);
  vec_init(a, node->stages, cst_e *, 0);
  return (cst_e *)node;
}

cst_e *cst_new_struct_lit(Arena *a, cst_t *type, Token *obrace, Token *cbrace) {
  STRUCT_LE_CSTN *node = arena_alloc(a, sizeof(STRUCT_LE_CSTN));
  node->kind = CST_STRUCT_LIT;
  // not including type cause it could be inferned i.e. NULL
  node->span = span_merge(span_from_token(obrace), span_from_token(cbrace));
  node->type = type;
  node->obrace = obrace;
  vec_init(a, node->fields, FieldInit *, 0);
  node->cbrace = cbrace;
  return (cst_e *)node;
}

cst_e *cst_new_array_lit(Arena *a, Token *obrack, Token *cbrack) {
  ARRAY_LE_CSTN *node = arena_alloc(a, sizeof(ARRAY_LE_CSTN));
  node->kind = CST_ARRAY_LIT;
  node->span = span_merge(span_from_token(obrack), span_from_token(cbrack));
  node->obrack = obrack;
  vec_init(a, node->lmnts, cst_e *, 0);
  node->cbrack = cbrack;
  return (cst_e *)node;
}

cst_e *cst_new_tuple_lit(Arena *a, Token *oparen, Token *cparen) {
  TUPLE_LE_CSTN *node = arena_alloc(a, sizeof(TUPLE_LE_CSTN));
  node->kind = CST_TUPLE_LIT;
  node->span = span_merge(span_from_token(oparen), span_from_token(cparen));
  node->oparen = oparen;
  vec_init(a, node->lmnts, cst_e *, 0);
  node->cparen = cparen;
  return (cst_e *)node;
}

cst_e *cst_new_block(Arena *a, Token *obrace, Token *cbrace) {
  BLOCK_CSTN *node = arena_alloc(a, sizeof(BLOCK_CSTN));
  node->kind = CST_BLOCK;
  node->span = span_merge(span_from_token(obrace), span_from_token(cbrace));
  node->obrace = obrace;
  vec_init(a, node->stmts, cst_s *, 0);
  node->tail = NULL;
  node->cbrace = cbrace;
  return (cst_e *)node;
}

/// TODO: update source span to match when parsing
cst_e *cst_new_if_expr(Arena *a, Token *_if, cst_e *cond, cst_e *then_branch,
                       Token *_else, cst_e *else_branch) {
  IF_E_CSTN *node = arena_alloc(a, sizeof(IF_E_CSTN));
  node->kind = CST_IF_EXPR;
  node->span = span_merge(span_from_token(_if),
                          else_branch ? else_branch->span : then_branch->span);
  node->_if = _if;
  node->cond = cond;
  node->then = then_branch;
  node->_else = _else;
  node->else_then = else_branch;
  return (cst_e *)node;
}

cst_e *cst_new_match_expr(Arena *a, Token *match, cst_e *target, Token *obrace,
                          Token *cbrace) {
  MATCH_E_CSTN *node = arena_alloc(a, sizeof(MATCH_E_CSTN));
  node->kind = CST_MATCH_EXPR;
  node->span = span_merge(span_from_token(match), span_from_token(cbrace));
  node->match = match;
  node->target = target;
  node->obrace = obrace;
  vec_init(a, node->arms, MatchArm *, 0);
  node->cbrace = cbrace;
  return (cst_e *)node;
}

/* pattern constructors */

cst_p *cst_new_wild_pat(Arena *a, Token *underscore) {
  WILD_P_CSTN *node = arena_alloc(a, sizeof(WILD_P_CSTN));
  node->kind = CST_PATTERN_WILD;
  node->span = span_from_token(underscore);
  node->underscore = underscore;
  return (cst_p *)node;
}

cst_p *cst_new_ident_pat(Arena *a, Token *name) {
  IDENT_P_CSTN *node = arena_alloc(a, sizeof(IDENT_P_CSTN));
  node->kind = CST_PATTERN_IDENT;
  node->span = span_from_token(name);
  node->name = name;
  node->id = name->id;
  return (cst_p *)node;
}

cst_p *cst_new_lit_pat(Arena *a, cst_e *literal) {
  LIT_P_CSTN *node = arena_alloc(a, sizeof(LIT_P_CSTN));
  node->kind = CST_PATTERN_LIT;
  node->span = literal->span;
  node->literal = literal;
  return (cst_p *)node;
}

/// mutate span once we have parsed enum args
cst_p *cst_new_enum_pat(Arena *a, Token *variant) {
  ENUM_P_CSTN *node = arena_alloc(a, sizeof(ENUM_P_CSTN));
  node->kind = CST_PATTERN_ENUM;
  node->span = span_from_token(variant);
  node->variant = variant;
  vec_init(a, node->args, cst_p *, 0);
  return (cst_p *)node;
}

cst_p *cst_new_struct_pat(Arena *a, Token *type_name, Token *obrace,
                          Token *cbrace) {
  STRUCT_P_CSTN *node = arena_alloc(a, sizeof(STRUCT_P_CSTN));
  node->kind = CST_PATTERN_STRUCT;
  node->span = span_merge(span_from_token(type_name), span_from_token(cbrace));
  node->type_name = type_name;
  node->obrace = obrace;
  vec_init(a, node->fields, FIELD_P *, 0);
  node->cbrace = cbrace;
  return (cst_p *)node;
}

cst_p *cst_new_tuple_pat(Arena *a, Token *oparen, Token *cparen) {
  TUPLE_P_CSTN *node = arena_alloc(a, sizeof(TUPLE_P_CSTN));
  node->kind = CST_PATTERN_TUPLE;
  node->span = span_merge(span_from_token(oparen), span_from_token(cparen));
  node->oparen = oparen;
  vec_init(a, node->lmnts, cst_p *, 0);
  node->cparen = cparen;
  return (cst_p *)node;
}

cst_p *cst_new_guard_pat(Arena *a, cst_p *inner, Token *_if, cst_e *guard) {

  GUARD_P_CSTN *node = arena_alloc(a, sizeof(GUARD_P_CSTN));
  node->kind = CST_PATTERN_GUARD;
  node->span = span_merge(inner->span, guard->span);
  node->inner = inner;
  node->_if = _if;
  node->guard = guard;
  return (cst_p *)node;
}

/* statement constructors */

cst_s *cst_new_expr_stmt(Arena *a, cst_e *expr, Token *semi) {
  EXPR_STMT_S_CSTN *node = arena_alloc(a, sizeof(EXPR_STMT_S_CSTN));
  node->kind = CST_EXPR_STMT;
  node->span = span_merge(expr->span, span_from_token(semi));
  node->expr = expr;
  node->semi = semi;
  return (cst_s *)node;
}

cst_s *cst_new_return_stmt(Arena *a, Token *ret, cst_e *expr, Token *semi) {
  RETURN_S_CSTN *node = arena_alloc(a, sizeof(RETURN_S_CSTN));
  node->kind = CST_RETURN_STMT;
  node->span = span_merge(span_from_token(ret), span_from_token(semi));
  if (expr) {
    node->span = span_merge(node->span, expr->span);
  }
  node->ret = ret;
  node->expr = expr;
  node->semi = semi;
  return (cst_s *)node;
}

cst_s *cst_new_break_stmt(Arena *a, Token *brk, Token *semi) {
  BREAK_S_CSTN *node = arena_alloc(a, sizeof(BREAK_S_CSTN));
  node->kind = CST_BREAK_STMT;
  node->span = span_merge(span_from_token(brk), span_from_token(semi));
  node->brk = brk;
  node->semi = semi;
  return (cst_s *)node;
}

cst_s *cst_new_continue_stmt(Arena *a, Token *cont, Token *semi) {
  CONT_S_CSTN *node = arena_alloc(a, sizeof(CONT_S_CSTN));
  node->kind = CST_CONTINUE_STMT;
  node->span = span_merge(span_from_token(cont), span_from_token(semi));
  node->cont = cont;
  node->semi = semi;
  return (cst_s *)node;
}

cst_s *cst_new_defer_stmt(Arena *a, Token *defer, cst_e *expr, Token *semi) {
  DEFER_S_CSTN *node = arena_alloc(a, sizeof(DEFER_S_CSTN));
  node->kind = CST_DEFER_STMT;
  node->span = span_merge(span_from_token(defer), span_from_token(semi));
  node->defer = defer;
  node->expr = expr;
  node->semi = semi;
  return (cst_s *)node;
}

/// TODO: update span after parsing branches
cst_s *cst_new_if_stmt(Arena *a, Token *_if, cst_e *cond, Token *_else) {
  IF_S_CSTN *node = arena_alloc(a, sizeof(IF_S_CSTN));
  node->kind = CST_IF_STMT;
  node->span = span_from_token(_if);
  node->_if = _if;
  node->cond = cond;
  node->then = NULL; // set by parser
  node->_else = _else;
  node->else_then = NULL; // set by parser
  return (cst_s *)node;
}

/// TODO: update span after parsing body
cst_s *cst_new_while_stmt(Arena *a, Token *whl, cst_e *cond) {
  WHILE_S_CSTN *node = arena_alloc(a, sizeof(WHILE_S_CSTN));
  node->kind = CST_WHILE_STMT;
  node->span = span_from_token(whl);
  node->_while = whl;
  node->cond = cond;
  node->body = NULL; // set by parser
  return (cst_s *)node;
}

/// TODO: update span after parsing body
cst_s *cst_new_loop_stmt(Arena *a, Token *loop) {
  LOOP_S_CSTN *node = arena_alloc(a, sizeof(LOOP_S_CSTN));
  node->kind = CST_LOOP_STMT;
  node->span = span_from_token(loop);
  node->loop = loop;
  node->body = NULL; // set by parser
  return (cst_s *)node;
}

/// TODO: update span after parsing pattern, iterable, and body
cst_s *cst_new_for_stmt(Arena *a, Token *_for, Token *_in) {
  FOR_S_CSTN *node = arena_alloc(a, sizeof(FOR_S_CSTN));
  node->kind = CST_FOR_STMT;
  node->span = span_from_token(_for);
  node->_for = _for;
  node->pattern = NULL; // set by parser
  node->_in = _in;
  node->iter = NULL; // set by parser
  node->body = NULL; // set by parser
  return (cst_s *)node;
}

cst_s *cst_new_match_stmt(Arena *a, Token *match, cst_e *target, Token *obrace,
                          Token *cbrace) {
  MATCH_S_CSTN *node = arena_alloc(a, sizeof(MATCH_S_CSTN));
  node->kind = CST_MATCH_STMT;
  node->span = span_merge(span_from_token(match), span_from_token(cbrace));
  node->match = match;
  node->target = target;
  node->obrace = obrace;
  vec_init(a, node->arms, MatchArm *, 0);
  node->cbrace = cbrace;
  return (cst_s *)node;
}

cst_s *cst_new_block_stmt(Arena *a, Token *obrace, Token *cbrace) {
  BLOCK_CSTN *node = arena_alloc(a, sizeof(BLOCK_CSTN));
  node->kind = CST_BLOCK;
  node->span = span_merge(span_from_token(obrace), span_from_token(cbrace));
  node->obrace = obrace;
  vec_init(a, node->stmts, cst_s *, 0);
  node->tail = NULL; // set by parser
  node->cbrace = cbrace;
  return (cst_s *)node;
}

/* declaration constructors */

/// TODO: update span after parsing pattern and init
cst_d *cst_new_var_decl(Arena *a, Token *let_or_mut, Token *eq, Token *semi) {
  VAR_D_CSTN *node = arena_alloc(a, sizeof(VAR_D_CSTN));
  node->kind = CST_VAR_DECL;
  node->span = span_from_token(let_or_mut);
  node->let_or_mut = let_or_mut;
  node->pattern = NULL; // set by parser
  node->eq = eq;
  node->init = NULL; // set by parser
  node->semi = semi;
  return (cst_d *)node;
}

/// TODO: update span after parsing params, return type, and body
cst_d *cst_new_func_decl(Arena *a, Token *name, Token *oparen, Token *cparen,
                         Token *arrow) {
  FUNC_D_CSTN *node = arena_alloc(a, sizeof(FUNC_D_CSTN));
  node->kind = CST_FUNCTION_DECL;
  node->span = span_from_token(name);
  node->name = name;
  node->oparen = oparen;
  vec_init(a, node->params, PARAM_D_CSTN *, 0);
  node->cparen = cparen;
  node->arrow = arrow;
  node->ret_type = NULL; // set by parser
  node->body = NULL;     // set by parser (NULL for forward decl)
  return (cst_d *)node;
}

cst_d *cst_new_struct_decl(Arena *a, Token *name, Token *obrace,
                           Token *cbrace) {
  STRUCT_D_CSTN *node = arena_alloc(a, sizeof(STRUCT_D_CSTN));
  node->kind = CST_STRUCT_DECL;
  node->span = span_merge(span_from_token(name), span_from_token(cbrace));
  node->name = name;
  vec_init(a, node->generic_params, cst_t *, 0);
  node->obrace = obrace;
  vec_init(a, node->fields, STRUCT_FIELD *, 0);
  vec_init(a, node->methods, FUNC_D_CSTN *, 0);
  node->cbrace = cbrace;
  return (cst_d *)node;
}

cst_d *cst_new_enum_decl(Arena *a, Token *name, Token *obrace, Token *cbrace) {
  ENUM_D_CSTN *node = arena_alloc(a, sizeof(ENUM_D_CSTN));
  node->kind = CST_ENUM_DECL;
  node->span = span_merge(span_from_token(name), span_from_token(cbrace));
  node->name = name;
  vec_init(a, node->generic_params, cst_t *, 0);
  node->obrace = obrace;
  vec_init(a, node->variants, ENUM_VARIANT *, 0);
  node->cbrace = cbrace;
  return (cst_d *)node;
}

cst_d *cst_new_interface_decl(Arena *a, Token *name, Token *obrace,
                              Token *cbrace) {
  INTERFACE_D_CSTN *node = arena_alloc(a, sizeof(INTERFACE_D_CSTN));
  node->kind = CST_INTERFACE_DECL;
  node->span = span_merge(span_from_token(name), span_from_token(cbrace));
  node->name = name;
  vec_init(a, node->generic_params, cst_t *, 0);
  node->obrace = obrace;
  vec_init(a, node->methods, FUNC_D_CSTN *, 0);
  node->cbrace = cbrace;
  return (cst_d *)node;
}

/// TODO: update span after parsing function definition
cst_d *cst_new_impl_decl(Arena *a, Token *type_name, Token *ccolon,
                         Token *method_name) {
  IMPL_D_CSTN *node = arena_alloc(a, sizeof(IMPL_D_CSTN));
  node->kind = CST_IMPL_DECL;
  node->span = span_from_token(type_name);
  node->type_name = type_name;
  node->ccolon = ccolon;
  node->method_name = method_name;
  node->func = NULL; // set by parser
  return (cst_d *)node;
}

cst_d *cst_new_import_decl(Arena *a, Token *import, Token *path, Token *semi) {
  IMPORT_D_CSTN *node = arena_alloc(a, sizeof(IMPORT_D_CSTN));
  node->kind = CST_IMPORT_DECL;
  node->span = span_merge(span_from_token(import), span_from_token(semi));
  node->import = import;
  node->path = path;
  node->as = NULL;    // set by parser if present
  node->alias = NULL; // set by parser if present
  node->semi = semi;
  return (cst_d *)node;
}

cst_d *cst_new_module(Arena *a) {
  MODULE_D_CSTN *node = arena_alloc(a, sizeof(MODULE_D_CSTN));
  node->kind = CST_MODULE;
  node->span = (SourceSpan){0}; // computed after parsing all decls
  vec_init(a, node->decls, cst_d *, 0);
  return (cst_d *)node;
}

/* parameter constructor */

PARAM_D_CSTN *cst_new_param(Arena *a, Token *mut, Token *name, Token *colon,
                            cst_t *type) {
  PARAM_D_CSTN *node = arena_alloc(a, sizeof(PARAM_D_CSTN));
  node->kind = CST_PARAM;
  node->span = span_merge(span_from_token(name), type->span);
  if (mut) {
    node->span = span_merge(span_from_token(mut), node->span);
  }
  node->mut = mut;
  node->name = name;
  node->colon = colon;
  node->type = type;
  return node;
}

/* match arm constructor */

MatchArm *cst_new_match_arm(Arena *a, cst_p *pattern, Token *fat_arrow) {
  MatchArm *arm = arena_alloc(a, sizeof(MatchArm));
  arm->span = span_merge(pattern->span, span_from_token(fat_arrow));
  arm->pattern = pattern;
  arm->fat_arrow = fat_arrow;
  arm->expr = NULL;  // set by parser (for match expr)
  arm->block = NULL; // set by parser (for match stmt)
  return arm;
}

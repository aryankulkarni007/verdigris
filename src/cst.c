#include "../include/cst.h"
#include "../include/cst_builder.h"
#include "../include/span.h"
#include "../include/vector.h"

/* literals */
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

/* types */
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

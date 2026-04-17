#include "../include/pretty.h"
#include "../include/cst.h"
#include "../include/intern.h"
#include <stdio.h>

static int indent_level = 0;
static FILE *out = NULL;

#define INDENT "  "
#define PRINT_INDENT()                                                         \
  for (int i = 0; i < indent_level; i++)                                       \
  fprintf(out, INDENT)

static InternTable *g_intern = NULL;

void cst_pretty_init(FILE *output, InternTable *intern) {
  out = output;
  g_intern = intern;
  indent_level = 0;
}

static void push_indent(void) { indent_level++; }
static void pop_indent(void) { indent_level--; }

/* forward declarations */
void print_type(cst_t *type);
void print_literal(cst_e *lit);
void print_expr(cst_e *expr);
void print_stmt(cst_s *stmt);
void print_pat(cst_p *pat);
void print_decl(cst_d *decl);

/* =========================================================================
 * types
 * ========================================================================= */

void print_type(cst_t *type) {
  if (!type) {
    fprintf(out, "<null>");
    return;
  }

  switch (type->kind) {

  case CST_TYPE_PRIMITIVE: {
    PRIM_T_CSTN *t = (PRIM_T_CSTN *)type;
    fprintf(out, "%s", intern_lookup(g_intern, t->token->id));
    break;
  }

  case CST_TYPE_NAMED: {
    NAMED_T_CSTN *t = (NAMED_T_CSTN *)type;
    fprintf(out, "%s", intern_lookup(g_intern, t->name->id));
    if (t->generic_args.len > 0) {
      fprintf(out, "[");
      bool first = true;
      vec_for_each_struct(t->generic_args, arg) {
        if (!first)
          fprintf(out, ", ");
        first = false;
        print_type(*arg);
      }
      fprintf(out, "]");
    }
    break;
  }

  case CST_TYPE_TUPLE: {
    TUPLE_T_CSTN *t = (TUPLE_T_CSTN *)type;
    fprintf(out, "(");
    bool first = true;
    vec_for_each_struct(t->types, elem) {
      if (!first)
        fprintf(out, ", ");
      first = false;
      print_type(*elem);
    }
    fprintf(out, ")");
    break;
  }

  case CST_TYPE_ARRAY: {
    ARRAY_T_CSTN *t = (ARRAY_T_CSTN *)type;
    fprintf(out, "[");
    print_type(t->lmt_t);
    fprintf(out, "; %s]", intern_lookup(g_intern, t->size->id));
    break;
  }

  case CST_TYPE_SLICE: {
    SLICE_T_CSTN *t = (SLICE_T_CSTN *)type;
    fprintf(out, "&[");
    print_type(t->lmt_t);
    fprintf(out, "]");
    break;
  }

  case CST_TYPE_UNION: {
    UNION_T_CSTN *t = (UNION_T_CSTN *)type;
    bool first = true;
    vec_for_each(t->types.data, t->types.len, elem) {
      if (!first)
        fprintf(out, " | ");
      first = false;
      print_type(*elem);
    }
    break;
  }

  case CST_TYPE_OPTIONAL: {
    OPT_T_CSTN *t = (OPT_T_CSTN *)type;
    print_type(t->inner);
    fprintf(out, "?");
    break;
  }

  default:
    fprintf(out, "<unknown-type>");
  }
}

/* =========================================================================
 * literals
 * ========================================================================= */

void print_literal(cst_e *lit) {
  switch (lit->kind) {
  case CST_INT_LIT:
    fprintf(out, "%lld", ((INT_L_CSTN *)lit)->value);
    break;
  case CST_FLOAT_LIT:
    fprintf(out, "%g", ((FLT_L_CSTN *)lit)->value);
    break;
  case CST_STRING_LIT:
    fprintf(out, "\"%s\"", intern_lookup(g_intern, ((STR_L_CSTN *)lit)->value));
    break;
  case CST_BOOL_LIT:
    fprintf(out, "%s", ((BOOL_L_CSTN *)lit)->value ? "true" : "false");
    break;
  case CST_NULL_LIT:
    fprintf(out, "null");
    break;
  case CST_IDENT:
    fprintf(out, "%s", intern_lookup(g_intern, ((IDENT_L_CSTN *)lit)->value));
    break;
  default:
    print_expr(lit);
  }
}

/* =========================================================================
 * expressions
 * ========================================================================= */

void print_expr(cst_e *expr) {
  if (!expr) {
    fprintf(out, "<null>");
    return;
  }

  switch (expr->kind) {

  case CST_INT_LIT:
  case CST_FLOAT_LIT:
  case CST_STRING_LIT:
  case CST_BOOL_LIT:
  case CST_NULL_LIT:
  case CST_IDENT:
    print_literal(expr);
    break;

  case CST_UNARY_EXPR: {
    UNARY_E_CSTN *e = (UNARY_E_CSTN *)expr;
    const char *op_str = tk_name(e->op->type);
    fprintf(out, "(%s ", op_str);
    print_expr(e->expr);
    fprintf(out, ")");
    break;
  }

  case CST_BINARY_EXPR: {
    BINARY_E_CSTN *e = (BINARY_E_CSTN *)expr;
    fprintf(out, "(");
    print_expr(e->left);
    fprintf(out, " %s ", intern_lookup(g_intern, e->op->id));
    print_expr(e->right);
    fprintf(out, ")");
    break;
  }

  case CST_FIELD_ACCESS: {
    FIELD_E_CSTN *e = (FIELD_E_CSTN *)expr;
    print_expr(e->target);
    fprintf(out, ".%s", intern_lookup(g_intern, e->field->id));
    break;
  }

  case CST_INDEX_EXPR: {
    INDEX_E_CSTN *e = (INDEX_E_CSTN *)expr;
    print_expr(e->target);
    fprintf(out, "[");
    print_expr(e->index);
    fprintf(out, "]");
    break;
  }

  case CST_CALL_EXPR: {
    CALL_E_CSTN *e = (CALL_E_CSTN *)expr;
    print_expr(e->callee);
    fprintf(out, "(");
    bool first = true;
    vec_for_each(e->args.data, e->args.len, arg) {
      if (!first)
        fprintf(out, ", ");
      first = false;
      print_expr(*arg);
    }
    fprintf(out, ")");
    break;
  }

  case CST_RANGE_EXPR: {
    RANGE_E_CSTN *e = (RANGE_E_CSTN *)expr;
    print_expr(e->start);
    fprintf(out, "%s", intern_lookup(g_intern, e->op->id));
    print_expr(e->end);
    break;
  }

  case CST_CAST_EXPR: {
    CAST_E_CSTN *e = (CAST_E_CSTN *)expr;
    print_expr(e->target);
    fprintf(out, " as ");
    print_type(e->type);
    break;
  }

  case CST_ERROR_PROP: {
    EPROP_E_CSTN *e = (EPROP_E_CSTN *)expr;
    print_expr(e->expr);
    fprintf(out, "!");
    break;
  }

  case CST_CATCH_EXPR: {
    CATCH_E_CSTN *e = (CATCH_E_CSTN *)expr;
    print_expr(e->expr);
    fprintf(out, " catch |%s| ", intern_lookup(g_intern, e->err_var->id));
    if (e->handler)
      print_expr(e->handler);
    break;
  }

  case CST_PIPELINE_EXPR: {
    PLINE_E_CSTN *e = (PLINE_E_CSTN *)expr;
    bool first = true;
    vec_for_each(e->stages.data, e->stages.len, stage) {
      if (!first)
        fprintf(out, " >> ");
      first = false;
      print_expr(*stage);
    }
    break;
  }

  case CST_STRUCT_LIT: {
    STRUCT_LE_CSTN *e = (STRUCT_LE_CSTN *)expr;
    if (e->type) {
      print_type(e->type);
      fprintf(out, " ");
    }
    fprintf(out, "{ ");
    bool first = true;
    vec_for_each(e->fields.data, e->fields.len, f) {
      if (!first)
        fprintf(out, ", ");
      first = false;
      fprintf(out, "%s", intern_lookup(g_intern, (*f)->name->id));
      if ((*f)->colon) {
        fprintf(out, ": ");
        print_expr((*f)->value);
      }
    }
    fprintf(out, " }");
    break;
  }

  case CST_ARRAY_LIT: {
    ARRAY_LE_CSTN *e = (ARRAY_LE_CSTN *)expr;
    fprintf(out, "[");
    bool first = true;
    vec_for_each(e->lmnts.data, e->lmnts.len, elem) {
      if (!first)
        fprintf(out, ", ");
      first = false;
      print_expr(*elem);
    }
    fprintf(out, "]");
    break;
  }

  case CST_TUPLE_LIT: {
    TUPLE_LE_CSTN *e = (TUPLE_LE_CSTN *)expr;
    fprintf(out, "(");
    bool first = true;
    vec_for_each(e->lmnts.data, e->lmnts.len, elem) {
      if (!first)
        fprintf(out, ", ");
      first = false;
      print_expr(*elem);
    }
    fprintf(out, ")");
    break;
  }

  case CST_BLOCK: {
    BLOCK_CSTN *b = (BLOCK_CSTN *)expr;
    fprintf(out, "{\n");
    push_indent();
    vec_for_each_struct(b->stmts, stmt) {
      PRINT_INDENT();
      print_stmt(*stmt);
      fprintf(out, "\n");
    }
    if (b->tail) {
      PRINT_INDENT();
      print_expr(b->tail);
      fprintf(out, "\n");
    }
    pop_indent();
    PRINT_INDENT();
    fprintf(out, "}");
    break;
  }

  case CST_IF_EXPR: {
    IF_E_CSTN *e = (IF_E_CSTN *)expr;
    fprintf(out, "if ");
    print_expr(e->cond);
    fprintf(out, " ");
    print_expr(e->then);
    if (e->else_then) {
      fprintf(out, " else ");
      print_expr(e->else_then);
    }
    break;
  }

  case CST_MATCH_EXPR: {
    MATCH_E_CSTN *m = (MATCH_E_CSTN *)expr;
    fprintf(out, "match ");
    print_expr(m->target);
    fprintf(out, " {\n");
    push_indent();
    vec_for_each_struct(m->arms, arm) {
      PRINT_INDENT();
      print_pat((*arm)->pattern);
      fprintf(out, " => ");
      if ((*arm)->expr)
        print_expr((*arm)->expr);
      if ((*arm)->block)
        print_stmt((*arm)->block);
      fprintf(out, ",\n");
    }
    pop_indent();
    PRINT_INDENT();
    fprintf(out, "}");
    break;
  }

  default:
    fprintf(out, "<unknown-expr>");
  }
}

/* =========================================================================
 * statements
 * ========================================================================= */

void print_stmt(cst_s *stmt) {
  if (!stmt) {
    fprintf(out, "<null>");
    return;
  }

  switch (stmt->kind) {

  case CST_EXPR_STMT: {
    EXPR_STMT_S_CSTN *s = (EXPR_STMT_S_CSTN *)stmt;
    print_expr(s->expr);
    fprintf(out, ";");
    break;
  }

  case CST_RETURN_STMT: {
    RETURN_S_CSTN *s = (RETURN_S_CSTN *)stmt;
    fprintf(out, "return");
    if (s->expr) {
      fprintf(out, " ");
      print_expr(s->expr);
    }
    fprintf(out, ";");
    break;
  }

  case CST_BREAK_STMT:
    fprintf(out, "break;");
    break;

  case CST_CONTINUE_STMT:
    fprintf(out, "continue;");
    break;

  case CST_DEFER_STMT: {
    DEFER_S_CSTN *s = (DEFER_S_CSTN *)stmt;
    fprintf(out, "defer ");
    print_expr(s->expr);
    fprintf(out, ";");
    break;
  }

  case CST_IF_STMT: {
    IF_S_CSTN *s = (IF_S_CSTN *)stmt;
    fprintf(out, "if ");
    print_expr(s->cond);
    fprintf(out, " ");
    print_stmt(s->then);
    if (s->else_then) {
      fprintf(out, " else ");
      print_stmt(s->else_then);
    }
    break;
  }

  case CST_WHILE_STMT: {
    WHILE_S_CSTN *s = (WHILE_S_CSTN *)stmt;
    fprintf(out, "while ");
    print_expr(s->cond);
    fprintf(out, " ");
    print_stmt(s->body);
    break;
  }

  case CST_LOOP_STMT: {
    LOOP_S_CSTN *s = (LOOP_S_CSTN *)stmt;
    fprintf(out, "loop ");
    print_stmt(s->body);
    break;
  }

  case CST_FOR_STMT: {
    FOR_S_CSTN *s = (FOR_S_CSTN *)stmt;
    fprintf(out, "for ");
    print_pat(s->pattern);
    fprintf(out, " in ");
    print_expr(s->iter);
    fprintf(out, " ");
    print_stmt(s->body);
    break;
  }

  case CST_MATCH_STMT: {
    MATCH_S_CSTN *m = (MATCH_S_CSTN *)stmt;
    fprintf(out, "match ");
    print_expr(m->target);
    fprintf(out, " {\n");
    push_indent();
    vec_for_each_struct(m->arms, arm) {
      PRINT_INDENT();
      print_pat((*arm)->pattern);
      fprintf(out, " => ");
      if ((*arm)->expr)
        print_expr((*arm)->expr);
      if ((*arm)->block)
        print_stmt((*arm)->block);
      fprintf(out, ",\n");
    }
    pop_indent();
    PRINT_INDENT();
    fprintf(out, "}");
    break;
  }

  case CST_BLOCK: {
    BLOCK_CSTN *b = (BLOCK_CSTN *)stmt;
    fprintf(out, "{\n");
    push_indent();
    vec_for_each_struct(b->stmts, s) {
      PRINT_INDENT();
      print_stmt(*s);
      fprintf(out, "\n");
    }
    if (b->tail) {
      PRINT_INDENT();
      print_expr(b->tail);
      fprintf(out, "\n");
    }
    pop_indent();
    PRINT_INDENT();
    fprintf(out, "}");
    break;
  }

  default:
    fprintf(out, "<unknown-stmt>");
  }
}

/* =========================================================================
 * patterns
 * ========================================================================= */

void print_pat(cst_p *pat) {
  if (!pat) {
    fprintf(out, "<null>");
    return;
  }

  switch (pat->kind) {

  case CST_PATTERN_WILD:
    fprintf(out, "_");
    break;

  case CST_PATTERN_IDENT:
    fprintf(out, "%s", intern_lookup(g_intern, ((IDENT_P_CSTN *)pat)->id));
    break;

  case CST_PATTERN_LIT:
    print_literal(((LIT_P_CSTN *)pat)->literal);
    break;

  case CST_PATTERN_ENUM: {
    ENUM_P_CSTN *p = (ENUM_P_CSTN *)pat;
    fprintf(out, "%s", intern_lookup(g_intern, p->variant->id));
    if (p->args.len > 0) {
      fprintf(out, "(");
      bool first = true;
      vec_for_each(p->args.data, p->args.len, arg) {
        if (!first)
          fprintf(out, ", ");
        first = false;
        print_pat(*arg);
      }
      fprintf(out, ")");
    }
    break;
  }

  case CST_PATTERN_STRUCT: {
    STRUCT_P_CSTN *p = (STRUCT_P_CSTN *)pat;
    fprintf(out, "%s { ", intern_lookup(g_intern, p->type_name->id));
    bool first = true;
    vec_for_each(p->fields.data, p->fields.len, f) {
      if (!first)
        fprintf(out, ", ");
      first = false;
      fprintf(out, "%s", intern_lookup(g_intern, (*f)->name->id));
      if ((*f)->colon) {
        fprintf(out, ": ");
        print_pat((*f)->Ptern);
      }
    }
    fprintf(out, " }");
    break;
  }

  case CST_PATTERN_TUPLE: {
    TUPLE_P_CSTN *p = (TUPLE_P_CSTN *)pat;
    fprintf(out, "(");
    bool first = true;
    vec_for_each(p->lmnts.data, p->lmnts.len, elem) {
      if (!first)
        fprintf(out, ", ");
      first = false;
      print_pat(*elem);
    }
    fprintf(out, ")");
    break;
  }

  case CST_PATTERN_GUARD: {
    GUARD_P_CSTN *p = (GUARD_P_CSTN *)pat;
    print_pat(p->inner);
    fprintf(out, " if ");
    print_expr(p->guard);
    break;
  }

  default:
    fprintf(out, "<unknown-pattern>");
  }
}

/* =========================================================================
 * declarations
 * ========================================================================= */

void print_decl(cst_d *decl) {
  if (!decl) {
    fprintf(out, "<null>");
    return;
  }

  switch (decl->kind) {

  case CST_MODULE: {
    MODULE_D_CSTN *m = (MODULE_D_CSTN *)decl;
    vec_for_each_struct(m->decls, d) {
      print_decl(*d);
      fprintf(out, "\n\n");
    }
    break;
  }

  case CST_VAR_DECL: {
    VAR_D_CSTN *d = (VAR_D_CSTN *)decl;
    fprintf(out, "%s ", intern_lookup(g_intern, d->let_or_mut->id));
    print_pat(d->pattern);
    fprintf(out, " = ");
    print_expr(d->init);
    fprintf(out, ";");
    break;
  }

  case CST_FUNCTION_DECL: {
    FUNC_D_CSTN *f = (FUNC_D_CSTN *)decl;
    fprintf(out, "%s(", intern_lookup(g_intern, f->name->id));
    bool first = true;
    vec_for_each(f->params.data, f->params.len, p) {
      if (!first)
        fprintf(out, ", ");
      first = false;
      if ((*p)->mut)
        fprintf(out, "mut ");
      fprintf(out, "%s: ", intern_lookup(g_intern, (*p)->name->id));
      print_type((*p)->type);
    }
    fprintf(out, ")");
    if (f->ret_type) {
      fprintf(out, " -> ");
      print_type(f->ret_type);
    }
    if (f->body) {
      fprintf(out, " ");
      print_stmt(f->body);
    } else {
      fprintf(out, ";");
    }
    break;
  }

  default:
    fprintf(out, "<unknown-decl>");
  }
}

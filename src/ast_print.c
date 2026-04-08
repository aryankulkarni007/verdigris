#include "../include/ast.h"
#include <stdio.h>

const char *pattern_kind_to_str(PatternKind k) {
  switch (k) {
#define AS_STR(kind, name)                                                     \
  case kind:                                                                   \
    return name;
    PATTERN_KINDS(AS_STR)
#undef AS_STR
  default:
    return "Unknown";
  }
}

const char *type_kind_to_str(TypeKind k) {
  switch (k) {
    // clang-format off
#define AS_CASE(kind, name) case kind: return name;
    // clang-format on
    TYPE_KINDS(AS_CASE)
#undef AS_CASE
  default:
    return "UnknownType";
  }
}

const char *expr_kind_to_str(ExprKind k) {
  switch (k) {
    // clang-format off
#define AS_CASE(kind, name) case kind: return name;
    // clang-format on
    EXPR_KINDS(AS_CASE)
#undef AS_CASE
  default:
    return "UnknownExpr";
  }
}

const char *stmt_kind_to_str(StmtKind k) {
  switch (k) {
    // clang-format off
#define AS_CASE(kind, name) case kind: return name;
    // clang-format on
    STMT_KINDS(AS_CASE)
#undef AS_CASE
  default:
    return "UnknownStmt";
  }
}

const char *decl_kind_to_str(DeclKind k) {
  switch (k) {
    // clang-format off
#define AS_CASE(kind, name) case kind: return name;
    // clang-format on
    DECL_KINDS(AS_CASE)
#undef AS_CASE
  default:
    return "UnknownDecl";
  }
}

// Koz Aesthetic Palette: Seoul256 Edition (Low-contrast, balanced)
#define KGRN                                                                   \
  "\x1B[38;5;179m" // Decls: Dusty Yellow/Straw (Function/Struct names)
#define KBLU "\x1B[38;5;108m" // Stmts: Sage Green (Let, While, If)
#define KMAG "\x1B[38;5;110m" // Exprs: Muted Blue/Steel (Binary Ops, Calls)
#define KCYN                                                                   \
  "\x1B[38;5;103m" // Types: Dusty Purple/Gray-Blue (Int, Float, Player)
#define KYEL                                                                   \
  "\x1B[38;5;174m" // Literals: Muted Terracotta/Peach (Numbers, Strings)
#define KGRY "\x1B[38;5;239m" // Connectors: Deep Charcoal (│ ├── └──)
#define KNRM "\x1B[0m"        // Reset

// Helper: Draws the vertical pipes for nesting
static void print_prefix(int indent, bool *last_mask) {
  for (int i = 0; i < indent - 1; i++) {
    if (last_mask[i])
      printf("    ");
    else
      printf(KGRY "│   " KNRM);
  }
}

// Helper: Draws the branch connector
static void print_branch(int indent, bool *last_mask, bool is_last) {
  print_prefix(indent, last_mask);
  if (indent > 0) {
    printf(KGRY "%s" KNRM, is_last ? "└── " : "├── ");
  }
}

void ast_print_type(Type *t) {
  if (!t) {
    printf(KCYN "infer" KNRM);
    return;
  }

  switch (t->kind) {
  case T_PRIMITIVE:
    switch (t->as.primitive) {
    case TOKEN_TINT:
      printf(KCYN "int" KNRM);
      break;
    case TOKEN_TFLOAT:
      printf(KCYN "float" KNRM);
      break;
    case TOKEN_TBOOL:
      printf(KCYN "bool" KNRM);
      break;
    case TOKEN_TSTRING:
      printf(KCYN "string" KNRM);
      break;
    case TOKEN_TCHAR:
      printf(KCYN "char" KNRM);
      break;
    default:
      printf(KCYN "primitive" KNRM);
      break;
    }
    break;

  case T_NAME:
    printf(KCYN "%s" KNRM, t->as.name);
    break;

  case T_ARRAY:
    printf(KCYN "[" KNRM);
    ast_print_type(t->as.element);
    printf(KCYN "]" KNRM);
    break;

  case T_GENERIC:
    printf(KCYN "%s" KNRM, t->as.generic.base);
    printf(KCYN "<" KNRM);
    for (size_t i = 0; i < t->as.generic.param_count; i++) {
      if (i > 0)
        printf(KCYN ", " KNRM);
      ast_print_type(t->as.generic.params[i]);
    }
    printf(KCYN ">" KNRM);
    break;

  case T_FUNCTION:
    printf(KCYN "(" KNRM);
    for (size_t i = 0; i < t->as.function.param_count; i++) {
      if (i > 0)
        printf(KCYN ", " KNRM);
      ast_print_type(t->as.function.params[i]);
    }
    printf(KCYN ") -> " KNRM);
    ast_print_type(t->as.function.return_type);
    break;

  case T_UNIT:
    printf(KCYN "()" KNRM);
    break;

  default:
    printf(KCYN "%s" KNRM, type_kind_to_str(t->kind));
    break;
  }
}

void ast_print_expr(Expr *e, int indent, bool *last_mask, bool is_last) {
  if (!e)
    return;
  last_mask[indent - 1] = is_last;
  print_branch(indent, last_mask, is_last);

  printf("[" KMAG "%s" KNRM "]", expr_kind_to_str(e->kind));

  switch (e->kind) {
  case E_INT_LIT:
    printf(": " KYEL "%ld" KNRM "\n", e->as.int_val);
    break;
  case E_FLOAT_LIT:
    printf(": " KYEL "%f" KNRM "\n", e->as.float_val);
    break;
  case E_STR_LIT:
    printf(": " KYEL "\"%s\"" KNRM "\n", e->as.str_val);
    break;
  case E_BOOL_LIT:
    printf(": " KYEL "%s" KNRM "\n", e->as.bool_val ? "true" : "false");
    break;
  case E_IDENT:
    printf(": " KNRM "%s\n", e->as.ident_name);
    break;

  case E_BINARY:
    printf(" (" KMAG "%s" KNRM ")\n", e->token.token);
    ast_print_expr(e->as.binary.left, indent + 1, last_mask, false);
    ast_print_expr(e->as.binary.right, indent + 1, last_mask, true);
    break;

  case E_UNARY:
    printf(" (" KMAG "%s" KNRM ")\n", e->token.token);
    ast_print_expr(e->as.unary.operand, indent + 1, last_mask, true);
    break;

  case E_CALL:
    printf("\n");
    ast_print_expr(e->as.call.callee, indent + 1, last_mask,
                   e->as.call.arg_count == 0);
    for (size_t i = 0; i < e->as.call.arg_count; i++) {
      ast_print_expr(e->as.call.args[i], indent + 1, last_mask,
                     i == e->as.call.arg_count - 1);
    }
    break;

  case E_BLOCK:
    printf("\n");
    Block b = e->as.block;
    for (size_t i = 0; i < b.stmt_count; i++) {
      bool last = (i == b.stmt_count - 1) && (b.expr_final == NULL);
      ast_print_stmt(b.statements[i], indent + 1, last_mask, last);
    }
    if (b.expr_final)
      ast_print_expr(b.expr_final, indent + 1, last_mask, true);
    break;

  case E_ACCESS:
    printf("\n");
    ast_print_expr(e->as.access.site, indent + 1, last_mask, false);
    print_prefix(indent + 1, last_mask);
    printf(KGRY "└── " KNRM ".%s\n", e->as.access.name);
    break;

  case E_INDEX:
    printf("\n");
    ast_print_expr(e->as.index.site, indent + 1, last_mask, false);
    ast_print_expr(e->as.index.index, indent + 1, last_mask, true);
    break;

  case E_METHOD:
    printf("\n");
    ast_print_expr(e->as.method.site, indent + 1, last_mask, false);
    print_prefix(indent + 1, last_mask);
    printf(KGRY "├── " KNRM ".%s\n", e->as.method.name);
    for (size_t i = 0; i < e->as.method.arg_count; i++) {
      bool last = (i == e->as.method.arg_count - 1);
      ast_print_expr(e->as.method.args[i], indent + 1, last_mask, last);
    }
    break;

  case E_RANGE:
    printf(" (%s)\n", e->as.range.is_inclusive ? "..=" : "..");
    ast_print_expr(e->as.range.start, indent + 1, last_mask, false);
    ast_print_expr(e->as.range.end, indent + 1, last_mask, true);
    break;

  case E_IF:
    printf("\n");
    ast_print_expr(e->as.if_expr.condition, indent + 1, last_mask,
                   e->as.if_expr.then_block == NULL &&
                       e->as.if_expr.else_block == NULL);
    if (e->as.if_expr.then_block)
      ast_print_stmt(e->as.if_expr.then_block, indent + 1, last_mask,
                     e->as.if_expr.else_block == NULL);
    if (e->as.if_expr.else_block)
      ast_print_stmt(e->as.if_expr.else_block, indent + 1, last_mask, true);
    break;

  case E_ARRAY:
    printf("\n");
    for (size_t i = 0; i < e->as._array.count; i++)
      ast_print_expr(e->as._array.elements[i], indent + 1, last_mask,
                     i == e->as._array.count - 1);
    break;

  case E_STRUCT:
    printf(": " KNRM "%s\n", e->as._struct.struct_name);
    for (size_t i = 0; i < e->as._struct.field_count; i++) {
      bool last = i == e->as._struct.field_count - 1;
      print_branch(indent + 1, last_mask, last);
      printf(KGRY "%s" KNRM " =\n", e->as._struct.fields[i].name);
      last_mask[indent] = last;
      ast_print_expr(e->as._struct.fields[i].value, indent + 2, last_mask,
                     true);
    }
    break;

  case E_MATCH:
    printf("\n");
    ast_print_expr(e->as._match.target, indent + 1, last_mask,
                   e->as._match.arm_count == 0);
    for (size_t i = 0; i < e->as._match.arm_count; i++) {
      bool last = i == e->as._match.arm_count - 1;
      MatchArm *arm = &e->as._match.arms[i];
      print_branch(indent + 1, last_mask, last);
      printf("[" KMAG "Arm" KNRM "]\n");
      last_mask[indent] = last;
      ast_print_pattern(arm->pattern, indent + 2, last_mask,
                        arm->guard == NULL && arm->body == NULL);
      if (arm->guard)
        ast_print_expr(arm->guard, indent + 2, last_mask, arm->body == NULL);
      if (arm->body)
        ast_print_expr(arm->body, indent + 2, last_mask, true);
    }
    break;

  case E_NONE:
    printf(": " KYEL "none" KNRM "\n");
    break;
  default:
    printf("\n");
    break;
  }
}

void ast_print_stmt(Stmt *s, int indent, bool *last_mask, bool is_last) {
  if (!s)
    return;
  last_mask[indent - 1] = is_last;
  print_branch(indent, last_mask, is_last);

  printf("[" KBLU "%s" KNRM "]", stmt_kind_to_str(s->kind));

  switch (s->kind) {
  case S_LET:
    printf(": " KNRM "%s%s ", s->as.let_binding.is_mut ? "mut " : "",
           s->as.let_binding.name);
    ast_print_type(s->as.let_binding.type_annotation);
    printf("\n");
    ast_print_expr(s->as.let_binding.init, indent + 1, last_mask, true);
    break;

  case S_EXPR:
    printf("\n");
    ast_print_expr(s->as.expression, indent + 1, last_mask, true);
    break;

  case S_BLOCK:
    printf("\n");
    Block b = s->as.block;
    for (size_t i = 0; i < b.stmt_count; i++) {
      bool last = (i == b.stmt_count - 1) && (b.expr_final == NULL);
      ast_print_stmt(b.statements[i], indent + 1, last_mask, last);
    }
    if (b.expr_final)
      ast_print_expr(b.expr_final, indent + 1, last_mask, true);
    break;

  case S_WHILE:
    printf("\n");
    ast_print_expr(s->as._while.condition, indent + 1, last_mask, false);
    ast_print_stmt(s->as._while.body, indent + 1, last_mask, true);
    break;
  case S_RETURN:
    printf("\n");
    if (s->as.return_value) {
      ast_print_expr(s->as.return_value, indent + 1, last_mask, true);
    } else {
      print_prefix(indent + 1, last_mask);
      printf(KGRY "└── " KNRM KBLU "void" KNRM "\n");
    }
    break;

  case S_BREAK:
    printf("\n");
    break;

  case S_CONTINUE:
    printf("\n");
    break;

  case S_FOR:
    printf(": " KNRM "%s\n", s->as._for.iterator);
    ast_print_expr(s->as._for.iterable, indent + 1, last_mask, false);
    ast_print_stmt(s->as._for.body, indent + 1, last_mask, true);
    break;

  case S_LOOP:
    printf("\n");
    ast_print_stmt(s->as._loop.body, indent + 1, last_mask, true);
    break;

  case S_ASSIGN:
    printf("\n");
    ast_print_expr(s->as.assign.target, indent + 1, last_mask, false);
    ast_print_expr(s->as.assign.value, indent + 1, last_mask, true);
    break;

  case S_OP_ASSIGN:
    printf(": " KNRM "%s\n", s->as.op_assign.op.token);
    ast_print_expr(s->as.op_assign.target, indent + 1, last_mask, false);
    ast_print_expr(s->as.op_assign.value, indent + 1, last_mask, true);
    break;

  default:
    printf("\n");
    break;
  }
}

void ast_print_decl(Decl *d, int indent, bool *last_mask, bool is_last) {
  if (!d)
    return;
  last_mask[indent - 1] = is_last;
  print_branch(indent, last_mask, is_last);

  printf("[" KGRN "%s" KNRM "]", decl_kind_to_str(d->kind));

  switch (d->kind) {

  case D_FUNC: {
    printf(": " KNRM "%s", d->as.function.name);
    // print params inline
    printf(" (");
    for (size_t i = 0; i < d->as.function.param_count; i++) {
      Param *param = &d->as.function.params[i];
      printf("%s: ", param->name);
      ast_print_type(param->type);
      if (i < d->as.function.param_count - 1)
        printf(", ");
    }
    printf(") -> ");
    ast_print_type(d->as.function.return_type);
    printf("\n");
    if (d->as.function.body)
      ast_print_stmt(d->as.function.body, indent + 1, last_mask, true);
    break;
  }

  case D_STRUCT:
    printf(": " KNRM "%s (%zu field(s))\n", d->as._struct.name,
           d->as._struct.field_count);
    for (size_t i = 0; i < d->as._struct.field_count; i++) {
      bool last_field = (i == d->as._struct.field_count - 1);
      print_prefix(indent + 1, last_mask);
      printf(KGRY "%s" KNRM "%s: ", last_field ? "└── " : "├── ",
             d->as._struct.fields[i].name);
      ast_print_type(d->as._struct.fields[i].type);
      printf("\n");
    }
    break;

  case D_ENUM:
    printf(": " KNRM "%s (%zu variant(s))\n", d->as._enum.name,
           d->as._enum.variant_count);
    for (size_t i = 0; i < d->as._enum.variant_count; i++) {
      bool last_variant = (i == d->as._enum.variant_count - 1);
      print_prefix(indent + 1, last_mask);
      printf(KGRY "%s" KNRM "%s", last_variant ? "└── " : "├── ",
             d->as._enum.variants[i].name);
      if (d->as._enum.variants[i].payload) {
        printf(KGRY "(" KNRM);
        ast_print_type(d->as._enum.variants[i].payload);
        printf(KGRY ")" KNRM);
      }
      printf("\n");
    }
    break;

  case D_IMPL:
    printf(": " KNRM);
    ast_print_type(d->as.impl.target_type);
    printf("\n");
    for (size_t i = 0; i < d->as.impl.method_count; i++) {
      bool last_method = (i == d->as.impl.method_count - 1);
      ast_print_decl(d->as.impl.methods[i], indent + 1, last_mask, last_method);
    }
    break;

  case D_EXTERN:
    printf("\n");
    for (size_t i = 0; i < d->as.extern_block.count; i++) {
      bool last_extern = (i == d->as.extern_block.count - 1);
      ast_print_decl(d->as.extern_block.decls[i], indent + 1, last_mask,
                     last_extern);
    }
    break;

  case D_TYPE:
    printf(": " KNRM "%s = ", d->as.type_alias.alias_name);
    ast_print_type(d->as.type_alias.target);
    printf("\n");
    break;

  default:
    printf("\n");
    break;
  }
}

void ast_print_pattern(Pattern *p, int indent, bool *last_mask, bool is_last) {
  if (!p)
    return;
  last_mask[indent - 1] = is_last;
  print_branch(indent, last_mask, is_last);
  printf("[" KCYN "%s" KNRM "]", pattern_kind_to_str(p->kind));

  switch (p->kind) {
  case P_WILDCARD:
    printf(": " KNRM "_\n");
    break;
  case P_IDENT:
    printf(": " KNRM "%s\n", p->as.name);
    break;
  case P_LITERAL:
    printf("\n");
    ast_print_expr(p->as.literal, indent + 1, last_mask, true);
    break;
  case P_ENUM:
    printf(": " KNRM "%s\n", p->as._enum.variant);
    if (p->as._enum.inner)
      ast_print_pattern(p->as._enum.inner, indent + 1, last_mask, true);
    break;
  case P_STRUCT:
    printf(": " KNRM "%s\n", p->as._struct.name);
    for (size_t i = 0; i < p->as._struct.field_count; i++) {
      bool last = i == p->as._struct.field_count - 1;
      print_branch(indent + 1, last_mask, last);
      printf(KGRY "%s" KNRM "\n", p->as._struct.fields[i]);
    }
    break;
  case P_RANGE:
    printf(" (%s)\n", p->as.range.is_inclusive ? "..=" : "..");
    ast_print_expr(p->as.range.start, indent + 1, last_mask, false);
    ast_print_expr(p->as.range.end, indent + 1, last_mask, true);
    break;
  }
}

void ast_print_module(Module *m) {
  if (!m)
    return;
  bool last_mask[256] = {0};
  printf(KGRY "Module" KNRM "\n");
  for (size_t i = 0; i < m->count; i++) {
    ast_print_decl(m->declarations[i], 1, last_mask, i == m->count - 1);
  }
}

// void run_ast_print_tests(void) {
//   printf("AST visualizer test...\n");
//   printf("==================================\n\n");
//
//   Token t_plus;
//   snprintf(t_plus.token, 255, "+");
//
//   Expr e5 = {.kind = E_INT_LIT, .as.int_val = 5};
//   Expr ey = {.kind = E_IDENT, .as.ident_name = "y"};
//   Expr e_sum = {.kind = E_BINARY,
//                 .token = t_plus,
//                 .as.binary.left = &e5,
//                 .as.binary.right = &ey};
//
//   Stmt s_let = {
//       .kind = S_LET,
//       .as.let_binding = {.name = "x", .is_mut = true, .init = &e_sum}};
//
//   Stmt *stmts[] = {&s_let};
//   Block b = {.statements = stmts, .stmt_count = 1};
//   Stmt s_body = {.kind = S_BLOCK, .as.block = b};
//
//   Decl d_func = {.kind = D_FUNC,
//                  .as.function = {.name = "main", .body = &s_body}};
//
//   Decl *decls[] = {&d_func};
//   Module mod = {.declarations = decls, .count = 1};
//
//   ast_print_module(&mod);
// }

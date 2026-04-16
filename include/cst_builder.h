#pragma once

#include "arena.h"
#include "cst.h"

/* literals */
cst_e *cst_new_ident(Arena *a, Token *t);
cst_e *cst_new_int(Arena *a, Token *t, int64_t value);
cst_e *cst_new_float(Arena *a, Token *t, double value);
cst_e *cst_new_str(Arena *a, Token *t);
cst_e *cst_new_bool(Arena *a, Token *t, bool value);
cst_e *cst_new_null(Arena *a, Token *t);

/* types */
cst_t *cst_new_prim_type(Arena *a, Token *t);
cst_t *cst_new_named_type(Arena *a, Token *name);
cst_t *cst_new_array_type(Arena *a, Token *obrack, cst_t *elem_type,
                          Token *size, Token *cbrack);
cst_t *cst_new_slice_type(Arena *a, Token *amp, cst_t *elem_type,
                          Token *cbrack);
cst_t *cst_new_tuple_type(Arena *a, Token *oparen, Token *cparen);
cst_t *cst_new_union_type(Arena *a, Token *pipe);
cst_t *cst_new_opt_type(Arena *a, Token *question, cst_t *inner);

/* expressions */
cst_e *cst_new_unary(Arena *a, Token *op, cst_e *expr);
cst_e *cst_new_binary(Arena *a, cst_e *left, Token *op, cst_e *right);
cst_e *cst_new_field(Arena *a, cst_e *target, Token *dot, Token *field);
cst_e *cst_new_index(Arena *a, cst_e *target, Token *obrack, cst_e *index,
                     Token *cbrack);
cst_e *cst_new_call(Arena *a, cst_e *callee, Token *oparen, Token *cparen);
cst_e *cst_new_range(Arena *a, cst_e *start, Token *op, cst_e *end);
cst_e *cst_new_cast(Arena *a, cst_e *target, Token *as, cst_t *type);
cst_e *cst_new_error_prop(Arena *a, cst_e *expr, Token *bang);
cst_e *cst_new_catch(Arena *a, cst_e *expr, Token *catch, Token *opipe,
                     Token *err_var, Token *cpipe);
cst_e *cst_new_pipeline(Arena *a, Token *pipe);
cst_e *cst_new_struct_lit(Arena *a, cst_t *type, Token *obrace, Token *cbrace);
cst_e *cst_new_array_lit(Arena *a, Token *obrack, Token *cbrack);
cst_e *cst_new_tuple_lit(Arena *a, Token *oparen, Token *cparen);
cst_e *cst_new_block(Arena *a, Token *obrace, Token *cbrace);
cst_e *cst_new_if_expr(Arena *a, Token *_if, cst_e *cond, cst_e *then_branch,
                       Token *_else, cst_e *else_branch);
cst_e *cst_new_match_expr(Arena *a, Token *match, cst_e *target, Token *obrace,
                          Token *cbrace);

/* patterns */
cst_p *cst_new_wild_pat(Arena *a, Token *underscore);
cst_p *cst_new_ident_pat(Arena *a, Token *name);
cst_p *cst_new_lit_pat(Arena *a, cst_e *literal);
cst_p *cst_new_enum_pat(Arena *a, Token *variant);
cst_p *cst_new_struct_pat(Arena *a, Token *type_name, Token *obrace,
                          Token *cbrace);
cst_p *cst_new_tuple_pat(Arena *a, Token *oparen, Token *cparen);
cst_p *cst_new_guard_pat(Arena *a, cst_p *inner, Token *_if, cst_e *guard);

/* statements */
cst_s *cst_new_expr_stmt(Arena *a, cst_e *expr, Token *semi);
cst_s *cst_new_return_stmt(Arena *a, Token *ret, cst_e *expr, Token *semi);
cst_s *cst_new_break_stmt(Arena *a, Token *brk, Token *semi);
cst_s *cst_new_continue_stmt(Arena *a, Token *cont, Token *semi);
cst_s *cst_new_defer_stmt(Arena *a, Token *defer, cst_e *expr, Token *semi);
cst_s *cst_new_if_stmt(Arena *a, Token *_if, cst_e *cond, Token *_else);
cst_s *cst_new_while_stmt(Arena *a, Token *whl, cst_e *cond);
cst_s *cst_new_loop_stmt(Arena *a, Token *loop);
cst_s *cst_new_for_stmt(Arena *a, Token *_for, Token *_in);
cst_s *cst_new_match_stmt(Arena *a, Token *match, cst_e *target, Token *obrace,
                          Token *cbrace);
cst_s *cst_new_block_stmt(Arena *a, Token *obrace, Token *cbrace);

/* declarations */
cst_d *cst_new_var_decl(Arena *a, Token *let_or_mut, Token *eq, Token *semi);
cst_d *cst_new_func_decl(Arena *a, Token *name, Token *oparen, Token *cparen,
                         Token *arrow);
cst_d *cst_new_struct_decl(Arena *a, Token *name, Token *obrace, Token *cbrace);
cst_d *cst_new_enum_decl(Arena *a, Token *name, Token *obrace, Token *cbrace);
cst_d *cst_new_interface_decl(Arena *a, Token *name, Token *obrace,
                              Token *cbrace);
cst_d *cst_new_impl_decl(Arena *a, Token *type_name, Token *ccolon,
                         Token *method_name);
cst_d *cst_new_import_decl(Arena *a, Token *import, Token *path, Token *semi);
cst_d *cst_new_module(Arena *a);

/* parameters */
PARAM_D_CSTN *cst_new_param(Arena *a, Token *mut, Token *name, Token *colon,
                            cst_t *type);

/* match arms */
MatchArm *cst_new_match_arm(Arena *a, cst_p *pattern, Token *fat_arrow);

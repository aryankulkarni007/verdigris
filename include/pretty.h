#pragma once
#include "cst.h"
#include "intern.h"
#include <stdio.h>

void cst_pretty_init(FILE *output, InternTable *intern);

void print_type(cst_t *type);
void print_expr(cst_e *expr);
void print_stmt(cst_s *stmt);
void print_pat(cst_p *pat);
void print_decl(cst_d *decl);

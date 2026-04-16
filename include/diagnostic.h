#pragma once

#include "span.h"
#include "vector.h"
#include <stdbool.h>
#include <stdio.h>

typedef enum { DIAG_ERROR, DIAG_WARNING, DIAG_NOTE, DIAG_HELP } DiagLevel;

typedef struct {
  DiagLevel level;
  SourceSpan span;
  const char *code;    // e.g. E0001
  const char *message; // primary message
  const char *note;    // optional additional context
  const char *help;    // optional suggestion
} Diagnostic;

typedef struct {
  VecHeader(Diagnostic) diagnostics;
  Arena *arena;
  bool has_errors;
  bool has_warnings;
} DiagBag;

DiagBag diag_init(Arena *arena);

// ... allows for variadic number of arguments
void diag_error(DiagBag *db, SourceSpan span, const char *code, const char *fmt,
                ...);
void diag_warning(DiagBag *db, SourceSpan span, const char *code,
                  const char *fmt, ...);
void diag_note(DiagBag *db, SourceSpan span, const char *fmt, ...);
void diag_help(DiagBag *db, const char *fmt, ...);

void diag_print_all(DiagBag *db, const char *src, FILE *out);

// query
bool diag_ok(DiagBag *bag);

/* NOTE: usage demo */

// In parser
// DiagBag diags = diag_init(&arena);

// if (!match(parser, TK_SEMI)) {
//   SourceSpan span = span_from_token(file_path, l_current_token);
//   diag_error(&diags, span, "E0001", "expected ';' after statement");
//   diag_help(&diags, "try adding a semicolon at the end of the line");
// }
//
// if (!diag_ok(&diags)) {
//   diag_print_all(&diags, source, stderr);
//   return 1;
// }

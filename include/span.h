#pragma once

#include "token.h"
#include <stddef.h>
#include <stdio.h>

/*\
 * NOTE: these are detailed spans, with their most immediate use in error
 * handling and recovery - parser, name resolution, type l_checking, ast,
 * runtime errors
\*/

typedef struct SourceSpan {
  const char *path;  // source file
  size_t start;      // byte offset in src
  size_t end;        // byte offset in src
  size_t line_start; // starting line
  size_t line_end;   // ending line
  size_t col_start;  // starting col (line based)
  size_t col_end;    // ending col
} SourceSpan;

SourceSpan span_from_token(Token *t);
/// dummy span for compiler-generated nodes
SourceSpan span_dummy(void);
/// Print source context for error messages
void span_print_context(SourceSpan span, const char *source, FILE *out);
SourceSpan span_merge(SourceSpan a, SourceSpan b);

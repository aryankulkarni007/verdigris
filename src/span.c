#include "../include/span.h"
#include "../include/token.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

SourceSpan span_from_token(const char *file_path, const Token *t) {
  return (SourceSpan){
      .path = file_path,
      .start = t->span.start,
      .end = t->span.end,
      .line_start = t->line,
      .line_end = t->line,
      .col_start = t->span.start - t->line_start_pos + 1,
      .col_end = t->span.end - t->line_start_pos + 1,
  };
}

SourceSpan span_merge(SourceSpan a, SourceSpan b) {
  return (SourceSpan){
      .path = a.path,
      .start = a.start < b.start ? a.start : b.start,
      .end = a.end > b.end ? a.end : b.end,
      .line_start = a.line_start,
      .line_end = b.line_end,
      .col_start = a.col_start,
      .col_end = b.col_end,
  };
}

SourceSpan span_dummy(void) {
  return (SourceSpan){
      .path = "<generated>",
      .start = 0,
      .end = 0,
      .line_start = 0,
      .line_end = 0,
      .col_start = 0,
      .col_end = 0,
  };
}

void span_print_context(SourceSpan span, const char *source, FILE *out) {
  if (!source || !out)
    return;

  fprintf(out, "  --> %s:%zu:%zu\n", span.path ? span.path : "<unknown>",
          span.line_start, span.col_start);

  // print the line containing the span
  const char *line_start = source + span.start - (span.col_start - 1);
  const char *line_end = line_start;
  while (*line_end && *line_end != '\n')
    line_end++;

  size_t line_len = line_end - line_start;
  fprintf(out, "   |\n");
  fprintf(out, "%3zu | ", span.line_start);
  fwrite(line_start, 1, line_len, out);
  fprintf(out, "\n   | ");

  // print caret(s)
  for (size_t i = 0; i < span.col_start - 1; i++) {
    fprintf(out, " ");
  }

  if (span.line_start == span.line_end) {
    // single line
    size_t width = span.col_end - span.col_start;
    if (width == 0)
      width = 1;
    for (size_t i = 0; i < width; i++) {
      fprintf(out, "^");
    }
  } else {
    // multi-line: ^~~~ to end of line
    size_t remaining = line_len - (span.col_start - 1);
    fprintf(out, "^");
    for (size_t i = 1; i < remaining; i++) {
      fprintf(out, "~");
    }
  }
  fprintf(out, "\n");
}

#include "../include/diagnostic.h"
#include "../include/vector.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

DiagBag diag_init(Arena *arena) {
  return (DiagBag){
      .diagnostics = NULL,
      .arena = arena,
      .has_errors = false,
      .has_warnings = false,
  };
}

static char *format_string(Arena *arena, const char *fmt, va_list args) {
  va_list args_copy;
  va_copy(args_copy, args);
  int len = vsnprintf(NULL, 0, fmt, args_copy);
  va_end(args_copy);

  char *buf = arena_alloc(arena, len + 1);
  vsnprintf(buf, len + 1, fmt, args);
  return buf;
}

void diag_error(DiagBag *db, SourceSpan span, const char *code, const char *fmt,
                ...) {
  va_list args;
  va_start(args, fmt);
  char *msg = format_string(db->arena, fmt, args);
  va_end(args);

  Diagnostic d = {
      .level = DIAG_ERROR, .span = span, .code = code, .message = msg};

  vec_push_struct(db->arena, db->diagnostics, d);
  db->has_errors = true;
}

void diag_warning(DiagBag *db, SourceSpan span, const char *code,
                  const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char *msg = format_string(db->arena, fmt, args);
  va_end(args);

  Diagnostic d = {
      .level = DIAG_WARNING, .span = span, .code = code, .message = msg};

  vec_push_struct(db->arena, db->diagnostics, d);
  db->has_warnings = true;
}

void diag_note(DiagBag *db, SourceSpan span, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char *msg = format_string(db->arena, fmt, args);
  va_end(args);

  Diagnostic d = {.level = DIAG_NOTE, .span = span, .message = msg};
  vec_push_struct(db->arena, db->diagnostics, d);
}

void diag_help(DiagBag *db, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char *msg = format_string(db->arena, fmt, args);
  va_end(args);

  Diagnostic d = {.level = DIAG_HELP, .message = msg};
  vec_push_struct(db->arena, db->diagnostics, d);
}

bool diag_ok(DiagBag *db) { return !db->has_errors; }
// clang-format off
static const char *level_color(DiagLevel level) {
    switch (level) {
        case DIAG_ERROR:   return "\033[1;31m";  // Bold red
        case DIAG_WARNING: return "\033[1;33m";  // Bold yellow
        case DIAG_NOTE:    return "\033[1;36m";  // Bold cyan
        case DIAG_HELP:    return "\033[1;32m";  // Bold green
        default:           return "\033[0m";
    }
}

static const char *level_name(DiagLevel level) {
    switch (level) {
        case DIAG_ERROR:   return "error";
        case DIAG_WARNING: return "warning";
        case DIAG_NOTE:    return "note";
        case DIAG_HELP:    return "help";
        default:           return "unknown";
    }
}
// clang-format on

void diag_print_all(DiagBag *db, const char *source, FILE *out) {
  for (size_t i = 0; i < db->diagnostics.len; ++i) {
    Diagnostic *d = &db->diagnostics.data[i];
    fprintf(out, "%s%s", level_color(d->level), level_name(d->level));
    if (d->code)
      fprintf(out, "[%s]", d->code);

    fprintf(out, ": %s\033[0m\n", d->message);
    // location
    if (d->span.path && d->span.line_start > 0)
      fprintf(out, " --> %s:%zu:%zu\n", d->span.path, d->span.line_start,
              d->span.col_start);

    // source context
    if (source && d->span.start < d->span.end) {
      // line start
      const char *line_start = source + d->span.start;
      while (line_start > source && line_start[-1] != '\n')
        line_start--;

      // line end
      const char *line_end = line_start;
      while (*line_end && *line_end != '\n')
        line_end++;

      // print line
      fprintf(out, "  |\n");
      fprintf(out, "%3zu | ", d->span.line_start);
      fwrite(line_start, 1, line_end - line_start, out);
      fprintf(out, "\n  | ");

      // print caret
      size_t col = d->span.col_start;
      for (size_t j = 1; j < col; ++j)
        fprintf(out, " ");

      size_t width = d->span.col_end - d->span.col_start;
      if (width == 0)
        width = 1;

      fprintf(out, "%s", level_color(d->level));
      for (size_t j = 0; j < width; j++)
        fprintf(out, "‾");
      fprintf(out, "\033[0m\n");
    }
    // note or help
    if (d->note)
      fprintf(out, "  = note: %s\n", d->note);
    if (d->help)
      fprintf(out, "  = help: %s\n", d->help);
    fprintf(out, "\n");
  }
}

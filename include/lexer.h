#ifndef LEXER_H
#define LEXER_H

#include "arena.h"
#include "intern.h"
#include "main.h"
#include "token.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct Lexer Lexer;

struct Lexer {
  const char *src;
  const char *file_path;
  size_t len;
  size_t pos;
  size_t line;
  size_t col;
  size_t line_start_pos;

  Arena *token_arena;
  Arena *trivia_arena;
  Arena *string_arena;
  InternTable *intern;
};

typedef struct {
  Token *data;
  size_t len;
  size_t cap;
} TStream;

TStream lex(Lexer *l);

/// returns current char
static inline char current(Lexer *l) {
  if (l->pos >= l->len) // i.e. eof file
    return '\0';
  return l->src[l->pos];
}

/// peek the char after current
static inline char peek(Lexer *l) {
  if (l->pos + 1 >= l->len)
    return '\0';
  return l->src[l->pos + 1];
}

/// peek 2 chars ahead of the current
static inline char peek_next(Lexer *l) {
  if (l->pos + 2 >= l->len)
    return '\0';
  return l->src[l->pos + 2];
}

/// handles lexer cusor pos and consumes the tk and advances 1
static inline void advance(Lexer *l) {
  char c = current(l);
  ++l->pos;
  ++l->col;
  if (c == '\n') {
    ++l->line;
    l->col = 1;
    l->line_start_pos = l->pos;
  }
  return;
}

static inline bool check(Lexer *l, char c) {
  // silly helper for checking char
  return current(l) == c;
}

void lexer_init(Lexer *l, Arena *token_arena, Arena *string_arena,
                Arena *trivia_arena, const Source *src, InternTable *table);

#endif // LEXER_H

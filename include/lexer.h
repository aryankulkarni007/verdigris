#ifndef LEXER_H
#define LEXER_H

#include "arena.h"
#include "main.h"
#include "token.h"
#include <stddef.h>

typedef struct {
  size_t line;
  size_t column;

  char *file;
  char *start;
  char *pos;
} Lexer;

char peek(Lexer *l);
char peek_next(Lexer *l);
char advance(Lexer *l);

void lex(Arena *stream, Lexer *lexer);
void lexer_new(Lexer *lexer, Source src);

#endif // LEXER_H

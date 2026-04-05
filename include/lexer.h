#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

typedef struct {
  size_t line;
  size_t column;

  char *file;
  char *start;
  char *pos;
} Lexer;

#endif // LEXER_H

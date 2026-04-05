#include "../include/token.h"
#include <stdio.h>
#include <string.h>

/// so we arena allocate the token* outside the function and then assign it's
/// values here
void token_new(Token *token, TType ttype, size_t line, size_t column,
               char *tok) {
  token->ttype = ttype;
  token->line = line;
  token->column = column;

  memcpy(token->token, tok, strlen(tok) + 1);
  return;
}

void token_print(const Token token) {
  printf("%d\n", token.ttype);
  printf("%zu\n", token.line);
  printf("%zu\n", token.column);
  printf("%s\n", token.token);
  return;
}

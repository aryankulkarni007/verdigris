#include "../include/token.h"
#include <stdio.h>
#include <string.h>

// REFACTOR: switched to x-macro instead of enum for ease of use

// clang-format off
#define AS_STR(name) case name: return #name;
// clang-format on

const char *ttype_to_str(TType ttype) {
  switch (ttype) {
    TOKEN_LIST(AS_STR)
  default:
    return "TOKEN_UNKNOWN";
  }
}

#undef AS_STR // you have to undefine this so that it doesn't leak

void token_new(Token *token, TType ttype, size_t line, size_t column,
               char *tok) {
  token->ttype = ttype;
  token->line = line;
  token->column = column;

  if (tok == NULL) {
    token->token[0] = '\0';
    return;
  }

  size_t len = strlen(tok);
  if (len >= sizeof(token->token)) {
    len = sizeof(token->token) - 1;
  }

  memcpy(token->token, tok, len);
  token->token[len] = '\0';
}

void token_print(const Token *token) {
  printf("[%zu:%zu] %-22s \"%s\"\n", token->line, token->column,
         ttype_to_str(token->ttype), token->token);
}

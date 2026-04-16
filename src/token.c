#include "../include/token.h"
#include "../include/trivia.h"
#include <stdio.h>

void print_token(Token *t, const char *source_buffer) {
  if (!t)
    return;

  // print leading trivia
  // clang-format off
  for (size_t i = 0; i < t->leading_count; i++) {
    Trivia *tr = &t->leading[i];
    size_t tr_len = tr->span.end - tr->span.start;
    const char *tr_lex = source_buffer + tr->span.start;

    printf("Trivia { Type: %s, Lexeme: '", tv_name(tr->type));
    for (size_t j = 0; j < tr_len; j++) {
      switch (tr_lex[j]) {
        case '\n': printf("\\n"); break;
        case '\t': printf("\\t"); break;
        case '\r': printf("\\r"); break;
        default:   printf("%c", tr_lex[j]); break;
      }
    }
    printf("' }\n");
  }
  // clang-format on

  size_t length = t->span.end - t->span.start;
  const char *lexeme = source_buffer + t->span.start;

  // print associated token
  printf("Token { Type: %s, Lexeme: '%.*s', Range: [%zu..%zu] }\n",
         tk_name(t->type), (int)length, lexeme, t->span.start, t->span.end);
}

Token new_token(Span span, TK_T type, uint32_t line, size_t line_start_pos,
                char *path) {
  return (Token){.span = span,
                 .type = type,
                 .line = line,
                 .line_start_pos = line_start_pos,
                 .path = path};
}

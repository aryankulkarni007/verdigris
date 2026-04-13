#include "../include/trivia.h"
#include "../include/lexer.h"

Trivia lex_wspace(Lexer *l) {
  Trivia wspace = (Trivia){.span = (Span){.start = l->pos, .end = l->pos + 1},
                           .type = TV_WSPACE};
  advance(l);
  return wspace;
}

Trivia lex_nline(Lexer *l) {
  Trivia wspace = (Trivia){.span = (Span){.start = l->pos, .end = l->pos + 1},
                           .type = TV_NEWLINE};
  advance(l);
  return wspace;
}

Trivia lex_tab(Lexer *l) {
  Trivia tab = (Trivia){.span = (Span){.start = l->pos, .end = l->pos + 1},
                        .type = TV_TAB};
  advance(l);
  return tab;
}

/// call before advancing past -- for comments
Trivia lex_comment(Lexer *l) {
  size_t start_pos = l->pos;
  while (current(l) != '\n' && current(l) != '\0') {
    advance(l);
  }
  return (Trivia){.span = {.start = start_pos, .end = l->pos},
                  .type = TV_COMMENT};
}

/// call before advancing past --- for doc comments
Trivia lex_docc(Lexer *l) {
  size_t start_pos = l->pos;
  while (current(l) != '\n' && current(l) != '\0') {
    advance(l);
  }
  return (Trivia){.span = {.start = start_pos, .end = l->pos}, .type = TV_DOCC};
}

/// call before advancing past -* for block comments
Trivia lex_blockc(Lexer *l) {
  size_t start_pos = l->pos;

  advance(l);
  advance(l);
  while (current(l) != '\0') {
    if (current(l) == '-' && peek(l) == '*') {
      advance(l); // '-'
      advance(l); // '*'
      break;
    }
    advance(l);
  }
  return (Trivia){.span = {.start = start_pos, .end = l->pos},
                  .type = TV_BLOCKC};
}

void append_trivia(Lexer *l, TriviaLexerFn lex_fn, Trivia *leading,
                   size_t *leading_count) {
  if (*leading_count < 64)
    leading[(*leading_count)++] = lex_fn(l);
}

#include "../include/trivia.h"
#include "../include/lexer.h"

void append_trivia_single(Trivia *leading, size_t *count, Trivia t) {
  if (*count < MAX_TRIVIA) {
    leading[(*count)++] = t;
  }
}

Trivia lex_wspace(Lexer *l) {
  Trivia wspace = (Trivia){.span = (Span){.start = l->pos, .end = l->pos + 1},
                           .type = TV_WSPACE};
  l_advance(l);
  return wspace;
}

Trivia lex_nline(Lexer *l) {
  Trivia wspace = (Trivia){.span = (Span){.start = l->pos, .end = l->pos + 1},
                           .type = TV_NEWLINE};
  l_advance(l);
  return wspace;
}

Trivia lex_tab(Lexer *l) {
  Trivia tab = (Trivia){.span = (Span){.start = l->pos, .end = l->pos + 1},
                        .type = TV_TAB};
  l_advance(l);
  return tab;
}

/// call before advancing past -- for comments
Trivia lex_comment(Lexer *l) {
  size_t start_pos = l->pos;
  while (l_current(l) != '\n' && l_current(l) != '\0') {
    l_advance(l);
  }
  return (Trivia){.span = {.start = start_pos, .end = l->pos},
                  .type = TV_COMMENT};
}

/// call before advancing past --- for doc comments
Trivia lex_docc(Lexer *l) {
  size_t start_pos = l->pos;
  while (l_current(l) != '\n' && l_current(l) != '\0') {
    l_advance(l);
  }
  return (Trivia){.span = {.start = start_pos, .end = l->pos}, .type = TV_DOCC};
}

/// call before advancing past -* for block comments
Trivia lex_blockc(Lexer *l) {
  size_t start_pos = l->pos;

  l_advance(l);
  l_advance(l);
  while (l_current(l) != '\0') {
    if (l_current(l) == '-' && l_peek(l) == '*') {
      l_advance(l); // '-'
      l_advance(l); // '*'
      break;
    }
    l_advance(l);
  }
  return (Trivia){.span = {.start = start_pos, .end = l->pos},
                  .type = TV_BLOCKC};
}

void append_trivia(Lexer *l, TriviaLexerFn lex_fn, Trivia *leading,
                   size_t *leading_count) {
  if (*leading_count < 64)
    leading[(*leading_count)++] = lex_fn(l);
}

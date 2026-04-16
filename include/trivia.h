#pragma once

#include "lexer.h"
#include "token.h"

typedef struct Trivia Trivia;
/// for function pointer in append_trivia
typedef Trivia (*TriviaLexerFn)(Lexer *l);

#define TRIVIA_T(X)                                                            \
  X(TV_ERROR)                                                                  \
  X(TV_WSPACE)                                                                 \
  X(TV_NEWLINE)                                                                \
  X(TV_TAB)                                                                    \
  X(TV_COMMENT)                                                                \
  X(TV_DOCC)                                                                   \
  X(TV_BLOCKC)

typedef enum {
#define AS_ENUM(name) name,
  TRIVIA_T(AS_ENUM)
#undef AS_ENUM

} TV_T;

static const char *tv_names[] = {
#define AS_STRING(name) #name,
    TRIVIA_T(AS_STRING)
#undef AS_STRING
};

static inline const char *tv_name(TV_T type) { return tv_names[type]; }

struct Trivia {
  Span span;
  TV_T type;
};

void append_trivia_single(Trivia *leading, size_t *count, Trivia t);

Trivia lex_wspace(Lexer *l);
Trivia lex_nline(Lexer *l);
Trivia lex_tab(Lexer *l);
Trivia lex_comment(Lexer *l);
Trivia lex_docc(Lexer *l);
Trivia lex_blockc(Lexer *l);
void append_trivia(Lexer *l, TriviaLexerFn lex_fn, Trivia *leading,
                   size_t *leading_count);

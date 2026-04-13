#ifndef TOKEN_H
#define TOKEN_H

#include "intern.h"
#include <stddef.h>
#include <stdint.h>

typedef struct Span Span;
typedef struct Token Token;

#define TOKEN_T(X)                                                             \
  X(TK_ILLEGAL)                                                                \
  X(TK_EOF)                                                                    \
  X(TK_AS)                                                                     \
  X(TK_IDENT)                                                                  \
  X(TK_INT)                                                                    \
  X(TK_FLOAT)                                                                  \
  X(TK_STRING)                                                                 \
  X(TK_TRUE)                                                                   \
  X(TK_FALSE)                                                                  \
  X(TK_LET)                                                                    \
  X(TK_MUT)                                                                    \
  X(TK_STRUCT)                                                                 \
  X(TK_ENUM)                                                                   \
  X(TK_FOR)                                                                    \
  X(TK_IN)                                                                     \
  X(TK_MATCH)                                                                  \
  X(TK_LOOP)                                                                   \
  X(TK_WHILE)                                                                  \
  X(TK_BREAK)                                                                  \
  X(TK_IF)                                                                     \
  X(TK_ELSE)                                                                   \
  X(TK_ASSIGN)                                                                 \
  X(TK_EQ)                                                                     \
  X(TK_NEQ)                                                                    \
  X(TK_LT)                                                                     \
  X(TK_GT)                                                                     \
  X(TK_LTEQ)                                                                   \
  X(TK_GTEQ)                                                                   \
  X(TK_AND)                                                                    \
  X(TK_OR)                                                                     \
  X(TK_PLUS)                                                                   \
  X(TK_MINUS)                                                                  \
  X(TK_STAR)                                                                   \
  X(TK_SLASH)                                                                  \
  X(TK_MODULO)                                                                 \
  X(TK_PLUSEQ)                                                                 \
  X(TK_MINUSEQ)                                                                \
  X(TK_STAREQ)                                                                 \
  X(TK_SLASHEQ)                                                                \
  X(TK_MODULOEQ)                                                               \
  X(TK_OPAREN)                                                                 \
  X(TK_CPAREN)                                                                 \
  X(TK_OBRACE)                                                                 \
  X(TK_CBRACE)                                                                 \
  X(TK_OBRACK)                                                                 \
  X(TK_CBRACK)                                                                 \
  X(TK_SEMI)                                                                   \
  X(TK_AMP)                                                                    \
  X(TK_COL)                                                                    \
  X(TK_CCOL)                                                                   \
  X(TK_ARROW)                                                                  \
  X(TK_FATARROW)                                                               \
  X(TK_DOT)                                                                    \
  X(TK_DDOT)                                                                   \
  X(TK_DDOTEQ)                                                                 \
  X(TK_UND)                                                                    \
  X(TK_COMMA)                                                                  \
  X(TK_PIPE)                                                                   \
  X(TK_BANG)

typedef enum {
#define AS_ENUM(name) name,
  TOKEN_T(AS_ENUM)
#undef AS_ENUM
} TK_T;

struct Span {
  size_t start;
  size_t end;
};

struct Token {
  Span span;
  TK_T type;
  uint32_t line; // for error reporting

  struct Trivia *leading;
  size_t leading_count;
  InternID id;
};

void print_token(Token *t, const char *source_buffer);
Token new_token(Span span, TK_T type, uint32_t line, struct Trivia *leading,
                size_t leading_count);

#endif // TOKEN_H

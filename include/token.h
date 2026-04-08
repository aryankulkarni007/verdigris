#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>

// REFACTOR: switched to x-macro instead of enum for ease of use

/// X-MACRO: defines all of the tokens
#define TOKEN_LIST(X)                                                          \
  X(TOKEN_ILLEGAL)                                                             \
  X(TOKEN_EOF)                                                                 \
  X(TOKEN_INT_LIT)                                                             \
  X(TOKEN_FLOAT_LIT)                                                           \
  X(TOKEN_STRING_LIT)                                                          \
  X(TOKEN_CHAR_LIT)                                                            \
  X(TOKEN_IDENT)                                                               \
  X(TOKEN_TINT)                                                                \
  X(TOKEN_TFLOAT)                                                              \
  X(TOKEN_TBOOL)                                                               \
  X(TOKEN_TCHAR)                                                               \
  X(TOKEN_TSTRING)                                                             \
  X(TOKEN_IMPL)                                                                \
  X(TOKEN_LET)                                                                 \
  X(TOKEN_MUT)                                                                 \
  X(TOKEN_STRUCT)                                                              \
  X(TOKEN_ENUM)                                                                \
  X(TOKEN_MATCH)                                                               \
  X(TOKEN_IF)                                                                  \
  X(TOKEN_ELSE)                                                                \
  X(TOKEN_LOOP)                                                                \
  X(TOKEN_WHILE)                                                               \
  X(TOKEN_FOR)                                                                 \
  X(TOKEN_IN)                                                                  \
  X(TOKEN_BREAK)                                                               \
  X(TOKEN_CONTINUE)                                                            \
  X(TOKEN_RETURN)                                                              \
  X(TOKEN_IMPORT)                                                              \
  X(TOKEN_MODULE)                                                              \
  X(TOKEN_TYPE)                                                                \
  X(TOKEN_EXTERN)                                                              \
  X(TOKEN_TRUE)                                                                \
  X(TOKEN_FALSE)                                                               \
  X(TOKEN_SOME)                                                                \
  X(TOKEN_NONE)                                                                \
  X(TOKEN_PLUS)                                                                \
  X(TOKEN_MINUS)                                                               \
  X(TOKEN_STAR)                                                                \
  X(TOKEN_SLASH)                                                               \
  X(TOKEN_PERCENT)                                                             \
  X(TOKEN_STARSTAR)                                                            \
  X(TOKEN_EQ)                                                                  \
  X(TOKEN_NEQ)                                                                 \
  X(TOKEN_LT)                                                                  \
  X(TOKEN_GT)                                                                  \
  X(TOKEN_LEQ)                                                                 \
  X(TOKEN_GEQ)                                                                 \
  X(TOKEN_AND)                                                                 \
  X(TOKEN_OR)                                                                  \
  X(TOKEN_NOT)                                                                 \
  X(TOKEN_BAND)                                                                \
  X(TOKEN_BOR)                                                                 \
  X(TOKEN_BXOR)                                                                \
  X(TOKEN_BNOT)                                                                \
  X(TOKEN_LSHIFT)                                                              \
  X(TOKEN_RSHIFT)                                                              \
  X(TOKEN_ASSIGN)                                                              \
  X(TOKEN_PLUS_ASSIGN)                                                         \
  X(TOKEN_MINUS_ASSIGN)                                                        \
  X(TOKEN_STAR_ASSIGN)                                                         \
  X(TOKEN_SLASH_ASSIGN)                                                        \
  X(TOKEN_PERCENT_ASSIGN)                                                      \
  X(TOKEN_ARROW)                                                               \
  X(TOKEN_FAT_ARROW)                                                           \
  X(TOKEN_QUESTION)                                                            \
  X(TOKEN_DOT)                                                                 \
  X(TOKEN_DOTDOT)                                                              \
  X(TOKEN_DOTDOTEQ)                                                            \
  X(TOKEN_COLON)                                                               \
  X(TOKEN_CCOLON)                                                              \
  X(TOKEN_COMMA)                                                               \
  X(TOKEN_UNDERSCORE)                                                          \
  X(TOKEN_LPAREN)                                                              \
  X(TOKEN_RPAREN)                                                              \
  X(TOKEN_LBRACE)                                                              \
  X(TOKEN_RBRACE)                                                              \
  X(TOKEN_LBRACK)                                                              \
  X(TOKEN_RBRACK)

// generate the enum from the x-macro
typedef enum {
#define AS_ENUM(name) name,
  TOKEN_LIST(AS_ENUM)
#undef AS_ENUM
} TType;

typedef struct {
  TType ttype;
  size_t line;
  size_t column;
  char token[256];
} Token;

// utility (ttype -> string)
const char *ttype_to_str(TType ttype);

void token_new(Token *token, TType ttype, size_t line, size_t column,
               char *tok);

void token_print(const Token *token);

#endif

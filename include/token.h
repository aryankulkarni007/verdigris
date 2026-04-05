#ifndef TOKEN_H
#define TOKEN_H
#include <stddef.h>

typedef enum {
  // special
  TOKEN_ILLEGAL,
  TOKEN_EOF,

  // literals
  TOKEN_INT_LIT,
  TOKEN_FLOAT_LIT,
  TOKEN_STRING_LIT,
  TOKEN_CHAR_LIT,

  // identifier
  TOKEN_IDENT,

  // primitive type keywords
  TOKEN_TINT,
  TOKEN_TFLOAT,
  TOKEN_TBOOL,
  TOKEN_TCHAR,
  TOKEN_TSTRING,

  // keywords
  TOKEN_IMPL,
  TOKEN_LET,
  TOKEN_MUT,
  // TOKEN_FN,
  TOKEN_STRUCT,
  TOKEN_ENUM,
  TOKEN_MATCH,
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_LOOP,
  TOKEN_WHILE,
  TOKEN_FOR,
  TOKEN_IN,
  TOKEN_BREAK,
  TOKEN_CONTINUE,
  TOKEN_RETURN,
  TOKEN_IMPORT,
  TOKEN_MODULE,
  TOKEN_PRIVATE,
  TOKEN_TYPE,
  TOKEN_EXTERN,
  TOKEN_TRUE,
  TOKEN_FALSE,
  TOKEN_SOME,
  TOKEN_NONE,

  // arithmetic operators
  TOKEN_PLUS,     // +
  TOKEN_MINUS,    // -
  TOKEN_STAR,     // *
  TOKEN_SLASH,    // /
  TOKEN_PERCENT,  // %
  TOKEN_STARSTAR, // **

  // comparison operators
  TOKEN_EQ,  // ==
  TOKEN_NEQ, // !=
  TOKEN_LT,  //
  TOKEN_GT,  // >
  TOKEN_LEQ, // <=
  TOKEN_GEQ, // >=

  // logical operators
  TOKEN_AND, // &&
  TOKEN_OR,  // ||
  TOKEN_NOT, // !

  // bitwise operators
  TOKEN_BAND,   // &
  TOKEN_BOR,    // |
  TOKEN_BXOR,   // ^
  TOKEN_BNOT,   // ~
  TOKEN_LSHIFT, //
  TOKEN_RSHIFT, // >>

  // assignment
  TOKEN_ASSIGN,         // =
  TOKEN_PLUS_ASSIGN,    // +=
  TOKEN_MINUS_ASSIGN,   // -=
  TOKEN_STAR_ASSIGN,    // *=
  TOKEN_SLASH_ASSIGN,   // /=
  TOKEN_PERCENT_ASSIGN, // %=

  // punctuation
  TOKEN_ARROW,      // ->
  TOKEN_FAT_ARROW,  // =>
  TOKEN_QUESTION,   // ?
  TOKEN_DOT,        // .
  TOKEN_DOTDOT,     // ..
  TOKEN_DOTDOTEQ,   // ..=
  TOKEN_COLON,      // :
  TOKEN_CCOLON,     // ::
  TOKEN_COMMA,      // ,
  TOKEN_UNDERSCORE, // _

  // delimiters
  TOKEN_LPAREN, // (
  TOKEN_RPAREN, // )
  TOKEN_LBRACE, // {
  TOKEN_RBRACE, // }
  TOKEN_LBRACK, // [
  TOKEN_RBRACK, // ]
} TType;

typedef struct {
  TType ttype;
  size_t line;
  size_t column;

  char *token;
} Token;

// TODO: implement this once we have made a bump arena allocator
Token *token_new(TType ttype, size_t line, size_t column, char *token);
void tokens_free(Token *tokens);

#endif // TOKEN_H

#include "../include/lexer.h"
#include "../include/token.h"
#include "../include/trivia.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

/* TODO: check before commit */

#define MAX_TRIVIA 64

static const TK_T char_tk[256] = {
    ['+'] = TK_PLUS,   ['-'] = TK_MINUS,  ['*'] = TK_STAR,   ['/'] = TK_SLASH,
    ['('] = TK_OPAREN, [')'] = TK_CPAREN, ['{'] = TK_OBRACE, ['}'] = TK_CBRACE,
    ['['] = TK_OBRACK, [']'] = TK_CBRACK, [';'] = TK_SEMI,   [','] = TK_COMMA,
    ['.'] = TK_DOT,    [':'] = TK_COL,    ['='] = TK_ASSIGN, ['<'] = TK_LT,
    ['>'] = TK_GT,     ['!'] = TK_BANG,   ['&'] = TK_AMP,    ['|'] = TK_PIPE,
    ['%'] = TK_MODULO
    // all other indices default to 0 (TK_ILLEGAL)
};

void new_lexer(Lexer *l, Arena *token_arena, Arena *string_arena,
               Arena *trivia_arena, const Source *src) {
  l->src = src->buffer;
  l->len = src->file_size;
  l->pos = 0;
  l->line = 1;
  l->col = 1;
  l->token_arena = token_arena;
  l->trivia_arena = trivia_arena;
  l->string_arena = string_arena;
}

static Token lex_ident(Lexer *l, Trivia *leading, size_t leading_count) {
  size_t start = l->pos;
  size_t line = l->line;
  while (isalnum(current(l)) || current(l) == '_')
    advance(l);

  return (Token){
      .span = {.start = start, .end = l->pos},
      .type = TK_IDENT,
      .line = line,
      .leading = leading,
      .leading_count = leading_count,
  };
}

static Token lex_num(Lexer *l, Trivia *leading, size_t leading_count) {
  size_t start = l->pos;
  size_t line = l->line;
  bool is_float = false;
  while (isdigit(current(l)) || current(l) == '.') {
    if (current(l) == '.') {
      if (peek(l) == '.')
        break; // check for range operator
      if (is_float)
        break; // second dot
      is_float = true;
    }
    advance(l);
  }

  return (Token){
      .span = {.start = start, .end = l->pos},
      .type = is_float ? TK_FLOAT : TK_INT,
      .line = line,
      .leading = leading,
      .leading_count = leading_count,
  };
}

static Token lex_string(Lexer *l, Trivia *leading, size_t leading_count) {
  advance(l); // '"'
  size_t start = l->pos;
  size_t line = l->line;
  size_t end = 0;
  while (current(l) != '"' && current(l) != '\0') {
    if (current(l) == '\\' && peek(l) == '"') {
      advance(l); // '\'
      advance(l); // '"'
      continue;
    }
    advance(l);
    end = l->pos;
  }
  advance(l); // '"'
  return (Token){
      .span = {.start = start, .end = end},
      .type = TK_STRING,
      .line = line,
      .leading = leading,
      .leading_count = leading_count,
  };
}

Token lex_single(Lexer *l, char tk, Trivia *leading, size_t leading_count) {
  size_t start = l->pos;
  size_t line = l->line;
  advance(l);
  return (Token){
      .span = {.start = start, .end = l->pos},
      .type = char_tk[(unsigned char)tk],
      .line = line,
      .leading = leading,
      .leading_count = leading_count,
  };
}

Token lex_double(Lexer *l, TK_T type, Trivia *leading, size_t leading_count) {
  size_t line = l->line;
  Span span = {0};
  span.start = l->pos;
  advance(l);
  advance(l); // '='
  span.end = l->pos;
  return new_token(span, type, line, leading, leading_count);
}

static void attach_trivia_to_token(Token *t, Lexer *l, Trivia *source,
                                   size_t count) {
  if (count == 0) {
    t->leading = NULL;
    t->leading_count = 0;
    return;
  }

  Trivia *attached = arena_alloc(l->trivia_arena, sizeof(Trivia) * count);
  memcpy(attached, source, sizeof(Trivia) * count);
  t->leading = attached;
  t->leading_count = count;
}

/// phase 1: collect trivia into stack
/// phase 2: attack trivia to next token
Token next_token(Lexer *l) {
  Trivia leading[MAX_TRIVIA];
  size_t leading_count = 0;

  // collect trivia
  while (true) {
    char c = current(l);
    if (c == ' ')
      append_trivia(l, lex_wspace, leading, &leading_count);
    else if (c == '\n')
      append_trivia(l, lex_nline, leading, &leading_count);
    else if (c == '\t')
      append_trivia(l, lex_tab, leading, &leading_count);
    else if (c == '-' && peek(l) == '-' && peek_next(l) == '-')
      append_trivia(l, lex_docc, leading, &leading_count);
    else if (c == '-' && peek(l) == '*')
      append_trivia(l, lex_blockc, leading, &leading_count);
    else if (c == '-' && peek(l) == '-')
      append_trivia(l, lex_comment, leading, &leading_count);
    else
      break;
  }

  Token t;

  // big
  if (isdigit(current(l))) {
    t = lex_num(l, leading, leading_count);
  } else if (isalnum(current(l)) || current(l) == '_') {
    t = lex_ident(l, leading, leading_count);
  } else if (current(l) == '"') {
    t = lex_string(l, leading, leading_count);
  } else if (current(l) == '\0') {
    t = (Token){
        .span = {.start = l->pos, .end = l->pos},
        .type = TK_EOF,
        .line = l->line,
        .leading = leading,
        .leading_count = leading_count,
    };
  } else {
    // double and triple
    char c = current(l);
    char n = peek(l);
    char nn = peek_next(l);

    if (c == '+' && n == '=')
      t = lex_double(l, TK_PLUSEQ, leading, leading_count);
    else if (c == '-' && n == '=')
      t = lex_double(l, TK_MINUSEQ, leading, leading_count);
    else if (c == '*' && n == '=')
      t = lex_double(l, TK_STAREQ, leading, leading_count);
    else if (c == '/' && n == '=')
      t = lex_double(l, TK_SLASHEQ, leading, leading_count);
    else if (c == '%' && n == '=')
      t = lex_double(l, TK_MODULOEQ, leading, leading_count);
    else if (c == '<' && n == '=')
      t = lex_double(l, TK_LTEQ, leading, leading_count);
    else if (c == '>' && n == '=')
      t = lex_double(l, TK_GTEQ, leading, leading_count);
    else if (c == '!' && n == '=')
      t = lex_double(l, TK_NEQ, leading, leading_count);
    else if (c == '=' && n == '=')
      t = lex_double(l, TK_EQ, leading, leading_count);
    else if (c == '-' && n == '>')
      t = lex_double(l, TK_ARROW, leading, leading_count);
    else if (c == '=' && n == '>')
      t = lex_double(l, TK_FATARROW, leading, leading_count);
    else if (c == '.' && n == '.')
      t = lex_double(l, TK_DDOT, leading, leading_count);
    else if (c == ':' && n == ':')
      t = lex_double(l, TK_CCOL, leading, leading_count);
    else if (c == '.' && n == '.' && nn == '=') {
      size_t line = l->line;
      Span span = {0};
      span.start = l->pos;
      advance(l);
      advance(l);
      advance(l);
      span.end = l->pos;
      t = new_token(span, TK_DDOTEQ, line, leading, leading_count);
    } else {
      // single char tokens
      t = lex_single(l, current(l), leading, leading_count);
    }
  }

  attach_trivia_to_token(&t, l, leading, leading_count);
  return t;
}

/// entry point into the lexer
Token *lex(Lexer *l) {
  Token *base = NULL;
  Token *t;
  do {
    t = arena_alloc(l->token_arena, sizeof(Token));
    if (!base)
      base = t;
    *t = next_token(l);
  } while (t->type != TK_EOF);
  return base;
}

#include "../include/lexer.h"
#include "../include/intern.h"
#include "../include/token.h"
#include "../include/trivia.h"
#include "../include/vector.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#define MAX_TRIVIA 64

static const TK_T char_tk[256] = {
    ['+'] = TK_PLUS,   ['-'] = TK_MINUS,    ['*'] = TK_STAR,
    ['/'] = TK_SLASH,  ['('] = TK_OPAREN,   [')'] = TK_CPAREN,
    ['{'] = TK_OBRACE, ['}'] = TK_CBRACE,   ['['] = TK_OBRACK,
    [']'] = TK_CBRACK, [';'] = TK_SEMI,     [','] = TK_COMMA,
    ['.'] = TK_DOT,    [':'] = TK_COL,      ['='] = TK_ASSIGN,
    ['<'] = TK_LT,     ['>'] = TK_GT,       ['!'] = TK_BANG,
    ['&'] = TK_AMP,    ['|'] = TK_PIPE,     ['%'] = TK_MODULO,
    ['@'] = TK_AT,     ['?'] = TK_QUESTION,
    // all other indices default to 0 (TK_ILLEGAL)
};

void lexer_init(Lexer *l, Arena *token_arena, Arena *string_arena,
                Arena *trivia_arena, const Source *src, InternTable *table) {
  l->src = src->buffer;
  l->file_path = src->file_path;
  l->len = src->file_size;
  l->pos = 0;
  l->line = 1;
  l->col = 1;
  l->line_start_pos = 0;
  l->token_arena = token_arena;
  l->trivia_arena = trivia_arena;
  l->string_arena = string_arena;
  l->intern = table;
}

static Token lex_ident(Lexer *l) {
  size_t start = l->pos;
  size_t line = l->line;
  size_t line_start = l->line_start_pos;
  while (isalnum(current(l)) || current(l) == '_')
    advance(l);

  size_t len = l->pos - start;
  InternID id = intern_string(l->intern, l->src + start, len);
  return (Token){.span = {.start = start, .end = l->pos},
                 .type = TK_IDENT,
                 .line = line,
                 .line_start_pos = line_start,
                 .id = id};
}

static Token lex_num(Lexer *l) {
  size_t start = l->pos;
  size_t line = l->line;
  size_t line_start = l->line_start_pos;
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
      .line_start_pos = line_start,
  };
}

static Token lex_string(Lexer *l) {
  advance(l); // '"'
  size_t start = l->pos;
  size_t line = l->line;
  size_t line_start = l->line_start_pos;
  while (current(l) != '"' && current(l) != '\0') {
    if (current(l) == '\\' && peek(l) == '"') {
      advance(l);
      advance(l);
      continue;
    }
    advance(l);
  }
  size_t end = l->pos;
  if (current(l) == '"') {
    advance(l); // closing quote
  }

  size_t len = end - start;
  InternID id = intern_string(l->intern, l->src + start, len);
  return (Token){
      .span = {.start = start, .end = end},
      .type = TK_STRING,
      .line = line,
      .line_start_pos = line_start,
      .id = id,
  };
}

Token lex_single(Lexer *l, char tk) {
  size_t start = l->pos;
  size_t line = l->line;
  size_t line_start = l->line_start_pos;
  advance(l);
  return (Token){
      .span = {.start = start, .end = l->pos},
      .type = char_tk[(unsigned char)tk],
      .line = line,
      .line_start_pos = line_start,
  };
}

Token lex_double(Lexer *l, TK_T type) {
  size_t line = l->line;
  size_t line_start = l->line_start_pos;
  Span span = {0};
  span.start = l->pos;
  advance(l);
  advance(l); // '='
  span.end = l->pos;
  return new_token(span, type, line, line_start);
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
/// phase 2: attach trivia to next token
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
    t = lex_num(l);
  } else if (isalnum(current(l)) || current(l) == '_') {
    t = lex_ident(l);
  } else if (current(l) == '"') {
    t = lex_string(l);
  } else if (current(l) == '\0') {
    t = (Token){.span = {.start = l->pos, .end = l->pos},
                .type = TK_EOF,
                .line = l->line,
                .line_start_pos = l->line_start_pos};
  } else {
    // double and triple
    char c = current(l);
    char n = peek(l);
    char nn = peek_next(l);

    if (c == '.' && n == '.' && nn == '=') {
      size_t line = l->line;
      size_t line_start = l->line_start_pos;
      Span span = {0};
      span.start = l->pos;
      advance(l);
      advance(l);
      advance(l);
      span.end = l->pos;
      t = new_token(span, TK_DDOTEQ, line, line_start);
    } else if (c == '&' && n == '&')
      t = lex_double(l, TK_AND);
    else if (c == '|' && n == '|')
      t = lex_double(l, TK_OR);
    else if (c == '+' && n == '=')
      t = lex_double(l, TK_PLUSEQ);
    else if (c == '-' && n == '=')
      t = lex_double(l, TK_MINUSEQ);
    else if (c == '*' && n == '=')
      t = lex_double(l, TK_STAREQ);
    else if (c == '/' && n == '=')
      t = lex_double(l, TK_SLASHEQ);
    else if (c == '%' && n == '=')
      t = lex_double(l, TK_MODULOEQ);
    else if (c == '<' && n == '=')
      t = lex_double(l, TK_LTEQ);
    else if (c == '>' && n == '=')
      t = lex_double(l, TK_GTEQ);
    else if (c == '>' && n == '>')
      t = lex_double(l, TK_PIPELINE);
    else if (c == '!' && n == '=')
      t = lex_double(l, TK_NEQ);
    else if (c == '=' && n == '=')
      t = lex_double(l, TK_EQ);
    else if (c == '-' && n == '>')
      t = lex_double(l, TK_ARROW);
    else if (c == '=' && n == '>')
      t = lex_double(l, TK_FATARROW);
    else if (c == '.' && n == '.')
      t = lex_double(l, TK_DDOT);
    else if (c == ':' && n == ':')
      t = lex_double(l, TK_CCOL);
    else {
      // single
      t = lex_single(l, current(l));
    }
  }

  attach_trivia_to_token(&t, l, leading, leading_count);
  return t;
}

/// entry point into the lexer
TStream lex(Lexer *l) {
  TStream stream = {0};

  Token t;
  do {
    t = next_token(l);
    vec_push_struct(l->token_arena, stream, t);
  } while (t.type != TK_EOF);

  return stream;
}

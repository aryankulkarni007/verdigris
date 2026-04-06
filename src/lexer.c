#include "../include/lexer.h"
#include "../include/arena.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *keyword;
  TType type;
} Keyword;

static Keyword keywords[] = {
    {"let", TOKEN_LET},
    {"mut", TOKEN_MUT},
    {"struct", TOKEN_STRUCT},
    {"enum", TOKEN_ENUM},
    {"match", TOKEN_MATCH},
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"loop", TOKEN_LOOP},
    {"while", TOKEN_WHILE},
    {"for", TOKEN_FOR},
    {"in", TOKEN_IN},
    {"break", TOKEN_BREAK},
    {"continue", TOKEN_CONTINUE},
    {"return", TOKEN_RETURN},
    {"import", TOKEN_IMPORT},
    {"module", TOKEN_MODULE},
    {"private", TOKEN_PRIVATE},
    {"type", TOKEN_TYPE},
    {"extern", TOKEN_EXTERN},
    {"true", TOKEN_TRUE},
    {"false", TOKEN_FALSE},
    {"int", TOKEN_TINT},
    {"float", TOKEN_TFLOAT},
    {"bool", TOKEN_TBOOL},
    {"char", TOKEN_TCHAR},
    {"string", TOKEN_TSTRING},
    {"some", TOKEN_SOME},
    {"none", TOKEN_NONE},
};

static size_t keyword_count = sizeof(keywords) / sizeof(keywords[0]);

char peek(Lexer *l) { return *l->pos; }

char peek_next(Lexer *l) { return *(l->pos + 1); }

char advance(Lexer *l) {
  char current = peek(l);

  if (*l->pos == '\n') {
    ++l->line;
    l->column = 1;
  } else {
    ++l->column;
  }

  l->pos += 1;
  return current;
}

void lexer_new(Lexer *lexer, Source src) {
  lexer->line = 1;
  lexer->column = 1;
  lexer->file = src.buffer;
  lexer->start = lexer->pos = lexer->file;
}

TType lookup(char *word) {
  for (size_t i = 0; i < keyword_count; i++) {
    if (strcmp(word, keywords[i].keyword) == 0)
      return keywords[i].type;
  }
  return TOKEN_IDENT;
}

void l_token_append(char *token, TType ttype, Arena *stream, Lexer *l) {
  Token *new = arena_allocate(stream, sizeof(Token));
  token_new(new, ttype, l->line, l->column, token);
  l->start = l->pos;
}

void lex(Arena *stream, Lexer *l) {
  while (*l->pos != '\0') {
    while (*l->pos == ' ' || *l->pos == '\n' || *l->pos == '\r' ||
           *l->pos == '\t') {
      advance(l);
    }

    if (*l->pos == '\0')
      break;

    l->start = l->pos;
    char cur = advance(l);

    if (isalpha(cur) || cur == '_') {
      while (isalnum(*l->pos) || *l->pos == '_')
        advance(l);

      size_t len = l->pos - l->start;

      if (len >= 256) {
        fprintf(stderr, "%s:[%zu:%zu] error: token too long (max 255 chars)\n",
                l->file, l->line, l->column);
        exit(1);
      }

      char tok[256];
      memcpy(tok, l->start, len);
      tok[len] = '\0';

      if (len == 1 && tok[0] == '_')
        l_token_append("_", TOKEN_UNDERSCORE, stream, l);
      else
        l_token_append(tok, lookup(tok), stream, l);
    } else if (isdigit(cur)) {
      bool is_float = false;
      while (isdigit(*l->pos)) {
        advance(l);
      }

      if (*l->pos == '.' && isdigit(*(l->pos + 1))) {
        is_float = true;
        advance(l);

        while (isdigit(*l->pos)) {
          advance(l);
        }
      }

      size_t len = l->pos - l->start;

      if (len >= 256) {
        fprintf(
            stderr,
            "%s:[%zu:%zu] error: numeric literal too long (max 255 chars)\n",
            l->file, l->line, l->column);
        exit(1);
      }

      char tok[256];
      memcpy(tok, l->start, len);
      tok[len] = '\0';

      if (!is_float) {
        l_token_append(tok, TOKEN_INT_LIT, stream, l);
      } else {
        l_token_append(tok, TOKEN_FLOAT_LIT, stream, l);
      }
    } else {
      switch (cur) {
      case '(':
        l_token_append("(", TOKEN_LPAREN, stream, l);
        break;
      case ')':
        l_token_append(")", TOKEN_RPAREN, stream, l);
        break;
      case '{':
        l_token_append("{", TOKEN_LBRACE, stream, l);
        break;
      case '}':
        l_token_append("}", TOKEN_RBRACE, stream, l);
        break;
      case '[':
        l_token_append("[", TOKEN_LBRACK, stream, l);
        break;
      case ']':
        l_token_append("]", TOKEN_RBRACK, stream, l);
        break;
      case ',':
        l_token_append(",", TOKEN_COMMA, stream, l);
        break;
      case '~':
        l_token_append("~", TOKEN_BNOT, stream, l);
        break;
      case '^':
        l_token_append("^", TOKEN_BXOR, stream, l);
        break;
      case '-':
        if (peek(l) == '>') {
          advance(l);
          l_token_append("->", TOKEN_ARROW, stream, l);
        } else if (peek(l) == '=') {
          advance(l);
          l_token_append("-=", TOKEN_MINUS_ASSIGN, stream, l);
        } else if (peek(l) == '-') {
          while (*l->pos != '\n' && *l->pos != '\0')
            advance(l);
          l->start = l->pos;
        } else {
          l_token_append("-", TOKEN_MINUS, stream, l);
        }
        break;
      case '=':
        if (peek(l) == '=') {
          advance(l);
          l_token_append("==", TOKEN_EQ, stream, l);
        } else if (peek(l) == '>') {
          advance(l);
          l_token_append("=>", TOKEN_FAT_ARROW, stream, l);
        } else {
          l_token_append("=", TOKEN_ASSIGN, stream, l);
        }
        break;
      case '!':
        if (peek(l) == '=') {
          advance(l);
          l_token_append("!=", TOKEN_NEQ, stream, l);
        } else {
          l_token_append("!", TOKEN_NOT, stream, l);
        }
        break;
      case '<':
        if (peek(l) == '=') {
          advance(l);
          l_token_append("<=", TOKEN_LEQ, stream, l);
        } else if (peek(l) == '<') {
          advance(l);
          l_token_append("<<", TOKEN_LSHIFT, stream, l);
        } else {
          l_token_append("<", TOKEN_LT, stream, l);
        }
        break;
      case '>':
        if (peek(l) == '=') {
          advance(l);
          l_token_append(">=", TOKEN_GEQ, stream, l);
        } else if (peek(l) == '>') {
          advance(l);
          l_token_append(">>", TOKEN_RSHIFT, stream, l);
        } else {
          l_token_append(">", TOKEN_GT, stream, l);
        }
        break;
      case '&':
        if (peek(l) == '&') {
          advance(l);
          l_token_append("&&", TOKEN_AND, stream, l);
        } else {
          l_token_append("&", TOKEN_BAND, stream, l);
        }
        break;
      case '|':
        if (peek(l) == '|') {
          advance(l);
          l_token_append("||", TOKEN_OR, stream, l);
        } else {
          l_token_append("|", TOKEN_BOR, stream, l);
        }
        break;
      case '+':
        if (peek(l) == '=') {
          advance(l);
          l_token_append("+=", TOKEN_PLUS_ASSIGN, stream, l);
        } else {
          l_token_append("+", TOKEN_PLUS, stream, l);
        }
        break;
      case '*':
        if (peek(l) == '*') {
          advance(l);
          l_token_append("**", TOKEN_STARSTAR, stream, l);
        } else if (peek(l) == '=') {
          advance(l);
          l_token_append("*=", TOKEN_STAR_ASSIGN, stream, l);
        } else {
          l_token_append("*", TOKEN_STAR, stream, l);
        }
        break;
      case '/':
        if (peek(l) == '=') {
          advance(l);
          l_token_append("/=", TOKEN_SLASH_ASSIGN, stream, l);
        } else {
          l_token_append("/", TOKEN_SLASH, stream, l);
        }
        break;
      case '%':
        if (peek(l) == '=') {
          advance(l);
          l_token_append("%=", TOKEN_PERCENT_ASSIGN, stream, l);
        } else {
          l_token_append("%", TOKEN_PERCENT, stream, l);
        }
        break;
      case ':':
        if (peek(l) == ':') {
          advance(l);
          l_token_append("::", TOKEN_CCOLON, stream, l);
        } else {
          l_token_append(":", TOKEN_COLON, stream, l);
        }
        break;
      case '.':
        if (peek(l) == '.' && peek_next(l) == '=') {
          advance(l);
          advance(l);
          l_token_append("..=", TOKEN_DOTDOTEQ, stream, l);
        } else if (peek(l) == '.') {
          advance(l);
          l_token_append("..", TOKEN_DOTDOT, stream, l);
        } else {
          l_token_append(".", TOKEN_DOT, stream, l);
        }
        break;
      case '@':
        l_token_append("@", TOKEN_IMPL, stream, l);
        break;
      case '?':
        l_token_append("?", TOKEN_QUESTION, stream, l);
        break;
      case '\'': {
        char tok[8] = {0};
        size_t idx = 0;

        if (peek(l) == '\'') {
          fprintf(stderr, "%s:[%zu:%zu] error: empty character literal\n",
                  l->file, l->line, l->column);
          exit(1);
        }

        if (peek(l) == '\\') {
          advance(l);
          char esc = advance(l);
          switch (esc) {
          case 'n':
            tok[idx++] = '\n';
            break;
          case 't':
            tok[idx++] = '\t';
            break;
          case 'r':
            tok[idx++] = '\r';
            break;
          case '0':
            tok[idx++] = '\0';
            break;
          case '\\':
            tok[idx++] = '\\';
            break;
          case '\'':
            tok[idx++] = '\'';
            break;
          case '"':
            tok[idx++] = '"';
            break;
          default:
            fprintf(stderr,
                    "%s:[%zu:%zu] error: unknown escape sequence '\\%c'\n",
                    l->file, l->line, l->column, esc);
            exit(1);
          }
        } else {
          tok[idx++] = advance(l);
        }

        if (peek(l) == '\0') {
          fprintf(stderr,
                  "%s:[%zu:%zu] error: unterminated character literal\n",
                  l->file, l->line, l->column);
          exit(1);
        }

        if (peek(l) != '\'') {
          fprintf(stderr,
                  "%s:[%zu:%zu] error: expected closing quote for character "
                  "literal\n",
                  l->file, l->line, l->column);
          exit(1);
        }
        advance(l);

        tok[idx] = '\0';
        l_token_append(tok, TOKEN_CHAR_LIT, stream, l);
        break;
      }
      case '"': {
        while (*l->pos != '"' && *l->pos != '\0')
          advance(l);

        if (*l->pos == '\0') {
          fprintf(stderr, "%s:[%zu:%zu] error: unterminated string literal\n",
                  l->file, l->line, l->column);
          exit(1);
        }

        size_t len = l->pos - l->start - 1;

        if (len >= 256) {
          fprintf(
              stderr,
              "%s:[%zu:%zu] error: string literal too long (max 255 chars)\n",
              l->file, l->line, l->column);
          exit(1);
        }

        char tok[256];
        memcpy(tok, l->start + 1, len);
        tok[len] = '\0';
        advance(l);
        l_token_append(tok, TOKEN_STRING_LIT, stream, l);
        break;
      }
      default:
        fprintf(stderr, "%s:[%zu:%zu] error: unexpected character '%c'\n",
                l->file, l->line, l->column, cur);
        exit(1);
      }
    }
  }
  l_token_append("", TOKEN_EOF, stream, l);
}

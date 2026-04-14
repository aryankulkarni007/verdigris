#ifndef TOKEN_H
#define TOKEN_H

#include "intern.h"
#include <stddef.h>
#include <stdint.h>

typedef struct Token Token;
typedef struct Span Span;

/* ai documentation comes to save the day */

#define TOKEN_T(X)                                                             \
  /* ── special ────────────────────────────────────────────────────────── */  \
  X(TK_ILLEGAL) /* unknown / unrecognised character                     */     \
  X(TK_EOF)     /* end of file                                          */     \
  /* ── literals ───────────────────────────────────────────────────────── */  \
  X(TK_IDENT)  /* identifier or keyword-as-ident: foo, int, let, ...   */      \
  X(TK_INT)    /* integer literal: 42, 0, 1000                         */      \
  X(TK_FLOAT)  /* float literal: 3.14, 0.5                             */      \
  X(TK_STRING) /* string literal: "hello"                              */      \
  X(TK_TRUE)   /* boolean true  (lexed as ident, reserved for later)   */      \
  X(TK_FALSE)  /* boolean false (lexed as ident, reserved for later)   */      \
  /* ── keywords ───────────────────────────────────────────────────────── */  \
  /* NOTE: all keywords are currently lexed as TK_IDENT and distinguished  */  \
  /* by the parser via intern pointer comparison. these entries are kept    */ \
  /* here as documentation and for future use if keyword lexing is added.  */  \
  X(TK_AS)        /* as    — type cast: x as float                        */   \
  X(TK_LET)       /* let   — immutable inferred binding: let x = 1        */   \
  X(TK_MUT)       /* mut   — mutable binding: mut x = 1                   */   \
  X(TK_STRUCT)    /* struct — struct declaration                          */   \
  X(TK_ENUM)      /* enum  — enum declaration                             */   \
  X(TK_FOR)       /* for   — for loop                                     */   \
  X(TK_IN)        /* in    — for i in array                               */   \
  X(TK_MATCH)     /* match — pattern match expression                     */   \
  X(TK_LOOP)      /* loop  — infinite loop                                */   \
  X(TK_WHILE)     /* while — condition loop                               */   \
  X(TK_BREAK)     /* break — exit loop                                    */   \
  X(TK_CONTINUE)  /* continue — skip to next iteration                   */    \
  X(TK_IF)        /* if    — conditional / if expression                  */   \
  X(TK_ELSE)      /* else  — else branch                                  */   \
  X(TK_RETURN)    /* return — explicit early return                       */   \
  X(TK_NULL)      /* null  — null type / null value                       */   \
  X(TK_VOID)      /* void  — unit return type                             */   \
  X(TK_SELF)      /* self  — receiver in method definitions               */   \
  X(TK_PUB)       /* pub   — public visibility modifier                   */   \
  X(TK_IMPORT)    /* import — module import                               */   \
  X(TK_INTERFACE) /* interface — structural contract definition           */   \
  X(TK_DEFER)     /* defer — deferred execution at scope exit             */   \
  X(TK_CATCH)     /* catch — inline error recovery: open()! catch |e| ... */   \
  /* ── assignment ─────────────────────────────────────────────────────── */  \
  X(TK_ASSIGN) /* =    — assignment / binding                          */      \
  /* ── comparison ─────────────────────────────────────────────────────── */  \
  X(TK_EQ)   /* ==   — equality                                      */        \
  X(TK_NEQ)  /* !=   — inequality                                    */        \
  X(TK_LT)   /* <    — less than                                     */        \
  X(TK_GT)   /* >    — greater than                                  */        \
  X(TK_LTEQ) /* <=   — less than or equal                            */        \
  X(TK_GTEQ) /* >=   — greater than or equal                        */         \
  /* ── logical ────────────────────────────────────────────────────────── */  \
  X(TK_AND)  /* &&   — logical and                                   */        \
  X(TK_OR)   /* ||   — logical or                                    */        \
  X(TK_BANG) /* !    — logical not / error propagation suffix        */        \
  /* ── arithmetic ─────────────────────────────────────────────────────── */  \
  X(TK_PLUS)   /* +    — addition                                      */      \
  X(TK_MINUS)  /* -    — subtraction / negation                        */      \
  X(TK_STAR)   /* *    — multiplication / pointer dereference          */      \
  X(TK_SLASH)  /* /    — division                                      */      \
  X(TK_MODULO) /* %    — modulo                                        */      \
  /* ── compound assignment ────────────────────────────────────────────── */  \
  X(TK_PLUSEQ)   /* +=   — add and assign                                */    \
  X(TK_MINUSEQ)  /* -=   — subtract and assign                           */    \
  X(TK_STAREQ)   /* *=   — multiply and assign                           */    \
  X(TK_SLASHEQ)  /* /=   — divide and assign                             */    \
  X(TK_MODULOEQ) /* %=   — modulo and assign                             */    \
  /* ── delimiters ─────────────────────────────────────────────────────── */  \
  X(TK_OPAREN) /* (    — open parenthesis                              */      \
  X(TK_CPAREN) /* )    — close parenthesis                             */      \
  X(TK_OBRACE) /* {    — open brace                                    */      \
  X(TK_CBRACE) /* }    — close brace                                   */      \
  X(TK_OBRACK) /* [    — open bracket                                  */      \
  X(TK_CBRACK) /* ]    — close bracket                                 */      \
  /* ── punctuation ────────────────────────────────────────────────────── */  \
  X(TK_SEMI)     /* ;    — statement terminator                          */    \
  X(TK_COMMA)    /* ,    — separator                                     */    \
  X(TK_DOT)      /* .    — field access / method call                    */    \
  X(TK_DDOT)     /* ..   — range operator: 0..10                         */    \
  X(TK_DDOTEQ)   /* ..=  — inclusive range: 0..=10                      */     \
  X(TK_COL)      /* :    — struct field separator in literals: { x: 0 }  */    \
  X(TK_CCOL)     /* ::   — namespace / method path: Point::distance      */    \
  X(TK_AMP)      /* &    — reference / slice prefix: &v, &[int]          */    \
  X(TK_PIPE)     /* |    — union return type: int | error                */    \
  X(TK_PIPELINE) /* >>   — left-to-right pipeline composition            */    \
  X(TK_ARROW)    /* ->   — function return type: add(a, b) -> int        */    \
  X(TK_FATARROW) /* =>   — match arm: 0 => 1                             */    \
  X(TK_UND)      /* _    — wildcard / discard pattern in match           */    \
  /* ── attributes ─────────────────────────────────────────────────────── */  \
  X(TK_AT) /* @    — attribute prefix: @inline, @extern            */          \
  /* ── optional / error handling ──────────────────────────────────────── */  \
  X(TK_QUESTION) /* ?    — optional type suffix: ?int                    */    \
  X(TK_ERROR)    /* for error handling */

typedef enum {
#define AS_ENUM(name) name,
  TOKEN_T(AS_ENUM)
#undef AS_ENUM
} TK_T;

static const char *tk_names[] = {
#define AS_STRING(name) #name,
    TOKEN_T(AS_STRING)
#undef AS_STRING
};

static inline const char *tk_name(TK_T type) { return tk_names[type]; }

struct Span {
  size_t start;
  size_t end;
};

struct Token {
  Span span;
  TK_T type;
  uint32_t line; // for error reporting
  size_t line_start_pos;

  struct Trivia *leading;
  size_t leading_count;
  InternID id;
};

void print_token(Token *t, const char *source_buffer);
Token new_token(Span span, TK_T type, uint32_t line, size_t line_start_pos);

#endif // TOKEN_H

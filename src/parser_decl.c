#include "../include/ast.h"
#include "../include/parser.h"
#include <stdio.h>
#include <stdlib.h>

Decl *parse_extern_decl(Parser *p, Arena *a) {
  Token extern_token = CURRENT(p);
  ADVANCE(p);

  EXPECT(p, TOKEN_LBRACE, "expected '{' after extern");

  Decl *local_decls[256];
  size_t decl_count = 0;

  while (CURRENT(p).ttype != TOKEN_RBRACE && CURRENT(p).ttype != TOKEN_EOF) {
    Decl *func = parse_func_decl(p, a);
    local_decls[decl_count++] = func;
  }

  EXPECT(p, TOKEN_RBRACE, "expected '}' after extern block");

  Decl **decls = NULL;
  if (decl_count > 0) {
    decls = arena_allocate(a, decl_count * sizeof(Decl *));
    for (size_t i = 0; i < decl_count; i++)
      decls[i] = local_decls[i];
  }

  return ast_decl_extern(a, extern_token, decls, decl_count);
}

Decl *parse_impl_decl(Parser *p, Arena *a) {
  Token at_token = CURRENT(p);
  ADVANCE(p);

  Type *target_type = parse_type(p, a);

  EXPECT(p, TOKEN_LBRACE, "expected '{' after impl target type");

  Decl *local_methods[256];
  size_t method_count = 0;
  while (CURRENT(p).ttype != TOKEN_RBRACE && CURRENT(p).ttype != TOKEN_EOF) {
    Decl *method = parse_func_decl(p, a);
    local_methods[method_count++] = method;
  }

  EXPECT(p, TOKEN_RBRACE, "expected '}' after impl body");

  Decl **methods = NULL;
  if (method_count > 0) {
    methods = arena_allocate(a, method_count * sizeof(Decl *));
    for (size_t i = 0; i < method_count; i++)
      methods[i] = local_methods[i];
  }

  return ast_decl_impl(a, at_token, target_type, methods, method_count);
}

Decl *parse_func_decl(Parser *p, Arena *a) {
  Token func_token = CURRENT(p);
  if (func_token.ttype != TOKEN_IDENT) {
    fprintf(stderr, "error at %zu:%zu: expected function name, got '%s'\n",
            func_token.line, func_token.column, func_token.token);
    exit(1);
  }
  char *func_name = func_token.token;
  ADVANCE(p);

  EXPECT(p, TOKEN_LPAREN, "expected '(' after function name");

  char *local_names[256];
  size_t name_count = 0;

  if (CURRENT(p).ttype != TOKEN_RPAREN) {
    while (1) {
      if (CURRENT(p).ttype != TOKEN_IDENT) {
        fprintf(stderr, "error at %zu:%zu: expected parameter name, got '%s'\n",
                CURRENT(p).line, CURRENT(p).column, CURRENT(p).token);
        exit(1);
      }
      local_names[name_count++] = CURRENT(p).token;
      ADVANCE(p);

      if (CURRENT(p).ttype == TOKEN_RPAREN)
        break;
      EXPECT(p, TOKEN_COMMA, "expected ',' between parameters");
    }
  }
  ADVANCE(p);

  EXPECT(p, TOKEN_CCOLON, "expected '::' after parameter list");

  EXPECT(p, TOKEN_LPAREN, "expected '(' for parameter types");

  Type *local_types[256];
  size_t type_count = 0;

  if (CURRENT(p).ttype != TOKEN_RPAREN) {
    while (1) {
      local_types[type_count++] = parse_type(p, a);

      if (CURRENT(p).ttype == TOKEN_RPAREN)
        break;
      EXPECT(p, TOKEN_COMMA, "expected ',' between parameter types");
    }
  }
  ADVANCE(p);

  if (name_count != type_count) {
    fprintf(
        stderr,
        "error at %zu:%zu: parameter name count (%zu) != type count (%zu)\n",
        func_token.line, func_token.column, name_count, type_count);
    exit(1);
  }

  EXPECT(p, TOKEN_ARROW, "expected '->' before return type");
  Type *return_type = parse_type(p, a);

  Stmt *body = NULL;
  if (CURRENT(p).ttype == TOKEN_LBRACE) {
    body = parse_block_stmt(p, a);
  }

  Param *params = NULL;
  if (name_count > 0) {
    params = arena_allocate(a, name_count * sizeof(Param));
    for (size_t i = 0; i < name_count; i++) {
      params[i].name = local_names[i];
      params[i].type = local_types[i];
    }
  }

  return ast_decl_func(a, func_token, func_name, params, name_count,
                       return_type, body);
}

Decl *parse_enum_decl(Parser *p, Arena *a) {
  Token enum_token = CURRENT(p);
  ADVANCE(p);
  if (CURRENT(p).ttype != TOKEN_IDENT) {
    fprintf(stderr,
            "error at %zu:%zu: expected enum name after 'enum', got '%s'\n",
            CURRENT(p).line, CURRENT(p).column, CURRENT(p).token);
    exit(1);
  }

  Token name_token = CURRENT(p);
  char *name = name_token.token;
  ADVANCE(p);

  EXPECT(p, TOKEN_LBRACE, "expected '{' after enum name");

  Variant local_variants[256];
  size_t variant_count = 0;

  while (CURRENT(p).ttype != TOKEN_RBRACE && CURRENT(p).ttype != TOKEN_EOF) {
    if (variant_count >= 256) {
      fprintf(stderr, "error at %zu:%zu: enum exceeds 256 variants\n",
              enum_token.line, enum_token.column);
      exit(1);
    }
    if (CURRENT(p).ttype != TOKEN_IDENT) {
      fprintf(stderr, "error at %zu:%zu: expected variant name, got '%s'\n",
              CURRENT(p).line, CURRENT(p).column, CURRENT(p).token);
      exit(1);
    }

    Token variant_name = CURRENT(p);
    ADVANCE(p);
    Type *payload = NULL;
    if (CURRENT(p).ttype == TOKEN_LPAREN) {
      ADVANCE(p);
      payload = parse_type(p, a);
      EXPECT(p, TOKEN_RPAREN, "expected ')' after variant payload");
    }

    local_variants[variant_count].name = variant_name.token;
    local_variants[variant_count].payload = payload;
    variant_count++;

    if (CURRENT(p).ttype == TOKEN_COMMA) {
      ADVANCE(p);
    } else if (CURRENT(p).ttype != TOKEN_RBRACE) {
      fprintf(stderr, "error at %zu:%zu: expected ',' or '}' after variant\n",
              CURRENT(p).line, CURRENT(p).column);
      exit(1);
    }
  }

  EXPECT(p, TOKEN_RBRACE, "expected '}' after enum body");

  Variant *variants = NULL;
  if (variant_count > 0) {
    variants = arena_allocate(a, variant_count * sizeof(Variant));
    for (size_t i = 0; i < variant_count; i++)
      variants[i] = local_variants[i];
  }

  return ast_decl_enum(a, enum_token, name, variants, variant_count);
}

Decl *parse_struct_decl(Parser *p, Arena *a) {
  Token struct_token = CURRENT(p);
  ADVANCE(p);

  if (CURRENT(p).ttype != TOKEN_IDENT) {
    fprintf(stderr,
            "error at %zu:%zu: expected struct name after 'struct', got '%s'\n",
            CURRENT(p).line, CURRENT(p).column, CURRENT(p).token);
    exit(1);
  }
  Token name_token = CURRENT(p);
  char *name = name_token.token;
  ADVANCE(p);

  EXPECT(p, TOKEN_LBRACE, "expected '{' after struct name");

  Field local_fields[256];
  size_t field_count = 0;

  while (CURRENT(p).ttype != TOKEN_RBRACE && CURRENT(p).ttype != TOKEN_EOF) {
    if (field_count >= 256) {
      fprintf(stderr, "error at %zu:%zu: struct exceeds 256 fields\n",
              struct_token.line, struct_token.column);
      exit(1);
    }

    if (CURRENT(p).ttype != TOKEN_IDENT) {
      fprintf(stderr, "error at %zu:%zu: expected field name, got '%s'\n",
              CURRENT(p).line, CURRENT(p).column, CURRENT(p).token);
      exit(1);
    }
    Token field_name = CURRENT(p);
    ADVANCE(p);

    EXPECT(p, TOKEN_COLON, "expected ':' after field name");

    Type *field_type = parse_type(p, a);

    local_fields[field_count].name = field_name.token;
    local_fields[field_count].type = field_type;
    ++field_count;

    if (CURRENT(p).ttype == TOKEN_COMMA) {
      ADVANCE(p);
    } else if (CURRENT(p).ttype != TOKEN_RBRACE) {
      fprintf(stderr, "error at %zu:%zu: expected ',' or '}' after field\n",
              CURRENT(p).line, CURRENT(p).column);
      exit(1);
    }
  }

  EXPECT(p, TOKEN_RBRACE, "expected '}' after struct body");

  Field *fields = NULL;
  if (field_count > 0) {
    fields = arena_allocate(a, field_count * sizeof(Field));
    for (size_t i = 0; i < field_count; i++)
      fields[i] = local_fields[i];
  }

  return ast_decl_struct(a, struct_token, name, fields, field_count);
}

Decl *parse_decl(Parser *p, Arena *a) {
  switch (CURRENT(p).ttype) {
  case TOKEN_STRUCT:
    return parse_struct_decl(p, a);
  case TOKEN_ENUM:
    return parse_enum_decl(p, a);
  case TOKEN_IMPL:
    return parse_impl_decl(p, a);
  case TOKEN_EXTERN:
    return parse_extern_decl(p, a);
  case TOKEN_IDENT:
    return parse_func_decl(p, a);
  default:
    fprintf(stderr, "error at %zu:%zu: unexpected token '%s' in declaration\n",
            CURRENT(p).line, CURRENT(p).column, CURRENT(p).token);
    exit(1);
  }
}

Module *parse(Arena *arena, Parser *p) {
  Decl *local_decls[1024];
  size_t decl_count = 0;

  while (!IS_AT_END(p)) {
    if (decl_count >= 1024) {
      fprintf(stderr, "error: module exceeds 1024 declarations\n");
      exit(1);
    }
    Decl *decl = parse_decl(p, arena);
    local_decls[decl_count++] = decl;
  }

  Decl **decls = arena_allocate(arena, decl_count * sizeof(Decl *));
  for (size_t i = 0; i < decl_count; i++) {
    decls[i] = local_decls[i];
  }

  Module *mod = arena_allocate(arena, sizeof(Module));
  mod->declarations = decls;
  mod->count = decl_count;
  return mod;
}

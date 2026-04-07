#include "../include/ast.h"
#include "../include/parser.h"
#include <stdio.h>
#include <stdlib.h>

Decl *parse_extern_decl(Parser *p, Arena *a) {
  Token extern_token = CURRENT(p);
  ADVANCE(p); // consume 'extern'

  EXPECT(p, TOKEN_LBRACE, "expected '{' after extern");

  Decl *local_decls[256];
  size_t decl_count = 0;

  while (CURRENT(p).ttype != TOKEN_RBRACE && CURRENT(p).ttype != TOKEN_EOF) {
    // parse_func_decl will see no '{' and set body = NULL
    Decl *func = parse_func_decl(p, a);
    local_decls[decl_count++] = func;
  }

  EXPECT(p, TOKEN_RBRACE, "expected '}' after extern block");

  // Copy to arena
  Decl **decls = arena_allocate(a, decl_count * sizeof(Decl *));
  for (size_t i = 0; i < decl_count; ++i) {
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

  Decl **methods = arena_allocate(a, method_count * sizeof(Decl *));
  for (size_t i = 0; i < method_count; i++) {
    methods[i] = local_methods[i];
  }
  return ast_decl_impl(a, at_token, target_type, methods, method_count);
}

Decl *parse_func_decl(Parser *p, Arena *a) {
  // function name
  Token func_token = CURRENT(p);
  if (func_token.ttype != TOKEN_IDENT) {
    fprintf(stderr, "error: expected function name\n");
    exit(1);
  }
  char *func_name = func_token.token;
  ADVANCE(p); // consume name

  // parameter names
  EXPECT(p, TOKEN_LPAREN, "expected '(' after function name");

  char *local_names[256];
  size_t name_count = 0;

  if (CURRENT(p).ttype != TOKEN_RPAREN) {
    while (1) {
      if (CURRENT(p).ttype != TOKEN_IDENT) {
        fprintf(stderr, "error: expected parameter name\n");
        exit(1);
      }
      local_names[name_count++] = CURRENT(p).token;
      ADVANCE(p); // consume name

      if (CURRENT(p).ttype == TOKEN_RPAREN)
        break;
      EXPECT(p, TOKEN_COMMA, "expected ',' between parameters");
    }
  }
  ADVANCE(p); // consume ')'

  // double colon
  EXPECT(p, TOKEN_CCOLON, "expected '::' after parameter list");

  // parameter types
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
  ADVANCE(p); // consume ')'

  // check counts match
  if (name_count != type_count) {
    fprintf(stderr, "error: parameter name count (%zu) != type count (%zu)\n",
            name_count, type_count);
    exit(1);
  }

  // return type
  EXPECT(p, TOKEN_ARROW, "expected '->' before return type");
  Type *return_type = parse_type(p, a);

  // body (optional — if no '{', this is a signature only)
  Stmt *body = NULL;
  if (CURRENT(p).ttype == TOKEN_LBRACE) {
    body = parse_block_stmt(p, a);
  }

  // pair names and types into param array
  Param *params = arena_allocate(a, name_count * sizeof(Param));
  for (size_t i = 0; i < name_count; i++) {
    params[i].name = local_names[i];
    params[i].type = local_types[i];
  }

  return ast_decl_func(a, func_token, func_name, params, name_count,
                       return_type, body);
}

Decl *parse_enum_decl(Parser *p, Arena *a) {
  Token enum_token = CURRENT(p);
  ADVANCE(p);
  if (CURRENT(p).ttype != TOKEN_IDENT) {
    fprintf(stderr, "error: expected enum name after 'enum'\n");
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
      fprintf(stderr, "error: enum exceeds 256 variants\n");
      exit(1);
    }
    if (CURRENT(p).ttype != TOKEN_IDENT) {
      fprintf(stderr, "error: expected variant name\n");
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
      fprintf(stderr, "error: expected ',' or '}' after variant\n");
      exit(1);
    }
  }

  EXPECT(p, TOKEN_RBRACE, "expected '}' after enum body");

  // Copy variants to arena
  Variant *variants = arena_allocate(a, variant_count * sizeof(Variant));
  for (size_t i = 0; i < variant_count; i++) {
    variants[i] = local_variants[i];
  }

  return ast_decl_enum(a, enum_token, name, variants, variant_count);
}

Decl *parse_struct_decl(Parser *p, Arena *a) {
  Token struct_token = CURRENT(p);
  ADVANCE(p);

  if (CURRENT(p).ttype != TOKEN_IDENT) {
    fprintf(stderr, "error: expected struct name after 'struct'\n");
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
      fprintf(stderr, "error: struct exceeds 256 fields\n");
      exit(1);
    }

    if (CURRENT(p).ttype != TOKEN_IDENT) {
      fprintf(stderr, "error: expected field name\n");
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
      fprintf(stderr, "error: expected ',' or '}' after field\n");
      exit(1);
    }
  }

  EXPECT(p, TOKEN_RBRACE, "expected '}' after struct body");

  // copy fields to arena
  Field *fields = arena_allocate(a, field_count * sizeof(Field));
  for (size_t i = 0; i < field_count; i++)
    fields[i] = local_fields[i];

  return ast_decl_struct(a, struct_token, name, fields, field_count);
}

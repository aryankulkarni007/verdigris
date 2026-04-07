#ifndef MAIN_H
#define MAIN_H

#include <stddef.h>

typedef struct {
  size_t file_size;
  char *buffer;
} Source;

void run_ast_tests(void);
void run_parser_tests(void);

#endif //  MAIN_H

#ifndef MAIN_H
#define MAIN_H

#include <stddef.h>

typedef struct {
  size_t file_size;
  const char *buffer;
  const char *file_path;
} Source;

#endif // MAIN_H

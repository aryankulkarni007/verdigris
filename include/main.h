#pragma once

#include <stddef.h>

typedef struct {
  size_t file_size;
  const char *buffer;
  const char *file_path;
} Source;

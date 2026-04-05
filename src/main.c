#include <stdio.h>
#include <stdlib.h>

void print_usage(char *exec);
char *file_handler(char *path);

/// WARNING: remember to free buffer
void print_usage(char *exec) {
  printf("usage: %s <*.koz>", exec);
  return;
}

char *file_handler(char *path) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    fprintf(stderr, "error: fopen\n");
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);
  if (file_size < 0) {
    fprintf(stderr, "error: ftell\n");
  }
  rewind(file);

  char *buffer = malloc(file_size + 1);
  if (!buffer) {
    fprintf(stderr, "error: file buffer malloc\n");
    return NULL;
  }

  size_t bytes_read = fread(buffer, 1, file_size, file);
  if (bytes_read != file_size) {
    fprintf(stderr, "error: fread\n");
    return NULL;
  }

  buffer[file_size] = '\0';
  return buffer;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  char *buffer = file_handler(argv[1]);
  printf("%s\n", buffer);
  return 0;
}

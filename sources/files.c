#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "../externals/stb/stb_image.h"

#include "../includes/defs.h"
void terminate(const char *message, ...);

unsigned getFileSizeIO(FILE *file) {
  struct stat info;
  fstat(fileno(file), &info);
  return info.st_size;
}

char* readFileIO(const char *path) {
  FILE* file = fopen(path, "r");
  unsigned size = getFileSizeIO(file), read;
  char* buffer = malloc(size + 1);
  if((read = fread(buffer, 1, size, file)) != size) terminate("Reading file %s failed (%d/%d), fatal\n", path, read, size);
  buffer[size] = '\0';
  return buffer;
}

unsigned char* loadImageIO(const char* path, int *width, int *height) {
  unsigned char *data = stbi_load(path, width, height, NULL, 4);
  if(data == NULL) terminate("%s\n", stbi_failure_reason());
  return data;
}

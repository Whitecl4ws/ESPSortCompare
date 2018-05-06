#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../externals/glad/glad.h"

#include "../includes/defs.h"
void terminate(const char *message, ...);
#include "../includes/algorithms.h"
#include "../includes/files.h"

uint32_t createShaderOGL(int32_t type, const char* path) {
  const char* source = readFileIO(path);
  uint32_t shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  free((char*)source);
  glCompileShader(shader);
  char infoLog[1024];
  glGetShaderInfoLog(shader, 1024, NULL, infoLog);
  if(infoLog[0] != '\0') terminate("%s\n%s", path, infoLog);
  return shader;
}

int32_t getUniformLocationOGL(uint32_t program, const char *name) {
  int32_t uniform = glGetUniformLocation(program, name);
  if(uniform == -1) terminate("Uniform %s inactive/inexistent, fatal\n", name);
  return uniform;
}

int32_t setVertexAttributeOGL(uint32_t program, const char *name, uint32_t size, uint32_t stride, const void *offset) {
  int32_t attribute = glGetAttribLocation(program, name);
  if(attribute == -1) terminate("Attribute %s of size %d inactive/inexistent, fatal\n", name, size);
  glEnableVertexAttribArray(attribute);
  glVertexAttribPointer(attribute, size, GL_FLOAT, GL_FALSE, stride, offset);
  return attribute;
}

uint32_t createProgramOGL(const char *vertexsh, const char *fragmentsh) {
  uint32_t shaders[2] = {
    createShaderOGL(GL_VERTEX_SHADER, vertexsh),
    createShaderOGL(GL_FRAGMENT_SHADER, fragmentsh)};
  uint32_t program = glCreateProgram();
  for(int i = 0; i < StaticArrayLength(shaders); ++i) glAttachShader(program, shaders[i]);
  glLinkProgram(program);
  for(int i = 0; i < StaticArrayLength(shaders); ++i) {
    glDetachShader(program, shaders[i]);
    glDeleteShader(shaders[i]);}
  return program;
}

uint32_t createTextureOGL(int32_t type, const char *path, int *width, int *height) {
  unsigned char* data = loadImageIO(path, width, height);
  uint32_t texture;
  glGenTextures(1, &texture);
  glBindTexture(type, texture);
  glTexImage2D(type, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  free(data);
  return texture;
}

void createSquareShapeOGL(SquareShapeOGL* shape) {
  const uint32_t indices[] = {0, 1, 2, 0, 3, 2};
  const float vertices[] = {0, 0, 1, 0, 1, -1, 0, -1};
  shape->count = StaticArrayLength(indices);
  glGenBuffers(1, &shape->indices);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape->indices);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  glGenBuffers(1, &shape->vertices);
  glBindBuffer(GL_ARRAY_BUFFER, shape->vertices);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}

uint32_t generateSurfacesVBOGL(uint32_t program, uint32_t capacity) {
  uint32_t surfaces;
  glGenBuffers(1, &surfaces);
  glBindBuffer(GL_ARRAY_BUFFER, surfaces);
  glBufferData(GL_ARRAY_BUFFER, capacity * sizeof(struct Surface), NULL, GL_STREAM_DRAW);
  glVertexAttribDivisor(setVertexAttributeOGL(program, "position", 2, sizeof(struct Surface), StructureVariableOffset(struct Surface, position)), 1);
  glVertexAttribDivisor(setVertexAttributeOGL(program, "pivot", 2, sizeof(struct Surface), StructureVariableOffset(struct Surface, pivot)), 1);
  glVertexAttribDivisor(setVertexAttributeOGL(program, "size", 2, sizeof(struct Surface), StructureVariableOffset(struct Surface, size)), 1);
  glVertexAttribDivisor(setVertexAttributeOGL(program, "rotation", 2, sizeof(struct Surface), StructureVariableOffset(struct Surface, rotation)), 1);
  return surfaces;
}

void createSurfaceRendererAndBufferCOGL(SurfaceRendererOGL* renderer, SurfaceBufferOGL* buffer, uint32_t program, uint32_t capacity) {
  memset(renderer, 0, sizeof(SurfaceRendererOGL));
  glGenVertexArrays(1, &renderer->configuration);
  glBindVertexArray(renderer->configuration);
  createSquareShapeOGL(&renderer->primitive);
  setVertexAttributeOGL(program, "point", 2, 0, 0);

  memset(buffer, 0, sizeof(SurfaceBufferOGL));
  renderer->surfaces = generateSurfacesVBOGL(program, capacity);
  buffer->capacity = capacity;
  buffer->surfaces = malloc(capacity * sizeof(struct Surface));

  glGenBuffers(1, &renderer->extra);
  glBindBuffer(GL_ARRAY_BUFFER, renderer->extra);
  glBufferData(GL_ARRAY_BUFFER, capacity * sizeof(Vec4f), NULL, GL_STREAM_DRAW);
  glVertexAttribDivisor(setVertexAttributeOGL(program, "color", 4, 0, 0), 1);
  buffer->extra = malloc(capacity * sizeof(Vec4f));
  buffer->stride = sizeof(Vec4f);
  glBindVertexArray(0);
}

void createSurfaceRendererAndBufferFOGL(SurfaceRendererOGL* renderer, SurfaceBufferOGL* buffer, uint32_t program, uint32_t capacity) {
  memset(renderer, 0, sizeof(SurfaceRendererOGL));
  glGenVertexArrays(1, &renderer->configuration);
  glBindVertexArray(renderer->configuration);
  createSquareShapeOGL(&renderer->primitive);
  setVertexAttributeOGL(program, "point", 2, 0, 0);

  memset(buffer, 0, sizeof(SurfaceBufferOGL));
  renderer->surfaces = generateSurfacesVBOGL(program, capacity);
  buffer->capacity = capacity;
  buffer->surfaces = malloc(capacity * sizeof(struct Surface));

  glGenBuffers(1, &renderer->extra);
  glBindBuffer(GL_ARRAY_BUFFER, renderer->extra);
  glBufferData(GL_ARRAY_BUFFER, capacity * 2 * sizeof(Vec2f), NULL, GL_STREAM_DRAW);
  glVertexAttribDivisor(setVertexAttributeOGL(program, "uv", 2, 2 * sizeof(Vec2f), 0), 1);
  glVertexAttribDivisor(setVertexAttributeOGL(program, "texsize", 2, 2 * sizeof(Vec2f), (void*) sizeof(Vec2f)), 1);
  buffer->extra = malloc(capacity * 2 * sizeof(Vec2f));
  buffer->stride = 2 * sizeof(Vec2f);
  glBindVertexArray(0);
}

void surfaceBufferAppendString(SurfaceBufferOGL* buffer, FontWrapper* font, float x, float y, float scale, char* string) {
  const uint32_t length = strlen(string);
  if(buffer->count + length > buffer->capacity) terminate("Surface buffer overflow, fatal\n");
  Vec2f pointer = {x, y};
  Vec4f *atlas = buffer->extra;
  for(int i = 0; i < length; ++i) {
    if(string[i] == '\n') {
      pointer[1] += font->jump * scale;
      pointer[0] = x; continue; }
    Character character = font->characters[font->characterArrayIndexMapper(string[i])];
    if(string[i] == ' ') {
      pointer[0] += character.advanceX * scale; continue; }
    int index = buffer->count++;
    buffer->surfaces[index].position[0] = pointer[0] - character.originX * scale;
    buffer->surfaces[index].position[1] = pointer[1] + (character.height - character.originY) * scale;
    buffer->surfaces[index].size[0] = character.width * scale;
    buffer->surfaces[index].size[1] = character.height * scale;
    buffer->surfaces[index].pivot[0] = 0;
    buffer->surfaces[index].pivot[1] = 0;
    buffer->surfaces[index].rotation[0] = 0;
    buffer->surfaces[index].rotation[1] = 0;
    atlas[index][0] = character.x;
    atlas[index][1] = character.y;
    atlas[index][2] = character.width / font->width;
    atlas[index][3] = character.height / font->height;
    pointer[0] += character.advanceX * scale * 1.1;
  }
}

void surfaceBufferRenderOGL(SurfaceRendererOGL* renderer, SurfaceBufferOGL* buffer) {
  if(buffer->invariability == 0 && buffer->count) {
    glBindBuffer(GL_ARRAY_BUFFER, renderer->surfaces);
    glBufferSubData(GL_ARRAY_BUFFER, 0, buffer->count * sizeof(struct Surface), buffer->surfaces);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->extra);
    glBufferSubData(GL_ARRAY_BUFFER, 0, buffer->count * sizeof(Vec4f), buffer->extra);
  }

  glBindVertexArray(renderer->configuration);
  glDrawElementsInstanced(GL_TRIANGLES, renderer->primitive.count, GL_UNSIGNED_INT, NULL, buffer->count);
  if(buffer->invariability == 0) buffer->count = 0;
}

void surfaceBufferLoadColorSpectrumBand(SurfaceBufferOGL* buffer, float padding, float y, float height) {
  Vec4f* colors = buffer->extra;
  buffer->count = buffer->capacity;
  uint32_t length = buffer->capacity - padding;
  for(int index = padding; index < length; ++index) {
    buffer->surfaces[index].position[0] = index;
    buffer->surfaces[index].position[1] = y;
    buffer->surfaces[index].pivot[0] = 0;
    buffer->surfaces[index].pivot[1] = 0;
    buffer->surfaces[index].size[0] = 1;
    buffer->surfaces[index].size[1] = height;
    buffer->surfaces[index].rotation[0] = 0;
    buffer->surfaces[index].rotation[1] = index + 1;
    colors[index][3] = 1;

    float hue = (float) index / length * 6;
    float n = 1 - fabs(fmod(hue, 2) - 1);

    if (hue >= 0 && hue <= 1) {
      colors[index][0] = 1;
      colors[index][1] = n;
      colors[index][2] = 0;
    } else if (hue >= 1 && hue <= 2) {
      colors[index][0] = n;
      colors[index][1] = 1;
      colors[index][2] = 0;
    } else if (hue >= 2 && hue <= 3) {
      colors[index][0] = 0;
      colors[index][1] = 1;
      colors[index][2] = n;
    } else if (hue >= 3 && hue <= 4) {
      colors[index][0] = 0;
      colors[index][1] = n;
      colors[index][2] = 1;
    } else if (hue >= 4 && hue <= 5) {
      colors[index][0] = n;
      colors[index][1] = 0;
      colors[index][2] = 1;
    } else if (hue >= 5 && hue <= 6) {
      colors[index][0] = 1;
      colors[index][1] = 0;
      colors[index][2] = n;
    }
  }
}

void surfaceBufferShuffle(SurfaceBufferOGL* buffer) {
  /* Follows the Fisher-Yates Shuffling Algorithm! */
  double x, y, h;
  for(uint32_t index = buffer->count; index > 0; --index) {
    unsigned new = getRandomIntegerALG(index);
    if(new == index) continue;
    x = buffer->surfaces[index].position[0];
    y = buffer->surfaces[index].position[1];
    h = buffer->surfaces[index].rotation[1];
    buffer->surfaces[index].position[0] = buffer->surfaces[new].position[0];
    buffer->surfaces[index].position[1] = buffer->surfaces[new].position[1];
    buffer->surfaces[index].rotation[1] = buffer->surfaces[new].rotation[1];
    buffer->surfaces[new].position[0] = x;
    buffer->surfaces[new].position[1] = y;
    buffer->surfaces[new].rotation[1] = h;
  }
}

/* Your implementation must take care of lower bound checking! */
uint32_t surfaceBufferShuffleOnce(SurfaceBufferOGL* buffer, uint32_t index) {
  double x, y, h;
  unsigned new = getRandomIntegerALG(index);
  if(new == index) return index - 1;
  x = buffer->surfaces[index].position[0];
  y = buffer->surfaces[index].position[1];
  h = buffer->surfaces[index].rotation[1];
  buffer->surfaces[index].position[0] = buffer->surfaces[new].position[0];
  buffer->surfaces[index].position[1] = buffer->surfaces[new].position[1];
  buffer->surfaces[index].rotation[1] = buffer->surfaces[new].rotation[1];
  buffer->surfaces[new].position[0] = x;
  buffer->surfaces[new].position[1] = y;
  buffer->surfaces[new].rotation[1] = h;
  if(!x) {
    printf("Wow! %d %d %f %f %f %f\n", index, new,
    buffer->surfaces[index].position[0],
    buffer->surfaces[index].position[1],
    buffer->surfaces[new].position[0],
    buffer->surfaces[new].position[1]);
  }
  return index - 1;
}

void destroySurfaceRenderer(SurfaceRendererOGL* renderer) {
  glDeleteBuffers(1, &renderer->extra);
  glDeleteBuffers(1, &renderer->surfaces);
  glDeleteBuffers(1, &renderer->primitive.vertices);
  glDeleteBuffers(1, &renderer->primitive.indices);
  glDeleteVertexArrays(1, &renderer->configuration);
}

void destroySurfaceBuffer(SurfaceBufferOGL* buffer) {
  free(buffer->extra);
  free(buffer->surfaces);
}

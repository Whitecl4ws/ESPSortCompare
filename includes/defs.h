#ifndef HEADERS_DEFS
#define HEADERS_DEFS
#include <stdint.h>
#define packed __attribute__((packed))
#define StaticArrayLength(array) (sizeof(array)/sizeof(array[0]))
#define StructureVariableOffset(struct, var) ((const void*) offsetof(struct, var))
typedef float Vec2f[2];
typedef float Vec3f[3];
typedef float Vec4f[4];
typedef float Mat9f[9];
typedef float Mat16f[16];
typedef uint32_t VBOGL;
typedef uint32_t VAOGL;

typedef struct {
  packed struct Surface {
    Vec2f position, pivot, size, rotation;
  } *surfaces;
  uint32_t stride, count, capacity;
  void *extra;
  char invariability;
} SurfaceBufferOGL;

typedef struct {
  VBOGL vertices, indices;
  uint32_t count;
} SquareShapeOGL;

typedef struct {
  SquareShapeOGL primitive;
  VAOGL configuration;
  VBOGL surfaces, extra;
} SurfaceRendererOGL;

typedef struct {
  char inputKeyboardS, inputKeyboardA;
  char timerActive, sortState;
  uint32_t shuffleIndex;
  double time, lastShuffleTime, lastSortTime;
  char *sortingAlgorithmLabel, sortingAlgorithm;
} ContextInformation;

typedef struct  {
  int value;
  float x, y, width, height, originX, originY, advanceX;
} Character;

typedef struct {
  float width, height, jump;
  uint32_t texture;
  const Character* characters;
  int (*characterArrayIndexMapper)(char);
} FontWrapper;


typedef struct {
  int beg, end, done;
} QuickSortIteration;
#endif

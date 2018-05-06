#ifndef HEADER_RENDER
#define HEADER_RENDER
#include "defs.h"
int32_t getUniformLocationOGL(uint32_t program, const char *name);
int32_t setVertexAttributeOGL(uint32_t program, const char *name, uint32_t size, uint32_t stride, const void *offset);
uint32_t createProgramOGL(const char *vertexsh, const char *fragmentsh);
uint32_t createTextureOGL(int32_t type, const char *path, int *width, int *height);
void createSurfaceRendererAndBufferCOGL(SurfaceRendererOGL* renderer, SurfaceBufferOGL* buffer, uint32_t program, uint32_t capacity);
void createSurfaceRendererAndBufferFOGL(SurfaceRendererOGL* renderer, SurfaceBufferOGL* buffer, uint32_t program, uint32_t capacity);
void surfaceBufferAppendString(SurfaceBufferOGL* buffer, FontWrapper* font, float x, float y, float scale, char* string);
void surfaceBufferRenderOGL(SurfaceRendererOGL* renderer, SurfaceBufferOGL* buffer);
void surfaceBufferLoadColorSpectrumBand(SurfaceBufferOGL* buffer, float padding, float y, float height);
void surfaceBufferShuffle(SurfaceBufferOGL* buffer);
void surfacesSwap(struct Surface *, struct Surface *);
uint32_t surfaceBufferShuffleOnce(SurfaceBufferOGL* buffer, uint32_t index);
void destroySurfaceRenderer(SurfaceRendererOGL* renderer);
void destroySurfaceBuffer(SurfaceBufferOGL* buffer);
#endif

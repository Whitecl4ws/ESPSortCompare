#include <math.h>
#include <string.h>

#include "../includes/math.h"

void createOrthographicProjectionMat16f(Mat16f out, float left, float right, float bottom, float top, float near, float far) {
  memset(out, 0, sizeof(Mat16f));
  const float sx = left - right,
  sy = bottom - top,
  sz = near - far;
  out[0]  = -2 / sx;
  out[5]  = -2 / sy;
  out[10] = 2 / sz;
  out[15] = 1;
  out[12] = (right + left) / sx;
  out[13] = (top + bottom) / sy;
  out[14] = (far + near) / sz;
}

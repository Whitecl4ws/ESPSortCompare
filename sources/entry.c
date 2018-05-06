#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../externals/glad/glad.h"
#include <unistd.h>

#include <GLFW/glfw3.h>

#include "../includes/defs.h"
#include "../includes/fonts.h"
#include "../includes/math.h"
#include "../includes/render.h"
#include "../includes/algorithms.h"

void terminate(const char *message, ...) {
  va_list arguments;
  va_start(arguments, message);
  vprintf(message, arguments);
  va_end(arguments);
  exit(EXIT_FAILURE);
}

void callbackTerminate(int code, const char* message) {
  terminate(message);
}

void callbackKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if(action == GLFW_PRESS) {
    ContextInformation* context = glfwGetWindowUserPointer(window);
    switch(key) {
      case GLFW_KEY_S: context->inputKeyboardS = 1; break;
      case GLFW_KEY_A: context->inputKeyboardA = 1; break;
    }
  }
}

void callbackWheel(GLFWwindow* window, double xs, double ys)
{
  ContextInformation* context = glfwGetWindowUserPointer(window);
  if(!context->timerActive) {
    context->sortingAlgorithm += ys;
    if (context->sortingAlgorithm < 0)
      context->sortingAlgorithm = 0;
    if (context->sortingAlgorithm > 5)
      context->sortingAlgorithm = 5;
    switch (context->sortingAlgorithm) {
      case 0: context->sortingAlgorithmLabel = "None"; break;
      case 1: context->sortingAlgorithmLabel = "Gnome Sort"; break;
      case 2: context->sortingAlgorithmLabel = "Bubble Sort"; break;
      case 3: context->sortingAlgorithmLabel = "Selection Sort"; break;
      case 4: context->sortingAlgorithmLabel = "Radix Sort"; break;
      case 5: context->sortingAlgorithmLabel = "Shell Sort"; break;
    }
  }
}

void callbackDebugOGL(uint32_t source, uint32_t type, uint32_t index, uint32_t severity, int32_t size, const char* message, const void* user) {
  if(type == GL_DEBUG_TYPE_ERROR_ARB) terminate(message);
}

double scanDoubleHLP(const char *message) {
  double intermediary;
  printf("%s", message);
  if(scanf("%lf", &intermediary) != 1) terminate("Input invalid, fatal\n");
  return intermediary;
}

int main(int argc, char const *argv[]) {
  const double shuffleDelay = scanDoubleHLP("shuffleDelay -> ") / 1000,
  sortDelay = scanDoubleHLP("sortDelay -> ") / 1000;

  glfwSetErrorCallback(callbackTerminate);
  glfwInit();
  atexit(glfwTerminate);
  const float WIDTH = 1024, HEIGHT = 480;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
  glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  // glfwWindowHint(GLFW_SAMPLES, 4);

  GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Hello, World!", NULL, NULL);
  glfwMakeContextCurrent(window);
  if(gladLoadGL() == 0) terminate("Graphics context not loaded successfully, fatal\n");
  printf("%s\n%s\n%s\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER), glGetString(GL_VERSION));
  glfwSetKeyCallback(window, callbackKeyboard);
  glfwSetScrollCallback(window, callbackWheel);

  ContextInformation context = {0};
  glfwSetWindowUserPointer(window, &context);
  context.sortingAlgorithmLabel = "None";

  glfwSwapInterval(0);
  glEnable(GL_BLEND);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDebugMessageCallbackARB(callbackDebugOGL, NULL);

  const uint32_t programs[] = {
    createProgramOGL("assets/shaders/base.vert", "assets/shaders/base.frag"),
    createProgramOGL("assets/shaders/text.vert", "assets/shaders/text.frag")
  }, uniforms[] = {
    getUniformLocationOGL(programs[0], "projection"),
    getUniformLocationOGL(programs[1], "projection"),
    getUniformLocationOGL(programs[1], "sampler"),
  };

  Mat16f projection;
  createOrthographicProjectionMat16f(projection, 0, WIDTH, HEIGHT, 0, 1, 1000);

  glUseProgram(programs[0]);
  glUniformMatrix4fv(uniforms[0], 1, GL_FALSE, projection);

  glUseProgram(programs[1]);
  glUniformMatrix4fv(uniforms[1], 1, GL_FALSE, projection);
  glUniform1i(uniforms[2], 0);

  SurfaceBufferOGL surfaces;
  SurfaceRendererOGL surfaceRenderer;
  createSurfaceRendererAndBufferCOGL(&surfaceRenderer, &surfaces, programs[0], WIDTH);
  surfaceBufferLoadColorSpectrumBand(&surfaces, 0, 250, 260 - HEIGHT);

  SurfaceBufferOGL fontSurfaces;
  SurfaceRendererOGL fontSurfaceRenderer;
  createSurfaceRendererAndBufferFOGL(&fontSurfaceRenderer, &fontSurfaces, programs[1], 300);

  createFontTextures();
  atexit(destroyFontTextures);
  FontWrapper arial = retrieveFontArial();
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, arial.texture);

  int done = 1, iterations = 0;
  int i, j, min_idx, max, exp, k, m, gap;
  double accumulator;
  char stringBuffer[300];
  double results[5] = {0};
  double timer = 0, lastTime = 0;
  struct Surface cluster[(int)WIDTH], temp;
  memset(cluster, 0, sizeof(cluster));
  int bucket[10];

  while(!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    context.time = glfwGetTime();
    if(context.timerActive) {
      timer += context.time - lastTime;
    }
    lastTime = context.time;

    if(context.inputKeyboardS) {
      context.timerActive = 0;
      context.shuffleIndex = surfaces.count - 1;
      if(context.sortingAlgorithm > 0) {
         results[(int) context.sortingAlgorithm - 1] = timer;
         context.sortState = 0;
      }
      timer = 0;
      context.inputKeyboardS = 0;
    } if(context.inputKeyboardA) {
      if(context.sortingAlgorithm > 0) {
        context.sortState = 1;
        context.timerActive = 1;
      }
      context.inputKeyboardA = 0;
      done = 0;
    }

    accumulator = context.time - context.lastShuffleTime;
    if(accumulator > shuffleDelay) {
      context.lastShuffleTime = context.time;
      while(context.shuffleIndex > 0 && accumulator > 0) {
        context.shuffleIndex = surfaceBufferShuffleOnce(&surfaces, context.shuffleIndex);
        accumulator -= shuffleDelay;
      }
    }

    accumulator = context.time - context.lastSortTime;
    if(accumulator > sortDelay) {
      context.lastSortTime = context.time;
      while(accumulator > 0 && context.sortState > 0) {
        if(context.sortState == 1) {
          iterations = 0;
          switch(context.sortingAlgorithm) {
            case 1:
            i = 0;
            break;
            case 2:
            i = WIDTH - 1;
            j = 0;
            break;
            case 3:
            i = 0;
            j = 1;
            min_idx = 0;
            break;
            case 4:
            i = 0;
            max = surfacesGetMaximumIndex(surfaces.surfaces, WIDTH);
            exp = 1;
            j = 1;
            k = WIDTH - 1;
            m = k;
            break;
            case 5:
            gap = WIDTH;
            i = gap / 2;
            temp = surfaces.surfaces[i];
            j = i;
            break;
            case 6:
            break;
          }
          context.sortState = 2;
        }

        iterations += 1;
        switch(context.sortingAlgorithm) {
          case 1:
          // Gnome Sort
          if(i < WIDTH) {
            if(i == 0) i++;
            if(surfaces.surfaces[i].rotation[1] >= surfaces.surfaces[i - 1].rotation[1]) i++;
            else {
              surfacesSwap(&surfaces.surfaces[i], &surfaces.surfaces[i - 1]);
              i -= 1;
            }
          } else {
            results[(int) context.sortingAlgorithm - 1] = timer;
            done = 1;
          }
          break;
          case 2:
          // Bubble Sort
          if(i > 0) {
            if(j < i) {
              if(surfaces.surfaces[j].rotation[1] > surfaces.surfaces[j+1].rotation[1]) {
                surfacesSwap(&surfaces.surfaces[j], &surfaces.surfaces[j+1]);
              }
              j += 1;
            } else {
              j = 0;
              i -= 1;
            }
          } else {
            results[(int) context.sortingAlgorithm - 1] = timer;
            done = 1;
          }
          break;
          case 3:
          // Selection Sort
          if(i < WIDTH - 1) {
            if(j < WIDTH) {
              if(surfaces.surfaces[j].rotation[1] < surfaces.surfaces[min_idx].rotation[1]) {
                min_idx = j;
              }
              j += 1;
            } else {
              surfacesSwap(&surfaces.surfaces[min_idx], &surfaces.surfaces[i]);
              i += 1;
              j = i + 1;
              min_idx = i;
            }
          } else {
            results[(int) context.sortingAlgorithm - 1] = timer;
            done = 1;
          }
          break;
          case 4:
          // Radix Sort
          if(max / exp > 0) {
            if(i < WIDTH) {
              bucket[((int) surfaces.surfaces[i].rotation[1] / exp) % StaticArrayLength(bucket)] += 1;
              i += 1;
            } else if(j < StaticArrayLength(bucket)) {
              bucket[j] += bucket[j - 1];
              j += 1;
            } else if(k >= 0) {
              int l = --bucket[((int) surfaces.surfaces[k].rotation[1] / exp) % 10];
              cluster[l] = surfaces.surfaces[k];
              k -= 1;
            } else if(m >= 0) {
              surfaces.surfaces[m] = cluster[m];
              m -= 1;
            } else {
              i = 0;
              j = 1;
              k = WIDTH - 1;
              m = WIDTH - 1;
              exp *= 10;
              memset(bucket, 0, sizeof(bucket));
            }
          } else {
            results[(int) context.sortingAlgorithm - 1] = timer;
            done = 1;
          }
          break;
          case 5:
          // Shell Sort
          if(gap > 0) {
            if(i < WIDTH - 1) {
              if(j >= gap && surfaces.surfaces[j - gap].rotation[1] > temp.rotation[1]) {
                surfaces.surfaces[j] = surfaces.surfaces[j - gap];
                j -= gap;
              } else {
                surfaces.surfaces[j] = temp;
                i += 1;
                temp = surfaces.surfaces[i];
                j = i;
              }
            } else {
              gap /= 2;
              i = gap;
            }
          } else {
            results[(int) context.sortingAlgorithm - 1] = timer;
            done = 1;
          }
          break;
        }

        accumulator -= sortDelay;
      }
    }

    if(done) {
      context.timerActive = 0;
      context.sortState = 0;
      timer = 0;
    }

    glUseProgram(programs[0]);
    surfaceBufferRenderOGL(&surfaceRenderer, &surfaces);
    surfaces.count = surfaces.capacity;

    sprintf(stringBuffer, "Current Shuffling Algorithm: %s\nShuffle Delay: %.6lf secs\nSort Delay: %.6lf secs\nTimer: %.4lf secs - Iterations: %d\n\nLast Results:\nGnome Sort - %.4lf secs\nBubble Sort - %.4lf secs\nSelection Sort - %.4lf secs\nRadix Sort - %.4lf secs\nShell Sort - %.4lf secs\n",
    context.sortingAlgorithmLabel, shuffleDelay, sortDelay, timer, iterations, results[0], results[1], results[2], results[3], results[4]);
    surfaceBufferAppendString(&fontSurfaces, &arial, 5, 20, 0.55, stringBuffer);

    glUseProgram(programs[1]);
    surfaceBufferRenderOGL(&fontSurfaceRenderer, &fontSurfaces);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  destroySurfaceBuffer(&surfaces);
  destroySurfaceRenderer(&surfaceRenderer);
  for(int i = 0; i < StaticArrayLength(programs); ++i) glDeleteProgram(programs[i]);
  glfwDestroyWindow(window);
  return 0;
}

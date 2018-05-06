#include <stdlib.h>
#include <time.h>

#include "../includes/defs.h"

void init() {
  srand(time(NULL));
}

int getRandomIntegerALG(unsigned range) {
  return rand() % range;
}


void surfacesSwap(struct Surface * a, struct Surface * b) {
  const struct Surface t = *a;
  *a = *b;
  *b = t;
}

float surfacesGetMaximumIndex(struct Surface * array, int n) {
  float max = array[0].rotation[1];
  for(int i = 0; i < n; ++i) {
    if(array[i].rotation[1] > max) {
      max = array[i].rotation[1];
    }
  }
  return max;
}

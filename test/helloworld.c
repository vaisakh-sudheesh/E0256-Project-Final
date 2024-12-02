#include <stdio.h>


int testing_function(int a, int b, int c) {
  printf("Hello World from: %s\n", __func__);
  printf("  number of arguments: %d\n", 3);
  return 0;
}

int main () {
  printf("Hello World from: %s\n", __func__);
  testing_function(1, 2, 3);
  putchar('\n');
  return 0;
}
#include <stdio.h>

int main () {
    printf ("%s\n", __FUNCTION__);
    printf("Hello, World!\n");
    for (int i = 0; i < 10; i++) {
        fprintf(stdout,"i = %d\n", i);
    }
    return 0;
}
#include <stdio.h>
#include <stdlib.h>

void test_ifelse_1() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    if (i < 10) {
        printf("i = %d\n", i);
    }
}

void test_ifelse_2() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    if (i < 10) {
        printf("i = %d\n", i);
    } else {
        fprintf( stdout,"i = %d\n", i);
    }
}

void test_ifelse_3() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    if (i < 10) {
        putc('a', stdout);
    } else if (i < 20) {
        fprintf(stderr,"i = %d\n", i);
        if (i < 30) {
            putchar('\n');
        } else {
            putc('a', stdout);
        }
        i++;
        putchar('\n');
        putc('a', stdout);
        i = 5;
    }
}

void test_ifelse_4() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    putchar('\n');
    if (i < 10) {
        printf("i = %d\n", i);
    } else if (i < 20) {
        fprintf( stdout,"i = %d\n", i);
    } else if (i < 30) {
        puts("i = 30");
    } else {
        putc('a', stdout);
        puts("i = 30");
    }
}

void test_ifelse_5() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    if (i < 10) {
        printf("i = %d\n", i);
    } else if (i < rand()) {
        fprintf( stdout,"i = %d\n", i);
    }
}

void test_ifelse_6() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    if (i < 10) {
        printf("i = %d\n", i);
    } else if (i < rand()) {
        fprintf( stdout,"i = %d\n", i);
    } else {
        printf("i = %d\n", i);
    }
}

void test_ifelse_7() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    if (i < 10) {
        printf("i = %d\n", i);
        putchar('\n');
    } else if (i < rand()) {
        i ++;
    } else {
        i += 3;
    }
    fprintf( stdout, "i = %d\n", i);
}

int main() {
    printf ("%s\n", __FUNCTION__);
    printf("Testing if-else statements\n");
    test_ifelse_1();
    test_ifelse_2();
    test_ifelse_3();
    test_ifelse_4();
    test_ifelse_5();
    test_ifelse_6();
    test_ifelse_7();
    printf("Done testing if-else statements\n");
    return 0;
}
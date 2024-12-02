#include <stdio.h>
#include <stdlib.h>

void test_switchcase_1() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    switch (i) {
        case 0:
            printf("i = %d\n", i);
            break;
    }
}

void test_switchcase_2() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    switch (i) {
        case 0:
            printf("i = %d\n", i);
            break;
        default:
            fprintf(stdout,"i = %d\n", i);
            break;
    }
}

void test_switchcase_3() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    switch (i) {
        case 0:
            printf("i = %d\n", i);
            break;
        case 1:
            fprintf(stdout,"i = %d\n", i);
            break;
    }
}

void test_switchcase_4() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    switch (i) {
        case 0:
            printf("i = %d\n", i);
            break;
        case 1:
            putchar('\n');
            break;
        default:
            fprintf(stdout,"i = %d\n", i);
            break;
    }
}

void test_switchcase_5() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    switch (i) {
        case 0:
            printf("i = %d\n", i);
            break;
        case 1:
            putchar('\n');
            break;
        case 2:
            fprintf(stdout,"i = %d\n", i);
            break;
    }
}

void test_switchcase_6() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    switch (i) {
        case 0:
            printf("i = %d\n", i);
            break;
        case 1:
             putchar('\n');
            break;
        case 2:
            putc('a', stdout);
            break;
        default:
            fprintf(stdout,"i = %d\n", i);
            break;
    }
}

void test_switchcase_7() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    switch (i) {
        case 0:
            printf("i = %d\n", i);
            break;
        case 1:
            putc('a', stdout);
        case 2:
            putchar('\n');
            break;
        default:
            fprintf(stdout,"i = %d\n", i);
            break;
    }
}

void test_switchcase_8() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    switch (rand()) {
        case 0:
            printf("i = %d\n", i);
            break;
        case 1:
            printf("i = %d\n", i);
            putchar('\n');
            break;
        case 2:
            printf("i = %d\n", i);
            for (int j = 0; j < 10; j++) {
                putchar('\n');
            }
            break;
        default:
            fprintf(stdout,"i = %d\n", i);
            break;
    }
}

int main() {
    printf ("%s\n", __FUNCTION__);
    printf("Testing switch-case statements\n");
    test_switchcase_1();
    test_switchcase_2();
    test_switchcase_3();
    test_switchcase_4();
    test_switchcase_5();
    test_switchcase_6();
    test_switchcase_7();
    test_switchcase_8();
    printf("Completed testing switch-case statements\n");
    return 0;
}
#include <stdio.h>
#include <stdlib.h>

void test_dowhile_1() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    do {
        printf("i = %d\n", i);
        i++;
    } while (i < 10);
    fprintf(stdout, "i = %d\n", i);
}

void test_dowhile_2() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    do {
        i++;
        fprintf(stdout,"i = %d\n", i);
        if (i == 5) {
            putc('a', stdout);
            break;
        }
        putchar('\n');
    } while (i < 10);
}

void test_dowhile_3() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    do {
        i++;
        if (i == 5) {
            i++;
            continue;
        }
        printf("i = %d\n", i);
    } while (i < 10);
}

void test_dowhile_4() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    do {
        i++;
        if (i == 5) {
            return;
        }
        printf("i = %d\n", i);
    } while (i < 10);
}

void test_dowhile_5() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    do {
        i++;
        if (i == 5) {
            return;
        }
        printf("i = %d\n", i);
    } while (i < rand());
}

void test_dowhile_6() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    do {
        i++;
        if (i == 5) {
            break;
        }
    } while (i < 20);
    putchar('\n');
}


int main() {
    printf ("%s\n", __FUNCTION__);
    printf("Testing do-while statements\n");
    test_dowhile_1();
    test_dowhile_2();
    test_dowhile_3();
    test_dowhile_4();
    test_dowhile_5();
    test_dowhile_6();
    printf("Completed testing do-while statements\n");
    return 0;
}

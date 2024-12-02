#include <stdio.h>
#include <stdlib.h>

void test_for_1() {
    int i;
    printf ("%s\n", __FUNCTION__);
    for (i = 0; i < 10; i++) {
        printf("i = %d\n", i);
        i = i + 2;
        putchar('\n');
        fprintf(stdout, "i = %d\n", i);
        if (i == 5) {
            fprintf(stdout, "i = %d\n", i);
            putchar('\n');
        } else {
            putc('a', stdout);
        }
    }
}

void test_for_2() {
    int i;
    printf ("%s\n", __FUNCTION__);
    if (rand() % 2) {
        i = 0;
    } else {
        i = 1;
    }
    for (i = 0; i < 10; i++) {
        printf("i = %d\n", i);
        if (i == 5) {
            continue;
        }
        i = i + 2;
        fprintf(stdout, "i = %d\n", i);
    }
}

void test_for_3() {
    int i;
    printf ("%s\n", __FUNCTION__);
    for (i = 0; i < 10; i++) {
        if (i == 5) {
            continue;
        }
        printf("i = %d\n", i);
    }
}

void test_for_4() {
    int i;
    printf ("%s\n", __FUNCTION__);
    for (i = 0; i < rand( ); i++) {
        if (i == 5) {
            return;
        } else {
            i = i + 2;
        }
        printf("i = %d\n", i);
    }
}

void test_for_5() {
    int i;
    printf ("%s\n", __FUNCTION__);
    for (i = (rand( ) % 20); i < 20; i++) {
        fprintf(stdout, "i = %d\n", i);
        if (i == 5) {
            break;
        }
    }
}

void test_for_6() {
    int i;
    printf ("%s\n", __FUNCTION__);
    for (i = 0; i < 20; i++) {
        printf("i = %d\n", i);
        if (i == 5) {
            continue;
        } else {
            for (int j = 0; j < 10; j++) {
                if (j == 5) {
                    break;
                }
                fprintf(stdout, "j = %d\n", j);
            }
        }
    }
    printf("i = %d\n", i);
}

int main() {
    printf ("%s\n", __FUNCTION__);
    printf("Testing for loops\n");
    test_for_1();
    test_for_2();
    test_for_3();
    test_for_4();
    test_for_5();
    test_for_6();
    printf("Done testing for loops\n");
    return 0;
}
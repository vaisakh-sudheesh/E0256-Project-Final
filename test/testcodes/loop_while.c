#include <stdio.h>
#include <stdlib.h>

void test_while_1() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    while (i < 10) {
        printf("i = %d\n", i);
        i++;
        printf("i = %d\n", i);
    }
}

void test_while_2() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    while (i < 10) {
        printf("i = %d\n", i);
        if (i == 5) {
            break;
        }
        i++;
    }
}

void test_while_3() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    while (i < 10) {
        if (i == 5) {
            i++;
            continue;
        }
        printf("i = %d\n", i);
        i++;
    }
}

void test_while_4() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    while (i < 10) {
        if (i == 5) {
            return;
        }
        printf("i = %d\n", i);
        i++;
    }
}

void test_while_5() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    while (i < rand()) {
        if (i == 5) {
            return;
        }
        printf("i = %d\n", i);
        i++;
    }
}

void test_while_6() {
    int i = 0;
    printf ("%s\n", __FUNCTION__);
    while (i < 20) {
        i++;
        if (i == 5) {
            return;
        } else {
            int j = 0;
            while (j < 10) {
                if (j == 5) {
                    break;
                }
                printf("j = %d\n", j);
                j++;
            }
        }
    }
    printf ("i = %d\n", i);
}

int main() {
    printf ("%s\n", __FUNCTION__);
    printf("Testing while loops\n");
    test_while_1();
    test_while_2();
    test_while_3();
    test_while_4();
    test_while_5();
    test_while_6();
    printf("End of while loops test\n");
    
    return 0;
}
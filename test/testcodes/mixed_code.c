#include <stdio.h>
#include <stdlib.h>

void test_switchcase_1() {
    int i = 0;
    for (i = 0; i < 10; i++) {
        printf("i = %d\n", i);
        switch (i)
        {
        case 0:
            printf("i = %d\n", i);
            break;        
        default:
            break;
        }
    }
}

void test_switchcase_2() {
    int i = 0;
    for (i = 0; i < 10; i++) {
        printf("i = %d\n", i);
        switch (i)
        {
        case 0:
            printf("i = %d\n", i);
            break;
        case 1:
            printf("i = %d\n", i);
            break;
        default:
            break;
        }
    }
}

void test_switchcase_3() {
    int i = 0;
    for (i = 0; i < 10; i++) {
        printf("i = %d\n", i);
        if (i == 5) {
            switch (i)
            {
            case 0:
                printf("i = %d\n", i);
                break;
            case 1:
                printf("i = %d\n", i);
                break;
            default:
                break;
            }
        }
    }
}

void test_switchcase_4() {
    int i = 0;
    for (i = 0; i < 10; i++) {
        printf("i = %d\n", i);
        if (i == 5) {
            switch (i)
            {
            case 0:
                printf("i = %d\n", i);
                break;
            case 1:
                printf("i = %d\n", i);
                break;
            default:
                break;
            }
        } else {
            switch (i)
            {
            case 0:
                printf("i = %d\n", i);
                break;
            case 1:
                printf("i = %d\n", i);
                break;
            default:
                break;
            }
        }
    }
}

void test_switchcase_5() {
    int i = 0;
    for (i = 0; i < 10; i++) {
        printf("i = %d\n", i);
        if (i == 5) {
            switch (i)
            {
            case 0:
                printf("i = %d\n", i);
                break;
            case 1:
                printf("i = %d\n", i);
                break;
            default:
                break;
            }
        } else {
            switch (i)
            {
            case 0:
                i += 10;
                break;
            case 1:
                i += 20;
                break;
            default:
                i += 30;
                break;
            }
        }
    }
}


int main() {
    test_switchcase_1();
    test_switchcase_2();
    test_switchcase_3();
    test_switchcase_4();
    test_switchcase_5();
    return 0;
}
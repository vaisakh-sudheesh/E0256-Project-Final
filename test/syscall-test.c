#include <stdio.h>
#include <sys/syscall.h>      /* Definition of SYS_* constants */
#include <unistd.h>

int main() {
    printf("Hello, World!\n");
    __pid_t tid = syscall(SYS_gettid);
    printf("TID: %d\n", tid);
    return 0;
}
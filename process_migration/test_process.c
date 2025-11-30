#include <stdio.h>
#include <unistd.h>

int main() {
    printf("Test process running. PID = %d\n", getpid());
    while (1) {
        sleep(1);
    }
    return 0;
}

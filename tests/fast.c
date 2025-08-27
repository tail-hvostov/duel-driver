#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

char buf[400];

int main(int argc, const char* argv[]) {
    int fast;

    puts("1. Writing 360 bytes.");
    fast = open("/dev/duel1", O_WRONLY);
    if (fast < 0) {
        puts("The file did not open.");
        goto fault;
    }
    if (360 != write(fast, buf, 360)) {
        puts("Couldn't write 360 bytes.");
        close(fast);
        goto fault;
    }
    close(fast);

    puts("2. Attempting to write 400 bytes.");
    fast = open("/dev/duel1", O_WRONLY);
    if (fast < 0) {
        puts("The file did not open.");
        goto fault;
    }
    if (400 == write(fast, buf, 400)) {
        puts("400 bytes were written.");
        close(fast);
        goto fault;
    }
    close(fast);

    puts("Success!");
    return 0;
fault:
    puts("Failure!");
    return 1;
}
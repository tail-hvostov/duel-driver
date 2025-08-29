#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define BUF_SIZE 400

char buf[BUF_SIZE];

int main(int argc, const char* argv[]) {
    int fast;

    puts("1. Writing 360 bytes.");
    fast = open("/dev/duel2", O_WRONLY);
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
    fast = open("/dev/duel2", O_WRONLY);
    if (fast < 0) {
        puts("The file did not open.");
        goto fault;
    }
    if (BUF_SIZE == write(fast, buf, BUF_SIZE)) {
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

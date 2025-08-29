#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define BUF_SIZE 400

char buf[BUF_SIZE];

int fill_ff(void) {
    int i;
    int fast;
    for (i = 0; i < 360; i++) {
        buf[i] = 0xFF;
    }

    fast = open("/dev/duel1", O_WRONLY);
    if (fast < 0) {
        return 1;
    }
    if (360 != write(fast, buf, 360)) {
        close(fast);
        return 2;
    }
    close(fast);
    return 0;
} 

int main(int argc, const char* argv[]) {
    int simple;

    puts("1. Writing 360 bytes.");
    simple = open("/dev/duel2", O_WRONLY);
    if (simple < 0) {
        puts("The file did not open.");
        goto fault;
    }
    if (360 != write(simple, buf, 360)) {
        puts("Couldn't write 360 bytes.");
        close(simple);
        goto fault;
    }
    close(simple);

    puts("2. Attempting to write 400 bytes.");
    simple = open("/dev/duel2", O_WRONLY);
    if (simple < 0) {
        puts("The file did not open.");
        goto fault;
    }
    if (BUF_SIZE == write(simple, buf, BUF_SIZE)) {
        puts("400 bytes were written.");
        close(simple);
        goto fault;
    }
    close(simple);

    if (fill_ff()) {
        puts("Fast device doesn't work.");
        goto fault;
    }

    puts("Success!");
    return 0;
fault:
    puts("Failure!");
    return 1;
}

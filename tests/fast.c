#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define BUF_SIZE 400

char buf[BUF_SIZE];

void fill_buf(void) {
    int i;
    char val = 0;
    for (i = 0; i < 360; i++) {
        buf[i] = val;
        val++;
    }
}

int check_buf(void) {
    int i;
    char val = 0;
    for (i = 0; i < 360; i++) {
        if (val != buf[i]) {
            return 0;
        }
        val++;
    }
    return 1;
}

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
    if (BUF_SIZE == write(fast, buf, BUF_SIZE)) {
        puts("400 bytes were written.");
        close(fast);
        goto fault;
    }
    close(fast);

    puts("3. Read & write test.");
    fast = open("/dev/duel1", O_WRONLY);
    if (fast < 0) {
        puts("The file did not open.");
        goto fault;
    }
    fill_buf();
    if (360 != write(fast, buf, 360)) {
        puts("Couldn't write 360 bytes.");
        close(fast);
        goto fault;
    }
    close(fast);
    fast = open("/dev/duel1", O_RDONLY);
    memset(buf, 0, 360);
    if (360 != read(fast, buf, 360)) {
        puts("Couldn't read 360 bytes.");
        close(fast);
        goto fault;
    }
    if (!check_buf()) {
        puts("Buffer check failed.");
        goto fault;
    }
    close(fast);

    puts("Success!");
    return 0;
fault:
    puts("Failure!");
    return 1;
}
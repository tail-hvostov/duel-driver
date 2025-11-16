#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

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

int check_buf2(void) {
    int i;
    char val = 180;
    for (i = 0; i < 180; i++) {
        if (val != buf[i]) {
            printf("i=%d   val=%d   buf[i]=%d", i, val, buf[i]);
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

    puts("4. Read & write test II.");
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
    if ((180 != read(fast, buf, 180)) || (180 != read(fast, buf + 180, 180))) {
        puts("Couldn't read 360 bytes.");
        close(fast);
        goto fault;
    }
    if (!check_buf()) {
        puts("Buffer check failed.");
        goto fault;
    }
    close(fast);

    puts("5. Read & write & seek test.");
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
    if ((off_t)-1 == lseek(fast, 180, SEEK_SET)) {
        puts("Unsuccessful lseek call.");
        printf("Errno=%d.\n", errno);
        close(fast);
        goto fault;
    }
    if (180 != read(fast, buf, 180)) {
        puts("Couldn't read 180 bytes.");
        close(fast);
        goto fault;
    }
    if (!check_buf2()) {
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

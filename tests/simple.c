#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define BUF_SIZE 400

char buf[BUF_SIZE];

void simple_pic(void) {
    char* pointer = buf;
    int i;
    for (i = 0; i < 72; i++) {
        *pointer = 0xFF;
        pointer += 1;
    }
    //9
    for (i = 0; i < 9; i++) {
        *pointer = 0xAA;
        pointer += 1;
    }
    for (i = 0; i < 9 * 31; i++) {
        *pointer = 0xFF;
        pointer += 1;
    }
}

void fast_pic(void) {
    char* pointer = buf;
    int i;
    for (i = 0; i < 72; i++) {
        *pointer = 0xBD;
        pointer += 1;
    }
    for (i = 0; i < 72; i++) {
        *pointer = 0xDB;
        pointer += 1;
    }
    for (i = 0; i < 72 * 3; i++) {
        *pointer = 0xFF;
        pointer += 1;
    }
}

int check_buf1(void) {
    int i = 0;
    char* pointer = buf;
    for (i = 0; i < 72; i++) {
        if (*pointer != 0xFF) {
            return 0;
        }
        pointer += 1;
    }
    for (i = 0; i < 72; i++) {
        if (i % 2 == 0) {
            if (*pointer != 0xFF) {
                return 0;
            }
        }
        else {
            if (*pointer != 0xFE) {
                return 0;
            }
        }
        pointer += 1;
    }
    for (i = 0; i < 72 * 3; i++) {
        if (*pointer != 0xFF) {
            return 0;
        }
        pointer += 1;
    }
    return 1;
} 

int check_buf2(void) {
    int i = 0;
    char* pointer = buf;
    for (i = 0; i < 9; i++) {
        if (*pointer != 0xFF) {
            return 0;
        }
        pointer += 1;
    }
    for (i = 0; i < 9; i++) {
        if (*pointer != 0) {
            return 0;
        }
        pointer += 1;
    }
    for (i = 0; i < 9 * 4; i++) {
        if (*pointer != 0xFF) {
            return 0;
        }
        pointer += 1;
    }
    //Здесь закончилось D и началось B
    for (i = 0; i < 9; i++) {
        if (*pointer != 0) {
            return 0;
        }
        pointer += 1;
    }
    for (i = 0; i < 9 * 3; i++) {
        if (*pointer != 0xFF) {
            return 0;
        }
        pointer += 1;
    }
    //Здесь закончилось B и началось другое B.
    for (i = 0; i < 9; i++) {
        if (*pointer != 0) {
            return 0;
        }
        pointer += 1;
    }
    for (i = 0; i < 9 * 2; i++) {
        if (*pointer != 0xFF) {
            return 0;
        }
        pointer += 1;
    }
    //Здесь закончилось B и началось D.
    for (i = 0; i < 9; i++) {
        if (*pointer != 0) {
            return 0;
        }
        pointer += 1;
    }
    for (i = 0; i < 9 * 2; i++) {
        if (*pointer != 0xFF) {
            return 0;
        }
        pointer += 1;
    }
    //Здесь закончилось D.
    for (i = 0; i < 72 * 3; i++) {
        if (*pointer != 0xFF) {
            return 0;
        }
        pointer += 1;
    }
    return 1;
}

void fill_buf(void) {
    int i;
    char val = 0;
    for (i = 0; i < 360; i++) {
        buf[i] = val;
        val++;
    }
}

int check_buf3(void) {
    int i;
    char val = 180;
    for (i = 0; i < 180; i++) {
        if (val != buf[i]) {
            printf("i=%d   val=%d   buf[i]=%d\n", i, val, buf[i]);
            printf("buf[1]=%d   buf[179]=%d\n", buf[1], buf[179]);
            return 0;
        }
        val++;
    }
    return 1;
}

int main(int argc, const char* argv[]) {
    int simple, fast;

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

    puts("3. Simple writing, fast reading.");
    simple_pic();
    simple = open("/dev/duel2", O_WRONLY);
    if (simple < 0) {
        puts("The file did not open.");
        goto fault;
    }
    fast = open("/dev/duel1", O_RDONLY);
    if (fast < 0) {
        puts("The fast device doesn't work properly.");
        close(simple);
        goto fault;
    }
    if ((72 != write(simple, buf, 72)) || (288 != write(simple, buf +  72, 288))) {
        puts("Couldn't write 360 bytes.");
        close(simple);
        close(fast);
        goto fault;
    }
    close(simple);
    if (360 != read(fast, buf, 360)) {
        puts("Couldn't read 360 bytes.");
        close(fast);
        goto fault;
    }
    close(fast);
    if (!check_buf1()) {
        puts("Buffer check failed.");
        goto fault;
    }

    puts("4. Fast writing, simple reading.");
    fast_pic();
    simple = open("/dev/duel2", O_RDONLY);
    if (simple < 0) {
        puts("The file did not open.");
        goto fault;
    }
    fast = open("/dev/duel1", O_WRONLY);
    if (fast < 0) {
        puts("The fast device doesn't work properly.");
        close(simple);
        goto fault;
    }
    if (360 != write(fast, buf, 360)) {
        puts("Couldn't write 360 bytes.");
        close(simple);
        close(fast);
        goto fault;
    }
    close(fast);
    if ((289 != read(simple, buf, 289)) || (71 != read(simple, buf + 289, 71))) {
        puts("Couldn't read 360 bytes.");
        close(simple);
        goto fault;
    }
    close(simple);
    if (!check_buf2()) {
        puts("Buffer check failed.");
        goto fault;
    }

    puts("5. Read & write & seek test.");
    simple = open("/dev/duel2", O_WRONLY);
    if (simple < 0) {
        puts("The file did not open.");
        goto fault;
    }
    fill_buf();
    if (360 != write(simple, buf, 360)) {
        puts("Couldn't write 360 bytes.");
        close(simple);
        goto fault;
    }
    close(simple);
    simple = open("/dev/duel2", O_RDONLY);
    memset(buf, 0, 360);
    if ((off_t)-1 == lseek(simple, 180, SEEK_SET)) {
        puts("Unsuccessful lseek call.");
        printf("Errno=%d.\n", errno);
        close(simple);
        goto fault;
    }
    if (180 != read(simple, buf, 180)) {
        puts("Couldn't read 180 bytes.");
        close(simple);
        goto fault;
    }
    if (!check_buf3()) {
        puts("Buffer check failed.");
        close(simple);
        goto fault;
    }
    close(simple);

    puts("Success!");
    return 0;
fault:
    puts("Failure!");
    return 1;
}

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define BUF_SIZE 400
#define SIMPLE_FILE "/dev/duel2"

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

int check_buf4(void) {
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

int test1() {
    int simple;
    puts("1. Writing 360 bytes.");
    simple = open(SIMPLE_FILE, O_WRONLY);
    if (simple < 0) {
        puts("The file did not open.");
        return 0;
    }
    if (360 != write(simple, buf, 360)) {
        puts("Couldn't write 360 bytes.");
        close(simple);
        return 0;
    }
    close(simple);
    return 1;
}

int test2() {
    int simple;
    puts("2. Attempting to write 400 bytes.");
    simple = open(SIMPLE_FILE, O_WRONLY);
    if (simple < 0) {
        puts("The file did not open.");
        return 0;
    }
    if (BUF_SIZE == write(simple, buf, BUF_SIZE)) {
        puts("400 bytes were written.");
        close(simple);
        return 0;
    }
    close(simple);
    return 1;
}

int test3() {
    int simple, fast;
    puts("3. Simple writing, fast reading.");
    simple_pic();
    simple = open(SIMPLE_FILE, O_WRONLY);
    if (simple < 0) {
        puts("The file did not open.");
        return 0;
    }
    fast = open("/dev/duel1", O_RDONLY);
    if (fast < 0) {
        puts("The fast device doesn't work properly.");
        close(simple);
        return 0;
    }
    if ((72 != write(simple, buf, 72)) || (288 != write(simple, buf +  72, 288))) {
        puts("Couldn't write 360 bytes.");
        close(simple);
        close(fast);
        return 0;
    }
    close(simple);
    if (360 != read(fast, buf, 360)) {
        puts("Couldn't read 360 bytes.");
        close(fast);
        return 0;
    }
    close(fast);
    if (!check_buf1()) {
        puts("Buffer check failed.");
        return 0;
    }
    return 1;
}

int test4() {
    int simple, fast;
    puts("4. Fast writing, simple reading.");
    fast_pic();
    simple = open(SIMPLE_FILE, O_RDONLY);
    if (simple < 0) {
        puts("The file did not open.");
        return 0;
    }
    fast = open("/dev/duel1", O_WRONLY);
    if (fast < 0) {
        puts("The fast device doesn't work properly.");
        close(simple);
        return 0;
    }
    if (360 != write(fast, buf, 360)) {
        puts("Couldn't write 360 bytes.");
        close(simple);
        close(fast);
        return 0;
    }
    close(fast);
    if ((289 != read(simple, buf, 289)) || (71 != read(simple, buf + 289, 71))) {
        puts("Couldn't read 360 bytes.");
        close(simple);
        return 0;
    }
    close(simple);
    if (!check_buf2()) {
        puts("Buffer check failed.");
        return 0;
    }
    return 1;
}

int test5() {
    int simple;
    puts("5. Simple writing, simple reading.");
    simple = open(SIMPLE_FILE, O_WRONLY);
    if (simple < 0) {
        puts("The file did not open.");
        return 0;
    }
    fill_buf();
    if (360 != write(simple, buf, 360)) {
        puts("Couldn't write 360 bytes.");
        close(simple);
        return 0;
    }
    close(simple);
    simple = open(SIMPLE_FILE, O_RDONLY);
    memset(buf, 0, 360);
    if (360 != read(simple, buf, 360)) {
        puts("Couldn't read 360 bytes.");
        close(simple);
        return 0;
    }
    if (!check_buf4()) {
        puts("Buffer check failed.");
        return 0;
    }
    close(simple);
    return 1;
}

int test6() {
    int simple;
    puts("6. Read & write & seek test.");
    simple = open(SIMPLE_FILE, O_WRONLY);
    if (simple < 0) {
        puts("The file did not open.");
        return 0;
    }
    fill_buf();
    if (360 != write(simple, buf, 360)) {
        puts("Couldn't write 360 bytes.");
        close(simple);
        return 0;
    }
    close(simple);
    simple = open(SIMPLE_FILE, O_RDONLY);
    memset(buf, 0, 360);
    if ((off_t)-1 == lseek(simple, 180, SEEK_SET)) {
        puts("Unsuccessful lseek call.");
        printf("Errno=%d.\n", errno);
        close(simple);
        return 0;
    }
    if (180 != read(simple, buf, 180)) {
        puts("Couldn't read 180 bytes.");
        close(simple);
        return 0;
    }
    if (!check_buf3()) {
        puts("Buffer check failed.");
        close(simple);
        return 0;
    }
    close(simple);
    return 1;
}

int main(int argc, const char* argv[]) {
    int result = test1() && test2() && test3() && test4() && test5() && test6();
    if (result) {
        puts("Success!");
        return 0;
    }
    else {
        puts("Failure!");
        return 1;
    }
}

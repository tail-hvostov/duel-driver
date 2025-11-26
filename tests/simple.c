#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cctype>

#include "common/common_ops.h"

unsigned int video_half;
unsigned int page_size;

void simple_pic(void) {
    char* pointer = buf;
    int i;
    for (i = 0; i < page_size; i++) {
        *pointer = 0xFF;
        pointer += 1;
    }
    //9
    for (i = 0; i < sc_w / 8; i++) {
        *pointer = 0xAA;
        pointer += 1;
    }
    for (i = 0; i < video_size - sc_w / 8 - page_size; i++) {
        *pointer = 0xFF;
        pointer += 1;
    }
}

void fast_pic(void) {
    char* pointer = buf;
    int i;
    for (i = 0; i < page_size; i++) {
        *pointer = 0xBD;
        pointer += 1;
    }
    for (i = 0; i < page_size; i++) {
        *pointer = 0xDB;
        pointer += 1;
    }
    for (i = 0; i < video_size - 2 * page_size; i++) {
        *pointer = 0xFF;
        pointer += 1;
    }
}

int check_buf1(void) {
    int i = 0;
    char* pointer = buf;
    for (i = 0; i < page_size; i++) {
        if (*pointer != 0xFF) {
            return 0;
        }
        pointer += 1;
    }
    for (i = 0; i < page_size; i++) {
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
    for (i = 0; i < video_size - page_size * 2; i++) {
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
    for (i = 0; i < sc_w / 8; i++) {
        if (*pointer != 0xFF) {
            return 0;
        }
        pointer += 1;
    }
    for (i = 0; i < sc_w / 8; i++) {
        if (*pointer != 0) {
            return 0;
        }
        pointer += 1;
    }
    for (i = 0; i < sc_w / 8 * 4; i++) {
        if (*pointer != 0xFF) {
            return 0;
        }
        pointer += 1;
    }
    //Здесь закончилось D и началось B
    for (i = 0; i < sc_w / 8; i++) {
        if (*pointer != 0) {
            return 0;
        }
        pointer += 1;
    }
    for (i = 0; i < sc_w / 8 * 3; i++) {
        if (*pointer != 0xFF) {
            return 0;
        }
        pointer += 1;
    }
    //Здесь закончилось B и началось другое B.
    for (i = 0; i < sc_w / 8; i++) {
        if (*pointer != 0) {
            return 0;
        }
        pointer += 1;
    }
    for (i = 0; i < sc_w / 8 * 2; i++) {
        if (*pointer != 0xFF) {
            return 0;
        }
        pointer += 1;
    }
    //Здесь закончилось B и началось D.
    for (i = 0; i < sc_w / 8; i++) {
        if (*pointer != 0) {
            return 0;
        }
        pointer += 1;
    }
    for (i = 0; i < sc_w / 8 * 2; i++) {
        if (*pointer != 0xFF) {
            return 0;
        }
        pointer += 1;
    }
    //Здесь закончилось D.
    while (pointer != (buf + video_size)) {
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
    for (i = 0; i < video_size; i++) {
        buf[i] = val;
        val++;
    }
}

int check_buf3(void) {
    int i;
    char val = video_half;
    for (i = 0; i < video_half; i++) {
        if (val != buf[i]) {
            printf("i=%d   val=%d   buf[i]=%d\n", i, val, buf[i]);
            printf("buf[1]=%d   buf[%d]=%d\n", buf[1], video_half - 1, buf[video_half - 1]);
            return 0;
        }
        val++;
    }
    return 1;
}

int check_buf4(void) {
    int i;
    char val = 0;
    for (i = 0; i < video_size; i++) {
        if (val != buf[i]) {
            return 0;
        }
        val++;
    }
    return 1;
}

int test1() {
    int simple;
    printf("1. Writing %u bytes.\n", video_size);
    simple = open(SIMPLE_FILE, O_WRONLY);
    if (simple < 0) {
        puts("The file did not open.");
        return 0;
    }
    if (video_size != write(simple, buf, video_size)) {
        printf("Couldn't write %u bytes.\n", video_size);
        close(simple);
        return 0;
    }
    close(simple);
    return 1;
}

int test2() {
    int simple;
    printf("2. Attempting to write %u bytes.\n", buf_size);
    simple = open(SIMPLE_FILE, O_WRONLY);
    if (simple < 0) {
        puts("The file did not open.");
        return 0;
    }
    if (buf_size == write(simple, buf, buf_size)) {
        printf("%u bytes were written.\n", buf_size);
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
    if ((page_size != write(simple, buf, page_size)) ||
        (video_size - page_size != write(simple, buf +  page_size, video_size - page_size))) {
        printf("Couldn't write %u bytes.\n", video_size);
        close(simple);
        close(fast);
        return 0;
    }
    close(simple);
    if (video_size != read(fast, buf, video_size)) {
        printf("Couldn't read %u bytes.\n", video_size);
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
    if (video_size != write(fast, buf, video_size)) {
        printf("Couldn't write %u bytes.\n", video_size);
        close(simple);
        close(fast);
        return 0;
    }
    close(fast);
    if ((page_size + 1 != read(simple, buf, page_size + 1)) ||
        ((video_size - page_size - 1) != read(simple, buf + page_size + 1, video_size - page_size - 1))) {
        printf("Couldn't read %u bytes.\n", video_size);
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
    if (video_size != write(simple, buf, video_size)) {
        printf("Couldn't write %u bytes.\n", video_size);
        close(simple);
        return 0;
    }
    close(simple);
    simple = open(SIMPLE_FILE, O_RDONLY);
    memset(buf, 0, video_size);
    if (video_size != read(simple, buf, video_size)) {
        printf("Couldn't read %u bytes.\n", video_size);
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
    if (video_size != write(simple, buf, video_size)) {
        printf("Couldn't write %u bytes.\n", video_size);
        close(simple);
        return 0;
    }
    close(simple);
    simple = open(SIMPLE_FILE, O_RDONLY);
    memset(buf, 0, video_size);
    if ((off_t)-1 == lseek(simple, video_half, SEEK_SET)) {
        puts("Unsuccessful lseek call.");
        printf("Errno=%d.\n", errno);
        close(simple);
        return 0;
    }
    if (video_size - video_half != read(simple, buf, video_size - video_half)) {
        printf("Couldn't read %u bytes.\n", video_size - video_half);
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
    if (init_video_params(40)) {
        puts("Couldn't extract display parameters.");
        goto fault;
    }
    video_half = video_size / 2;
    page_size = sc_w * 8;

    int result = test1() && test2() && test3() && test4() && test5() && test6();
    if (result) {
        puts("Success!");
        delete [] buf;
        return 0;
    }
fault:
    puts("Failure!");
    delete [] buf;
    return 1;
}

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "common/common_ops.h"

unsigned int video_half;

void fill_buf(void) {
    int i;
    char val = 0;
    for (i = 0; i < video_size; i++) {
        buf[i] = val;
        val++;
    }
}

int check_buf(void) {
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

int check_buf2(void) {
    int i;
    char val = video_half;
    for (i = 0; i < (video_size - video_half); i++) {
        if (val != buf[i]) {
            printf("i=%d   val=%d   buf[i]=%d", i, val, buf[i]);
            return 0;
        }
        val++;
    }
    return 1;
}

int main(int argc, const char* argv[]) {
    if (init_video_params(40)) {
        puts("Couldn't extract display parameters.");
        goto fault;
    }
    video_half = video_size / 2;

    int fast;

    printf("1. Writing %u bytes.\n", video_size);
    fast = open("/dev/duel1", O_WRONLY);
    if (fast < 0) {
        puts("The file did not open.");
        goto fault;
    }
    if (video_size != write(fast, buf, video_size)) {
        printf("Couldn't write %u bytes.\n", video_size);
        close(fast);
        goto fault;
    }
    close(fast);

    printf("2. Attempting to write %u bytes.\n", buf_size);
    fast = open("/dev/duel1", O_WRONLY);
    if (fast < 0) {
        puts("The file did not open.");
        goto fault;
    }
    if (buf_size == write(fast, buf, buf_size)) {
        printf("%u bytes were written.\n", buf_size);
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
    if (video_size != write(fast, buf, video_size)) {
        printf("Couldn't write %u bytes.\n", video_size);
        close(fast);
        goto fault;
    }
    close(fast);
    fast = open("/dev/duel1", O_RDONLY);
    memset(buf, 0, video_size);
    if (video_size != read(fast, buf, video_size)) {
        printf("Couldn't read %u bytes.\n", video_size);
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
    if (video_size != write(fast, buf, video_size)) {
        printf("Couldn't write %u bytes.\n", video_size);
        close(fast);
        goto fault;
    }
    close(fast);
    fast = open("/dev/duel1", O_RDONLY);
    memset(buf, 0, video_size);
    if ((video_half != read(fast, buf, video_half)) ||
        ((video_size - video_half) != read(fast, buf + video_half, video_size - video_half))) {
        printf("Couldn't read %u bytes.\n", video_size);
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
    if (video_size != write(fast, buf, video_size)) {
        printf("Couldn't write %u bytes.\n", video_size);
        close(fast);
        goto fault;
    }
    close(fast);
    fast = open("/dev/duel1", O_RDONLY);
    memset(buf, 0, video_size);
    if ((off_t)-1 == lseek(fast, video_half, SEEK_SET)) {
        puts("Unsuccessful lseek call.");
        printf("Errno=%d.\n", errno);
        close(fast);
        goto fault;
    }
    if (video_size - video_half != read(fast, buf, video_size - video_half)) {
        printf("Couldn't read %u bytes.\n", video_size - video_half);
        close(fast);
        goto fault;
    }
    if (!check_buf2()) {
        puts("Buffer check failed.");
        close(fast);
        goto fault;
    }
    close(fast);

    puts("Success!");
    delete [] buf;
    return 0;
fault:
    puts("Failure!");
    delete [] buf;
    return 1;
}

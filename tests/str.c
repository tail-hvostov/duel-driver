#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>

#define STR_FILE "/dev/duel0"

void fill_buf_with_numbers(char* buf, off_t len) {
    char c = '0';
    for (off_t i = 0; i < len; i++) {
        buf[i] = c;
        c = ('9' == c) ? '0' : (c + 1);
    }
}

int check_buf_with_numbers(const char* buf, off_t len) {
    char c = '0';
    for (off_t i = 0; i < len; i++) {
        if (c != buf[i]) {
            return 0;
        }
        c = ('9' == c) ? '0' : (c + 1);
    }
    return 1;
}

int main() {
    int str;
    off_t buf_size;
    off_t test_buf_size;
    char* buf = nullptr;

    puts("1. Getting buffer size through llseek.");
    str = open(STR_FILE, O_RDONLY);
    if (str < 0) {
        puts("The file did not open.");
        goto fault;
    }
    buf_size = lseek(str, 0, SEEK_END);
    if ((off_t)-1 == buf_size) {
        puts("Unsuccessful lseek call.");
        printf("Errno=%d.\n", errno);
        close(str);
        goto fault;
    }
    printf("Buffer size is %jd.\n", (intmax_t)buf_size);
    close(str);

    test_buf_size = buf_size + 20;
    buf = new char[test_buf_size];

    printf("2. Writing %jd bytes.\n", (intmax_t)buf_size);
    str = open(STR_FILE, O_WRONLY);
    if (str < 0) {
        puts("The file did not open.");
        goto fault;
    }
    if (buf_size != write(str, buf, buf_size)) {
        printf("Couldn't write %jd bytes.\n", (intmax_t)buf_size);
        close(str);
        goto fault;
    }
    close(str);

    printf("3. Attempting to write %jd bytes.\n", (intmax_t)test_buf_size);
    str = open(STR_FILE, O_WRONLY);
    if (str < 0) {
        puts("The file did not open.");
        goto fault;
    }
    if (test_buf_size == write(str, buf, test_buf_size)) {
        printf("%jd bytes were written.\n", (intmax_t)test_buf_size);
        close(str);
        goto fault;
    }
    close(str);

    puts("4. Read & write test.");
    str = open(STR_FILE, O_WRONLY);
    if (str < 0) {
        puts("The file did not open.");
        goto fault;
    }
    fill_buf_with_numbers(buf, buf_size);
    if (buf_size != write(str, buf, buf_size)) {
        printf("Couldn't write %jd bytes.\n", (intmax_t)buf_size);
        close(str);
        goto fault;
    }
    close(str);
    str = open(STR_FILE, O_RDONLY);
    memset(buf, 0, buf_size);
    if (buf_size != read(str, buf, buf_size)) {
        printf("Couldn't read %jd bytes.\n", (intmax_t)buf_size);
        close(str);
        goto fault;
    }
    if (!check_buf_with_numbers(buf, buf_size)) {
        puts("Buffer check failed.");
        goto fault;
    }
    close(str);

    puts("5. Read & write test II.");
    str = open(STR_FILE, O_WRONLY);
    if (str < 0) {
        puts("The file did not open.");
        goto fault;
    }
    fill_buf_with_numbers(buf, buf_size);
    if (buf_size != write(str, buf, buf_size)) {
        printf("Couldn't write %jd bytes.\n", (intmax_t)buf_size);
        close(str);
        goto fault;
    }
    close(str);
    str = open(STR_FILE, O_RDONLY);
    memset(buf, 0, buf_size);
    if (((buf_size / 2) != read(str, buf, buf_size / 2)) ||
        ((buf_size - buf_size / 2) != read(str, buf + buf_size / 2, buf_size - buf_size / 2))) {
        printf("Couldn't read %jd bytes.\n", (intmax_t)buf_size);
        close(str);
        goto fault;
    }
    if (!check_buf_with_numbers(buf, buf_size)) {
        puts("Buffer check failed.");
        goto fault;
    }
    close(str);

    puts("Success!");
    delete [] buf;
    return 0;
fault:
    puts("Failure!");
    delete [] buf;
    return 1;
}

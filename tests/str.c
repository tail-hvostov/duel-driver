#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>

#define STR_FILE "/dev/duel0"

int main() {
    int str;
    off_t buf_size;
    off_t test_buf_size;
    char* test_buf = nullptr;

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
    test_buf = new char[test_buf_size];

    printf("2. Writing %u bytes.\n", buf_size);
    str = open(STR_FILE, O_WRONLY);
    if (str < 0) {
        puts("The file did not open.");
        goto fault;
    }
    if (buf_size != write(str, buf, buf_size)) {
        printf("Couldn't write %u bytes.\n", buf_size);
        close(str);
        goto fault;
    }
    close(str);

    printf("3. Attempting to write %u bytes.\n", test_buf_size);
    str = open(STR_FILE, O_WRONLY);
    if (str < 0) {
        puts("The file did not open.");
        goto fault;
    }
    if (test_buf_size == write(str, buf, test_buf_size)) {
        printf("%u bytes were written.\n", test_buf_size);
        close(str);
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

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

    puts("Success!");
    //delete [] buf;
    return 0;
fault:
    puts("Failure!");
    //delete [] buf;
    return 1;
}

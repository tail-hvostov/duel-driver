#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int main() {
    int str_file, simple_file, fast_file, str_file2;

    puts("1. Str device opening.");
    str_file = open("/dev/duel0", O_RDWR);
    if (str_file < 0) {
        printf("errno is %d.\n", errno);
        goto fault;
    }
    close(str_file);

    puts("2. Simple device opening.");
    simple_file = open("/dev/duel2", O_WRONLY);
    if (simple_file < 0) {
        printf("errno is %d.\n", errno);
        goto fault;
    }
    close(simple_file);

    puts("3. Fast and simple device opening for writing.");
    fast_file = open("/dev/duel1", O_RDWR);
    simple_file = open("/dev/duel2", O_WRONLY);
    if (simple_file >= 0) {
        close(simple_file);
        goto fault;
    }
    close(fast_file);

    puts("3. Fast and simple device opening for writing.");
    fast_file = open("/dev/duel1", O_RDWR);
    simple_file = open("/dev/duel2", O_WRONLY);
    if (simple_file >= 0) {
        close(simple_file);
        goto fault;
    }
    close(fast_file);

    puts("4. Str opening for reading while raw writing.");
    fast_file = open("/dev/duel1", O_RDWR);
    str_file = open("/dev/duel0", O_RDONLY);
    if (str_file >= 0) {
        close(str_file);
        goto fault;
    }
    close(fast_file);

    puts("5. Fast opening for writing while str reading.");
    str_file = open("/dev/duel0", O_RDONLY);
    fast_file = open("/dev/duel1", O_WRONLY);
    if (fast_file >= 0) {
        close(fast_file);
        goto fault;
    }
    close(str_file);

    puts("6. Fast opening for reading while str reading.");
    str_file = open("/dev/duel0", O_RDONLY);
    fast_file = open("/dev/duel1", O_RDONLY);
    if (fast_file < 0) {
        close(str_file);
        goto fault;
    }
    close(fast_file);
    close(str_file);

    puts("7. 2x str opening for reading.");
    str_file = open("/dev/duel0", O_RDONLY);
    str_file2 = open("/dev/duel0", O_RDONLY);
    if (str_file2 < 0) {
        close(str_file);
        goto fault;
    }
    close(str_file2);
    close(str_file);

    puts("8. Simple device opening after str reading.");
    simple_file = open("/dev/duel2", O_WRONLY);
    if (simple_file < 0) {
        printf("errno is %d.\n", errno);
        goto fault;
    }
    close(simple_file);

    puts("Success!");
    return 0;
fault:
    puts("Failure!");
    return 1;
}

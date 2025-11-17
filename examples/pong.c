#include <termios.h>
#include <stdio.h>
#include <unistd.h>

#define FRAME_TIMEOUT 100000
#define BRICK_HEIGHT 6
#define BRICK_HOR_MARGIN 4;
#define SCREEN_WIDTH 72
#define SCREEN_HEIGHT 40
#define SCREEN_MEMORY (SCREEN_HEIGHT * SCREEN_WIDTH / 8)

struct termios old_termios;
struct termios game_termios;
int fast;

int brick1_y;
int brick2_y;
char buf[SCREEN_MEMORY];

void paint() {
    memset(buf, 0, SCREEN_MEMORY);
    write(fast, buf, SCREEN_MEMORY);
}

int main(int argc, const char* argv[]) {
    fast = open("/dev/duel1", O_WRONLY);
    if (fast < 0) {
        puts("The file did not open.");
        return 0;
    }

    tcgetattr(STDIN_FILENO, &old_termios);
    game_termios = old_termios;
    game_termios.c_lflag &= ~(ECHO | ICANON);
    game_termios.c_cc[VMIN] = 0;
    game_termios.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &game_termios);

    int running = 1;
    char sym;
    while (running) {
        if (read(STDIN_FILENO, &sym, 1) > 0) {
            switch (sym) {
            case 'p':
                running = 0;
                break;
            }
        }
        paint();
        usleep(FRAME_TIMEOUT);
    }

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_termios);
    close(fast);
    return 0;
}

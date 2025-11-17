#include <termios.h>
#include <stdio.h>
#include <unistd.h>

#define FRAME_TIMEOUT 100000
#define BRICK_HEIGHT 6
#define BRICK_HOR_MARGIN 4;
#define SCREEN_WIDTH 72
#define SCREEN_HEIGHT 40
#define SCREEN_MEMORY (SCREEN_HEIGHT * SCREEN_WIDTH / 8)
#define SCREEN_PAGES (SCREEN_HEIGHT / 8)

struct termios old_termios;
struct termios game_termios;
int fast;

int brick1_y;
int brick2_y;
char buf[SCREEN_MEMORY];

void init_game() {
    brick1_y = (SCREEN_HEIGHT - BRICK_HEIGHT) / 2;
    brick2_y = brick1_y;
}

void draw_bricks() {
    int start_page, stop_page;
    char* cur_byte;
    start_page = brick1_y / 8;
    stop_page = (brick1_y + BRICK_HEIGHT - 1) / 8;
    cur_byte = buf + BRICK_HOR_MARGIN + SCREEN_WIDTH * start_page;
    int start_y = brick1_y  % 8;
    if (BRICK_HEIGHT > (8 - start_y)) {
        *cur_byte = 0xFF >> start_y;
        cur_byte += SCREEN_WIDTH;
    }
    else {
        *cur_byte = (0xFF >> (8 - BRICK_HEIGHT)) << (start_y - l);
        cur_byte += SCREEN_WIDTH;
    }
    for (int i = start_page + 1; i < stop_page; i++) {
        *cur_byte = 0xFF;
        cur_byte += SCREEN_WIDTH;
    }
}

void paint() {
    memset(buf, 0, SCREEN_MEMORY);
    draw_bricks();
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
    init_game();
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

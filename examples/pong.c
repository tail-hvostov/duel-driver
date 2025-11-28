#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>

#include "common/common_ops.h"

#define FRAME_TIMEOUT 60000
#define BRICK_HEIGHT 12
#define BRICK_HOR_MARGIN 4
#define BRICK_SHIFT 3
#define BALL_SIZE 2
#define BALL_START_SPEED 3
#define BALL_MIN_SPEED 2
#define BALL_MAX_SPEED 5

struct termios old_termios;
struct termios game_termios;
int fast;

unsigned int brick1_y;
unsigned int brick2_y;
unsigned int ball_x;
unsigned int ball_y;
unsigned int ball_vx;
unsigned int ball_vy;

void init_game() {
    brick1_y = (sc_h - BRICK_HEIGHT) / 2;
    brick2_y = brick1_y;
    ball_x = (sc_w - BALL_SIZE) / 2;
    ball_y = (sc_h - BALL_SIZE) / 2;
    ball_vx = BALL_START_SPEED - 2 * (rand() % 2)  * BALL_START_SPEED;
    ball_vy = 0;
}

void draw_brick(int brick_y, char* offset) {
    unsigned int start_page, stop_page;
    char* cur_byte;
    start_page = brick_y / 8;
    stop_page = (brick_y + BRICK_HEIGHT) / 8;
    cur_byte = offset;
    cur_byte += (sc_w * start_page);
    unsigned int start_y = brick_y  % 8;
    unsigned int start_taken = 8 - start_y;
    #if BRICK_HEIGHT <= 8
    if (BRICK_HEIGHT > start_taken) {
        *cur_byte = 0xFF;
        *cur_byte <<= start_y;
    }
    else {
        *cur_byte = 0xFF;
        *cur_byte <<= 8 - BRICK_HEIGHT;
        *cur_byte >>= 8 - BRICK_HEIGHT;
        *cur_byte <<= start_y;
    }
    #else
    *cur_byte = 0xFF;
    *cur_byte <<= start_y;
    #endif
    cur_byte += sc_w;
    for (unsigned int i = start_page + 1; i < stop_page; i++) {
        *cur_byte = 0xFF;
        cur_byte += sc_w;
    }
    if (stop_page > start_page) {
        unsigned int stop_taken = (BRICK_HEIGHT - start_taken) % 8;
        *cur_byte = 0xFF;
        *cur_byte >>= 8 - stop_taken;
    }
}

void draw_ball() {
    unsigned int start_page, stop_page;
    char* cur_byte;
    start_page = ball_y / 8;
    stop_page = (ball_y + BALL_SIZE) / 8;
    cur_byte = video_buf + ball_x;
    cur_byte += (sc_w * start_page);
    unsigned int start_y = ball_y  % 8;
    unsigned int start_taken = 8 - start_y;
    #if BALL_SIZE <= 8
    if (BALL_SIZE > start_taken) {
        char c = 0xFF;
        c <<= start_y;
        memset(cur_byte, c, BALL_SIZE);
    }
    else {
        char c = 0xFF;
        c <<= 8 - BALL_SIZE;
        c >>= 8 - BALL_SIZE;;
        c <<= start_y;
        memset(cur_byte, c, BALL_SIZE);
    }
    #else
    char c = 0xFF;
    c <<= start_y;
    memset(cur_byte, c, BALL_SIZE);
    #endif
    cur_byte += sc_w;
    for (unsigned int i = start_page + 1; i < stop_page; i++) {
        memset(cur_byte, 0xFF, BALL_SIZE);
        cur_byte += sc_w;
    }
    if (stop_page > start_page) {
        unsigned int stop_taken = (BALL_SIZE - start_taken) % 8;
        char c = 0xFF;
        c >>= 8 - stop_taken;
        memset(cur_byte, c, BALL_SIZE);
    }
}

void draw_bricks() {
    draw_brick(brick1_y, video_buf + BRICK_HOR_MARGIN);
    draw_brick(brick2_y, video_buf + (sc_w - 1 - BRICK_HOR_MARGIN));
}

void paint() {
    memset(video_buf, 0, video_size);
    draw_bricks();
    draw_ball();
    lseek(fast, 0, SEEK_SET);
    write(fast, video_buf, video_size);
}

void move_brick_up(unsigned int* brick_y) {
    *brick_y -= BRICK_SHIFT;
    if (*brick_y < 0) {
        *brick_y = 0;
    }
}

void move_brick_down(unsigned int* brick_y) {
    *brick_y += BRICK_SHIFT;
    unsigned int delta = sc_h - BRICK_HEIGHT - *brick_y;
    if (delta < 0) {
        *brick_y += delta;
    }
}

void move_ball() {
    unsigned int old_y;
    old_y = ball_y;
    ball_y += ball_vy;
    if (ball_y < 0) {
        ball_y = -ball_y;
        ball_vy = -ball_vy;
    }
    else {
        unsigned int delta = sc_h - BRICK_HEIGHT - ball_y;
        if (delta < 0) {
            ball_y += delta;
            ball_vy = -ball_vy;
        }
    }
    ball_x += ball_vx;
    if (ball_x <= BRICK_HOR_MARGIN) {
        unsigned int mid_y = (old_y + ball_y) / 2;
        if ((mid_y + BALL_SIZE <= brick1_y + BRICK_HEIGHT) && (mid_y >= brick1_y)) {
            ball_vx = rand() % (BALL_MAX_SPEED - BALL_MIN_SPEED) + BALL_MIN_SPEED;
            ball_x = BRICK_HOR_MARGIN + 1;
            ball_vy = rand() % (BALL_MAX_SPEED - BALL_MIN_SPEED) + BALL_MIN_SPEED;
            ball_vy -= 2 * (rand() % 2) * ball_vy;
        }
        else {
            puts("Player 1 missed the ball.");
            init_game();
        }
    }
    else if (ball_x + BALL_SIZE >= sc_w - 1 - BRICK_HOR_MARGIN) {
        unsigned int mid_y = (old_y + ball_y) / 2;
        if ((mid_y + BALL_SIZE <= brick2_y + BRICK_HEIGHT) && (mid_y >= brick2_y)) {
            ball_vx = -(rand() % (BALL_MAX_SPEED - BALL_MIN_SPEED) + BALL_MIN_SPEED);
            ball_x = sc_w - BALL_SIZE - 2 - BRICK_HOR_MARGIN;
            ball_vy = rand() % (BALL_MAX_SPEED - BALL_MIN_SPEED) + BALL_MIN_SPEED;
            ball_vy -= 2 * (rand() % 2) * ball_vy;
        }
        else {
            puts("Player 2 missed the ball.");
            init_game();
        }
    }
}

int main() {
    if (!init_video_params()) {
        puts("Couldn't extract display parameters.");
        return 0;
    }

    srand(time(NULL));
    fast = open("/dev/duel1", O_WRONLY);
    if (fast < 0) {
        puts("The file did not open.");
        delete [] video_buf;
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
            case '7':
                running = 0;
                break;
            case 'w':
                move_brick_up(&brick1_y);
                break;
            case 's':
                move_brick_down(&brick1_y);
                break;
            case 'o':
                move_brick_up(&brick2_y);
                break;
            case 'l':
                move_brick_down(&brick2_y);
                break;
            }
        }
        move_ball();
        paint();
        usleep(FRAME_TIMEOUT);
    }

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_termios);
    close(fast);
    delete [] video_buf;
    return 0;
}

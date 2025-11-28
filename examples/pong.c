#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>

#include "common/common_ops.h"

#define FRAME_TIMEOUT 60000
#define BRICK_HOR_MARGIN 4

int brick_height;
int brick_shift;
int ball_size;
int ball_start_speed;
int ball_min_speed;
int ball_max_speed;

struct termios old_termios;
struct termios game_termios;
int fast;

int brick1_y;
int brick2_y;
int ball_x;
int ball_y;
int ball_vx;
int ball_vy;

void init_game() {
    brick1_y = (sc_h - brick_height) / 2;
    brick2_y = brick1_y;
    ball_x = (sc_w - ball_size) / 2;
    ball_y = (sc_h - ball_size) / 2;
    ball_vx = ball_start_speed - 2 * (rand() % 2)  * ball_start_speed;
    ball_vy = 0;
}

void draw_brick(int brick_y, char* offset) {
    int start_page, stop_page;
    char* cur_byte;
    start_page = brick_y / 8;
    stop_page = (brick_y + brick_height) / 8;
    cur_byte = offset;
    cur_byte += (sc_w * start_page);
    int start_y = brick_y  % 8;
    int start_taken = 8 - start_y;
    if (brick_height > start_taken) {
        *cur_byte = 0xFF;
        *cur_byte <<= start_y;
    }
    else {
        *cur_byte = 0xFF;
        *cur_byte <<= 8 - brick_height;
        *cur_byte >>= 8 - brick_height;
        *cur_byte <<= start_y;
    }
    cur_byte += sc_w;
    for (int i = start_page + 1; i < stop_page; i++) {
        *cur_byte = 0xFF;
        cur_byte += sc_w;
    }
    if (stop_page > start_page) {
        int stop_taken = (brick_height - start_taken) % 8;
        *cur_byte = 0xFF;
        *cur_byte >>= 8 - stop_taken;
    }
}

void draw_ball() {
    int start_page, stop_page;
    char* cur_byte;
    start_page = ball_y / 8;
    stop_page = (ball_y + ball_size) / 8;
    cur_byte = video_buf + ball_x;
    cur_byte += (sc_w * start_page);
    int start_y = ball_y  % 8;
    int start_taken = 8 - start_y;
    if (ball_size > start_taken) {
        char c = 0xFF;
        c <<= start_y;
        memset(cur_byte, c, ball_size);
    }
    else {
        char c = 0xFF;
        c <<= 8 - ball_size;
        c >>= 8 - ball_size;;
        c <<= start_y;
        memset(cur_byte, c, ball_size);
    }
    cur_byte += sc_w;
    for (int i = start_page + 1; i < stop_page; i++) {
        memset(cur_byte, 0xFF, ball_size);
        cur_byte += sc_w;
    }
    if (stop_page > start_page) {
        int stop_taken = (ball_size - start_taken) % 8;
        char c = 0xFF;
        c >>= 8 - stop_taken;
        memset(cur_byte, c, ball_size);
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

void move_brick_up(int* brick_y) {
    *brick_y -= brick_shift;
    if (*brick_y < 0) {
        *brick_y = 0;
    }
}

void move_brick_down(int* brick_y) {
    *brick_y += brick_shift;
    int delta = sc_h - brick_height - *brick_y;
    if (delta < 0) {
        *brick_y += delta;
    }
}

void move_ball() {
    int old_y;
    old_y = ball_y;
    ball_y += ball_vy;
    if (ball_y < 0) {
        ball_y = -ball_y;
        ball_vy = -ball_vy;
    }
    else {
        int delta = sc_h - brick_height - ball_y;
        if (delta < 0) {
            ball_y += delta;
            ball_vy = -ball_vy;
        }
    }
    ball_x += ball_vx;
    if (ball_x <= BRICK_HOR_MARGIN) {
        int mid_y = (old_y + ball_y) / 2;
        if ((mid_y + ball_size <= brick1_y + brick_height) && (mid_y >= brick1_y)) {
            ball_vx = rand() % (ball_max_speed - ball_min_speed) + ball_min_speed;
            ball_x = BRICK_HOR_MARGIN + 1;
            ball_vy = rand() % (ball_max_speed - ball_min_speed) + ball_min_speed;
            ball_vy -= 2 * (rand() % 2) * ball_vy;
        }
        else {
            puts("Player 1 missed the ball.");
            init_game();
        }
    }
    else if (ball_x + ball_size >= (int)sc_w - 1 - BRICK_HOR_MARGIN) {
        int mid_y = (old_y + ball_y) / 2;
        if ((mid_y + ball_size <= brick2_y + brick_height) && (mid_y >= brick2_y)) {
            ball_vx = -(rand() % (ball_max_speed - ball_min_speed) + ball_min_speed);
            ball_x = sc_w - ball_size - 2 - BRICK_HOR_MARGIN;
            ball_vy = rand() % (ball_max_speed - ball_min_speed) + ball_min_speed;
            ball_vy -= 2 * (rand() % 2) * ball_vy;
        }
        else {
            puts("Player 2 missed the ball.");
            init_game();
        }
    }
}

void init_game_params() {
    if (sc_w == 72) {
        brick_height = 12;
        brick_shift = 3;
        ball_size = 2;
        ball_start_speed = 3;
        ball_min_speed = 2;
        ball_max_speed = 5;
    }
    else {
        brick_height = 16;
        brick_shift = 5;
        ball_size = 3;
        ball_start_speed = 4;
        ball_min_speed = 3;
        ball_max_speed = 6;
    }
}

int main() {
    if (!init_video_params()) {
        puts("Couldn't extract display parameters.");
        return 0;
    }
    init_game_params();

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

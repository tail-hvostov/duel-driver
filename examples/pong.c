#include <termios.h>
#include <stdio.h>
#include <unistd.h>

#define FRAME_TIMEOUT 100000

struct termios old_termios;
struct termios game_termios;

int main(int argc, const char* argv[]) {
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
        usleep(FRAME_TIMEOUT);
    }

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_termios);
    return 0;
}

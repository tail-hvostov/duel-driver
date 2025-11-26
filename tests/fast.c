#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cctype>

char* buf = nullptr;
unsigned int sc_w;
unsigned int sc_h;
unsigned int buf_size;
unsigned int video_size;
unsigned int video_half;

bool extractWidthHeight(unsigned int* width, unsigned int* height) {
    std::ifstream file("/proc/duel-params");
    if (!file.is_open()) {
        puts("Error: failed to open /proc/duel-params");
        return false;
    }

    bool hasWidth = false;
    bool hasHeight = false;
    
    std::string line;
    while (std::getline(file, line)) {
        // Пропускаем пустые строки
        if (line.empty()) continue;
        
        // Ищем разделитель ':'
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            continue; // Пропускаем строки без двоеточия
        }
        
        // Извлекаем ключ и значение
        std::string key = line.substr(0, colonPos);
        std::string value = line.substr(colonPos + 1);
        
        // Убираем пробелы вокруг ключа и значения
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        // Обрабатываем параметры
        if (key == "width") {
            hasWidth = true;
            
            // Проверяем, что значение - целое неотрицательное число > 0
            for (char c : value) {
                if (!std::isdigit(c)) {
                    printf("Error: width must be an integer: %s\n", value.c_str());
                    file.close();
                    return false;
                }
            }
            
            unsigned int w = std::atoi(value.c_str());
            if (w <= 0) {
                printf("Error: width must be greater than 0: %s\n", value.c_str());
                file.close();
                return false;
            }
            
            *width = w;
            
        } else if (key == "height") {
            hasHeight = true;
            
            // Проверяем, что значение - целое неотрицательное число > 0
            for (char c : value) {
                if (!std::isdigit(c)) {
                    printf("Error: height must be an integer: %s\n", value.c_str());
                    file.close();
                    return false;
                }
            }
            
            unsigned int h = std::atoi(value.c_str());
            if (h <= 0) {
                printf("Error: height must be greater than 0: %s\n", value.c_str());
                file.close();
                return false;
            }
            
            *height = h;
        }
    }
    
    file.close();
    
    // Проверяем, что оба параметра присутствуют
    if (!hasWidth) {
        puts("Error: missing parameter width");
        return false;
    }
    if (!hasHeight) {
        puts("Error: missing parameter height");
        return false;
    }
    
    return true;
}

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
    if (!extractWidthHeight(&sc_w, &sc_h)) {
        puts("Couldn't extract display parameters.");
        goto fault;
    }
    video_size = sc_w * sc_h / 8;
    buf_size = video_size + 40;
    buf = new char[buf_size];
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
    return 0;
fault:
    puts("Failure!");
    delete [] buf;
    return 1;
}

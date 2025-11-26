#include "common_ops.h"

char* buf = nullptr;
unsigned int sc_w;
unsigned int sc_h;
unsigned int buf_size;
unsigned int video_size;

bool extract_width_height(unsigned int* width, unsigned int* height) {
    std::ifstream file("/proc/duel-params");
    if (!file.is_open()) {
        puts("Error: failed to open /proc/duel-params");
        return false;
    }

    bool has_width = false;
    bool has_height = false;
    
    std::string line;
    while (std::getline(file, line)) {
        // Пропускаем пустые строки
        if (line.empty()) continue;
        
        // Ищем разделитель ':'
        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) {
            continue; // Пропускаем строки без двоеточия
        }
        
        // Извлекаем ключ и значение
        std::string key = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);
        
        // Убираем пробелы вокруг ключа и значения
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        // Обрабатываем параметры
        if (key == "width") {
            has_width = true;
            
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
            has_height = true;
            
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
    if (!has_width) {
        puts("Error: missing parameter width");
        return false;
    }
    if (!has_height) {
        puts("Error: missing parameter height");
        return false;
    }
    
    return true;
}

bool init_video_params(unsigned int extra_buf) {
    if (!extract_width_height(&sc_w, &sc_h)) {
        return false;
    }
    video_size = sc_w * sc_h / 8;
    buf_size = video_size + 40;
    buf = new char[buf_size];
    return true;
}
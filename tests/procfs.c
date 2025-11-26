#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cctype>

bool parse_and_validate_params() {
    std::ifstream file("/proc/duel-params");
    if (!file.is_open()) {
        puts("Couldn't open /proc/duel-params.");
        return false;
    }

    bool has_width = false;
    bool has_height = false;
    bool has_memory_mode = false;
    
    std::string line;
    while (std::getline(file, line)) {
        // Пропускаем пустые строки
        if (line.empty()) continue;
        
        // Ищем разделитель ':'
        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) {
            printf("Invalid format in line: %s.\n", line.c_str());
            file.close();
            return false;
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
                    printf("Width must be an integer: %s.\n", value.c_str());
                    file.close();
                    return false;
                }
            }
            
            int width = std::atoi(value.c_str());
            if (width <= 0) {
                printf("Width must be greater than 0: %s.\n", value.c_str());
                file.close();
                return false;
            }
            
        } else if (key == "height") {
            has_height = true;
            
            // Проверяем, что значение - целое неотрицательное число > 0
            for (char c : value) {
                if (!std::isdigit(c)) {
                    printf("Height must be an integer: %s.\n", value.c_str());
                    file.close();
                    return false;
                }
            }
            
            int height = std::atoi(value.c_str());
            if (height <= 0) {
                printf("Height must be greater than 0: %s.\n", value.c_str());
                file.close();
                return false;
            }
            
        } else if (key == "memory_mode") {
            has_memory_mode = true;
            
            // Проверяем, что значение равно "page"
            if (value != "page") {
                printf("Memory_mode must be 'page', got: %s.\n", value.c_str());
                file.close();
                return false;
            }
        }
    }
    
    file.close();
    
    // Проверяем, что все обязательные параметры присутствуют
    if (!has_width) {
        puts("Missing parameter: \"width\".");
        return false;
    }
    if (!has_height) {
        puts("Missing parameter: \"height\".");
        return false;
    }
    if (!has_memory_mode) {
        puts("Missing parameter: \"memory_mode\".");
        return false;
    }
    
    return true;
}

int main() {
    puts("1. Procfs test.");
    
    if (parse_and_validate_params()) {
        puts("Success!");
        return 0;
    } else {
        puts("Failure!");
        return 1;
    }
}
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cctype>

bool parseAndValidateParams() {
    std::ifstream file("/proc/duel-params");
    if (!file.is_open()) {
        puts("Couldn't open /proc/duel-params.");
        return false;
    }

    bool hasWidth = false;
    bool hasHeight = false;
    bool hasMemoryMode = false;
    
    std::string line;
    while (std::getline(file, line)) {
        // Пропускаем пустые строки
        if (line.empty()) continue;
        
        // Ищем разделитель ':'
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            printf("Invalid format in line: %s.\n", line.c_str());
            file.close();
            return false;
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
            hasHeight = true;
            
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
            hasMemoryMode = true;
            
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
    if (!hasWidth) {
        puts("Missing parameter: \"width\".");
        return false;
    }
    if (!hasHeight) {
        puts("Missing parameter: \"height\".");
        return false;
    }
    if (!hasMemoryMode) {
        puts("Missing parameter: \"memory_mode\".");
        return false;
    }
    
    return true;
}

int main() {
    puts("1. Procfs test.");
    
    if (parseAndValidateParams()) {
        puts("Success!");
        return 0;
    } else {
        puts("Failure!");
        return 1;
    }
}
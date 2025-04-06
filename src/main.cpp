#include "../include/MusicStoreDB.h"
#include "../include/UserInterface.h"
#include <iostream>
#include <memory>

int main() {
    // Установка русской локали для корректного отображения кириллицы
    std::setlocale(LC_ALL, "Russian");
    
    std::cout << "=== Музыкальный салон - Консольное приложение ===" << std::endl;
    
    try {
        // Создание объекта базы данных
        std::shared_ptr<MusicStoreDB> db = std::make_shared<MusicStoreDB>("music_store.db");
        
        // Создание и запуск пользовательского интерфейса
        UserInterface ui(db); // false - изначально не администратор
        ui.run();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Программа завершена." << std::endl;
    
    return 0;
}
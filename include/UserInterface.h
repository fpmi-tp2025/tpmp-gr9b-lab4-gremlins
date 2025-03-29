#pragma once

#include "MusicStoreDB.h"
#include <memory>

/**
 * @brief Класс пользовательского интерфейса консольного приложения
 */
class UserInterface {
private:
    std::shared_ptr<MusicStoreDB> db; // Указатель на базу данных
                  // Признак администратора
    
    /**
     * @brief Вывод меню администратора
     */
    void displayAdminMenu();
    
    /**
     * @brief Вывод меню обычного пользователя
     */
    void displayUserMenu();
    
    /**
     * @brief Проверка ввода пользователя
     * 
     * @param min Минимальное допустимое значение
     * @param max Максимальное допустимое значение
     * @return int Введенное пользователем значение
     */
    int getMenuChoice(int min, int max);
    
    /**
     * @brief Обработка команд администратора
     * 
     * @param choice Выбранная команда
     */
    void processAdminCommand(int choice);
    
    /**
     * @brief Обработка команд обычного пользователя
     * 
     * @param choice Выбранная команда
     */
    void processUserCommand(int choice);

public:
    /**
     * @brief Конструктор
     * 
     * @param db Указатель на базу данных
     * @param isAdmin Признак администратора
     */
    UserInterface(std::shared_ptr<MusicStoreDB> db);
    
    /**
     * @brief Запуск пользовательского интерфейса
     */
    void run();
    
    /**
     * @brief Аутентификация пользователя
     * 
     * @return true если аутентификация успешна
     */
    bool authenticate();
};
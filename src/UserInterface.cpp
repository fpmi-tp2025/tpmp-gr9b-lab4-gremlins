#include "../include/UserInterface.h"
#include <iostream>
#include <limits>

// Конструктор
UserInterface::UserInterface(std::shared_ptr<MusicStoreDB> db) 
    : db(db) {
}

// Аутентификация пользователя
bool UserInterface::authenticate() {
    std::string username, password;
    bool loggedIn = false;
    
    while (!loggedIn) {
        std::cout << "\n=== Музыкальный салон - Вход в систему ===" << std::endl;
        std::cout << "Введите имя пользователя: ";
        std::cin >> username;
        std::cout << "Введите пароль: ";
        std::cin >> password;
        
        loggedIn = db->login(username, password);
        
        if (!loggedIn) {
            std::cout << "Неверные учетные данные. Пожалуйста, попробуйте снова." << std::endl;
            
            // Спросим, хочет ли пользователь повторить попытку
            std::cout << "Хотите попробовать снова? (1 - Да, 0 - Нет): ";
            int choice;
            std::cin >> choice;
            
            if (choice != 1) {
                return false;
            }
        }
    }
    
    std::cout << "Вход выполнен успешно!" << std::endl;
    return true;
}

// Запуск интерфейса
void UserInterface::run() {
    if (!authenticate()) {
        return;
    }
    if (db->isUserAdmin()) {
        displayAdminMenu();
    } else {
        displayUserMenu();
    }
}

// Вывод меню администратора
void UserInterface::displayAdminMenu() {
    while (true) {
        std::cout << "\n=== Музыкальный салон - Меню администратора ===" << std::endl;
        std::cout << "1. Просмотреть информацию о всех компакт-дисках" << std::endl;
        std::cout << "2. Просмотреть информацию о продажах компакт-диска" << std::endl;
        std::cout << "3. Просмотреть информацию о самом популярном компакт-диске" << std::endl;
        std::cout << "4. Просмотреть информацию о самом популярном исполнителе" << std::endl;
        std::cout << "5. Просмотреть информацию о продажах по авторам" << std::endl;
        std::cout << "6. Рассчитать статистику за период" << std::endl;
        std::cout << "7. Добавить новый компакт-диск" << std::endl;
        std::cout << "8. Добавить музыкальное произведение" << std::endl;
        std::cout << "9. Зарегистрировать поступление компакт-дисков" << std::endl;
        std::cout << "10. Зарегистрировать продажу компакт-дисков" << std::endl;
        std::cout << "11. Обновить информацию о компакт-диске" << std::endl;
        std::cout << "12. Удалить компакт-диск" << std::endl;
        std::cout << "0. Выход" << std::endl;
        
        int choice = getMenuChoice(0, 12);
        if (choice == 0) {
            break;
        }
        
        processAdminCommand(choice);
    }
}

// Вывод меню обычного пользователя
void UserInterface::displayUserMenu() {
    while (true) {
        std::cout << "\n=== Музыкальный салон - Меню пользователя ===" << std::endl;
        std::cout << "1. Просмотреть информацию о самом популярном компакт-диске" << std::endl;
        std::cout << "2. Просмотреть информацию о самом популярном исполнителе" << std::endl;
        std::cout << "3. Получить информацию о продажах компакт-диска" << std::endl;
        std::cout << "0. Выход" << std::endl;
        
        int choice = getMenuChoice(0, 3);
        if (choice == 0) {
            break;
        }
        
        processUserCommand(choice);
    }
}

// Получение выбора пользователя из меню
int UserInterface::getMenuChoice(int min, int max) {
    int choice;
    
    while (true) {
        std::cout << "Выберите опцию: ";
        std::cin >> choice;
        
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Неверный ввод, пожалуйста, попробуйте снова." << std::endl;
            continue;
        }
        
        if (choice < min || choice > max) {
            std::cout << "Неверный выбор, пожалуйста, введите число от " << min << " до " << max << "." << std::endl;
            continue;
        }
        
        break;
    }
    
    return choice;
}

// Обработка команд администратора
void UserInterface::processAdminCommand(int choice) {
    switch (choice) {
        case 1:
            db->showCompactInventory();
            break;
        case 2: {
            int compactId;
            std::string startDate, endDate;
            
            std::cout << "Введите ID компакт-диска: ";
            std::cin >> compactId;
            std::cout << "Введите начальную дату (YYYY-MM-DD): ";
            std::cin >> startDate;
            std::cout << "Введите конечную дату (YYYY-MM-DD): ";
            std::cin >> endDate;
            
            db->showCompactSales(compactId, startDate, endDate);
            break;
        }
        case 3:
            db->showMostPopularCompact();
            break;
        case 4:
            db->showMostPopularPerformer();
            break;
        case 5:
            db->showAuthorSales();
            break;
        case 6: {
            std::string startDate, endDate;
            
            std::cout << "Введите начальную дату (YYYY-MM-DD): ";
            std::cin >> startDate;
            std::cout << "Введите конечную дату (YYYY-MM-DD): ";
            std::cin >> endDate;
            
            db->calculatePeriodStatistics(startDate, endDate);
            break;
        }
        case 7: {
            std::string productionDate, company;
            float price;
            
            std::cout << "Введите дату производства (YYYY-MM-DD): ";
            std::cin >> productionDate;
            std::cin.ignore();
            std::cout << "Введите компанию-производителя: ";
            std::getline(std::cin, company);
            std::cout << "Введите цену: ";
            std::cin >> price;
            
            db->addCompactDisc(productionDate, company, price);
            break;
        }
        case 8: {
            std::string title, author, performer;
            int compactId;
            
            std::cout << "Введите ID компакт-диска: ";
            std::cin >> compactId;
            std::cin.ignore();
            std::cout << "Введите название произведения: ";
            std::getline(std::cin, title);
            std::cout << "Введите автора: ";
            std::getline(std::cin, author);
            std::cout << "Введите исполнителя: ";
            std::getline(std::cin, performer);
            
            db->addMusicalWork(title, author, performer, compactId);
            break;
        }
        case 9: {
            int compactId, quantity;
            
            std::cout << "Введите ID компакт-диска: ";
            std::cin >> compactId;
            std::cout << "Введите количество: ";
            std::cin >> quantity;
            
            db->registerOperation("поступление", compactId, quantity);
            break;
        }
        case 10: {
            int compactId, quantity;
            
            std::cout << "Введите ID компакт-диска: ";
            std::cin >> compactId;
            std::cout << "Введите количество: ";
            std::cin >> quantity;
            
            db->registerOperation("продажа", compactId, quantity);
            break;
        }
        case 11: {
            int compactId;
            std::string company;
            float price;
            
            std::cout << "Введите ID компакт-диска: ";
            std::cin >> compactId;
            std::cin.ignore();
            std::cout << "Введите новую компанию-производителя: ";
            std::getline(std::cin, company);
            std::cout << "Введите новую цену: ";
            std::cin >> price;
            
            db->updateCompactDisc(compactId, company, price);
            break;
        }
        case 12: {
            int compactId;
            
            std::cout << "Введите ID компакт-диска для удаления: ";
            std::cin >> compactId;
            
            db->deleteCompactDisc(compactId);
            break;
        }
    }
}

// Обработка команд обычного пользователя
void UserInterface::processUserCommand(int choice) {
    switch (choice) {
        case 1:
            db->showMostPopularCompact();
            break;
        case 2:
            db->showMostPopularPerformer();
            break;
        case 3: {
            int compactId;
            std::string startDate, endDate;
            
            std::cout << "Введите ID компакт-диска: ";
            std::cin >> compactId;
            std::cout << "Введите начальную дату (YYYY-MM-DD): ";
            std::cin >> startDate;
            std::cout << "Введите конечную дату (YYYY-MM-DD): ";
            std::cin >> endDate;
            
            db->getCompactSalesInfo(compactId, startDate, endDate);
            break;
        }
    }
}
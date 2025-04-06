#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sqlite3.h>

/**
 * @brief Класс для работы с базой данных музыкального салона
 */
class MusicStoreDB
{
private:
    sqlite3 *db;        // Указатель на соединение с базой данных
    std::string dbPath; // Путь к файлу базы данных
    bool isAdmin;       // Признак того, что пользователь - администратор
    int userId;         // Идентификатор текущего пользователя
    static bool headers;

    /**
     * @brief Выполнение SQL-запроса без возврата результатов
     *
     * @param sql SQL-запрос
     * @return true если запрос выполнен успешно
     */
    bool executeQuery(const std::string &sql);

    /**
     * @brief Callback-функция для обработки результатов запроса
     */
    static int callback(void *data, int argc, char **argv, char **azColName);

    /**
     * @brief Callback-функция для вывода результатов запроса
     */
    static int printCallback(void *notUsed, int argc, char **argv, char **azColName);

    /**
     * @brief Инициализация базы данных (создание таблиц, индексов, триггеров)
     */
    void initializeDB();

public:
    /**
     * @brief Конструктор
     *
     * @param dbPath Путь к файлу базы данных
     */
    MusicStoreDB(const std::string &dbPath);

    /**
     * @brief Деструктор
     */
    ~MusicStoreDB();

    /**
     * @brief Аутентификация пользователя
     *
     * @param username Имя пользователя
     * @param password Пароль
     * @return true если аутентификация успешна
     */
    bool login(const std::string &username, const std::string &password);

    /**
     * @brief Получение информации о всех компакт-дисках
     */
    void showCompactInventory();

    /**
     * @brief Получение информации о продажах компакт-диска за период
     *
     * @param compactId Идентификатор компакт-диска
     * @param startDate Начальная дата периода
     * @param endDate Конечная дата периода
     */
    void showCompactSales(int compactId, const std::string &startDate, const std::string &endDate);

    /**
     * @brief Получение информации о самом популярном компакт-диске
     */
    void showMostPopularCompact();

    /**
     * @brief Получение информации о самом популярном исполнителе
     */
    void showMostPopularPerformer();

    /**
     * @brief Получение информации о продажах по авторам
     */
    void showAuthorSales();

    /**
     * @brief Получение информации о продажах компакт-диска за период
     *
     * @param compactId Идентификатор компакт-диска
     * @param startDate Начальная дата периода
     * @param endDate Конечная дата периода
     */
    void getCompactSalesInfo(int compactId, const std::string &startDate, const std::string &endDate);

    /**
     * @brief Расчет статистики за период
     *
     * @param startDate Начальная дата периода
     * @param endDate Конечная дата периода
     */
    void calculatePeriodStatistics(const std::string &startDate, const std::string &endDate);

    /**
     * @brief Добавление нового компакт-диска
     *
     * @param productionDate Дата изготовления
     * @param company Компания-производитель
     * @param price Цена
     */
    void addCompactDisc(const std::string &productionDate, const std::string &company, float price);

    /**
     * @brief Добавление музыкального произведения
     *
     * @param title Название произведения
     * @param author Автор произведения
     * @param performer Исполнитель
     * @param compactId Идентификатор компакт-диска
     */
    void addMusicalWork(const std::string &title, const std::string &author,
                        const std::string &performer, int compactId);

    /**
     * @brief Регистрация операции (поступление/продажа)
     *
     * @param operationType Тип операции ("поступление" или "продажа")
     * @param compactId Идентификатор компакт-диска
     * @param quantity Количество
     */
    void registerOperation(const std::string &operationType, int compactId, int quantity);

    /**
     * @brief Обновление информации о компакт-диске
     *
     * @param compactId Идентификатор компакт-диска
     * @param company Компания-производитель
     * @param price Цена
     */
    void updateCompactDisc(int compactId, const std::string &company, float price);

    /**
     * @brief Удаление компакт-диска
     *
     * @param compactId Идентификатор компакт-диска
     */
    void deleteCompactDisc(int compactId);

    /**
     * @brief Проверка, является ли текущий пользователь администратором
     *
     * @return true если пользователь администратор
     */
    bool isUserAdmin() const { return isAdmin; }
};

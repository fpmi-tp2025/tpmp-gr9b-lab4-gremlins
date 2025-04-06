#include "../include/MusicStoreDB.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <stdexcept>


bool MusicStoreDB::headers = true;
// Конструктор
MusicStoreDB::MusicStoreDB(const std::string& dbPath) : dbPath(dbPath), isAdmin(false), userId(-1) {
    int rc = sqlite3_open(dbPath.c_str(), &db);
    
    if (rc != SQLITE_OK) {
        std::cerr << "Не удалось открыть базу данных: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        exit(1);
    }
    
    initializeDB();
}

// Деструктор
MusicStoreDB::~MusicStoreDB() {
    sqlite3_close(db);
}

// Инициализация базы данных
void MusicStoreDB::initializeDB() {
    std::vector<std::string> tables = {
        // Таблица пользователей
        "CREATE TABLE IF NOT EXISTS users ("
        "    user_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    username TEXT NOT NULL UNIQUE,"
        "    password_hash TEXT NOT NULL,"
        "    role TEXT NOT NULL CHECK(role IN ('admin', 'user')),"
        "    created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");",
        
        // Таблица компакт-дисков
        "CREATE TABLE IF NOT EXISTS compact_discs ("
        "    compact_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    production_date DATE NOT NULL,"
        "    company TEXT NOT NULL,"
        "    price REAL NOT NULL CHECK(price > 0)"
        ");",
        
        // Таблица музыкальных произведений
        "CREATE TABLE IF NOT EXISTS musical_works ("
        "    work_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    title TEXT NOT NULL,"
        "    author TEXT NOT NULL,"
        "    performer TEXT NOT NULL,"
        "    compact_id INTEGER NOT NULL,"
        "    FOREIGN KEY (compact_id) REFERENCES compact_discs(compact_id) ON DELETE CASCADE"
        ");",
        
        // Таблица операций
        "CREATE TABLE IF NOT EXISTS operations ("
        "    operation_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    operation_date DATE NOT NULL,"
        "    operation_type TEXT NOT NULL CHECK(operation_type IN ('поступление', 'продажа')),"
        "    compact_id INTEGER NOT NULL,"
        "    quantity INTEGER NOT NULL CHECK(quantity > 0),"
        "    FOREIGN KEY (compact_id) REFERENCES compact_discs(compact_id) ON DELETE RESTRICT"
        ");",
        
        // Таблица отчетов
        "CREATE TABLE IF NOT EXISTS report_results ("
        "    report_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    start_date DATE NOT NULL,"
        "    end_date DATE NOT NULL,"
        "    compact_id INTEGER NOT NULL,"
        "    received_quantity INTEGER NOT NULL DEFAULT 0,"
        "    sold_quantity INTEGER NOT NULL DEFAULT 0,"
        "    FOREIGN KEY (compact_id) REFERENCES compact_discs(compact_id) ON DELETE CASCADE"
        ");"
    };
    
    // Создание таблиц
    for (const auto& sql : tables) {
        executeQuery(sql);
    }
    
    // Создание триггера для контроля продаж
    std::string triggerSQL = 
        "CREATE TRIGGER IF NOT EXISTS check_sale_quantity "
        "BEFORE INSERT ON operations "
        "WHEN NEW.operation_type = 'продажа' "
        "BEGIN "
        "    SELECT "
        "        CASE "
        "            WHEN ( "
        "                SELECT COALESCE(SUM(quantity), 0) "
        "                FROM operations "
        "                WHERE compact_id = NEW.compact_id AND operation_type = 'поступление' "
        "            ) - ( "
        "                SELECT COALESCE(SUM(quantity), 0) "
        "                FROM operations "
        "                WHERE compact_id = NEW.compact_id AND operation_type = 'продажа' "
        "            ) < NEW.quantity "
        "            THEN RAISE(ABORT, 'Невозможно продать больше компактов, чем имеется в наличии') "
        "        END; "
        "END;";
    
    executeQuery(triggerSQL);
    
    // Создание индексов для оптимизации
    std::vector<std::string> indexes = {
        "CREATE INDEX IF NOT EXISTS idx_musical_works_compact_id ON musical_works(compact_id);",
        "CREATE INDEX IF NOT EXISTS idx_operations_compact_id ON operations(compact_id);",
        "CREATE INDEX IF NOT EXISTS idx_operations_type ON operations(operation_type);",
        "CREATE INDEX IF NOT EXISTS idx_operations_date ON operations(operation_date);",
        "CREATE INDEX IF NOT EXISTS idx_report_results_dates ON report_results(start_date, end_date);",
        "CREATE INDEX IF NOT EXISTS idx_report_results_compact_id ON report_results(compact_id);"
    };
    
    for (const auto& sql : indexes) {
        executeQuery(sql);
    }
    
    // Проверка наличия администратора, и создание дефолтного если нет
    std::string checkAdmin = "SELECT COUNT(*) FROM users WHERE role = 'admin';";
    std::vector<std::vector<std::string>> results;
    
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, checkAdmin.c_str(), callback, &results, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return;
    }
    
    if (results.size() > 0 && results[0].size() > 0 && results[0][0] == "0") {
        // Создание дефолтного администратора (пароль: admin)
        std::string createAdmin = 
            "INSERT INTO users (username, password_hash, role) "
            "VALUES ('admin', 'admin', 'admin');";
        executeQuery(createAdmin);
        
        std::cout << "Создан дефолтный администратор. Логин: admin, Пароль: admin" << std::endl;
    }
    std::string checkUser = "SELECT COUNT(*) FROM users WHERE role = 'user';";
results.clear(); // Очищаем предыдущие результаты
rc = sqlite3_exec(db, checkUser.c_str(), callback, &results, &errMsg);

if (rc != SQLITE_OK) {
    std::cerr << "SQL error: " << errMsg << std::endl;
    sqlite3_free(errMsg);
    return;
}

if (results.size() > 0 && results[0].size() > 0 && results[0][0] == "0") {
    // Создание дефолтного пользователя (пароль: user)
    std::string createUser = 
        "INSERT INTO users (username, password_hash, role) "
        "VALUES ('user', 'user', 'user');";
    executeQuery(createUser);
    
    std::cout << "Создан дефолтный пользователь. Логин: user, Пароль: user" << std::endl;
}
}

// Выполнение SQL-запроса
bool MusicStoreDB::executeQuery(const std::string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    
    return true;
}

// Callback-функция для обработки результатов запроса
int MusicStoreDB::callback(void* data, int argc, char** argv, char** azColName) {
    auto* rows = static_cast<std::vector<std::vector<std::string>>*>(data);
    
    std::vector<std::string> row;
    for (int i = 0; i < argc; i++) {
        row.push_back(argv[i] ? argv[i] : "NULL");
    }
    
    rows->push_back(row);
    return 0;
}

// Callback-функция для вывода результатов запроса
int MusicStoreDB::printCallback(void* notUsed, int argc, char** argv, char** azColName) {
    // Вывод заголовков при первом вызове
    
    if (headers) {
        for (int i = 0; i < argc; i++) {
            std::cout << std::left << std::setw(20) << azColName[i] << " | ";
        }
        std::cout << std::endl;
        std::cout << std::string(argc * 23, '-') << std::endl;
        headers = false;
    }
    
    // Вывод данных
    for (int i = 0; i < argc; i++) {
        std::cout << std::left << std::setw(20) << (argv[i] ? argv[i] : "NULL") << " | ";
    }
    std::cout << std::endl;
    
    return 0;
}

// Аутентификация пользователя
bool MusicStoreDB::login(const std::string& username, const std::string& password) {
    std::string sql = "SELECT user_id, role FROM users WHERE username = ? AND password_hash = ?;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW) {
        userId = sqlite3_column_int(stmt, 0);
        std::string role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        isAdmin = (role == "admin");
        
        sqlite3_finalize(stmt);
        return true;
    }
    
    sqlite3_finalize(stmt);
    return false;
}

// Информация о компакт-дисках
void MusicStoreDB::showCompactInventory() {
    headers=true;
    std::string sql = 
        "SELECT "
        "    cd.compact_id, "
        "    cd.company, "
        "    cd.production_date, "
        "    cd.price, "
        "    COALESCE(received.total_received, 0) AS total_received, "
        "    COALESCE(sold.total_sold, 0) AS total_sold, "
        "    COALESCE(received.total_received, 0) - COALESCE(sold.total_sold, 0) AS remaining, "
        "    (COALESCE(received.total_received, 0) - COALESCE(sold.total_sold, 0)) * cd.price AS stock_value "
        "FROM "
        "    compact_discs cd "
        "LEFT JOIN ( "
        "    SELECT "
        "        compact_id, "
        "        SUM(quantity) AS total_received "
        "    FROM "
        "        operations "
        "    WHERE "
        "        operation_type = 'поступление' "
        "    GROUP BY "
        "        compact_id "
        ") received ON cd.compact_id = received.compact_id "
        "LEFT JOIN ( "
        "    SELECT "
        "        compact_id, "
        "        SUM(quantity) AS total_sold "
        "    FROM "
        "        operations "
        "    WHERE "
        "        operation_type = 'продажа' "
        "    GROUP BY "
        "        compact_id "
        ") sold ON cd.compact_id = sold.compact_id "
        "ORDER BY "
        "    (COALESCE(received.total_received, 0) - COALESCE(sold.total_sold, 0)) DESC;";
    
    std::cout << "\n=== Информация о запасах компакт-дисков ===" << std::endl;
    
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), printCallback, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

// Информация о продажах компакт-диска за период
void MusicStoreDB::showCompactSales(int compactId, const std::string& startDate, const std::string& endDate) {
    std::string sql = 
        "SELECT "
        "    cd.compact_id, "
        "    cd.company, "
        "    cd.production_date, "
        "    cd.price, "
        "    SUM(op.quantity) AS quantity_sold, "
        "    SUM(op.quantity * cd.price) AS total_value "
        "FROM "
        "    operations op "
        "JOIN "
        "    compact_discs cd ON op.compact_id = cd.compact_id "
        "WHERE "
        "    op.operation_type = 'продажа' "
        "    AND op.compact_id = ? "
        "    AND op.operation_date BETWEEN ? AND ? "
        "GROUP BY "
        "    cd.compact_id;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    
    sqlite3_bind_int(stmt, 1, compactId);
    sqlite3_bind_text(stmt, 2, startDate.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, endDate.c_str(), -1, SQLITE_STATIC);
    
    std::cout << "\n=== Информация о продажах компакта #" << compactId << " за период " 
              << startDate << " - " << endDate << " ===" << std::endl;
    
    // Вывод заголовков
    std::cout << std::left 
              << std::setw(10) << "ID" << " | "
              << std::setw(20) << "Компания" << " | "
              << std::setw(15) << "Дата выпуска" << " | "
              << std::setw(10) << "Цена" << " | "
              << std::setw(15) << "Кол-во продано" << " | "
              << std::setw(15) << "Общая сумма" << std::endl;
    std::cout << std::string(95, '-') << std::endl;
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::cout << std::left 
                  << std::setw(10) << sqlite3_column_int(stmt, 0) << " | "
                  << std::setw(20) << sqlite3_column_text(stmt, 1) << " | "
                  << std::setw(15) << sqlite3_column_text(stmt, 2) << " | "
                  << std::setw(10) << sqlite3_column_double(stmt, 3) << " | "
                  << std::setw(15) << sqlite3_column_int(stmt, 4) << " | "
                  << std::setw(15) << sqlite3_column_double(stmt, 5) << std::endl;
    }
    
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }
    
    sqlite3_finalize(stmt);
}

// Расчет статистики за период
void MusicStoreDB::calculatePeriodStatistics(const std::string& startDate, const std::string& endDate) {
    // Очистка предыдущих результатов для этого периода
    std::string clearSQL = 
        "DELETE FROM report_results WHERE start_date = ? AND end_date = ?;";
        
    sqlite3_stmt* clearStmt;
    int rc = sqlite3_prepare_v2(db, clearSQL.c_str(), -1, &clearStmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    
    sqlite3_bind_text(clearStmt, 1, startDate.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(clearStmt, 2, endDate.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(clearStmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(clearStmt);
        return;
    }
    
    sqlite3_finalize(clearStmt);
    
    // Вставка новых результатов
    std::string insertSQL = 
        "INSERT INTO report_results (start_date, end_date, compact_id, received_quantity, sold_quantity) "
        "SELECT "
        "    ?, "
        "    ?, "
        "    cd.compact_id, "
        "    COALESCE((SELECT SUM(quantity) FROM operations "
        "              WHERE compact_id = cd.compact_id "
        "              AND operation_type = 'поступление' "
        "              AND operation_date BETWEEN ? AND ?), 0) AS received_quantity, "
        "    COALESCE((SELECT SUM(quantity) FROM operations "
        "              WHERE compact_id = cd.compact_id "
        "              AND operation_type = 'продажа' "
        "              AND operation_date BETWEEN ? AND ?), 0) AS sold_quantity "
        "FROM "
        "    compact_discs cd;";
        
    sqlite3_stmt* insertStmt;
    rc = sqlite3_prepare_v2(db, insertSQL.c_str(), -1, &insertStmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    
    sqlite3_bind_text(insertStmt, 1, startDate.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insertStmt, 2, endDate.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insertStmt, 3, startDate.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insertStmt, 4, endDate.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insertStmt, 5, startDate.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insertStmt, 6, endDate.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(insertStmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(insertStmt);
        return;
    }
    
    sqlite3_finalize(insertStmt);
    
    // Вывод отчета
    std::string reportSQL = 
        "SELECT "
        "    cd.compact_id, "
        "    cd.company, "
        "    rr.received_quantity, "
        "    rr.sold_quantity, "
        "    rr.received_quantity - rr.sold_quantity AS remaining "
        "FROM "
        "    report_results rr "
        "JOIN "
        "    compact_discs cd ON rr.compact_id = cd.compact_id "
        "WHERE "
        "    rr.start_date = ? AND rr.end_date = ? "
        "ORDER BY "
        "    cd.compact_id;";
        
    sqlite3_stmt* reportStmt;
    rc = sqlite3_prepare_v2(db, reportSQL.c_str(), -1, &reportStmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    
    sqlite3_bind_text(reportStmt, 1, startDate.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(reportStmt, 2, endDate.c_str(), -1, SQLITE_STATIC);
    
    std::cout << "\n=== Отчет по операциям за период " << startDate << " - " << endDate << " ===" << std::endl;
    
    // Вывод заголовков
    std::cout << std::left 
              << std::setw(10) << "ID" << " | "
              << std::setw(20) << "Компания" << " | "
              << std::setw(15) << "Поступило" << " | "
              << std::setw(15) << "Продано" << " | "
              << std::setw(15) << "Остаток" << std::endl;
    std::cout << std::string(85, '-') << std::endl;
    
    while ((rc = sqlite3_step(reportStmt)) == SQLITE_ROW) {
        std::cout << std::left 
                  << std::setw(10) << sqlite3_column_int(reportStmt, 0) << " | "
                  << std::setw(20) << sqlite3_column_text(reportStmt, 1) << " | "
                  << std::setw(15) << sqlite3_column_int(reportStmt, 2) << " | "
                  << std::setw(15) << sqlite3_column_int(reportStmt, 3) << " | "
                  << std::setw(15) << sqlite3_column_int(reportStmt, 4) << std::endl;
    }
    
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }
    
    sqlite3_finalize(reportStmt);
}

// Добавление нового компакт-диска
void MusicStoreDB::addCompactDisc(const std::string& productionDate, const std::string& company, float price) {
    std::string sql = 
        "INSERT INTO compact_discs (production_date, company, price) "
        "VALUES (?, ?, ?);";
        
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    
    sqlite3_bind_text(stmt, 1, productionDate.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, company.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, price);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return;
    }
    
    int compactId = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    
    std::cout << "Добавлен новый компакт-диск с ID: " << compactId << std::endl;
}

// Добавление музыкального произведения
void MusicStoreDB::addMusicalWork(const std::string& title, const std::string& author, 
                  const std::string& performer, int compactId) {
    std::string sql = 
        "INSERT INTO musical_works (title, author, performer, compact_id) "
        "VALUES (?, ?, ?, ?);";
        
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    
    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, author.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, performer.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, compactId);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return;
    }
    
    int workId = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    
    std::cout << "Добавлено новое музыкальное произведение с ID: " << workId << std::endl;
}

// Регистрация операции (поступление/продажа)
void MusicStoreDB::registerOperation(const std::string& operationType, int compactId, int quantity) {
    // Получение текущей даты
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    char dateStr[11];
    std::strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", now);
    
    std::string sql = 
        "INSERT INTO operations (operation_date, operation_type, compact_id, quantity) "
        "VALUES (?, ?, ?, ?);";
        
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    
    sqlite3_bind_text(stmt, 1, dateStr, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, operationType.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, compactId);
    sqlite3_bind_int(stmt, 4, quantity);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return;
    }
    
    int operationId = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    
    std::cout << "Зарегистрирована операция (" << operationType << ") с ID: " << operationId << std::endl;
}

// Обновление информации о компакт-диске
void MusicStoreDB::updateCompactDisc(int compactId, const std::string& company, float price) {
    std::string sql = 
        "UPDATE compact_discs SET company = ?, price = ? WHERE compact_id = ?;";
        
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    
    sqlite3_bind_text(stmt, 1, company.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, price);
    sqlite3_bind_int(stmt, 3, compactId);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return;
    }
    
    sqlite3_finalize(stmt);
    
    std::cout << "Обновлена информация о компакт-диске с ID: " << compactId << std::endl;
}

// Удаление компакт-диска
void MusicStoreDB::deleteCompactDisc(int compactId) {
    std::string sql = "DELETE FROM compact_discs WHERE compact_id = ?;";
        
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    
    sqlite3_bind_int(stmt, 1, compactId);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return;
    }
    
    sqlite3_finalize(stmt);
    
    std::cout << "Удален компакт-диск с ID: " << compactId << std::endl;
}

// Информация о самом популярном компакт-диске
void MusicStoreDB::showMostPopularCompact() {
    headers=true;
    std::string sql = 
        "    SELECT "
        "        compact_id, "
        "        SUM(quantity) AS total_sold "
        "    FROM "
        "        operations "
        "    WHERE "
        "        operation_type = 'продажа' "
        "    GROUP BY "
        "        compact_id "
        "    ORDER BY "
        "        total_sold DESC "
        "    LIMIT 1 ;";
        
    std::cout << "\n=== Самый популярный компакт-диск ===" << std::endl;
    
    // Проверка наличия данных о продажах
    std::string checkSQL = 
        "SELECT COUNT(*) FROM operations WHERE operation_type = 'продажа';";
    
    std::vector<std::vector<std::string>> checkResults;
    char* checkErrMsg = nullptr;
    int checkRc = sqlite3_exec(db, checkSQL.c_str(), callback, &checkResults, &checkErrMsg);
    
    if (checkRc != SQLITE_OK) {
        std::cerr << "SQL error при проверке наличия продаж: " << checkErrMsg << std::endl;
        sqlite3_free(checkErrMsg);
        return;
    }
    
    // Если нет продаж, выводим сообщение
    if (checkResults.size() > 0 && checkResults[0].size() > 0 && checkResults[0][0] == "0") {
        std::cout << "Нет данных о продажах компакт-дисков." << std::endl;
        return;
    }
    
    // Сброс статуса вывода заголовков для printCallback
    // Используем отдельный запрос для сброса
    
    // Выполнение основного запроса
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), printCallback, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return;
    }
    // Дополнительный вывод произведений на этом компакт-диске
    if (checkResults.size() > 0 && checkResults[0].size() > 0 && checkResults[0][0] != "0") {
        std::string compactIdSQL = 
            "SELECT compact_id FROM operations "
            "WHERE operation_type = 'продажа' "
            "GROUP BY compact_id "
            "ORDER BY SUM(quantity) DESC "
            "LIMIT 1;";
            
        std::vector<std::vector<std::string>> compactIdResults;
        sqlite3_exec(db, compactIdSQL.c_str(), callback, &compactIdResults, nullptr);
        
        if (compactIdResults.size() > 0 && compactIdResults[0].size() > 0) {
            headers=true;
            std::string worksSql = 
                "SELECT work_id, title, author, performer "
                "FROM musical_works "
                "WHERE compact_id = " + compactIdResults[0][0] + ";";
                
            std::cout << "\n=== Музыкальные произведения на самом популярном компакт-диске ===" << std::endl;
            
            // Сброс заголовков
            
            char* worksErrMsg = nullptr;
            int worksRc = sqlite3_exec(db, worksSql.c_str(), printCallback, nullptr, &worksErrMsg);
            
            if (worksRc != SQLITE_OK) {
                std::cerr << "SQL error при получении произведений: " << worksErrMsg << std::endl;
                sqlite3_free(worksErrMsg);
            }
        }
    }
}
// Реализация метода showMostPopularPerformer
void MusicStoreDB::showMostPopularPerformer() {
    headers=true;
    std::string sql = 
        "WITH PerformerSales AS ( "
        "    SELECT "
        "        mw.performer, "
        "        SUM(op.quantity) AS total_sold "
        "    FROM "
        "        operations op "
        "    JOIN "
        "        musical_works mw ON op.compact_id = mw.compact_id "
        "    WHERE "
        "        op.operation_type = 'продажа' "
        "    GROUP BY "
        "        mw.performer "
        "    ORDER BY "
        "        total_sold DESC "
        "    LIMIT 1 "
        ") "
        "SELECT "
        "    ps.performer, "
        "    ps.total_sold, "
        "    mw.title, "
        "    mw.author, "
        "    cd.company "
        "FROM "
        "    PerformerSales ps "
        "JOIN "
        "    musical_works mw ON ps.performer = mw.performer "
        "JOIN "
        "    compact_discs cd ON mw.compact_id = cd.compact_id;";
        
    std::cout << "\n=== Самый популярный исполнитель ===" << std::endl;
    
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), printCallback, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

// Реализация метода showAuthorSales
void MusicStoreDB::showAuthorSales() {
    headers=true;
    std::string sql = 
        "SELECT "
        "    mw.author, "
        "    SUM(op.quantity) AS total_sold, "
        "    COUNT(DISTINCT mw.work_id) AS works_count, "
        "    SUM(op.quantity * cd.price) AS total_revenue "
        "FROM "
        "    operations op "
        "JOIN "
        "    musical_works mw ON op.compact_id = mw.compact_id "
        "JOIN "
        "    compact_discs cd ON op.compact_id = cd.compact_id "
        "WHERE "
        "    op.operation_type = 'продажа' "
        "GROUP BY "
        "    mw.author "
        "ORDER BY "
        "    total_sold DESC;";
        
    std::cout << "\n=== Продажи по авторам ===" << std::endl;
    
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), printCallback, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

// Реализация метода getCompactSalesInfo
void MusicStoreDB::getCompactSalesInfo(int compactId, const std::string& startDate, const std::string& endDate) {
    // Этот метод похож на showCompactSales, но с другим форматированием вывода
    // для обычных пользователей
    headers=true;
    std::string sql = 
        "SELECT "
        "    cd.compact_id, "
        "    cd.company, "
        "    cd.production_date, "
        "    cd.price, "
        "    SUM(op.quantity) AS quantity_sold, "
        "    SUM(op.quantity * cd.price) AS total_value "
        "FROM "
        "    operations op "
        "JOIN "
        "    compact_discs cd ON op.compact_id = cd.compact_id "
        "WHERE "
        "    op.operation_type = 'продажа' "
        "    AND op.compact_id = ? "
        "    AND op.operation_date BETWEEN ? AND ? "
        "GROUP BY "
        "    cd.compact_id;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    
    sqlite3_bind_int(stmt, 1, compactId);
    sqlite3_bind_text(stmt, 2, startDate.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, endDate.c_str(), -1, SQLITE_STATIC);
    
    std::cout << "\n=== Информация о продажах компакт-диска #" << compactId << " за период " 
              << startDate << " - " << endDate << " ===" << std::endl;
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        std::cout << "Компания-производитель: " << sqlite3_column_text(stmt, 1) << std::endl;
        std::cout << "Дата производства: " << sqlite3_column_text(stmt, 2) << std::endl;
        std::cout << "Цена: " << sqlite3_column_double(stmt, 3) << std::endl;
        std::cout << "Количество проданных экземпляров: " << sqlite3_column_int(stmt, 4) << std::endl;
        std::cout << "Общая сумма продаж: " << sqlite3_column_double(stmt, 5) << std::endl;
    } else {
        std::cout << "Нет данных о продажах для указанного компакт-диска за указанный период." << std::endl;
    }
    
    sqlite3_finalize(stmt);
}
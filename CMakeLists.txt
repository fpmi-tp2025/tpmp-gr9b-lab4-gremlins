cmake_minimum_required(VERSION 3.10)
project(MusicStore VERSION 1.0 LANGUAGES CXX)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Опции проекта
option(BUILD_SHARED_LIBS "Сборка с использованием динамических библиотек" OFF)

# Поиск SQLite3
find_package(SQLite3 REQUIRED)
if(NOT SQLite3_FOUND)
    message(FATAL_ERROR "SQLite3 не найден. Пожалуйста, установите SQLite3")
endif()

# Добавление каталога с заголовочными файлами
include_directories(include ${SQLite3_INCLUDE_DIRS})

# Определение исходных файлов
set(SOURCES
    src/main.cpp
    src/MusicStoreDB.cpp
    src/UserInterface.cpp
)

# Создание исполняемого файла
add_executable(music_store_app ${SOURCES})

# Связывание с SQLite3
target_link_libraries(music_store_app PRIVATE ${SQLite3_LIBRARIES})

target_compile_options(music_store_app PRIVATE -Wall -Wextra -Wpedantic)

# Установка
install(TARGETS music_store_app 
        RUNTIME DESTINATION bin)

cmake_minimum_required(VERSION 3.10)
project(MusicStore VERSION 1.0 LANGUAGES CXX)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Опции проекта
option(BUILD_SHARED_LIBS "Сборка с использованием динамических библиотек" OFF)
option(BUILD_TESTS "Build the tests" ON)
option(CODE_COVERAGE "Enable code coverage" ON)

# Enable testing functionality
enable_testing()

# Code coverage setup
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(STATUS "Enabling code coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# Поиск SQLite3
find_package(SQLite3 REQUIRED)
if(NOT SQLite3_FOUND)
    message(FATAL_ERROR "SQLite3 не найден. Пожалуйста, установите SQLite3")
endif()

# Добавление каталога с заголовочными файлами
include_directories(include ${SQLite3_INCLUDE_DIRS})

# Define source files without main.cpp
set(LIB_SOURCES
    src/MusicStoreDB.cpp
    src/UserInterface.cpp
)

# Create a library for testing
add_library(music_store_lib STATIC ${LIB_SOURCES})
target_link_libraries(music_store_lib PRIVATE ${SQLite3_LIBRARIES})
target_include_directories(music_store_lib PUBLIC include)

# Определение исходных файлов для основного приложения
set(APP_SOURCES
    src/main.cpp
)

# Создание исполняемого файла
add_executable(music_store_app ${APP_SOURCES})
target_link_libraries(music_store_app PRIVATE music_store_lib)

# Compile options
target_compile_options(music_store_app PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(music_store_lib PRIVATE -Wall -Wextra -Wpedantic)

# Установка
install(TARGETS music_store_app 
        RUNTIME DESTINATION bin)

# Add tests subdirectory if testing is enabled
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Add a custom target for generating coverage report
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    find_program(LCOV lcov REQUIRED)
    find_program(GENHTML genhtml REQUIRED)
    
    add_custom_target(coverage
        # Cleanup lcov
        COMMAND ${LCOV} --directory . --zerocounters
        
        # Run tests
        COMMAND ctest
        
        # Capture lcov counters and generate report
        COMMAND ${LCOV} --directory . --capture --output-file coverage.info --ignore-errors mismatch --ignore-errors source
        
        # Filter out system and test files
        COMMAND ${LCOV} --remove coverage.info '/usr/*' '*/tests/*' '*/googletest/*' --output-file coverage.info
        
        # Generate HTML report
        COMMAND ${GENHTML} --output-directory coverage_report coverage.info
        
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating code coverage report"
    )
endif()
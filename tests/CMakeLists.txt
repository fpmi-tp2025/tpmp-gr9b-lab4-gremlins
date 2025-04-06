# Find Google Test package
find_package(GTest QUIET)

# If not found, fetch it
if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
endif()
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Find SQLite3 package
find_package(SQLite3 REQUIRED)

# Add the thread_tests executable
add_executable(thread_tests 
    thread_tests.cpp
)

# Add music_store_db_tests executable
add_executable(music_store_db_tests
    music_store_db_tests.cpp
)

# Add user_interface_tests executable
add_executable(user_interface_tests
    user_interface_tests.cpp
)

# Include directories for test executables
target_include_directories(thread_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${SQLite3_INCLUDE_DIRS}
)
target_include_directories(music_store_db_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${SQLite3_INCLUDE_DIRS}
)
target_include_directories(user_interface_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${SQLite3_INCLUDE_DIRS}
)

# Link with libraries - music_store_lib should be available from the parent CMakeLists.txt
target_link_libraries(thread_tests PRIVATE 
    music_store_lib 
    gtest_main
    ${SQLite3_LIBRARIES}
)
target_link_libraries(music_store_db_tests PRIVATE 
    music_store_lib 
    gtest_main
    ${SQLite3_LIBRARIES}
)
target_link_libraries(user_interface_tests PRIVATE 
    music_store_lib 
    gtest_main
    ${SQLite3_LIBRARIES}
)

# Add filesystem library if using GCC 7 or 8
if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(thread_tests PRIVATE stdc++fs)
    target_link_libraries(music_store_db_tests PRIVATE stdc++fs)
    target_link_libraries(user_interface_tests PRIVATE stdc++fs)
endif()

# Add the tests
add_test(NAME thread_tests COMMAND thread_tests)
add_test(NAME music_store_db_tests COMMAND music_store_db_tests)
add_test(NAME user_interface_tests COMMAND user_interface_tests)
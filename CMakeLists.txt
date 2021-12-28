cmake_minimum_required(VERSION 3.16)

project(main)

add_executable(main main.cpp)

if (WIN32)
    add_definitions(-mwindows)
endif (WIN32)

install(TARGETS main)

enable_testing()
add_test(NAME main COMMAND main)
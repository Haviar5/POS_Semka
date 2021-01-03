cmake_minimum_required(VERSION 3.7)
project(Semestralka C)

set(CMAKE_C_STANDARD 99)

add_executable(Semestralka main.c)
target_link_libraries(Semestralka pthread)
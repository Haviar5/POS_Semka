cmake_minimum_required(VERSION 3.7)
project(POS_Server C)

set(CMAKE_C_STANDARD 99)

add_executable(POS_Server main.c)

target_link_libraries(POS_Server pthread)
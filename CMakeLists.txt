cmake_minimum_required(VERSION 3.7)
project(POS_Semka-Klient C)

set(CMAKE_C_STANDARD 99)

add_executable(POS_Semka-Klient main.c)
target_link_libraries(POS_Semka-Klient pthread)
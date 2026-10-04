cmake_minimum_required(VERSION 3.20)
project(Crumb)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(crumb 
    src/main.cpp
    src/board.cpp 
    
    include/crumb/bitboard.hpp 
    include/crumb/board.hpp
    include/crumb/core.hpp
)

target_include_directories(crumb PRIVATE include)

target_compile_options(crumb PRIVATE
    $<$<CONFIG:Debug>:
        -O0
        -g
        -fsanitize=address,undefined
    >
    $<$<CONFIG:Release>:
        -O3
        -march=native
        -flto
    >
    -Wall
    -Wextra
    -Wpedantic
)

target_link_options(crumb PUBLIC
    $<$<CONFIG:Debug>:
        -fsanitize=address,undefined
    >
    $<$<CONFIG:Release>:
        -flto
    >
)
cmake_minimum_required(VERSION 3.20)
project(crumb)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(crumb
    src/attacks.cpp
    src/board.cpp
    src/main.cpp
    src/movegen.cpp 
)

target_include_directories(crumb PRIVATE src)

# object generation
target_compile_options(crumb PRIVATE
    # debug flags
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>

    # release flags
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Release>:-march=native>
    $<$<CONFIG:Release>:-mbmi2>
    $<$<CONFIG:Release>:-flto>
    $<$<CONFIG:Release>:-DNDEBUG>
)

# linking
target_link_options(crumb PRIVATE
    $<$<CONFIG:Release>:-flto>
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)
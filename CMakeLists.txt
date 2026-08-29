#    Crumb is a UCI chess engine
#    Copyright (C) 2026  GoobusTheNoobus
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.20)
project(crumb)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(crumb
    src/attacks.cpp
    src/board.cpp
    src/eval.cpp
    src/main.cpp
    src/movegen.cpp 
    src/perft.cpp
    src/search.cpp
    src/uci.cpp
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
/*
    Crumb is a UCI chess engine
    Copyright (C) 2026  GoobusTheNoobus

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include "core.hpp"
namespace crumb
{
    
constexpr u64 square_mask(Square sq)
{
    return 1ULL << (int)sq;
}

constexpr u64 file_mask(int file)
{
    return 0x0101010101010101ULL << file;
}

constexpr u64 rank_mask(int rank)
{
    return 0xFFULL << (rank * 8);
}

constexpr bool is_occupied(u64 bb, Square sq)
{
    return square_mask(sq) & bb;
}

constexpr int trailing_zero(u64 bb) 
{
    return std::__countr_zero(bb);
}

constexpr int popcount(u64 bb)
{
    return std::__popcount(bb);
}

inline int pop_lsb(u64& bb)
{
    int tz = trailing_zero(bb);
    bb &= bb - 1;
    return tz;
}
}
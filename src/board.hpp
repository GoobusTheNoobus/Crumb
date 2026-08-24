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
#include <ostream>

namespace crumb
{
    constexpr u8 CASTLING_WK = 1, CASTLING_WQ = 2, CASTLING_BK = 4, CASTLING_BQ = 8;
    constexpr const char* FEN_STARTING = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    struct Board 
    {
        Board() = default;
        Board(const std::string& fen);

        u64 piece_bb[PIECETYPE_NB] = {0x00FF00000000FF00, 0x4200000000000042, 0x2400000000000024,
            0x8100000000000081, 0x0800000000000008, 0x1000000000000010}; // indexed by piece type ordinal
        u64 color_bb[COLOR_NB] = {0xFFFF, 0xFFFF000000000000};     // indexed by color type ordinal
        u64 occ = 0xFFFF00000000FFFF;

        Color side_to_move      = Color::WHITE;
        Square ep_square        = Square::NONE;
        u8 castling_rights      = 0;
        u8 halfmove_clock       = 0;
    };

    std::ostream& operator<<(std::ostream&, const Board&);
}
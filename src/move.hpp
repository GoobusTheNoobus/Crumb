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
    using Move = u32;
    enum class MoveFlag : u8 { NORMAL, DOUBLE_PUSH, CASTLING, EN_PASSANT, PROMOQ, PROMOR, PROMOB, PROMON };

    /*
    Move Packing:
    0-5 from
    6-11 to
    12-15 piece
    16-19 flag
    */

    constexpr Square move_from(Move move) 
    {
        return Square(move & 0x3F);
    }

    constexpr Square move_to(Move move)
    {
        return Square((move >> 6) & 0x3F);
    }

    constexpr Piece move_piece(Move move)
    {
        return Piece((move >> 12) & 0xF);
    }

    constexpr MoveFlag move_flag(Move move)
    {
        return MoveFlag((move >> 16) & 0x0F);
    }

    constexpr Move create_move(Square from, Square to, Piece moving, MoveFlag flag)
    {
        return (int)from | (int)to << 6 | (int)moving << 12 | (int)flag << 16;
    }

    inline std::string to_string(Move move)
    {
        std::string from = algebraic(move_from(move));
        std::string to   = algebraic(move_to(move));

        if (move_flag(move) >= MoveFlag::PROMOQ)
        {
            constexpr char PROMO_CHAR[] = {'q', 'r', 'b', 'n'};
            return from + to + PROMO_CHAR[(int)move_flag(move) - (int)MoveFlag::PROMOQ];
        }

        return from + to;
    }
}
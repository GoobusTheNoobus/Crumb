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

namespace crumb::attacks
{
    void load_attacks();

    u64 get_pawn_attacks(Square sq, Color);
    u64 get_knight_attacks(Square sq);
    u64 get_king_attacks(Square sq);

    u64 get_bishop_attacks(Square sq, u64 blockers);
    u64 get_rook_attacks(Square sq, u64 blockers);
}
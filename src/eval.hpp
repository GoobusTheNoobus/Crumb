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

#include "board.hpp"
#include "core.hpp"
namespace crumb::eval
{

void load_eval_tables();

int get_phase(const u64 piece_bb[]);
int mg_table(Color, PieceType, Square);
int eg_table(Color, PieceType, Square);

int evaluate(const Board&);

}
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
#include "movegen.hpp"

namespace crumb {
class OrderingMoveList {
public:
    OrderingMoveList(const Board& board, Move tt_move) {
        MoveGenerator generator;
        count = generator.generate_moves(board, moves);

        for (int i = 0; i < count; ++i)
            scores[i] = calculate_score(moves[i], board, tt_move);
    }

    inline Move operator[](int i) const { return moves[i]; }
    inline usize size() const { return count; }

    bool next(int i);

private:
    Move moves[256];
    int scores[256];
    usize count;

    int calculate_score(Move move, const Board& board, Move tt_move) const;
};

} // namespace crumb
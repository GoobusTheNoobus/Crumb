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

#include "perft.hpp"
#include "move.hpp"
#include "movelist.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>

namespace crumb::perft {

void perft_divide(const Board& board, Depth depth) {
    if (depth <= 0) {
        std::cout << "Perft depth must be more than 0\n";
        return;
    }

    auto start = std::chrono::steady_clock::now();

    MoveList moves(board);
    u64 total_nodes = 0;

    for (int i = 0; i < moves.size(); ++i) {
        Move move = moves[i];

        Board child = board;
        bool is_legal = child.try_move(move);
        if (!is_legal)
            continue;

        u64 nodes = perft(child, depth - 1);

        total_nodes += nodes;

        std::cout << "Move #" << (i + 1) << ": " << to_string(move) << " -> " << nodes << '\n';
    }

    auto end = std::chrono::steady_clock::now();

    int elapsed = std::max<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(), 1);

    std::cout << "\nTotal nodes searched: " << total_nodes << '\n';
    std::cout << "Elapsed: " << elapsed << '\n';
    std::cout << "Nodes per second: " << (total_nodes * 1000 / elapsed) << '\n' << std::endl;
}

u64 perft(const Board& board, Depth depth) {
    if (depth <= 0) {
        return 1;
    }

    MoveList moves(board);
    u64 total_nodes = 0;

    for (int i = 0; i < moves.size(); ++i) {
        Move move = moves[i];

        Board child = board;
        bool is_legal = child.try_move(move);
        if (!is_legal)
            continue;

        u64 nodes = perft(child, depth - 1);

        total_nodes += nodes;
    }

    return total_nodes;
}

} // namespace crumb::perft
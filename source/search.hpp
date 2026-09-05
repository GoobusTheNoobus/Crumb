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
#include "tt.hpp"
#include <atomic>
#include <chrono>
namespace crumb {

struct Timer {
    TimePoint start_time;
    int max_ms;
    std::atomic_bool stop_flag;

    inline int elapsed() {
        return std::max<int>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                 std::chrono::steady_clock::now() - start_time)
                                 .count(),
                             1);
    }

    inline bool should_stop() {
        bool yes = stop_flag || (max_ms > 0 && elapsed() >= max_ms);

        return yes;
    }
};

struct Info {
    u64 nodes_searched = 0;
    Move best_move = 0;
    Depth seldepth = 0;
};

class Searcher {
public:
    Searcher() = default;

    Board board;
    HashStack hashes;

    void start_search(Depth depth, int max_ms);
    void stop_search();

private:
    Timer timer;
    TranspositionTable tt;

    enum class NodeType : u8 { ROOT, NON_PV, PV };

    template <NodeType type>
    Score search(Info& info, const Board& board, Depth depth, Depth plies, Score alpha, Score beta);
    Score qsearch(Info& info, const Board& board, Depth plies, Score alpha, Score beta);

    bool is_noisy(const Board&, Move) const;
    bool is_terminal() const;

    static constexpr Score RFP_BASE = 150;
    static constexpr Score ASPIR_WINDOW = 50;
    static constexpr Score ASPIR_EXPANSION = 3;
};

} // namespace crumb
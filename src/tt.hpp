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
#include "move.hpp"
#include <vector>

namespace crumb {

inline constexpr usize DEFAULT_TT_MB = 64;

enum class TTFlag {
    EXACT,
    LOWERBOUND,
    UPPERBOUND,
};

struct TranspositionEntry {
    u64 full_key;
    Move best_move;
    Score score;
    Depth depth;
    TTFlag flag;

    inline bool is_null() { return best_move == 0; }
};

class TranspositionTable {
public:
    TranspositionTable();

    const TranspositionEntry* probe(u64 key) const;
    void store(TranspositionEntry);

    int hashfull() const;
    void clear();

private:
    std::vector<TranspositionEntry> data;

    usize mb_to_size(int mb) const;
    usize index_of(u64 key) const;
};
} // namespace crumb
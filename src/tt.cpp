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

#include "tt.hpp"
#include <cstring>

namespace crumb {

TranspositionTable::TranspositionTable() { data.resize(mb_to_size(DEFAULT_TT_MB)); }

const TranspositionEntry* TranspositionTable::probe(u64 key) const {
    int index = index_of(key);

    if (data[index].full_key != key)
        return nullptr;

    return &data[index];
}

void TranspositionTable::store(TranspositionEntry entry) {
    int index = index_of(entry.full_key);

    if (data[index].depth <= entry.depth) {
        data[index] = entry;
    }
}

int TranspositionTable::hashfull() const {

    int count = 0;
    for (int i = 0; i < 1000; ++i) {
        if (data[i].depth > 0) {
            ++count;
        }
    }

    return count;
}

void TranspositionTable::clear() {

    std::memset(data.data(), 0, data.size() * sizeof(TranspositionEntry));
}

usize TranspositionTable::mb_to_size(int mb) const {
    return mb * 1024 * 1024 / sizeof(TranspositionEntry);
}

usize TranspositionTable::index_of(u64 key) const { return key % data.size(); }

} // namespace crumb
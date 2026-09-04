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
#include <random>
namespace crumb::zobrist {
inline u64 piece_square[PIECE_NB][BOARD_SIZE];
inline u64 castling[4];
inline u64 ep_files[BOARD_WIDTH];
inline u64 black_to_move;

// we fill all variables with random value for
// zobrist hashing
inline void load_randoms() {
    std::mt19937_64 random(67);

    // piece_square
    for (int p = 0; p < PIECE_NB; ++p) {
        for (int sq = 0; sq < BOARD_SIZE; ++sq) {
            piece_square[p][sq] = random();
        }
    }

    // castling
    for (int i = 0; i < 4; ++i) {
        castling[i] = random();
    }

    // ep_files
    for (int i = 0; i < BOARD_WIDTH; ++i) {
        ep_files[i] = random();
    }

    black_to_move = random();
}

inline u64 castling_hash(u8 rights) {
    u64 hash = 0;

    hash ^= (rights & CASTLING_WK) ? castling[0] : 0;
    hash ^= (rights & CASTLING_WQ) ? castling[1] : 0;
    hash ^= (rights & CASTLING_BK) ? castling[2] : 0;
    hash ^= (rights & CASTLING_BQ) ? castling[3] : 0;

    return hash;
}

inline u64 compute_hash(const Board& board) {
    u64 hash = 0;

    for (int sq = 0; sq < BOARD_SIZE; ++sq) {
        if (board.mailbox[sq] != Piece::NONE)
            hash ^= piece_square[(int)board.mailbox[sq]][sq];
    }

    hash ^= castling_hash(board.hash);

    hash ^= board.ep_square != Square::NONE ? ep_files[file_of(board.ep_square)] : 0;
    hash ^= (bool)board.side_to_move ? black_to_move : 0;

    return hash;
}
} // namespace crumb::zobrist
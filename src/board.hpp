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
#include <optional>
#include <ostream>

namespace crumb {
constexpr u8 CASTLING_WK = 1, CASTLING_WQ = 2, CASTLING_BK = 4, CASTLING_BQ = 8;

constexpr const char* FEN_STARTING = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
constexpr const char* FEN_KIWIPETE =
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
constexpr const char* FEN_EN_PASSANT =
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"; // named that because it tests en passant bugs in
                                                 // perft

struct HashStack {
    u64 hashes[2048];
    usize count = 0;

    bool is_repetition(u64 current, u8 rule50) const;
};

struct Board {
    Board() = default;
    Board(const std::string& fen);
    Board(const Board& other) = default;

    bool is_attacked(Square, Color by) const;
    bool is_in_check(Color by) const;
    bool is_in_check() const;

    void load_fen(const std::string& fen);
    void make_move(Move);
    bool try_move(Move);

    std::optional<Move> parse(const std::string&);

    void clear_square(Square square);
    void place_piece(Square square, Piece piece);

    Piece mailbox[BOARD_SIZE];

    u64 piece_bb[PIECETYPE_NB]; // indexed by piece type ordinal
    u64 color_bb[COLOR_NB];     // indexed by color type ordinal
    u64 occ;
    u64 hash;

    Score mg_score = 0;
    Score eg_score = 0;

    Color side_to_move = Color::WHITE;
    Square ep_square = Square::NONE;
    u8 castling_rights = 0;
    u8 halfmove_clock = 0;
};

std::ostream& operator<<(std::ostream&, const Board&);

} // namespace crumb
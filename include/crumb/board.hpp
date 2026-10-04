/*
 * Copyright (c) 2026 Goobus
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "core.hpp"
#include <array>
#include <string>

namespace crumb {

struct Board {

    // constructors

    // default constructor, sets up starting position
    Board();

    // copy constructor
    Board(const Board&) = default;

    // from fen constructor
    Board(const std::string& fen);

    void parse_fen(const std::string& fen);

    // primary board representation: mailbox array
    std::array<Piece, BoardSize> mailbox{};

    // secondary board representation: bitboards

    std::array<u64, PieceTypeNum> piece_bitboards{};
    std::array<u64, ColourNum> colour_bitboards{};

    // other game states
    Colour side_to_move      = Colour::White;
    Square en_passant_square = Square::None;
    u8 fifty_move_clock      = 0;
    u8 castling_rights       = 0; // use bitmasks declared below to add or remove

    inline static constexpr u8 CastlingWK = 1, CastlingWQ = 2, CastlingBK = 4, CastlingBQ = 8;
    inline static constexpr char StartingFen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
};

std::ostream& operator<<(std::ostream&, const Board&);

} // namespace crumb
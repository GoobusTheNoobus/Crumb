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

#include "crumb/board.hpp"
#include "crumb/bitboard.hpp"
#include "crumb/core.hpp"
#include <cassert>
#include <cctype>
#include <ostream>
#include <sstream>

namespace crumb {

namespace {

// converts a Piece to a character representation (White Knight -> 'N')
constexpr char piece_char(Piece piece) {
    constexpr std::array<char, PieceNum + 1> map = {'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k', '.'};
    return map[static_cast<i32>(piece)];
}

// converts a character to a Piece ('p' -> Black Pawn)
constexpr Piece char_piece(char c) {
    switch (c) {
    case 'P':
        return Piece::WhitePawn;
    case 'N':
        return Piece::WhiteKnight;
    case 'B':
        return Piece::WhiteBishop;
    case 'R':
        return Piece::WhiteRook;
    case 'Q':
        return Piece::WhiteQueen;
    case 'K':
        return Piece::WhiteKing;
    case 'p':
        return Piece::BlackPawn;
    case 'n':
        return Piece::BlackKnight;
    case 'b':
        return Piece::BlackBishop;
    case 'r':
        return Piece::BlackRook;
    case 'q':
        return Piece::BlackQueen;
    case 'k':
        return Piece::BlackKing;
    default:
        return Piece::None;
    }
}

} // namespace

// printing support for Board
std::ostream& operator<<(std::ostream& os, const Board& board) {
    for (i32 rank = BoardHeight - 1; rank >= 0; --rank) {

        // add rank label followed by a space
        os << rank + 1 << ' ';
        for (i32 file = 0; file < BoardWidth; ++file) {
            os << piece_char(board.mailbox[static_cast<i32>(make_square(rank, file))]) << ' ';
        }
        os << '\n';
    }

    // add bottom file labels before returning
    return os << "  a b c d e f g h\n\n";
}

Board::Board() : Board(StartingFen) {
}

Board::Board(const std::string& fen) {
    parse_fen(fen);
}

void Board::parse_fen(const std::string& fen) {

    // we clear all board states before doing work
    mailbox          = {};
    piece_bitboards  = {};
    colour_bitboards = {};

    side_to_move      = Colour::White;
    en_passant_square = Square::None;
    fifty_move_clock  = 0;
    castling_rights   = 0;

    std::istringstream stream(fen);
    std::string token;

#define RETURN_IF_STREAM_EMPTY                                                                                         \
    if (!(stream >> token))                                                                                            \
    return

    RETURN_IF_STREAM_EMPTY;

    // parse board part
    i32 rank = 7, file = 0;
    for (unsigned char c : token) {

        // piece character
        if (std::isalpha(c)) {
            Piece piece   = char_piece(c);
            Square square = make_square(rank, file);

            if (piece == Piece::None)
                continue;

            mailbox[static_cast<i32>(square)] = piece;
            bitboard::set_bit(&piece_bitboards[static_cast<i32>(type_of(piece))], square);
            bitboard::set_bit(&colour_bitboards[static_cast<i32>(colour_of(piece))], square);

            ++file;
        }

        // next rank character
        else if (c == '/') {
            assert(file == 8);

            file = 0;
            --rank;
        }

        // number character
        else if (std::isdigit(c)) {
            file += c - '0';
        }

        // we ignore everything else
    }

    RETURN_IF_STREAM_EMPTY;

    // parse side to move part
    if (token == "b")
        side_to_move = Colour::Black;

    RETURN_IF_STREAM_EMPTY;

    // parse castling rights

    for (char c : token) {
        switch (c) {
        case 'K':
            castling_rights |= CastlingWK;
            break;
        case 'Q':
            castling_rights |= CastlingWQ;
            break;
        case 'k':
            castling_rights |= CastlingBK;
            break;
        case 'q':
            castling_rights |= CastlingBQ;
            break;
        }
    }

    RETURN_IF_STREAM_EMPTY;

    // parse en passant square

    en_passant_square = make_square(token);

    RETURN_IF_STREAM_EMPTY;

    // parse rule 50 clock

    fifty_move_clock = std::stoi(token);

#undef RETURN_IF_STREAM_EMPTY
}
} // namespace crumb
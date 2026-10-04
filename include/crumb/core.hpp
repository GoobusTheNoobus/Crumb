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

#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>

namespace crumb {

// Rust style types because prettier and more reliable

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;
using f64 = double;

using usize = std::size_t;
using isize = std::ptrdiff_t;

// clang-format off

// squares are represented as enumerates while ranks and files are just 32-bit integers

enum class Square : u8 {
    A1, B1, C1, D1, E1, F1, G1, H1, 
    A2, B2, C2, D2, E2, F2, G2, H2, 
    A3, B3, C3, D3, E3, F3, G3, H3, 
    A4, B4, C4, D4, E4, F4, G4, H4, 
    A5, B5, C5, D5, E5, F5, G5, H5, 
    A6, B6, C6, D6, E6, F6, G6, H6, 
    A7, B7, C7, D7, E7, F7, G7, H7, 
    A8, B8, C8, D8, E8, F8, G8, H8, 
    None,
};

// clang-format on
inline constexpr i32 BoardHeight = 8, BoardWidth = 8, BoardSize = 64;

// colours, piece types, and pieces are all represented using enumerates

enum class Colour : u8 { White, Black };
enum class PieceType : u8 { Pawn, Knight, Bishop, Rook, Queen, King };
enum class Piece : u8 {
    WhitePawn,
    WhiteKnight,
    WhiteBishop,
    WhiteRook,
    WhiteQueen,
    WhiteKing,
    BlackPawn,
    BlackKnight,
    BlackBishop,
    BlackRook,
    BlackQueen,
    BlackKing,
    None,
};

inline constexpr i32 ColourNum = 2, PieceTypeNum = 6, PieceNum = 12;

// helper/conversion functions for these core types

inline constexpr Square make_square(i32 rank, i32 file) {
    return static_cast<Square>(rank << 3 | file);
}

inline constexpr i32 file_of(Square square) {
    return static_cast<i32>(square) & 7;
}

inline constexpr i32 rank_of(Square square) {
    return static_cast<i32>(square) >> 3;
}

inline constexpr Piece make_piece(Colour colour, PieceType type) {
    return static_cast<Piece>(static_cast<int>(colour) * 6 + static_cast<int>(type));
}

inline constexpr Colour colour_of(Piece piece) {
    return static_cast<Colour>(static_cast<i32>(piece) / 6);
}

inline constexpr PieceType type_of(Piece piece) {
    return static_cast<PieceType>(static_cast<i32>(piece) % 6);
}

// use "~some_colour" to get the opposite colour
inline constexpr Colour operator~(Colour colour) {
    return static_cast<Colour>(static_cast<i32>(colour) ^ 1);
}

// converts an algebraic notation string like "a3" to the enum
inline constexpr Square make_square(const std::string& str) {

    if (str == "-") {
        return Square::None;
    }

    assert(str.size() == 2);

    return make_square(str[1] - '1', str[0] - 'a');
}

} // namespace crumb
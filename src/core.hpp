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
#include <cassert>
#include <cstdint>
#include <string>

namespace crumb
{
    // define type aliases because c++ types are ugly af

    using u64 = uint64_t;
    using u32 = uint32_t;
    using u16 = uint16_t;
    using u8  = uint8_t;

    using i64 = int64_t;
    using i32 = int32_t;
    using i16 = int16_t;
    using i8  = int8_t;

    using usize = size_t;

    // squares are enums, while ranks and files are integers

    constexpr int BOARD_WIDTH = 8, BOARD_HEIGHT = 8, BOARD_SIZE = BOARD_WIDTH * BOARD_HEIGHT;

    enum class Square : u8
    {
        A1, B1, C1, D1, E1, F1, G1, H1,
        A2, B2, C2, D2, E2, F2, G2, H2,
        A3, B3, C3, D3, E3, F3, G3, H3,
        A4, B4, C4, D4, E4, F4, G4, H4,
        A5, B5, C5, D5, E5, F5, G5, H5,
        A6, B6, C6, D6, E6, F6, G6, H6,
        A7, B7, C7, D7, E7, F7, G7, H7,
        A8, B8, C8, D8, E8, F8, G8, H8,

        NONE,
    };

    // helpers for square & file & rank

    [[nodiscard]] constexpr Square make_square(int rank, int file)
    {
        return (Square)(rank * 8 + file);
    }

    [[nodiscard]] constexpr int rank_of(Square squ)
    {
        return (int)squ / 8;
    }

    [[nodiscard]] constexpr int file_of(Square squ)
    {
        return (int)squ % 8;
    }

    [[nodiscard]] inline std::string algebraic(Square squ)
    {
        if (squ == Square::NONE)
        {
            return "00";
        }

        int file = file_of(squ);
        int rank = rank_of(squ);

        return {(char)(file + 'a'), (char)(rank + '1')};
    }

    [[nodiscard]] inline Square make_square(const std::string& str)
    {
        if (str.size() != 2)
            return Square::NONE;

        int file = str[0] - 'a';
        int rank = str[1] - '1';

        return make_square(rank, file);
    }

    // enums for piece types (just the piece, no color), piece, color

    constexpr int COLOR_NB = 2, PIECETYPE_NB = 6, PIECE_NB = COLOR_NB * PIECETYPE_NB;

    enum class Piece : u8 
    {
        WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
        BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
        NONE,
    };

    enum class PieceType : u8
    {
        PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
    };

    enum class Color : u8 
    {
        WHITE, BLACK
    };

    // helpers for piece and color

    [[nodiscard]] constexpr Piece make_piece(PieceType type, Color color)
    {
        return (Piece)((int)type + (int)color * 6);
    }

    [[nodiscard]] constexpr PieceType type_of(Piece piece)
    {
        return (PieceType)((int)piece % 6);
    }

    [[nodiscard]] constexpr Color color_of(Piece piece)
    {
        return (Color)((int)piece / 6);
    }

    [[nodiscard]] constexpr char piece_char(Piece piece)
    {
        switch (piece) 
        {
            case Piece::WHITE_PAWN:         return 'P';
            case Piece::WHITE_KNIGHT:       return 'N';
            case Piece::WHITE_BISHOP:       return 'B';
            case Piece::WHITE_ROOK:         return 'R';
            case Piece::WHITE_QUEEN:        return 'Q';
            case Piece::WHITE_KING:         return 'K';
            case Piece::BLACK_PAWN:         return 'p';
            case Piece::BLACK_KNIGHT:       return 'n';
            case Piece::BLACK_BISHOP:       return 'b';
            case Piece::BLACK_ROOK:         return 'r';
            case Piece::BLACK_QUEEN:        return 'q';
            case Piece::BLACK_KING:         return 'k';
            default:                        return '*';
        }
    }

    [[nodiscard]] constexpr Piece char_to_piece(char c)
    {
        switch (c) 
        {
            case 'P':  return Piece::WHITE_PAWN;
            case 'N':  return Piece::WHITE_KNIGHT;
            case 'B':  return Piece::WHITE_BISHOP;
            case 'R':  return Piece::WHITE_ROOK;
            case 'Q':  return Piece::WHITE_QUEEN;
            case 'K':  return Piece::WHITE_KING;
            case 'p':  return Piece::BLACK_PAWN;
            case 'n':  return Piece::BLACK_KNIGHT;
            case 'b':  return Piece::BLACK_BISHOP;
            case 'r':  return Piece::BLACK_ROOK;
            case 'q':  return Piece::BLACK_QUEEN;
            case 'k':  return Piece::BLACK_KING;
            default:   return Piece::NONE;
        }
    }
}


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

#include "movegen.hpp"
#include "attacks.hpp"
#include "bitboard.hpp"
#include "board.hpp"
#include "core.hpp"
#include "move.hpp"
#include <iostream>

namespace crumb 
{
    int MoveGenerator::generate_moves(const Board& board, Move moves[])
    {
        arr = moves;

        generate_pawn_moves(board);
        generate_piece_moves<PieceType::KNIGHT>(board);
        generate_piece_moves<PieceType::BISHOP>(board);
        generate_piece_moves<PieceType::ROOK>(board);
        generate_piece_moves<PieceType::QUEEN>(board);
        generate_piece_moves<PieceType::KING>(board);

        generate_castling(board);

        return size;
    } 

    void MoveGenerator::generate_pawn_moves(const Board& board)
    {
        Color us = board.side_to_move;
        Color them = opposite(us);

        bool is_white = us == Color::WHITE;
        
        Piece moving = make_piece(PieceType::PAWN, us);

        u64 pawns = board.piece_bb[(int)PieceType::PAWN] & board.color_bb[(int)us];
        u64 enemies = board.color_bb[(int)them];

        u64 rank_3_from_bottom = is_white ? rank_mask(2) : rank_mask(5);
        u64 rank_8_from_bottom = is_white ? rank_mask(7) : rank_mask(0);

        int single_push_offset = is_white ? 8 : -8;
        int double_push_offset = single_push_offset * 2;
        int left_capture_offset = is_white ? 7 : -9;
        int right_capture_offset = is_white ? 9 : -7;

        u64 single_push_bb = is_white ? pawns << 8 : pawns >> 8;
        single_push_bb &= ~board.occ;

        u64 double_push_bb = is_white ? (single_push_bb & rank_3_from_bottom) << 8 : 
                                        (single_push_bb & rank_3_from_bottom) >> 8;
        double_push_bb &= ~board.occ;

        u64 left_capture_bb = pawns & ~file_mask(0);
        u64 right_capture_bb = pawns & ~file_mask(7);

        left_capture_bb = is_white ? left_capture_bb << 7 : left_capture_bb >> 9;
        right_capture_bb = is_white ? right_capture_bb << 9 : right_capture_bb >> 7;
        left_capture_bb &= enemies;
        right_capture_bb &= enemies;

        u64 single_push_promotion_bb = single_push_bb & rank_8_from_bottom;
        u64 left_capture_promotion_bb = left_capture_bb & rank_8_from_bottom;
        u64 right_capture_promotion_bb = right_capture_bb & rank_8_from_bottom;

        single_push_bb &= ~rank_8_from_bottom;
        left_capture_bb &= ~rank_8_from_bottom;
        right_capture_bb &= ~rank_8_from_bottom;

        extract_pawn(single_push_bb, single_push_offset, moving);
        extract_pawn(left_capture_bb, left_capture_offset, moving);
        extract_pawn(right_capture_bb, right_capture_offset, moving);
        extract_double_push(double_push_bb, double_push_offset, moving);

        extract_pawn_promotion(single_push_promotion_bb, single_push_offset, moving);
        extract_pawn_promotion(left_capture_promotion_bb, left_capture_offset, moving);
        extract_pawn_promotion(right_capture_promotion_bb, right_capture_offset, moving);

        if (board.ep_square != Square::NONE)
        {
            u64 ep_pawns = pawns & attacks::get_pawn_attacks(board.ep_square, them);
            while (ep_pawns)
            {
                int lsb = pop_lsb(ep_pawns);
                add(create_move((Square)lsb, board.ep_square, make_piece(PieceType::PAWN, us), MoveFlag::EN_PASSANT));
            }
        }
    }

    template <PieceType Type>
    void MoveGenerator::generate_piece_moves(const Board& board)
    {
        static_assert(Type != PieceType::PAWN, "Generat");

        Color us = board.side_to_move;
        Color them = opposite(us);

        Piece moving = make_piece(Type, us);

        u64 friendlies = board.color_bb[(int)us];
        u64 pieces = board.piece_bb[(int)Type] & friendlies;

        while (pieces)
        {
            Square from = (Square)pop_lsb(pieces);

            u64 attack_bb = 0;
            switch (Type) {
                case PieceType::KNIGHT: attack_bb = attacks::get_knight_attacks(from); break;
                case PieceType::BISHOP: attack_bb = attacks::get_bishop_attacks(from, board.occ); break;
                case PieceType::ROOK:   attack_bb = attacks::get_rook_attacks(from, board.occ); break;
                case PieceType::QUEEN:  attack_bb = attacks::get_rook_attacks(from, board.occ) 
                                                    | attacks::get_bishop_attacks(from, board.occ); break;
                case PieceType::KING:   attack_bb = attacks::get_king_attacks(from); break;
                default:
                    break;
            }

            attack_bb &= ~friendlies;

            while (attack_bb)
            {
                Square to = (Square)pop_lsb(attack_bb);
                add(create_move(from, to, moving, MoveFlag::NORMAL));
            }

        }
    }

    // the bitboard representing the squares that need to be empty in order for the castling of that 
    // type to be performed

    constexpr u64 WK_CASTLING_EMPTY = square_mask(Square::F1) | square_mask(Square::G1);
    constexpr u64 WQ_CASTLING_EMPTY = square_mask(Square::D1) | square_mask(Square::C1) | square_mask(Square::B1);
    constexpr u64 BK_CASTLING_EMPTY = square_mask(Square::F8) | square_mask(Square::G8);
    constexpr u64 BQ_CASTLING_EMPTY = square_mask(Square::D8) | square_mask(Square::C8) | square_mask(Square::B8);

    void MoveGenerator::generate_castling(const Board& board)
    {
        Color us = board.side_to_move;
        Color them = opposite(us);

        Piece moving = make_piece(PieceType::KING, us);

        if (us == Color::WHITE && !board.is_attacked(Square::E1, them))
        {
            if ((board.castling_rights & CASTLING_WK) && !(board.occ & WK_CASTLING_EMPTY) && 
                !board.is_attacked(Square::F1, them) && !board.is_attacked(Square::G1, them))
                add(create_move(Square::E1, Square::G1, moving, MoveFlag::CASTLING));

            if ((board.castling_rights & CASTLING_WQ) && !(board.occ & WQ_CASTLING_EMPTY) &&
                !board.is_attacked(Square::D1, them) && !board.is_attacked(Square::C1, them))
                add(create_move(Square::E1, Square::C1, moving, MoveFlag::CASTLING));
        }

        else if (us == Color::BLACK && !board.is_attacked(Square::E8, them))
        {
            if ((board.castling_rights & CASTLING_BK) && !(board.occ & BK_CASTLING_EMPTY) &&
                !board.is_attacked(Square::F8, them) && !board.is_attacked(Square::G8, them))
                add(create_move(Square::E8, Square::G8, moving, MoveFlag::CASTLING));

            if ((board.castling_rights & CASTLING_BQ) && !(board.occ & BQ_CASTLING_EMPTY) &&
                !board.is_attacked(Square::D8, them) && !board.is_attacked(Square::C8, them))
                add(create_move(Square::E8, Square::C8, moving, MoveFlag::CASTLING));
        }
    }

    void MoveGenerator::add(Move move)
    {
        arr[size++] = move;
    }

    void MoveGenerator::extract_pawn(u64 bb, int offset, Piece piece)
    {
        while (bb)
        {
            int lsb = pop_lsb(bb);
            add(create_move((Square)(lsb - offset), (Square)lsb, piece, MoveFlag::NORMAL));
        }
    }

    void MoveGenerator::extract_pawn_promotion(u64 bb, int offset, Piece piece)
    {
        while (bb) 
        {
            int lsb = pop_lsb(bb);
            add(create_move((Square)(lsb - offset), (Square)lsb, piece, MoveFlag::PROMOQ));
            add(create_move((Square)(lsb - offset), (Square)lsb, piece, MoveFlag::PROMOR));
            add(create_move((Square)(lsb - offset), (Square)lsb, piece, MoveFlag::PROMOB));
            add(create_move((Square)(lsb - offset), (Square)lsb, piece, MoveFlag::PROMON));
        }
    }

    void MoveGenerator::extract_double_push(u64 bb, int offset, Piece piece)
    {
        while (bb)
        {
            int lsb = pop_lsb(bb);
            add(create_move((Square)(lsb - offset), (Square)lsb, piece, MoveFlag::DOUBLE_PUSH));
        }
    }
}
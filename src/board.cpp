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

#include "board.hpp"
#include "attacks.hpp"
#include "bitboard.hpp"
#include "core.hpp"
#include "eval.hpp"
#include "move.hpp"
#include "movelist.hpp"
#include "zobrist.hpp"
#include <cctype>
#include <cstring>
#include <ios>
#include <iostream>
#include <optional>
#include <sstream>

namespace crumb 
{

std::ostream& operator<<(std::ostream& os, const Board& board)
{
    for (int rank = 7; rank >= 0; --rank)
    {
        os << rank + 1 << " ";
        for (int file = 0; file <= 7; ++file)
        {
            Square sq = make_square(rank, file);
            Piece p_there = board.mailbox[(int)sq];

            os << piece_char(p_there) << " ";
        }

        os << std::endl;
    }

    os << "  a b c d e f g h\n\n";
    os << "  side to move: " << ((bool)board.side_to_move ? "black" : "white") << '\n';
    os << "  white castling kingside?: "   << std::boolalpha << (bool)(board.castling_rights & CASTLING_WK) << '\n';
    os << "  white castling queenside?: "  << std::boolalpha << (bool)(board.castling_rights & CASTLING_WQ) << '\n';
    os << "  black castling kingside?: "   << std::boolalpha << (bool)(board.castling_rights & CASTLING_BK) << '\n';
    os << "  black castling queenside?: "  << std::boolalpha << (bool)(board.castling_rights & CASTLING_BQ) << '\n';
    os << "  en passant square: " << algebraic(board.ep_square) << '\n';
    os << "  rule-50 halfmove: " << board.halfmove_clock << '\n' << '\n';
    os << "  mg material score: " << board.mg_score << '\n';
    os << "  eg material score: " << board.eg_score << '\n' << '\n';
    os << "  hash: 0x" << std::dec << board.hash << '\n' << std::endl;

    return os;
}

void Board::load_fen(const std::string& fen)
{
    std::memset(piece_bb, 0, sizeof(piece_bb));
    std::memset(color_bb, 0, sizeof(color_bb));
    std::memset(mailbox, (int)Piece::NONE, sizeof(mailbox));

    occ = 0ULL;
    hash = 0;

    side_to_move            = Color::WHITE;
    ep_square               = Square::NONE;
    castling_rights         = 0;
    halfmove_clock          = 0;

    mg_score = 0;
    eg_score = 0;

    constexpr int FEN_PART_BOARD = 0, FEN_PART_SIDE = 1, FEN_PART_CASTLING_RIGHTS = 2,
                    FEN_PART_EP = 3, FEN_PART_HALFMOVE = 4;

    std::istringstream iss(fen);
    int part = 0;

    std::string token;
    int rank = 7, file = 0;
    while (iss >> token && part <= FEN_PART_HALFMOVE)
    {
        // std::cout << token << std::endl;
        switch (part)
        {
            case FEN_PART_BOARD:
            {
                for (char c : token)
                {
                    if (std::isdigit(c))
                    {
                        file += c - '0';
                        continue;
                    }

                    if (c == '/')
                    {
                        file = 0; 
                        --rank;
                        continue;
                    }

                    Piece p = char_to_piece(c);
                    Square sq = make_square(rank, file);

                    place_piece(sq, p);

                    ++file;
                }

                break;
            }

            case FEN_PART_SIDE:
            {
                if (token == "b")
                {
                    side_to_move = Color::BLACK;
                }

                break;
            }

            case FEN_PART_CASTLING_RIGHTS:
            {
                for (char c : token)
                {
                    switch (c) {
                        case 'K': castling_rights |= CASTLING_WK; break;
                        case 'Q': castling_rights |= CASTLING_WQ; break;
                        case 'k': castling_rights |= CASTLING_BK; break;
                        case 'q': castling_rights |= CASTLING_BQ; break;
                        default: break;
                    }
                }

                break;
            }

            case FEN_PART_EP:
            {
                ep_square = make_square(token);
                break;
            }

            case FEN_PART_HALFMOVE:
            {
                halfmove_clock = std::atoi(token.data());
                break;
            }

            default:
                break;
        }

        ++part;
    }

    hash = zobrist::compute_hash(*this);
}

bool Board::is_attacked(Square square, Color by) const
{
    u64 our_pieces = color_bb[(int)by];

    u64 pawns = piece_bb[(int)PieceType::PAWN] & our_pieces;
    if (pawns & attacks::get_pawn_attacks(square, opposite(by)))
        return true;

    u64 knights = piece_bb[(int)PieceType::KNIGHT] & our_pieces;
    if (knights & attacks::get_knight_attacks(square))
        return true;

    u64 king = piece_bb[(int)PieceType::KING] & our_pieces;
    if (king & attacks::get_king_attacks(square))
        return true;

    u64 bishops = piece_bb[(int)PieceType::BISHOP] & our_pieces;
    u64 queens = piece_bb[(int)PieceType::QUEEN] & our_pieces;

    if ((bishops | queens) & attacks::get_bishop_attacks(square, occ))
        return true;

    u64 rooks = piece_bb[(int)PieceType::ROOK] & our_pieces;

    if ((rooks | queens) & attacks::get_rook_attacks(square, occ))
        return true;

    return false;
}

bool Board::is_in_check(Color color) const
{
    Square king_square = (Square)trailing_zero(piece_bb[(int)PieceType::KING] & color_bb[(int)color]);
    return is_attacked(king_square, opposite(color));
}

bool Board::is_in_check() const
{
    return is_in_check(side_to_move);
}

void Board::clear_square(Square square)
{
    if (!(occ & square_mask(square)))
        return;

    Piece piece_there = mailbox[(int)square];
    PieceType piece_type = type_of(piece_there);
    Color piece_color = color_of(piece_there);

    u64 mask = ~square_mask(square);

    mailbox[(int)square] = Piece::NONE;
    piece_bb[(int)piece_type] &= mask;
    color_bb[(int)piece_color] &= mask;

    occ &= mask;

    hash ^= zobrist::piece_square[(int)piece_there][(int)square];

    mg_score -= eval::mg_table(piece_color, piece_type, square);
    eg_score -= eval::eg_table(piece_color, piece_type, square);
}

void Board::place_piece(Square square, Piece piece)
{
    if (piece == Piece::NONE)
    {
        clear_square(square);
        return;
    }

    PieceType piece_type = type_of(piece);
    Color piece_color = color_of(piece);

    u64 mask = square_mask(square);

    mailbox[(int)square] = piece;
    piece_bb[(int)piece_type] |= mask;
    color_bb[(int)piece_color] |= mask;

    occ |= mask;

    hash ^= zobrist::piece_square[(int)piece][(int)square];

    mg_score += eval::mg_table(piece_color, piece_type, square);
    eg_score += eval::eg_table(piece_color, piece_type, square);
}

constexpr PieceType PROMO_PIECES[] = {PieceType::QUEEN, PieceType::ROOK, PieceType::BISHOP, PieceType::KNIGHT};
constexpr u8 CASTLING_MASKS[] = 
{
    13, 15, 15, 15, 12, 15, 15, 14,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
     7, 15, 15, 15,  3, 15, 15, 11,
};

void Board::make_move(Move move)
{
    Color us = side_to_move;
    bool is_white = us == Color::WHITE;

    Square from = move_from(move);
    Square to   = move_to(move);
    auto flag = move_flag(move);

    Piece moving = move_piece(move);
    PieceType moving_type = type_of(moving);
    
    hash ^= zobrist::black_to_move;
    side_to_move = opposite(side_to_move);

    if (ep_square != Square::NONE)
    {
        hash ^= zobrist::ep_files[file_of(ep_square)];
        ep_square = Square::NONE;
    }

    if (mailbox[(int)to] != Piece::NONE || moving_type == PieceType::PAWN)
    {
        halfmove_clock = 0;
    } 
    else {
        ++halfmove_clock;
    }
    
    switch (flag)
    {
        case MoveFlag::NORMAL:
        {
            clear_square(from);
            clear_square(to);
            place_piece(to, moving);
            break;
        }

        case MoveFlag::CASTLING:
        {
            bool king_side = to == Square::G1 || to == Square::G8;

            Square rook_from = is_white ? (king_side ? Square::H1 : Square::A1) :
                                          (king_side ? Square::H8 : Square::A8);
            Square rook_to = is_white ? (king_side ? Square::F1 : Square::D1) :
                                        (king_side ? Square::F8 : Square::D8);

            clear_square(rook_from);
            clear_square(from);

            place_piece(to, moving);
            place_piece(rook_to, make_piece(PieceType::ROOK, us));

            break;
        }

        case MoveFlag::EN_PASSANT:
        {
            Square capture_square = is_white ? Square((int)to - 8) : Square((int)to + 8);

            clear_square(capture_square);
            clear_square(from);
            place_piece(to, moving);

            break;
        }

        case MoveFlag::DOUBLE_PUSH:
        {
            ep_square = is_white ? Square((int)to - 8) : Square((int)to + 8);

            clear_square(from);
            place_piece(to, moving);

            break;
        }

        // promotion
        default:
        {
            clear_square(from);
            clear_square(to);
            place_piece(to, make_piece(PROMO_PIECES[(int)flag - (int)MoveFlag::PROMOQ], us));

            break;
        }
    }

    hash ^= zobrist::castling_hash(castling_rights);

    castling_rights &= CASTLING_MASKS[(int)from];
    castling_rights &= CASTLING_MASKS[(int)to];

    hash ^= zobrist::castling_hash(castling_rights);

    if (ep_square != Square::NONE)
    {
        hash ^= zobrist::ep_files[file_of(ep_square)];
    }
}

bool Board::try_move(Move move)
{
    make_move(move);

    if (is_in_check(opposite(side_to_move)))
        return false;

    return true;
}

bool HashStack::is_repetition(u64 current, u8 rule50) const
{
    if (count < 2)
        return false;

    int occurrences = 0;

    int last_irreversible = count > rule50 ? count - rule50 : 0;

    for (int i = static_cast<int>(count) - 2; i >= last_irreversible; i -= 2)
    {
        if (hashes[i] == current)
        {
            ++occurrences;

            if (occurrences >= 2)
                return true;
        }
    }

    return false;
}

std::optional<Move> Board::parse(const std::string& str)
{
    MoveList moves(*this);

    for (int i = 0; i < moves.size(); ++i)
    {
        if (to_string(moves[i]) == str)
        {
            Board copy = *this;
            if (copy.try_move(moves[i]))
                return moves[i];
            else
                return std::nullopt;
        }
            
    }

    return std::nullopt;
}
}
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
#include "bitboard.hpp"
#include "core.hpp"
#include <cctype>
#include <cstring>
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
                Piece p_there = Piece::NONE;
                Square sq = make_square(rank, file);

                if (is_occupied(board.occ, sq))
                    for (int p = 0; p < 6; ++p)
                    {
                        if (is_occupied(board.piece_bb[p], sq))
                        {
                            p_there = is_occupied(board.color_bb[0], sq) ? 
                                      make_piece(PieceType(p), Color::WHITE) : 
                                      make_piece(PieceType(p), Color::BLACK);
                            break;
                        }
                    }

                os << piece_char(p_there) << " ";
            }

            os << std::endl;
        }

        os << "  a b c d e f g h\n\n";
        return os;
    }

    Board::Board(const std::string& fen)
    {
        std::memset(piece_bb, 0, sizeof(piece_bb));
        std::memset(color_bb, 0, sizeof(color_bb));
        occ = 0ULL;

        constexpr int FEN_PART_BOARD = 0, FEN_PART_SIDE = 1, FEN_PART_CASTLING_RIGHTS = 2,
                      FEN_PART_EP = 3, FEN_PART_HALFMOVE = 4;

        std::istringstream iss(fen);
        int part = 0;

        std::string token;
        int rank = 7, file = 0;
        while (iss >> token && part <= FEN_PART_HALFMOVE)
        {
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
                            ++rank;
                            continue;
                        }

                        ++file;
                        Piece p = char_to_piece(c);
                        Square sq = make_square(rank, file);

                        piece_bb[(int)type_of(p)] |= square_mask(sq);
                        color_bb[(int)color_of(p)] |= square_mask(sq);
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
        }
    }
}
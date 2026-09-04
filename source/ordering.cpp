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

#include "ordering.hpp"
#include "core.hpp"
#include "move.hpp"

namespace crumb {

bool OrderingMoveList::next(int i) {
    if (i >= count)
        return false;

    int highest_idx = i;
    int highest_score = scores[i];

    for (int j = i; j < count; ++j) {
        if (scores[j] > highest_score) {
            highest_score = scores[j];
            highest_idx = j;
        }
    }

    std::swap(moves[i], moves[highest_idx]);
    std::swap(scores[i], scores[highest_idx]);

    return true;
}

static constexpr int MVV_LVA[PIECETYPE_NB][PIECETYPE_NB] = {
    {809000, 831000, 832000, 849000, 889000, 999000},
    {806800, 828800, 829800, 846800, 886800, 996800},
    {806700, 828700, 829700, 846700, 886700, 996700},
    {805000, 827000, 828000, 845000, 885000, 995000},
    {801000, 823000, 824000, 841000, 881000, 991000},
    {799000, 821000, 822000, 839000, 879000, 989000}};

static constexpr int PROMOTION_SCORES[4] = {790000, 750000, 732000, 731000};

int OrderingMoveList::calculate_score(Move move, const Board& board, Move tt_move) const {
    if (move == tt_move)
        return 1000000;

    Square from = move_from(move);
    Square to = move_to(move);
    MoveFlag flag = move_flag(move);

    if (board.mailbox[(int)to] != Piece::NONE) {
        int mvvlva_score =
            MVV_LVA[(int)type_of(board.mailbox[(int)from])][(int)type_of(board.mailbox[(int)to])];

        return mvvlva_score;
    }

    if (flag >= MoveFlag::PROMOQ) {
        int promo_score = PROMOTION_SCORES[(int)flag - (int)MoveFlag::PROMOQ];
        return promo_score;
    }

    return 0;
}
} // namespace crumb
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

#include "search.hpp"
#include "board.hpp"
#include "core.hpp"
#include "eval.hpp"
#include "move.hpp"
#include "movelist.hpp"
#include <cmath>
#include <cstdlib>
#include <string>

namespace crumb
{

namespace
{
std::string score_string(Score score)
{
    if (std::abs(score) <= MAX_CP_SCORE)
        return "cp " + std::to_string(score);

    int mate_distance = MATE_SCORE - std::abs(score);

    mate_distance = score > 0 ? mate_distance : -mate_distance;
    mate_distance = (int)std::ceil(mate_distance / 2.0);

    return "mate " + std::to_string(mate_distance);
}
}

void Searcher::stop_search()
{
    timer.stop_flag = true;
}

void Searcher::start_search(Depth depth, int max_ms)
{
    if (is_terminal())
    {
        std::cout << "info string position is terminal\n"
                  << "bestmove none" << std::endl;
        return;
    }

    timer.start_time = SteadyClock::now();
    timer.max_ms = max_ms;
    timer.stop_flag = false;

    Info info;
    Move previous_best_move;
    Score previous_score;

    // iterative deepening
    for (int current_depth = 1; current_depth <= depth; ++current_depth)
    {
        int score = search<NodeType::ROOT>(info, board, current_depth, 0, -INF_SCORE, INF_SCORE);

        if (timer.should_stop())
            break;

        previous_best_move = info.best_move;
        previous_score = score;

        int elapsed = timer.elapsed();

        std::cout << "info depth " << current_depth << " score " << score_string(score) << " nodes " 
                  << info.nodes_searched << " nps " << (info.nodes_searched * 1000 / elapsed) 
                  << " time " << elapsed << " pv " << to_string(info.best_move )<< std::endl;
    }

    std::cout << "bestmove " << to_string(previous_best_move) << std::endl;

}

template <Searcher::NodeType Type>
Score Searcher::search(Info& info, const Board& board, Depth depth, Depth plies, Score alpha, Score beta)
{
    ++info.nodes_searched;

    if (timer.should_stop())
        return 0;

    if (stack.is_repetition(board.hash, board.halfmove_clock) || board.halfmove_clock >= 100)
        return DRAW_SCORE;

    if (depth <= 0 && Type != NodeType::ROOT)
    {
        return eval::evaluate(board);
    }

    MoveList moves(board);

    stack.hashes[stack.count++] = board.hash;

    Score best_score = -INF_SCORE;
    Move best_move = 0;
    int move_count = 0;

    for (int i = 0; i < moves.size(); ++i)
    {
        Move move = moves[i];

        Board child = board;
        if (!child.try_move(move))
        {
            continue;
        }

        ++move_count;

        int score = -search<NodeType::NON_ROOT>(info, child, depth - 1, plies + 1, -beta, -alpha);

        if (score > best_score)
        {
            best_score = score;
            best_move = move;
        }

        alpha = std::max(best_score, alpha);

        if (alpha > beta)
            break;
    }

    stack.count--;

    if (move_count == 0)
    {
        return (board.is_in_check() ? -MATE_SCORE + plies + 1 : DRAW_SCORE);
    }

    if (Type == NodeType::ROOT)
        info.best_move = best_move;

    return best_score;
}

bool Searcher::is_terminal() const
{
    MoveList moves(board);
    for (int i = 0; i < moves.size(); ++i)
    {
        Board child = board;
        if (child.try_move(moves[i]))
            return false;
    }

    return true;
}

}
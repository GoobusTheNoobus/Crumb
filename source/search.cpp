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
#include "ordering.hpp"
#include "tt.hpp"
#include <cmath>
#include <cstdlib>
#include <string>

namespace crumb {

namespace {

std::string score_string(Score score) {
    if (std::abs(score) <= MAX_CP_SCORE)
        return "cp " + std::to_string(score);

    int mate_distance = MATE_SCORE - std::abs(score);

    mate_distance = score > 0 ? mate_distance : -mate_distance;
    mate_distance = (int)std::ceil(mate_distance / 2.0);

    return "mate " + std::to_string(mate_distance);
}

} // namespace

void Searcher::stop_search() { timer.stop_flag = true; }

void Searcher::start_search(Depth max_depth, int max_ms) {
    if (is_terminal()) {
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
    for (int depth = 1; depth <= max_depth; ++depth) {

        // aspiration window
        Score score;
        if (depth == 1)
            score = search<NodeType::ROOT>(info, board, depth, 0, -INF_SCORE, INF_SCORE);
        else {
            Score delta = ASPIR_WINDOW;

            Score alpha = previous_score - delta;
            Score beta = previous_score + delta;

            while (true) {
                score = search<NodeType::ROOT>(info, board, depth, 0, alpha, beta);

                if (timer.should_stop())
                    break;

                if (score <= alpha) {
                    alpha -= delta;
                    delta *= ASPIR_EXPANSION;
                    continue;
                }

                if (score >= beta) {
                    beta += delta;
                    delta *= ASPIR_EXPANSION;

                    continue;
                }

                break;
            }
        }

        // we make sure the score is valid before assigning to previous_best_move
        if (timer.should_stop())
            break;

        previous_best_move = info.best_move;
        previous_score = score;

        int elapsed = timer.elapsed();

        // report uci info
        std::cout << "info depth " << depth << " seldepth " << (int)info.seldepth << " score "
                  << score_string(score) << " nodes " << info.nodes_searched << " hashfull "
                  << tt.hashfull() << " nps " << (info.nodes_searched * 1000 / elapsed) << " time "
                  << elapsed << " pv " << to_string(info.best_move) << std::endl;
    }

    std::cout << "bestmove " << to_string(previous_best_move) << std::endl;
    tt.clear();
}

template <Searcher::NodeType Type>
Score Searcher::search(Info& info, const Board& board, Depth depth, Depth plies, Score alpha,
                       Score beta) {
    if (timer.should_stop())
        return 0;
    ++info.nodes_searched;

    // 3-fold repetition and rule 50
    if (hashes.is_repetition(board.hash, board.halfmove_clock) || board.halfmove_clock >= 100)
        return DRAW_SCORE;

    // leaf node
    if (depth <= 0 && Type != NodeType::ROOT) {
        return qsearch(info, board, plies + 1, alpha, beta);
    }

    info.seldepth = std::max(info.seldepth, plies);

    Score original_alpha = alpha;

    // tt probe

    auto entry = tt.probe(board.hash);
    Move tt_move = 0;
    if (entry && entry->full_key == board.hash) {

        // we use the tt_move regardless for ordering
        tt_move = entry->best_move;

        // we use the score and stuff if the probe depth is higher than the depth we will search,
        // and we are in a NON_PV node
        if (entry->depth >= depth && Type == NodeType::NON_PV) {
            Score tt_score = entry->score;

            // tt scores that are mate-in-n are adjusted to mate in n from the perspective of that
            // position, but we require a mate-in-n relative to the root position we are searching
            if (tt_score > MAX_CP_SCORE)
                tt_score -= plies;
            else if (tt_score < MIN_CP_SCORE)
                tt_score += plies;

            if (entry->flag == TTFlag::EXACT)
                return tt_score;

            if (entry->flag == TTFlag::LOWERBOUND)
                alpha = std::max(alpha, tt_score);

            if (entry->flag == TTFlag::UPPERBOUND)
                beta = std::min(beta, tt_score);

            if (alpha >= beta)
                return tt_score;
        }
    }

    Score static_eval = eval::evaluate(board);
    bool in_check = board.is_in_check();

    // reverse futility pruning
    // if the static eval is already a good amount above beta, we return that
    // the higher the search depth, the higher the static eval has to be above beta to do this
    Score margin = RFP_BASE * depth;
    if (!in_check && Type == NodeType::NON_PV && static_eval >= beta + margin)
        return static_eval;

    // null move pruning
    // try passing the turn to the opponent, if the position still produce a cutoff, we assume the
    // current side's position is strong enough to return, since passing the move is usually
    // worsening the position
    // we check if the board has non-pawn materials to avoid zugswang
    if (Type == NodeType::NON_PV && !in_check && board.has_non_pawn_material() &&
        static_eval > beta + NMP_EVAL_MARGIN && depth >= NMP_MIN_DEPTH) {
        Board null_board = board;
        null_board.make_null();

        Score score = -search<NodeType::NON_PV>(info, null_board, depth - NMP_REDUCTION, plies + 1,
                                                -beta, -beta + 1);

        if (score >= beta) {
            return score;
        }
    }

    // generate moves, then score them on how likely they are to be good
    // for example, captures and promotions are usually better than quiet moves
    // we search them first so cutoffs are more likely
    OrderingMoveList moves(board, tt_move);
    hashes.push_hash(board.hash);

    Score best_score = -INF_SCORE;
    Move best_move = 0;
    int move_count = 0;

    int i = 0;
    while (moves.next(i)) {
        Move move = moves[i];
        ++i;

        Board child = board;
        bool is_legal = child.try_move(move);
        if (!is_legal) {
            continue;
        }

        ++move_count;

        // log info currmove
        if (Type == NodeType::ROOT && timer.elapsed() > 1000 && !timer.should_stop()) {
            std::cout << "info currmovenumber " << move_count << " currmove " << to_string(move)
                      << " nodes " << info.nodes_searched << " time " << timer.elapsed() << " nps "
                      << (info.nodes_searched * 1000 / timer.elapsed()) << std::endl;
        }

        // principle variation search:
        // we search the first move with full window, then search remaining moves with null winow
        // (-alpha - 1 as opposed to -beta)
        // if that move is cutoff, we

        Score score;
        if (Type == NodeType::NON_PV || move_count > 1) {
            score =
                -search<NodeType::NON_PV>(info, child, depth - 1, plies + 1, -alpha - 1, -alpha);
        }

        if (Type != NodeType::NON_PV && (move_count == 1 || score > alpha)) {
            score = -search<NodeType::PV>(info, child, depth - 1, plies + 1, -beta, -alpha);
        }

        if (score > best_score) {
            best_score = score;
            best_move = move;
        }

        alpha = std::max(best_score, alpha);

        if (alpha >= beta)
            break;
    }

    hashes.pop_hash();

    // no legal moves means its checkmate or draw
    // if we are in check, the score is mate-in-plies, otherwise DRAW_SCORE which is 0
    if (move_count == 0) {
        return (board.is_in_check() ? -MATE_SCORE + plies + 1 : DRAW_SCORE);
    }

    if (Type == NodeType::ROOT)
        info.best_move = best_move;

    // just like what we did earlier in the tt probe
    // store_score must be relative to the current position, not the

    Score store_score = best_score;
    if (best_score > MAX_CP_SCORE)
        store_score += plies;
    if (best_score < MIN_CP_SCORE)
        store_score -= plies;

    TTFlag store_flag;

    if (best_score <= original_alpha)
        store_flag = TTFlag::UPPERBOUND;
    else if (best_score >= beta)
        store_flag = TTFlag::LOWERBOUND;
    else
        store_flag = TTFlag::EXACT;

    tt.store(TranspositionEntry{board.hash, best_move, store_score, depth, store_flag});

    return best_score;
}

// search until the position is quiet or the static eval is more than beta
Score Searcher::qsearch(Info& info, const Board& board, Depth plies, Score alpha, Score beta) {
    ++info.nodes_searched;

    if (timer.should_stop())
        return 0;

    if (hashes.is_repetition(board.hash, board.halfmove_clock) || board.halfmove_clock >= 100)
        return DRAW_SCORE;

    bool in_check = board.is_in_check();
    Score static_eval = eval::evaluate(board);

    if (!in_check && static_eval >= beta)
        return static_eval;

    alpha = std::max(alpha, static_eval);

    info.seldepth = std::max(info.seldepth, plies);

    MoveList moves(board);
    hashes.hashes[hashes.count++] = board.hash;

    for (int i = 0; i < moves.size(); ++i) {
        Move move = moves[i];

        bool search_move = in_check || is_noisy(board, move);

        if (!search_move)
            continue;

        Board child = board;
        if (!child.try_move(move))
            continue;

        Score score = -qsearch(info, child, plies + 1, -beta, -alpha);

        alpha = std::max(alpha, score);

        if (alpha >= beta)
            break;
    }

    hashes.count--;

    return alpha;
}

bool Searcher::is_terminal() const {
    MoveList moves(board);
    for (int i = 0; i < moves.size(); ++i) {
        Board child = board;
        if (child.try_move(moves[i]))
            return false;
    }

    return true;
}

bool Searcher::is_noisy(const Board& board, Move move) const {
    return move_flag(move) >= MoveFlag::EN_PASSANT ||
           board.mailbox[(int)move_to(move)] != Piece::NONE;
}

} // namespace crumb
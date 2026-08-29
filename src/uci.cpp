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

#include "uci.hpp"
#include "board.hpp"
#include "core.hpp"
#include "perft.hpp"
#include "search.hpp"
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <thread>

namespace crumb::uci
{
namespace {
Searcher search;
std::thread search_thread;
}
 
void loop()
{
    search.board.load_fen(FEN_STARTING);
    while (true)
    {
        std::string input;
        std::getline(std::cin, input);

        std::istringstream stream(input);
        std::string command;
        stream >> command;

        if (command == "position")
        {
            handle_position(stream);
        }
        else if (command == "go")
        {
            handle_go(stream);
        }
        else if (command == "isready")
        {
            std::cout << "readyok\n";
        }
        else if (command == "uci")
        {
            std::cout << "id name " << ENGINE_NAME << ' ' << ENGINE_VERSION << '\n';
            std::cout << "id author GoobusTheNoobus\n";
            std::cout << "uciok\n";
        }
        else if (command == "stop")
        {
            search.stop_search();

            if (search_thread.joinable())
                search_thread.join();
        }
        else if (command == "quit")
        {
            search.stop_search();

            if (search_thread.joinable())
                search_thread.join();

            return;
        }
    }

    search.stop_search();

    if (search_thread.joinable())
        search_thread.join();
}

void handle_position(std::istringstream& stream)
{
    search.stop_search();

    std::string token;
    stream >> token;

    if (token == "startpos")
    {
        search.board.load_fen(FEN_STARTING);
        stream >> token;
    }

    else if (token == "fen")
    {
        std::string fen;

        while (stream >> token && token != "moves")
        {
            fen += token + " ";
        }

        search.board.load_fen(fen);
    }

    search.hashes.count = 0;

    if (token == "moves")
    {
        while (stream >> token)
        {
            std::optional<Move> move = search.board.parse(token);

            if (!move) break;

            search.hashes.hashes[search.hashes.count++] = search.board.hash;
            search.board.make_move(move.value());
        }
    }
}

void handle_go(std::istringstream& stream)
{
    search.stop_search();

    if (search_thread.joinable())
        search_thread.join();

    std::string token;

    Depth depth = 0;
    int movetime = 0;

    int winc = 0;
    int binc = 0;
    int wtime = 0;
    int btime = 0;

    while (stream >> token)
    {
        if (token == "depth")
            stream >> depth;
        else if (token == "movetime")
            stream >> movetime;
        else if (token == "wtime")
            stream >> wtime;
        else if (token == "btime")
            stream >> btime;
        else if (token == "winc")
            stream >> winc;
        else if (token == "binc")
            stream >> binc;
        else if (token == "perft")
        {
            Depth perft_depth;
            stream >> perft_depth;

            perft::perft_divide(search.board, perft_depth);
        }
    }

    int time_ms = 0;

    if (movetime > 0)
        time_ms = movetime;

    else if (wtime > 0 || btime > 0)
    {
        int our_time = search.board.side_to_move == Color::WHITE ? wtime : btime;
        int our_inc  = search.board.side_to_move == Color::WHITE ? winc : binc;

        time_ms = std::min(our_time / 20 + our_inc / 2, our_time);
    }

    if (depth < 1 || depth > MAX_DEPTH)
        depth = MAX_DEPTH;

    search_thread = std::thread(
        [&]()
        {
            search.start_search(depth, time_ms);
        }
    );
}
}

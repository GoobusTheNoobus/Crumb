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

#include <cstdlib>
#include <iostream>
#include "attacks.hpp"
#include "eval.hpp"
#include "uci.hpp"
#include "zobrist.hpp"

using namespace crumb;

int main(void)
{
    std::cout << ENGINE_NAME << ' ' << ENGINE_VERSION << std::endl;

    // init
    attacks::load_attacks();
    zobrist::load_randoms();
    eval::load_eval_tables();

    uci::loop();
    
    return EXIT_SUCCESS;
}
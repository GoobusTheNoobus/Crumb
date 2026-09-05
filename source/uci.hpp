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

#include <sstream>
namespace crumb {

inline constexpr const char* ENGINE_NAME = "Crumb";
inline constexpr const char* ENGINE_VERSION = "0.1.3";

namespace uci {

void loop();

void handle_position(std::istringstream&);
void handle_go(std::istringstream&);

} // namespace uci

} // namespace crumb
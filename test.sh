#    Crumb is a UCI chess engine
#    Copyright (C) 2026  GoobusTheNoobus
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.

#!/usr/bin/env bash

set -e

./fastchess -engine cmd="./crumb_new" name="Crumb new version" \
            -engine cmd="./crumb_old" name="Crumb base version" \
            -each tc="10+0.1" \
            -openings file="8moves_v3.pgn" format=pgn order=random \
            -rounds 10000 \
            -repeat \
            -concurrency 10 \
            -sprt elo0=0 elo1=10 alpha=0.05 beta=0.05 \
            -pgnout file=sprt.pgn \

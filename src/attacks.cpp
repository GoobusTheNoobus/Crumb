/*
 * Copyright (c) 2026 Goobus
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "crumb/attacks.hpp"
#include "crumb/bitboard.hpp"
#include "crumb/core.hpp"

#include <array>
#include <bit>

namespace crumb::attacks {

namespace {

u64 pawn_attacks[ColourNum][BoardSize];
u64 knight_attacks[BoardSize];
u64 king_attacks[BoardSize];

u64 bishop_attacks[BoardSize][512];
u64 rook_attacks[BoardSize][4096];

u64 bishop_masks[BoardSize], bishop_relevancies[BoardSize];
u64 rook_masks[BoardSize], rook_relevancies[BoardSize];

constexpr u64 BishopMagics[BoardSize] = {
    0x0002020202020200ULL, 0x0002020202020000ULL, 0x0004010202000000ULL, 0x0004040080000000ULL,
    0x0001104000000000ULL, 0x0000821040000000ULL, 0x0000410410400000ULL, 0x0000104104104000ULL,
    0x0000040404040400ULL, 0x0000020202020200ULL, 0x0000040102020000ULL, 0x0000040400800000ULL,
    0x0000011040000000ULL, 0x0000008210400000ULL, 0x0000004104104000ULL, 0x0000002082082000ULL,
    0x0004000808080800ULL, 0x0002000404040400ULL, 0x0001000202020200ULL, 0x0000800802004000ULL,
    0x0000800400A00000ULL, 0x0000200100884000ULL, 0x0000400082082000ULL, 0x0000200041041000ULL,
    0x0002080010101000ULL, 0x0001040008080800ULL, 0x0000208004010400ULL, 0x0000404004010200ULL,
    0x0000840000802000ULL, 0x0000404002011000ULL, 0x0000808001041000ULL, 0x0000404000820800ULL,
    0x0001041000202000ULL, 0x0000820800101000ULL, 0x0000104400080800ULL, 0x0000020080080080ULL,
    0x0000404040040100ULL, 0x0000808100020100ULL, 0x0001010100020800ULL, 0x0000808080010400ULL,
    0x0000820820004000ULL, 0x0000410410002000ULL, 0x0000082088001000ULL, 0x0000002011000800ULL,
    0x0000080100400400ULL, 0x0001010101000200ULL, 0x0002020202000400ULL, 0x0001010101000200ULL,
    0x0000410410400000ULL, 0x0000208208200000ULL, 0x0000002084100000ULL, 0x0000000020880000ULL,
    0x0000001002020000ULL, 0x0000040408020000ULL, 0x0004040404040000ULL, 0x0002020202020000ULL,
    0x0000104104104000ULL, 0x0000002082082000ULL, 0x0000000020841000ULL, 0x0000000000208800ULL,
    0x0000000010020200ULL, 0x0000000404080200ULL, 0x0000040404040400ULL, 0x0002020202020200ULL};

constexpr u64 RookMagics[BoardSize] = {
    0x0080001020400080ULL, 0x0040001000200040ULL, 0x0080081000200080ULL, 0x0080040800100080ULL,
    0x0080020400080080ULL, 0x0080010200040080ULL, 0x0080008001000200ULL, 0x0080002040800100ULL,
    0x0000800020400080ULL, 0x0000400020005000ULL, 0x0000801000200080ULL, 0x0000800800100080ULL,
    0x0000800400080080ULL, 0x0000800200040080ULL, 0x0000800100020080ULL, 0x0000800040800100ULL,
    0x0000208000400080ULL, 0x0000404000201000ULL, 0x0000808010002000ULL, 0x0000808008001000ULL,
    0x0000808004000800ULL, 0x0000808002000400ULL, 0x0000010100020004ULL, 0x0000020000408104ULL,
    0x0000208080004000ULL, 0x0000200040005000ULL, 0x0000100080200080ULL, 0x0000080080100080ULL,
    0x0000040080080080ULL, 0x0000020080040080ULL, 0x0000010080800200ULL, 0x0000800080004100ULL,
    0x0000204000800080ULL, 0x0000200040401000ULL, 0x0000100080802000ULL, 0x0000080080801000ULL,
    0x0000040080800800ULL, 0x0000020080800400ULL, 0x0000020001010004ULL, 0x0000800040800100ULL,
    0x0000204000808000ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
    0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000010002008080ULL, 0x0000004081020004ULL,
    0x0000204000800080ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
    0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000800100020080ULL, 0x0000800041000080ULL,
    0x00FFFCDDFCED714AULL, 0x007FFCDDFCED714AULL, 0x003FFFCDFFD88096ULL, 0x0000040810002101ULL,
    0x0001000204080011ULL, 0x0001000204000801ULL, 0x0001000082000401ULL, 0x0001FFFAABFAD1A2ULL};

struct Direction {
    i32 rank, file;
};

void load_pawn_attacks(Square square) {

    u64 white_mask = 0ULL;
    u64 black_mask = 0ULL;

    i32 file = file_of(square);
    i32 rank = rank_of(square);

    if (rank < 7 && file > 0) {
        bitboard::set_bit(&white_mask, make_square(rank + 1, file - 1));
    }

    if (rank < 7 && file < 7) {
        bitboard::set_bit(&white_mask, make_square(rank + 1, file + 1));
    }

    if (rank > 0 && file > 0) {
        bitboard::set_bit(&black_mask, make_square(rank - 1, file - 1));
    }

    if (rank > 0 && file < 7) {
        bitboard::set_bit(&black_mask, make_square(rank - 1, file + 1));
    }

    pawn_attacks[static_cast<i32>(Colour::White)][static_cast<i32>(square)] = white_mask;
    pawn_attacks[static_cast<i32>(Colour::Black)][static_cast<i32>(square)] = black_mask;
}

constexpr bool out_of_bounds(i32 rank, i32 file) {
    return rank < 0 || rank >= 8 || file < 0 || file >= 8;
}

void load_knight_attacks(Square square) {

    u64 mask = 0ULL;
    i32 file = file_of(square);
    i32 rank = rank_of(square);

    constexpr std::array<Direction, 8> dirs = {
        {{1, 2}, {1, -2}, {-1, 2}, {-1, -2}, {2, 1}, {2, -1}, {-2, 1}, {-2, -1}}};

    for (const Direction& dir : dirs) {
        i32 new_file = file + dir.file;
        i32 new_rank = rank + dir.rank;

        if (!out_of_bounds(new_rank, new_file)) {
            bitboard::set_bit(&mask, make_square(new_rank, new_file));
        }
    }

    knight_attacks[static_cast<i32>(square)] = mask;
}

void load_king_attacks(Square square) {

    u64 mask = 0ULL;
    i32 file = file_of(square);
    i32 rank = rank_of(square);

    constexpr std::array<Direction, 8> dirs = {
        {{1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}}};

    for (const Direction& dir : dirs) {
        i32 new_file = file + dir.file;
        i32 new_rank = rank + dir.rank;

        if (!out_of_bounds(new_rank, new_file)) {
            bitboard::set_bit(&mask, make_square(new_rank, new_file));
        }
    }

    king_attacks[static_cast<i32>(square)] = mask;
}

void load_bishop_mask(Square square) {

    u64 mask = 0ULL;

    i32 rank = rank_of(square);
    i32 file = file_of(square);

    for (i32 newr = rank + 1, newf = file + 1; newr <= 6 && newf <= 6; ++newr, ++newf) {
        bitboard::set_bit(&mask, make_square(newr, newf));
    }

    for (i32 newr = rank + 1, newf = file - 1; newr <= 6 && newf >= 1; ++newr, --newf) {
        bitboard::set_bit(&mask, make_square(newr, newf));
    }

    for (i32 newr = rank - 1, newf = file + 1; newr >= 1 && newf <= 6; --newr, ++newf) {
        bitboard::set_bit(&mask, make_square(newr, newf));
    }

    for (i32 newr = rank - 1, newf = file - 1; newr >= 1 && newf >= 1; --newr, --newf) {
        bitboard::set_bit(&mask, make_square(newr, newf));
    }

    bishop_relevancies[static_cast<i32>(square)] = std::popcount(mask);
    bishop_masks[static_cast<i32>(square)] = mask;
}

void load_rook_mask(Square square) {

    u64 mask = 0ULL;

    i32 rank = rank_of(square);
    i32 file = file_of(square);

    for (i32 newr = rank + 1; newr <= 6; ++newr) {
        bitboard::set_bit(&mask, make_square(newr, file));
    }

    for (i32 newr = rank - 1; newr >= 1; --newr) {
        bitboard::set_bit(&mask, make_square(newr, file));
    }

    for (i32 newf = file - 1; newf >= 1; --newf) {
        bitboard::set_bit(&mask, make_square(rank, newf));
    }

    for (i32 newf = file + 1; newf <= 6; ++newf) {
        bitboard::set_bit(&mask, make_square(rank, newf));
    }

    rook_relevancies[static_cast<i32>(square)] = std::popcount(mask);
    rook_masks[static_cast<i32>(square)] = mask;
}

void load_relevancies(Square square) {
    bishop_relevancies[static_cast<i32>(square)] =
        std::popcount(bishop_masks[static_cast<i32>(square)]);
    rook_relevancies[static_cast<i32>(square)] =
        std::popcount(rook_masks[static_cast<i32>(square)]);
}

u64 raycast_bishop_attacks(Square square, u64 blockers) {

    u64 mask = 0ULL;
    i32 file = file_of(square);
    i32 rank = rank_of(square);

    constexpr std::array<Direction, 4> dirs = {{{1, 1}, {1, -1}, {-1, 1}, {-1, -1}}};

    for (const Direction& dir : dirs) {
        for (i32 distance = 1; distance <= 7; ++distance) {
            i32 new_file = file + distance * dir.file;
            i32 new_rank = rank + distance * dir.rank;
            Square new_square = make_square(new_rank, new_file);

            if (out_of_bounds(new_rank, new_file)) {
                break;
            }

            bitboard::set_bit(&mask, new_square);

            if (bitboard::is_occupied(blockers, new_square))
                break;
        }
    }

    return mask;
}

u64 raycast_rook_attacks(Square square, u64 blockers) {

    u64 mask = 0ULL;
    i32 file = file_of(square);
    i32 rank = rank_of(square);

    constexpr std::array<Direction, 4> dirs = {{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};

    for (const Direction& dir : dirs) {
        for (i32 distance = 1; distance <= 7; ++distance) {
            i32 new_file = file + distance * dir.file;
            i32 new_rank = rank + distance * dir.rank;
            Square new_square = make_square(new_rank, new_file);

            if (out_of_bounds(new_rank, new_file)) {
                break;
            }

            bitboard::set_bit(&mask, new_square);

            if (bitboard::is_occupied(blockers, new_square))
                break;
        }
    }

    return mask;
}

i32 get_bishop_hash(Square square, u64 blockers) {
    blockers &= bishop_masks[static_cast<i32>(square)];
    blockers *= BishopMagics[static_cast<i32>(square)];
    blockers >>= 64 - bishop_relevancies[static_cast<i32>(square)];
    return static_cast<i32>(blockers);
}

i32 get_rook_hash(Square square, u64 blockers) {
    blockers &= rook_masks[static_cast<i32>(square)];
    blockers *= RookMagics[static_cast<i32>(square)];
    blockers >>= 64 - rook_relevancies[static_cast<i32>(square)];
    return static_cast<i32>(blockers);
}

u64 get_blocker(i32 index, u64 mask) {
    u64 blocker = 0ULL;
    i32 n = 0;

    while (mask) {
        i32 square = bitboard::pop_lsb(&mask);

        if (index & bitboard::mask(static_cast<Square>(n))) {
            blocker |= bitboard::mask(static_cast<Square>(square));
        }

        ++n;
    }

    return blocker;
}

} // namespace

u64 get_pawn_attacks(Square square, Colour colour) {
    return pawn_attacks[static_cast<i32>(colour)][static_cast<i32>(square)];
}

u64 get_knight_attacks(Square square) {
    return knight_attacks[static_cast<i32>(square)];
}

u64 get_king_attacks(Square square) {
    return king_attacks[static_cast<i32>(square)];
}

u64 get_bishop_attacks(Square square, u64 blockers) {
    return bishop_attacks[static_cast<i32>(square)][get_bishop_hash(square, blockers)];
}

u64 get_rook_attacks(Square square, u64 blockers) {
    return rook_attacks[static_cast<i32>(square)][get_rook_hash(square, blockers)];
}

u64 get_queen_attacks(Square square, u64 blockers) {
    return get_bishop_attacks(square, blockers) | get_rook_attacks(square, blockers);
}

void init_attacks() {
    for (Square square = Square::A1; square <= Square::H8;
         square = static_cast<Square>(static_cast<i32>(square) + 1)) {

        load_pawn_attacks(square);
        load_knight_attacks(square);
        load_king_attacks(square);

        load_bishop_mask(square);
        load_rook_mask(square);

        load_relevancies(square);

        for (i32 index = 0; index < (1 << bishop_relevancies[static_cast<i32>(square)]); ++index) {
            u64 blockers = get_blocker(index, bishop_masks[static_cast<i32>(square)]);
            u64 attacks = raycast_bishop_attacks(square, blockers);
            i32 hash = get_bishop_hash(square, blockers);

            bishop_attacks[static_cast<i32>(square)][hash] = attacks;
        }

        for (i32 index = 0; index < (1 << rook_relevancies[static_cast<i32>(square)]); ++index) {
            u64 blockers = get_blocker(index, rook_masks[static_cast<i32>(square)]);
            u64 attacks = raycast_rook_attacks(square, blockers);
            i32 hash = get_rook_hash(square, blockers);

            rook_attacks[static_cast<i32>(square)][hash] = attacks;
        }
    }
}

} // namespace crumb::attacks
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

#include "attacks.hpp"
#include "core.hpp"
#include "bitboard.hpp"

namespace crumb::attacks
{
    namespace {

    constexpr int BISHOP_TABLE_SIZE = 5248, ROOK_TABLE_SIZE = 102400;

    u64 pawn_attacks[COLOR_NB][BOARD_SIZE];
    u64 knight_attacks[BOARD_SIZE];
    u64 king_attacks[BOARD_SIZE];

    u64 bishop_attacks[BISHOP_TABLE_SIZE];
    u64 rook_attacks[ROOK_TABLE_SIZE];

    // slider helper tables
    u64 bishop_masks[BOARD_SIZE];
    u64 rook_masks[BOARD_SIZE];

    u64 bishop_relevancies[BOARD_SIZE];
    u64 rook_relevancies[BOARD_SIZE];

    constexpr u64 BISHOP_MAGICS[BOARD_SIZE] = 
    {
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
        0x0000000010020200ULL, 0x0000000404080200ULL, 0x0000040404040400ULL, 0x0002020202020200ULL
    };

    constexpr u64 ROOK_MAGICS[BOARD_SIZE] = 
    {
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
        0x0001000204080011ULL, 0x0001000204000801ULL, 0x0001000082000401ULL, 0x0001FFFAABFAD1A2ULL
    };

    constexpr int BISHOP_OFFSETS[BOARD_SIZE] = 
    {
           0,   64,   96,  128,  160,  192,  224,  256,
         320,  352,  384,  416,  448,  480,  512,  544,
         576,  608,  640,  768,  896, 1024, 1152, 1184,
        1216, 1248, 1280, 1408, 1920, 2432, 2560, 2592,
        2624, 2656, 2688, 2816, 3328, 3840, 3968, 4000,
        4032, 4064, 4096, 4224, 4352, 4480, 4608, 4640,
        4672, 4704, 4736, 4768, 4800, 4832, 4864, 4896,
        4928, 4992, 5024, 5056, 5088, 5120, 5152, 5184,
    };

    constexpr int ROOK_OFFSETS[BOARD_SIZE] = 
    {
            0,  4096,  6144,  8192, 10240, 12288, 14336, 16384,
        20480, 22528, 23552, 24576, 25600, 26624, 27648, 28672,
        30720, 32768, 33792, 34816, 35840, 36864, 37888, 38912,
        40960, 43008, 44032, 45056, 46080, 47104, 48128, 49152,
        51200, 53248, 54272, 55296, 56320, 57344, 58368, 59392,
        61440, 63488, 64512, 65536, 66560, 67584, 68608, 69632,
        71680, 73728, 74752, 75776, 76800, 77824, 78848, 79872,
        81920, 86016, 88064, 90112, 92160, 94208, 96256, 98304,
    };

    struct Delta { int r, f; };
    constexpr bool out_of_bounds(int r, int f) { return unsigned(r) >= 8 || unsigned(f) >= 8; }

    void load_pawn_attacks(Square square)
    {
        int rank = rank_of(square);
        int file = file_of(square);

        u64 white_mask = 0ULL;
        u64 black_mask = 0ULL;

        // white pawn attacks 

        if (!out_of_bounds(rank + 1, file - 1))
            white_mask |= square_mask(make_square(rank + 1, file - 1));

        if (!out_of_bounds(rank + 1, file + 1))
            white_mask |= square_mask(make_square(rank + 1, file + 1));

        // black pawn attacks

        if (!out_of_bounds(rank - 1, file - 1))
            black_mask |= square_mask(make_square(rank - 1, file - 1));

        if (!out_of_bounds(rank - 1, file + 1))
            black_mask |= square_mask(make_square(rank - 1, file + 1));

        pawn_attacks[0][(int)square] = white_mask;
        pawn_attacks[1][(int)square] = black_mask;
    }

    constexpr Delta knight_deltas[8] = {
        {1, 2}, {1, -2}, {-1, 2}, {-1, -2},
        {2, 1}, {2, -1}, {-2, 1}, {-2, -1}
    };

    void load_knight_attacks(Square square)
    {
        u64 mask = 0ULL;

        for (const Delta& delta : knight_deltas)
        {
            int newr = rank_of(square) + delta.r;
            int newf = file_of(square) + delta.f;

            if (!out_of_bounds(newr,newf))
                mask |= square_mask(make_square(newr, newf));
        }

        knight_attacks[(int)square] = mask;
    }

    constexpr Delta king_deltas[8] = {
        { 1, 0}, { 1,  1}, {0,  1}, {-1,  1},
        {-1, 0}, {-1, -1}, {0, -1}, { 1, -1}
    };

    void load_king_attacks(Square square)
    {
        u64 mask = 0ULL;

        for (const Delta& delta : king_deltas)
        {
            int newr = rank_of(square) + delta.r;
            int newf = file_of(square) + delta.f;

            if (!out_of_bounds(newr, newf))
                mask |= square_mask(make_square(newr, newf));
        }

        king_attacks[(int)square] = mask;
    }

    void load_bishop_mask(Square square)
    {
        int rank = rank_of(square);
        int file = file_of(square);

        u64 mask = 0ULL;

        // {.r=1, .f=1}
        for (int newr = rank + 1, newf = file + 1; newr <= 6 && newf <= 6; newr++, newf++)
        {
            mask |= square_mask(make_square(newr, newf));
        }

        // {.r=1, .f=-1}
        for (int newr = rank + 1, newf = file - 1; newr <= 6 && newf >= 1; newr++, newf--)
        {
            mask |= square_mask(make_square(newr, newf));
        }

        // {.r=-1, .f=1}
        for (int newr = rank - 1, newf = file + 1; newr >= 1 && newf <= 6; newr--, newf++)
        {
            mask |= square_mask(make_square(newr, newf));
        }

        // {.r=-1, .f=-1}
        for (int newr = rank - 1, newf = file - 1; newr >= 1 && newf >= 1; newr--, newf--)
        {
            mask |= square_mask(make_square(newr, newf));
        }

        bishop_masks[(int)square] = mask;
    }

    void load_rook_mask(Square square)
    {
        int rank = rank_of(square);
        int file = file_of(square);

        u64 mask = 0ULL;

        // {.r=1, .f=0}
        for (int newr = rank + 1; newr <= 6; newr++)
        {
            mask |= square_mask(make_square(newr, file));
        }

        // {.r=-1, .f=0}
        for (int newr = rank - 1; newr >= 1; newr--)
        {
            mask |= square_mask(make_square(newr, file));
        }

        // {.r=0, .f=1}
        for (int newf = file + 1; newf <= 6; newf++)
        {
            mask |= square_mask(make_square(rank, newf));
        }

        // {.r=0, .f=-1}
        for (int newf = file - 1; newf >= 1; newf--)
        {
            mask |= square_mask(make_square(rank, newf));
        }

        rook_masks[(int)square] = mask;
    }

    int hash_bishop(Square square, u64 blockers)
    {
        blockers &= bishop_masks[(int)square];

        blockers *= BISHOP_MAGICS[(int)square];
        blockers >>= 64 - bishop_relevancies[(int)square];

        return (int)blockers;
    }

    int hash_rook(Square square, u64 blockers)
    {
        blockers &= rook_masks[(int)square];

        blockers *= ROOK_MAGICS[(int)square];
        blockers >>= 64 - rook_relevancies[(int)square];

        return (int)blockers;
    }

    constexpr Delta BISHOP_DELTAS[] = {
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
    };

    u64 gen_bishop_rays(Square square, u64 blockers)
    {
        u64 mask = 0ULL;

        int rank = rank_of(square);
        int file = file_of(square);

        for (const Delta& delta : BISHOP_DELTAS)
        {
            for (int n = 1; n <= 7; ++n)
            {
                int newr = rank + delta.r * n;
                int newf = file + delta.f * n;

                if (out_of_bounds(newr, newf))
                    break;

                Square new_square = make_square(newr, newf);
                mask |= square_mask(new_square);

                if (blockers & square_mask(new_square))
                    break;
            }   
        }

        return mask;
    }

    constexpr Delta ROOK_DELTAS[] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
    };

    u64 gen_rook_rays(Square square, u64 blockers)
    {
        u64 mask = 0ULL;

        int rank = rank_of(square);
        int file = file_of(square);

        for (const Delta& delta : ROOK_DELTAS)
        {
            for (int n = 1; n <= 7; ++n)
            {
                int newr = rank + delta.r * n;
                int newf = file + delta.f * n;

                if (out_of_bounds(newr, newf))
                    break;

                Square new_square = make_square(newr, newf);
                mask |= square_mask(new_square);

                if (blockers & square_mask(new_square))
                    break;
            }   
        }

        return mask;
    }

    u64 generate_blocker_combination(u64 mask, int i)
    {
        u64 blocker = 0ULL;
        int n = 0;

        while (mask)
        {
            int sq = pop_lsb(mask);

            if (i & square_mask(Square(n)))
                blocker |= square_mask(Square(sq));

            ++n;
        }

        return blocker;
    }

    }

    void load_attacks()
    {
        for (Square square = Square::A1; square <= Square::H8; square = Square((int)square + 1))
        {
            load_pawn_attacks(square);
            load_knight_attacks(square);
            load_king_attacks(square);

            load_bishop_mask(square);
            load_rook_mask(square);

            bishop_relevancies[(int)square] = popcount(bishop_masks[(int)square]);
            rook_relevancies[(int)square] = popcount(rook_masks[(int)square]);

            for (int i = 0; i < (1 << bishop_relevancies[(int)square]); ++i)
            {
                u64 blockers = generate_blocker_combination(bishop_masks[(int)square], i);
                u64 attacks = gen_bishop_rays(square, blockers);

                int index = hash_bishop(square, blockers) + BISHOP_OFFSETS[(int)square];

                bishop_attacks[index] = attacks;
            }

            for (int i = 0; i < (1 << rook_relevancies[(int)square]); ++i)
            {
                u64 blockers = generate_blocker_combination(rook_masks[(int)square], i);
                u64 attacks = gen_rook_rays(square, blockers);

                int index = hash_rook(square, blockers) + ROOK_OFFSETS[(int)square];

                rook_attacks[index] = attacks;
            }
        }
    }

    u64 get_pawn_attacks(Square sq, Color c)
    {
        return pawn_attacks[(int)c][(int)sq];
    }

    u64 get_knight_attacks(Square sq)
    {
        return knight_attacks[(int)sq];
    }

    u64 get_king_attacks(Square sq)
    {
        return king_attacks[(int)sq];
    }

    u64 get_bishop_attacks(Square sq, u64 blockers)
    {
        return bishop_attacks[hash_bishop(sq, blockers) + BISHOP_OFFSETS[(int)sq]];
    }

    u64 get_rook_attacks(Square sq, u64 blockers)
    {
        return rook_attacks[hash_rook(sq, blockers) + ROOK_OFFSETS[(int)sq]];
    }
}
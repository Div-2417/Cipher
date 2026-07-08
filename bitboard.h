//bitboard.h

#ifndef bitboard_H
#define bitboard_H

#include <string>

#include "defs.h"

// set/get/pop bit macros
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))

namespace BitBoards {
    void init();
    void printBoard(bitboard board);  // lowercase bitboard
    void printWholeBoard();  // prints all bitboards
    void clearBoard();
}


constexpr bitboard FileABB = 0x0101010101010101ULL;
constexpr bitboard FileBBB = FileABB << 1;
constexpr bitboard FileCBB = FileABB << 2;
constexpr bitboard FileDBB = FileABB << 3;
constexpr bitboard FileEBB = FileABB << 4;
constexpr bitboard FileFBB = FileABB << 5;
constexpr bitboard FileGBB = FileABB << 6;
constexpr bitboard FileHBB = FileABB << 7;

constexpr bitboard Rank1BB = 0xFF;
constexpr bitboard Rank2BB = Rank1BB << (8 * 1);
constexpr bitboard Rank3BB = Rank1BB << (8 * 2);
constexpr bitboard Rank4BB = Rank1BB << (8 * 3);
constexpr bitboard Rank5BB = Rank1BB << (8 * 4);
constexpr bitboard Rank6BB = Rank1BB << (8 * 5);
constexpr bitboard Rank7BB = Rank1BB << (8 * 6);
constexpr bitboard Rank8BB = Rank1BB << (8 * 7);



#endif
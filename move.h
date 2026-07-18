#ifndef MOVE_H
#define MOVE_H

#include <cstdint>
#include <string>

using Move = uint32_t;

//move encoding macro
#define encode_move(source, target, piece, promoted, capture, dbl, enpassant, castling) \
    ((source) |          \
    ((target) << 6) |     \
    ((piece) << 12) |     \
    ((promoted) << 16) |  \
    ((capture) << 20) |   \
    ((dbl) << 21) |       \
    ((enpassant) << 22) | \
    ((castling) << 23))

//macros for decoding move
#define get_move_source(move)    ((move) & 0x3f)
#define get_move_target(move)    (((move) >> 6) & 0x3f)
#define get_move_piece(move)     (((move) >> 12) & 0xf)
#define get_move_promoted(move)  (((move) >> 16) & 0xf)
#define get_move_capture(move)   ((move) & 0x100000)
#define get_move_double(move)    ((move) & 0x200000)
#define get_move_enpassant(move) ((move) & 0x400000)
#define get_move_castling(move)  ((move) & 0x800000)

struct MoveList {
    Move moves[256];
    int count = 0;
    void add(Move m) { moves[count++] = m; }
};

struct MoveUndo {
    int captured_piece; // piece type captured, -1 if none (NOT set for enpassant — deterministic)
    int enpassant;       // enpassant square BEFORE this move
    int castle;          // castle rights BEFORE this move
};


/*
                           castling   move     in      in
                              right update     binary  decimal

 king & rooks didn't move:     1111 & 1111  =  1111    15

        white king  moved:     1111 & 1100  =  1100    12
  white king's rook moved:     1111 & 1110  =  1110    14
 white queen's rook moved:     1111 & 1101  =  1101    13
     
         black king moved:     1111 & 0011  =  1011    3
  black king's rook moved:     1111 & 1011  =  1011    11
 black queen's rook moved:     1111 & 0111  =  0111    7

*/

// castling rights update constants
const int castling_rights[64] = {
    13, 15, 15, 15, 12, 15, 15, 14,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
     7, 15, 15, 15,  3, 15, 15, 11
};


namespace move {
    std::string moveToString(Move move);
    int makeMove(Move move, int side);
    void unmakeMove(Move move, int side);
}

#endif
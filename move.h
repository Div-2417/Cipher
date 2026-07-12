#ifndef MOVE_H
#define MOVE_H

#include <cstdint>

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

#endif
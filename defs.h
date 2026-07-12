#ifndef DEFS_H
#define DEFS_H

#include <cassert>
#include <cstddef>
#include <cstdint>

using bitboard = uint64_t;

extern bitboard bitboards[13]; 
extern bitboard occupancies[3];
extern int enpassant;
extern int castle;

enum { no_sq = 64 };

enum { Wp=1, Wn=2,Wb=3,Wr=4,Wq=5,Wk=6,
        Bp=7,Bn=8,Bb=9,Br=10,Bq=11,Bk=12 };

enum {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
};

enum { white, black, both };

//castle
enum { wk = 1, wq = 2, bk = 4, bq = 8 }; // castle rights bits

struct fen{
        bool whiteToMove = true;
        bool castleWK = false, castleWQ = false, castleBK = false, castleBQ = false;
        int enPassantSquare = -1;
        int halfmoveClock = 0;
        int fullmoveNumber = 1;
};


#endif
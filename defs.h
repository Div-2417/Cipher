//defs.h

#ifndef DEFS_H
#define DEFS_H

#include <cassert>
#include <cstddef>
#include <cstdint>

using bitboard = uint64_t;

extern bitboard bitboards[13]; 
extern bitboard occupancies[3];

enum { Wp=1, Wn=2,Wb=3,Wr=4,Wq=5,Wk=6,
        Bp=7,Bn=8,Bb=9,Br=10,Bq=11,Bk=12 };

enum { white, black, both };

struct fen{
        bool whiteToMove = true;
        bool castleWK = false, castleWQ = false, castleBK = false, castleBQ = false;
        int enPassantSquare = -1;
        int halfmoveClock = 0;
        int fullmoveNumber = 1;
};


#endif
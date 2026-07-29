#ifndef HASH_H
#define HASH_H

#include <cstdint>
#include "defs.h"

extern bitboard pieceKeys[13][64];
extern bitboard sideKey;
extern bitboard castleKeys[16];
extern bitboard enpassantKeys[64];

extern bitboard hashKey;

namespace Zobrist{
    void init();
    bitboard computeHash(int side);

}

#endif
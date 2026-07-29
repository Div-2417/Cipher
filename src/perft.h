#ifndef PERFT_H
#define PERFT_H

#include "defs.h"
#include "move.h"
#include "bitboard.h"
#include "movegen.h"
#include "position.h"

namespace perft {
    std::string moveToString(Move move);
    long long perft(int depth, int side);
    void divide(int depth, int side);
    
}

#endif
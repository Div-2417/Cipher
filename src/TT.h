#ifndef TT_H
#define TT_H

#include <cstddef>
#include <cstdint>

#include "move.h"
#include "defs.h"

enum TTFlag { TT_EXACT, TT_ALPHA, TT_BETA };

struct TTEntry {
    uint64_t key   = 0;
    int32_t  score = 0;
    Move     bestMove = 0;
    int16_t  depth = -1;
    int8_t   flag  = TT_EXACT;
};

namespace TT {
    void init(size_t sizeMB);
    void clear();

    bool probe(uint64_t key, int depth, int alpha, int beta, int ply, int& outScore, Move& outMove);
    void store(uint64_t key, int depth, int score, int flag, Move bestMove, int ply);
    
}

#endif
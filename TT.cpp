#include <vector>
#include <algorithm>

#include "TT.h"
#include "defs.h"

namespace {
    std::vector<TTEntry> table;
    uint64_t indexMask = 0;

    // Mate scores are stored/retrieved relative to ply-from-root so that a mate
    // found deep in one search is still valid when reused from a different ply.
    int toTT(int score, int ply){
        if(score >=  MATE_SCORE - 1000) return score + ply;
        if(score <= -MATE_SCORE + 1000) return score - ply;
        return score;
    }
    int fromTT(int score, int ply){
        if(score >=  MATE_SCORE - 1000) return score - ply;
        if(score <= -MATE_SCORE + 1000) return score + ply;
        return score;
    }
}

void TT::init(size_t sizeMB){
    size_t bytes = sizeMB * 1024ULL * 1024ULL;
    size_t entries = bytes / sizeof(TTEntry);

    size_t pow2 = 1;
    while(pow2 * 2 <= entries) pow2 *= 2;
    if(pow2 == 0) pow2 = 1;

    table.assign(pow2, TTEntry{});
    indexMask = pow2 - 1;
}

void TT::clear(){
    std::fill(table.begin(), table.end(), TTEntry{});
}

bool TT::probe(uint64_t key, int depth, int alpha, int beta, int ply, int& outScore, Move& outMove){
    if(table.empty()) return false;

    TTEntry& e = table[key & indexMask];
    outMove = 0;
    if(e.key != key) return false;

    outMove = e.bestMove; // usable as ordering hint regardless of stored depth

    if(e.depth < depth) return false;

    int score = fromTT(e.score, ply);
    if(e.flag == TT_EXACT){ outScore = score; return true; }
    if(e.flag == TT_ALPHA && score <= alpha){ outScore = alpha; return true; }
    if(e.flag == TT_BETA  && score >= beta){  outScore = beta;  return true; }
    return false;
}

void TT::store(uint64_t key, int depth, int score, int flag, Move bestMove, int ply){
    if(table.empty()) return;

    TTEntry& e = table[key & indexMask];
    // Depth-preferred replacement: keep the deeper/more-relevant entry per slot.
    if(e.key != key || depth >= e.depth){
        e.key      = key;
        e.depth    = (int16_t)depth;
        e.flag     = (int16_t)flag;
        e.score    = (int32_t)toTT(score, ply);
        e.bestMove = bestMove;
    }
}
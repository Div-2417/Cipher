#ifndef SEARCH_H
#define SEARCH_H

#include "defs.h"
#include "move.h"

namespace search {
    int evaluate(int side);
    int negamax(int alpha, int beta, int depth, int side, int ply);
    void iterativeDeepening(int maxDepth);
    Move searchPosition(int depth, int side);
    int quiescence(int alpha, int beta, int side, int ply);

}

#endif // SEARCH_H
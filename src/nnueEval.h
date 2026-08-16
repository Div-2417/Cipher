#ifndef NNUE_EVAL_H
#define NNUE_EVAL_H

namespace nnue{
    //wrapper functions for nnue probe lib
    void init_nnue(char *filename);
    int evaluate_nnue(int player, int *pieces, int *squares);
    int evaluate_fen_nnue(char *fen);
}

#endif
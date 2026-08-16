#include "./nnue/nnue.h"
#include "nnueEval.h"

void nnue::init_nnue(char *filename){
    nnue_init(filename);
}

int nnue::evaluate_nnue(int player, int *pieces, int *squares){
    return nnue_evaluate(player, pieces, squares);
}

int nnue::evaluate_fen_nnue(char *fen){
    return nnue_evaluate_fen(fen);
}
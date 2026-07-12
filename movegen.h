#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "defs.h"
#include "move.h"

namespace helper{
    bool isSquareAttacked(int square, int side);
    void AddMove(int source, int target, int piece, MoveList& moveList);
    void AddCapture(int source, int target, int piece, int promoted, MoveList& moveList);
    void AddEnPassant(int source, int target, int piece, MoveList& moveList);
    void AddCastling(int source, int target, int piece, MoveList& moveList);
}

namespace moveGen{
    void generateAllMoves(int side, MoveList& moveList);
    void generateAllCaptures(int side, MoveList& moveList);
}

#endif
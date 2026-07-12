#ifndef movegen_H
#define movegen_H

#include "defs.h"
#include "move.h"

namespace helper{
    bool isSquareAttacked(int square, int side);
    void AddMove(int s);
    void AddCapture();
    void AddEnPassant();
    void AddCastling();
}

namespace moveGen{
    void generatePawnMoves(int side, MoveList& moveList);
    void generatePawnCaptures(int side, MoveList& moveList);
    void generateCastlingMoves(int side, MoveList& moveList);
    void generateAllMoves(int side, MoveList& moveList);
    void generateAllCaptures(int side, MoveList& moveList);

}



#endif
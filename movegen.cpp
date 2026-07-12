#include <iostream>

#include "defs.h"
#include "bitboard.h"
#include "movegen.h"
#include "move.h"

bool helper::isSquareAttacked(int square, int side){
    //by pawns
    if((side ==white) && (pawnAttack[black][square] & bitboards[Wp])) return true;
    if((side ==black) && (pawnAttack[white][square] & bitboards[Bp])) return true;

    //by knights
    if(knightAttack[square] & ((side == white) ? bitboards[Wk] : bitboards[Bk])) return true;;

    //by bishop
    if (Magic::computeBishopAttackOTF(square, occupancies[both]) & ((side == white) ? bitboards[Wb] : bitboards[Bb])) return true;

    //by rook
    if (Magic::computeRookAttackOTF(square,occupancies[both]) & ((side == white) ? bitboards[Wr] : bitboards[Br])) return true;

    //by queen

    //by king
    
}

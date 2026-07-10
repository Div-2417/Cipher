//bitboard.cpp

#include "defs.h"
#include <iostream>
#include "bitboard.h"

bitboard bitboards[13];

void BitBoard::printBoard(bitboard board) {
    std::cout << "\n";

    for (int rank = 7; rank >= 0; rank--){
        for (int file = 0; file < 8; file++){

            int square = rank * 8 + file;
            if (!file)
                printf("  %d ", rank +1);
           printf(" %d", get_bit(board, square) ? 1 : 0); 

        }
        std::cout << "\n";
    }
    
    printf("\n     a b c d e f g h\n\n");
    printf("     Bitboard: %lu\n\n", board);
}

void BitBoard::printWholeBoard(){
    const char* pieceChars = " PNBRQKpnbrqk";

    std::cout<<"\n";

    for(int rank =7; rank >=0;rank--){

        std::cout<< rank+1 << "  ";

        for(int file=0;file <8 ;file++){

            int square =rank*8 + file;
            char c{'.'};

            for(int piece=1;piece<13;piece++){

                if(get_bit(bitboards[piece],square)){
                    c = pieceChars[piece];
                    break;
                }
            }
            std::cout << c << " ";
        }
        std::cout << "\n";
    }

    std::cout << "\n   a b c d e f g h\n\n";
}

void BitBoard::clearBoard() {
    for (int i=0;i<13;i++) bitboards[i]=0ULL;
}

void BitBoard::init() {
    
}


bitboard Magic::ComputePawnAttack(int side, int square){
    bitboard attacks = 0ULL;
    bitboard pieceBB = 0ULL;

    set_bit(pieceBB, square);

    if(side == white){
        //7 for left attack, 9 for right attack
        if(pieceBB >> 7 & ~FileHBB) attacks |= (pieceBB >> 7);
        if(pieceBB >> 9 & ~FileABB) attacks |= (pieceBB >> 9);
    } else {
        if(pieceBB << 7 & ~FileABB) attacks |= (pieceBB << 7);
        if(pieceBB << 9 & ~FileHBB) attacks |= (pieceBB << 9);
    }

    return attacks;
}

bitboard Magic::ComputeKnightAttack(int square){
    bitboard attacks = 0ULL;
    bitboard pieceBB = 0ULL;

    set_bit(pieceBB, square);

    //understand the wrap around condition later
    if(pieceBB >> 17 & ~FileABB) attacks |= (pieceBB >> 17);
    if(pieceBB >> 15 & ~FileHBB) attacks |= (pieceBB >> 15);
    if(pieceBB >> 10 & ~FileABB & ~FileBBB) attacks |= (pieceBB >> 10);
    if(pieceBB >> 6 & ~FileHBB & ~FileHBB) attacks |= (pieceBB >> 6);
    if(pieceBB << 6 & ~FileABB & ~FileBBB) attacks |= (pieceBB << 6);
    if(pieceBB << 10 & ~FileHBB & ~FileHBB) attacks |= (pieceBB << 10);
    if(pieceBB << 15 & ~FileABB) attacks |= (pieceBB << 15);
    if(pieceBB << 17 & ~FileHBB) attacks |= (pieceBB << 17);

    return attacks;
}

bitboard Magic::ComputeKingAttack(int square){
    bitboard attacks = 0ULL;
    bitboard pieceBB = 0ULL;

    set_bit(pieceBB, square);

    if(pieceBB >> 9 & ~FileABB) attacks |= (pieceBB >> 9);
    if(pieceBB >> 8) attacks |= (pieceBB >> 8);
    if(pieceBB >> 7 & ~FileHBB) attacks |= (pieceBB >> 7);
    if(pieceBB >> 1 & ~FileABB) attacks |= (pieceBB >> 1);
    if(pieceBB << 1 & ~FileHBB) attacks |= (pieceBB << 1);
    if(pieceBB << 7 & ~FileABB) attacks |= (pieceBB << 7);
    if(pieceBB << 8) attacks |= (pieceBB << 8);
    if(pieceBB << 9 & ~FileHBB) attacks |= (pieceBB << 9);

    return attacks;
}

void Magic::initLeaperAttacks(){

    for (int sq{0}; sq < 64; sq++) {
        //pawns
        pawnAttack[white][sq] = Magic::ComputePawnAttack(white, sq);
        pawnAttack[black][sq] = Magic::ComputePawnAttack(black, sq);

        //knights
        knightAttack[sq] = Magic::ComputeKnightAttack(sq);

        //king
        kingAttack[sq] = Magic::ComputeKingAttack(sq);
    }
}

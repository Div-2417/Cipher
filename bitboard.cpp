//bitboard.cpp

#include "defs.h"
#include "bitboard.h"

bitboard bitboards[13];

void BitBoards::printBoard(bitboard board) {
    printf("\n");

    for (int rank = 7; rank >= 0; rank--)
    {
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;
            if (!file)
                printf("  %d ", 8 - rank);

            printf(" %d", get_bit(board, square) ? 1 : 0);    
        }
        printf("\n");
    }
    
    printf("\n     a b c d e f g h\n\n");
    printf("     Bitboard: %llud\n\n", board);
}

void BitBoards::clearBoard() {
    for (int i=0;i<13;i++) bitboards[i]=0ULL;
}



void BitBoards::init() {
    
}


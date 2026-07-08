//bitboard.cpp

#include "defs.h"
#include <iostream>
#include "bitboard.h"

bitboard bitboards[13];

void BitBoards::printBoard(bitboard board) {
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

void BitBoards::printWholeBoard(){
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

void BitBoards::clearBoard() {
    for (int i=0;i<13;i++) bitboards[i]=0ULL;
}



void BitBoards::init() {
    
}


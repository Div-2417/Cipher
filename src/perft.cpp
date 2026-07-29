#include <iostream>
#include <string>

#include "defs.h"
#include "bitboard.h"
#include "move.h"
#include "movegen.h"
#include "position.h"
#include "perft.h"

std::string perft::moveToString(Move move) {
    static const char* squares[64] = {
        "a1","b1","c1","d1","e1","f1","g1","h1",
        "a2","b2","c2","d2","e2","f2","g2","h2",
        "a3","b3","c3","d3","e3","f3","g3","h3",
        "a4","b4","c4","d4","e4","f4","g4","h4",
        "a5","b5","c5","d5","e5","f5","g5","h5",
        "a6","b6","c6","d6","e6","f6","g6","h6",
        "a7","b7","c7","d7","e7","f7","g7","h7",
        "a8","b8","c8","d8","e8","f8","g8","h8"
    };

    int source = get_move_source(move);
    int target = get_move_target(move);
    int promoted = get_move_promoted(move);

    std::string result = squares[source];
    result += squares[target];

    if (promoted) {
        char promo_char = ' ';
        switch (promoted) {
            case Wq: case Bq: promo_char = 'q'; break;
            case Wr: case Br: promo_char = 'r'; break;
            case Wb: case Bb: promo_char = 'b'; break;
            case Wn: case Bn: promo_char = 'n'; break;
            default: break;
        }
        result += promo_char;
    }

    return result;
}

long long perft::perft(int depth, int side){
    if(depth == 0) return 1;

    long long nodes = 0;
    MoveList moveList;
    moveGen::generateAllMoves(side, moveList);

    for(int i{0}; i< moveList.count; i++){
        Move move = moveList.moves[i];

        if(!move::makeMove(move, side)) continue;

        nodes += perft(depth - 1, side ^ 1);
        move::unmakeMove(move, side);
    }

    return nodes;
}

void perft::divide(int depth, int side){
    MoveList moveList;
    moveGen::generateAllMoves(side, moveList);

    long long totalNodes{0};
    for(int i{0}; i< moveList.count; i++){
        Move move = moveList.moves[i];

        if(!move::makeMove(move, side)) continue;

        long long nodes = perft(depth - 1, side ^ 1);
        move::unmakeMove(move, side);

        std::cout << moveToString(move) << ": " << nodes << std::endl;
        totalNodes += nodes;
    }
    std::cout << "Total nodes: " << totalNodes << std::endl;
}
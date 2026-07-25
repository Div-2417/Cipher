#include <iostream>
#include <stdlib.h>
#include <chrono>

#include "defs.h"
#include "search.h"
#include "bitboard.h"
#include "movegen.h"
#include "move.h"
#include "perft.h"
#include "time.h"

long long nodeCount = 0;
int search::evaluate(int side){
int score = 0;

    for(int piece = Wp; piece <= Bk; piece++){
        bitboard bb = bitboards[piece];
        while(bb){
            int sq = __builtin_ctzll(bb);
            bool isWhite = (piece <= Wk);
            int pstSq = isWhite ? sq : mirror(sq);

            score += pieceValue[piece]; // material, unchanged

            switch(piece){
                case Wp: case Bp: score += isWhite ? pawnPST[pstSq]   : -pawnPST[pstSq]; break;
                case Wn: case Bn: score += isWhite ? knightPST[pstSq] : -knightPST[pstSq]; break;
                case Wb: case Bb: score += isWhite ? bishopPST[pstSq] : -bishopPST[pstSq]; break;
                case Wr: case Br: score += isWhite ? rookPST[pstSq]   : -rookPST[pstSq]; break;
                case Wq: case Bq: score += isWhite ? queenPST[pstSq]  : -queenPST[pstSq]; break;
                case Wk: case Bk: score += isWhite ? kingMidPST[pstSq]: -kingMidPST[pstSq]; break;
            }
            pop_bit(bb, sq);
        }
    }
    return (side == white) ? score : -score;
}


int search::negamax(int alpha, int beta, int depth, int side, int ply){
    if(Time::shouldStop()) return 0;
    nodeCount++;

    if(depth == 0) return quiescence(alpha, beta, side, ply);

    MoveList moveList;
    moveGen::generateAllMoves(side, moveList);

    // --- score + order moves (MVV-LVA), highest first ---
    int scores[256];
    for(int i = 0; i < moveList.count; i++)
        scores[i] = helper::moveScore(moveList.moves[i]);

    for(int i = 0; i < moveList.count; i++){
        int best = i;
        for(int j = i + 1; j < moveList.count; j++)
            if(scores[j] > scores[best]) best = j;
        if(best != i){
            std::swap(scores[i], scores[best]);
            std::swap(moveList.moves[i], moveList.moves[best]);
        }
    }
    // --- end ordering ---

    int legalMoves = 0;
    int bestScore = -INF;

    for(int i = 0; i < moveList.count; i++){
        Move mv = moveList.moves[i];
        if(!move::makeMove(mv, side)) continue;
        legalMoves++;

        int score = -negamax(-beta, -alpha, depth - 1, side ^ 1, ply + 1);
        move::unmakeMove(mv, side);

        if(score > bestScore) bestScore = score;
        if(bestScore > alpha) alpha = bestScore;
        if(alpha >= beta) break;
    }

    if(legalMoves == 0){
        int kingSq = __builtin_ctzll(bitboards[(side == white) ? Wk : Bk]);
        if(helper::isSquareAttacked(kingSq, side ^ 1))
            return -MATE_SCORE + ply;
        return 0;
    }

    return bestScore;
}

int search::quiescence(int alpha, int beta, int side, int ply){
    if(Time::shouldStop()) return 0;
    nodeCount++;

    if(ply >= MAX_PLY) return evaluate(side);   // NEW: hard stop, no exceptions

    int kingSq = __builtin_ctzll(bitboards[(side == white) ? Wk : Bk]);
    bool inCheck = helper::isSquareAttacked(kingSq, side ^ 1);

    int standPat = search::evaluate(side);
    if(!inCheck){
        if(standPat >= beta) return beta;
        if(standPat > alpha) alpha = standPat;
    }

    MoveList moveList;
    if(inCheck){
        moveGen::generateAllMoves(side, moveList);
    } else {
        moveGen::generateAllCaptures(side, moveList);
    }

    int legalMoves = 0;
    for(int i = 0; i < moveList.count; i++){
        Move mv = moveList.moves[i];
        if(!move::makeMove(mv, side)) continue;
        legalMoves++;

        int score = -quiescence(-beta, -alpha, side ^ 1, ply + 1);
        move::unmakeMove(mv, side);

        if(score >= beta) return beta;
        if(score > alpha) alpha = score;
    }

    if(inCheck && legalMoves == 0)
        return -MATE_SCORE + ply;

    return alpha;
}

Move search::searchPosition(int maxDepth, int side){
    Move bestMove = 0;
    int bestScore = -INF;
    nodeCount = 0;
    auto searchStart = std::chrono::steady_clock::now();

    for(int d = 1; d <= maxDepth; d++){
        if(Time::shouldStop()) break;

        MoveList moveList;
        moveGen::generateAllMoves(side, moveList);

        int scores[256];
        for(int i = 0; i < moveList.count; i++)
            scores[i] = helper::moveScore(moveList.moves[i]);
        for(int i = 0; i < moveList.count; i++){
            int best = i;
            for(int j = i + 1; j < moveList.count; j++)
                if(scores[j] > scores[best]) best = j;
            if(best != i){
                std::swap(scores[i], scores[best]);
                std::swap(moveList.moves[i], moveList.moves[best]);
            }
        }

        int currentBestScore = -INF;
        Move currentBestMove = 0;

        for(int i = 0; i < moveList.count; i++){
            if(Time::shouldStop()) break;
            Move mv = moveList.moves[i];
            if(!move::makeMove(mv, side)) continue;

            int score = -negamax(-INF, INF, d - 1, side ^ 1, 1);
            move::unmakeMove(mv, side);

            if(score > currentBestScore){
                currentBestScore = score;
                currentBestMove = mv;
            }
        }

        if(!Time::shouldStop() && currentBestMove){
            bestMove = currentBestMove;
            bestScore = currentBestScore;

            auto now = std::chrono::steady_clock::now();
            long long elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - searchStart).count();
            long long nps = (elapsedMs > 0) ? (nodeCount * 1000 / elapsedMs) : nodeCount;

            std::cout << "info depth " << d
                      << " score cp " << bestScore
                      << " nodes " << nodeCount
                      << " time " << elapsedMs
                      << " nps " << nps
                      << " pv " << perft::moveToString(bestMove) << "\n";
        }
    }
    return bestMove;
}
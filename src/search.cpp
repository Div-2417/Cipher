#include <iostream>
#include <stdlib.h>
#include <chrono>
#include <algorithm>

#include "defs.h"
#include "search.h"
#include "bitboard.h"
#include "movegen.h"
#include "move.h"
#include "perft.h"
#include "time.h"
#include "hash.h"
#include "TT.h"
#include "nnueEval.h"

long long nodeCount = 0;

int nnue_pieces[12] = {6,5,4,3,2,1,12,11,10,9,8,7};

namespace {
    // headroom beyond MAX_PLY so a custom "go depth N>64" from a GUI can't index out of bounds
    constexpr int SEARCH_ARR = 128;

    Move killerMoves[2][SEARCH_ARR];
    int  historyScore[13][64];
    Move pvTable[SEARCH_ARR][SEARCH_ARR];
    int  pvLength[SEARCH_ARR];

    inline int clampPly(int ply){ return (ply < SEARCH_ARR) ? ply : SEARCH_ARR - 1; }

    int gamePhase(){
        int knights = __builtin_popcountll(bitboards[Wn] | bitboards[Bn]);
        int bishops = __builtin_popcountll(bitboards[Wb] | bitboards[Bb]);
        int rooks   = __builtin_popcountll(bitboards[Wr] | bitboards[Br]);
        int queens  = __builtin_popcountll(bitboards[Wq] | bitboards[Bq]);
        int phase = knights + bishops + rooks * 2 + queens * 4;
        return phase > 24 ? 24 : phase;
    }

    bool hasNonPawnMaterial(int side){
        if(side == white) return (bitboards[Wn] | bitboards[Wb] | bitboards[Wr] | bitboards[Wq]) != 0ULL;
        return (bitboards[Bn] | bitboards[Bb] | bitboards[Br] | bitboards[Bq]) != 0ULL;
    }

    // move-ordering score: TT move first, then captures by SEE sign, then killers, then history
    int scoreMoveForOrdering(Move mv, int ply, Move ttMove){
        if(ttMove && mv == ttMove) return 2000000;

        if(get_move_capture(mv)){
            int seeVal = helper::see(mv);
            return (seeVal >= 0) ? (1000000 + seeVal) : (-1000000 + seeVal);
        }

        int p = clampPly(ply);
        if(mv == killerMoves[0][p]) return 900000;
        if(mv == killerMoves[1][p]) return 800000;

        return historyScore[get_move_piece(mv)][get_move_target(mv)];
    }

    void orderMoves(MoveList& moveList, int ply, Move ttMove, int* outScores = nullptr){
        int scores[256];
        for(int i = 0; i < moveList.count; i++)
            scores[i] = scoreMoveForOrdering(moveList.moves[i], ply, ttMove);

        for(int i = 0; i < moveList.count; i++){
            int best = i;
            for(int j = i + 1; j < moveList.count; j++)
                if(scores[j] > scores[best]) best = j;
            if(best != i){
                std::swap(scores[i], scores[best]);
                std::swap(moveList.moves[i], moveList.moves[best]);
            }
        }

        // scores stay in sync with the sorted move list
        if(outScores)
            for(int i = 0; i < moveList.count; i++)
                outScores[i] = scores[i];
    }

    void recordCutoff(Move mv, int depth, int ply){
        if(get_move_capture(mv)) return; // killers/history are for quiet moves only

        int p = clampPly(ply);
        if(killerMoves[0][p] != mv){
            killerMoves[1][p] = killerMoves[0][p];
            killerMoves[0][p] = mv;
        }

        int& h = historyScore[get_move_piece(mv)][get_move_target(mv)];
        h += depth * depth;
        if(h > (1 << 20)) h = (1 << 20);
    }

    void updatePV(int ply, Move mv){
        int p = clampPly(ply);
        int pn = clampPly(ply + 1);
        pvTable[p][p] = mv;
        for(int next = p + 1; next < pvLength[pn]; next++)
            pvTable[p][next] = pvTable[pn][next];
        pvLength[p] = pvLength[pn];
    }

    bool isRepetition(){
        int limit = undo_index - fifty;
        if(limit < 0) limit = 0;
        for(int i = undo_index - 2; i >= limit; i -= 2)
            if(repetitionHistory[i] == hashKey) return true;
        return false;
    }

    // null-move: flips side to move without touching the board
    struct NullUndo { int enpassant; uint64_t hashKey; };

    NullUndo makeNullMove(){
        NullUndo u{enpassant, hashKey};
        if(enpassant != no_sq) hashKey ^= enpassantKeys[enpassant];
        enpassant = no_sq;
        hashKey ^= sideKey;
        return u;
    }

    void unmakeNullMove(const NullUndo& u){
        enpassant = u.enpassant;
        hashKey = u.hashKey;
    }

    struct RootResult { int score = -INF; Move move = 0; };

    RootResult searchRoot(int alpha, int beta, int depth, int side){
        RootResult result;

        Move ttMove = 0;
        int ttScore = 0;
        TT::probe(hashKey, depth, alpha, beta, 0, ttScore, ttMove);

        MoveList moveList;
        moveGen::generateAllMoves(side, moveList);
        orderMoves(moveList, 0, ttMove);

        int originalAlpha = alpha;
        int legalMoves = 0;
        pvLength[0] = 0;

        for(int i = 0; i < moveList.count; i++){
            Move mv = moveList.moves[i];
            if(!move::makeMove(mv, side)) continue;
            legalMoves++;

            int score = -search::negamax(-beta, -alpha, depth - 1, side ^ 1, 1);
            move::unmakeMove(mv, side);

            if(Time::shouldStop()) break;

            if(score > result.score){
                result.score = score;
                result.move = mv;
                updatePV(0, mv);
            }
            if(result.score > alpha) alpha = result.score;
            if(alpha >= beta) break;
        }

        if(legalMoves == 0){
            int kingSq = __builtin_ctzll(bitboards[(side == white) ? Wk : Bk]);
            result.score = helper::isSquareAttacked(kingSq, side ^ 1) ? -MATE_SCORE : 0;
        }

        if(!Time::shouldStop() && legalMoves > 0){
            int flag = (result.score <= originalAlpha) ? TT_ALPHA
                     : (result.score >= beta ? TT_BETA : TT_EXACT);
            TT::store(hashKey, depth, result.score, flag, result.move, 0);
        }

        return result;
    }
}

int search::evaluate(int side){
    /*int score = 0;
    int phase = gamePhase();

    for(int piece = Wp; piece <= Bk; piece++){
        bitboard bb = bitboards[piece];
        while(bb){
            int sq = __builtin_ctzll(bb);
            bool isWhite = (piece <= Wk);
            int pstSq = isWhite ? mirror(sq) : sq;
            int file = sq % 8;
            int ownSide = isWhite ? white : black;

            score += pieceValue[piece]; // material, unchanged

            switch(piece){
                case Wp: case Bp: {
                    score += isWhite ? pawnPST[pstSq] : -pawnPST[pstSq];

                    bitboard ownPawns   = isWhite ? bitboards[Wp] : bitboards[Bp];
                    bitboard enemyPawns = isWhite ? bitboards[Bp] : bitboards[Wp];

                    if(__builtin_popcountll(ownPawns & fileMasks[file]) > 1)
                        score += isWhite ? -10 : 10; // doubled

                    if((ownPawns & adjacentFileMask[file]) == 0ULL)
                        score += isWhite ? -10 : 10; // isolated

                    if((enemyPawns & passedPawnMask[ownSide][sq]) == 0ULL){
                        int rank = sq / 8;
                        int rankFromPromo = isWhite ? rank : 7 - rank;
                        static const int passedBonus[8] = {0,10,20,35,60,100,150,0};
                        score += isWhite ? passedBonus[rankFromPromo] : -passedBonus[rankFromPromo];
                    }
                    break;
                }
                case Wn: case Bn: {
                    score += isWhite ? knightPST[pstSq] : -knightPST[pstSq];
                    int mob = __builtin_popcountll(knightAttack[sq] & ~occupancies[ownSide]);
                    score += isWhite ? mob * 4 : -mob * 4;
                    break;
                }
                case Wb: case Bb: {
                    score += isWhite ? bishopPST[pstSq] : -bishopPST[pstSq];
                    int mob = __builtin_popcountll(Magic::getBishopAttacks(sq, occupancies[both]) & ~occupancies[ownSide]);
                    score += isWhite ? mob * 4 : -mob * 4;
                    break;
                }
                case Wr: case Br: {
                    score += isWhite ? rookPST[pstSq] : -rookPST[pstSq];
                    int mob = __builtin_popcountll(Magic::getRookAttacks(sq, occupancies[both]) & ~occupancies[ownSide]);
                    score += isWhite ? mob * 2 : -mob * 2;
                    break;
                }
                case Wq: case Bq: {
                    score += isWhite ? queenPST[pstSq] : -queenPST[pstSq];
                    bitboard att = Magic::getBishopAttacks(sq, occupancies[both]) | Magic::getRookAttacks(sq, occupancies[both]);
                    int mob = __builtin_popcountll(att & ~occupancies[ownSide]);
                    score += isWhite ? mob : -mob;
                    break;
                }
                case Wk: case Bk: {
                    int tapered = (kingMidPST[pstSq] * phase + kingEndPST[pstSq] * (24 - phase)) / 24;
                    score += isWhite ? tapered : -tapered;
                    break;
                }
            }
            pop_bit(bb, sq);
        }
    }
    return (side == white) ? score : -score;*/

    bitboard bitboard;
    int piece,square;

    int pieces[33];
    int squares[33];

    int index{2};

    for (int bbPiece = Wp; bbPiece <= Bk; bbPiece++) {
        bitboard = bitboards[bbPiece];
        
        while (bitboard){
            piece = bbPiece;
            square = __builtin_ctzll(bitboard);

            if(piece == Wk){
                pieces[0]= nnue_pieces[piece - 1];
                squares[0]= square;
            }else if(piece == Bk){
                pieces[1]= nnue_pieces[piece - 1];
                squares[1]= square;
            }else{
                pieces[index]= nnue_pieces[piece - 1];
                squares[index]= square;
                index++;
            }

            pop_bit(bitboard, square);
        }
    }

    pieces[index] = 0;
    squares[index] = 0;

    return nnue::evaluate_nnue(side,pieces,squares);
}

int search::negamax(int alpha, int beta, int depth, int side, int ply){
    if(Time::shouldStop()) return 0;
    nodeCount++;

    pvLength[clampPly(ply)] = ply;

    if(ply > 0 && (fifty >= 100 || isRepetition())) return 0;

    if(depth == 0) return quiescence(alpha, beta, side, ply);

    Move ttMove = 0;
    int ttScore = 0;
    if(TT::probe(hashKey, depth, alpha, beta, ply, ttScore, ttMove)) return ttScore;

    int kingSq = __builtin_ctzll(bitboards[(side == white) ? Wk : Bk]);
    bool inCheck = helper::isSquareAttacked(kingSq, side ^ 1);

    // check extension: tactical lines get one extra ply
    if(inCheck) depth += 1;

    // reverse futility pruning: if even a generous static-eval margin can't
    // reach beta, the node is hopeless; return the static eval
    if(depth <= 2 && !inCheck && ply > 0){
        int staticEval = evaluate(side);
        if(staticEval - 150 * depth >= beta) return staticEval;
    }

    // null-move pruning: skip a turn and see if we're still doing fine; if so, this
    // node is unlikely to need full search
    if(depth >= 3 && !inCheck && ply > 0 && hasNonPawnMaterial(side)){
        NullUndo nu = makeNullMove();
        int R = (depth > 6) ? 3 : 2;
        int nullScore = -negamax(-beta, -beta + 1, depth - 1 - R, side ^ 1, ply + 1);
        unmakeNullMove(nu);

        if(Time::shouldStop()) return 0;
        if(nullScore >= beta) return beta;
    }

    MoveList moveList;
    moveGen::generateAllMoves(side, moveList);
    orderMoves(moveList, ply, ttMove);

    int originalAlpha = alpha;
    int legalMoves = 0;
    int bestScore = -INF;
    Move bestMoveLocal = 0;

    for(int i = 0; i < moveList.count; i++){
        Move mv = moveList.moves[i];

        // Late Move Pruning: at shallow depth, quiet moves deep in the ordering
        // are unlikely to beat alpha; skip them before even making the move
        if(depth <= 3 && !inCheck && legalMoves > 4 + depth * depth
           && !get_move_capture(mv) && !get_move_promoted(mv) && mv != ttMove) continue;

        if(!move::makeMove(mv, side)) continue;
        legalMoves++;

        int newDepth = depth - 1;
        int score;

        if(legalMoves == 1){
            score = -negamax(-beta, -alpha, newDepth, side ^ 1, ply + 1);
        } else {
            int reduction = 0;
            // Late Move Reduction: search quiet, non-promoting late moves shallower first
            if(depth >= 3 && legalMoves > 4 && !get_move_capture(mv) && !get_move_promoted(mv) && !inCheck){
                reduction = (legalMoves > 10 && depth >= 6) ? 2 : 1;
            }

            if(reduction > 0){
                score = -negamax(-alpha - 1, -alpha, newDepth - reduction, side ^ 1, ply + 1);
                if(score > alpha){
                    score = -negamax(-beta, -alpha, newDepth, side ^ 1, ply + 1); // re-search, no reduction
                }
            } else {
                score = -negamax(-beta, -alpha, newDepth, side ^ 1, ply + 1);
            }
        }

        move::unmakeMove(mv, side);
        if(Time::shouldStop()) return 0;

        if(score > bestScore){
            bestScore = score;
            bestMoveLocal = mv;
            updatePV(ply, mv);
        }
        if(bestScore > alpha) alpha = bestScore;
        if(alpha >= beta){
            recordCutoff(mv, depth, ply);
            break;
        }
    }

    if(legalMoves == 0){
        if(inCheck) return -MATE_SCORE + ply;
        return 0;
    }

    int flag = (bestScore <= originalAlpha) ? TT_ALPHA : (bestScore >= beta ? TT_BETA : TT_EXACT);
    TT::store(hashKey, depth, bestScore, flag, bestMoveLocal, ply);

    return bestScore;
}

int search::quiescence(int alpha, int beta, int side, int ply){
    if(Time::shouldStop()) return 0;
    nodeCount++;

    if(ply >= MAX_PLY) return evaluate(side);   // hard stop, no exceptions

    int kingSq = __builtin_ctzll(bitboards[(side == white) ? Wk : Bk]);
    bool inCheck = helper::isSquareAttacked(kingSq, side ^ 1);

    int standPat = evaluate(side);
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

    // scores[] carries the SEE values already computed for move ordering, so
    // the SEE pruning below is free
    int scores[256];
    orderMoves(moveList, ply, 0, scores);

    int legalMoves = 0;
    for(int i = 0; i < moveList.count; i++){
        Move mv = moveList.moves[i];

        if(!inCheck && get_move_capture(mv)){

            // delta pruning: even the captured piece plus a margin can't close
            // the gap to alpha, so this capture is hopeless
            int victim = get_move_enpassant(mv) ? 100 : std::abs(pieceValue[mailbox[get_move_target(mv)]]);
            if(standPat + victim + 200 < alpha) continue;

            // SEE pruning: don't even try captures that lose material outright,
            // unless we're in check (then every legal reply matters)
            if(scores[i] < 0) continue;
        }

        if(!move::makeMove(mv, side)) continue;
        legalMoves++;

        int score = -quiescence(-beta, -alpha, side ^ 1, ply + 1);
        move::unmakeMove(mv, side);

        if(Time::shouldStop()) return 0;
        if(score >= beta) return beta;
        if(score > alpha) alpha = score;
    }

    if(inCheck && legalMoves == 0)
        return -MATE_SCORE + ply;

    return alpha;
}

Move search::searchPosition(int maxDepth, int side){
    Move bestMove = 0;
    int bestScore = 0;
    nodeCount = 0;
    auto searchStart = std::chrono::steady_clock::now();

    for(int d = 1; d <= maxDepth; d++){
        if(Time::shouldStop()) break;

        int alpha = -INF, beta = INF, window = 25;
        if(d >= 4){
            alpha = bestScore - window;
            beta  = bestScore + window;
        }

        RootResult result;
        while(true){
            result = searchRoot(alpha, beta, d, side);
            if(Time::shouldStop()) break;

            if(result.score <= alpha){
                window *= 2;
                alpha = (bestScore - window < -INF) ? -INF : bestScore - window;
                continue;
            }
            if(result.score >= beta){
                window *= 2;
                beta = (bestScore + window > INF) ? INF : bestScore + window;
                continue;
            }
            break;
        }

        if(!Time::shouldStop() && result.move){
            bestMove = result.move;
            bestScore = result.score;

            auto now = std::chrono::steady_clock::now();
            long long elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - searchStart).count();
            long long nps = (elapsedMs > 0) ? (nodeCount * 1000 / elapsedMs) : nodeCount;

            std::cout << "info depth " << d
                      << " score cp " << bestScore
                      << " nodes " << nodeCount
                      << " time " << elapsedMs
                      << " nps " << nps
                      << " pv";
            for(int i = 0; i < pvLength[0]; i++)
                std::cout << " " << perft::moveToString(pvTable[0][i]);
            std::cout << std::endl;
        }
    }
    return bestMove;
}

Move search::getPonderMove() {
    if (pvLength[0] >= 2) return pvTable[0][1];
    return 0;
}

void search::resetSearchState(){
    TT::clear();
    for(int k = 0; k < 2; k++)
        for(int p = 0; p < SEARCH_ARR; p++)
            killerMoves[k][p] = 0;
    for(int p = 0; p < 13; p++)
        for(int sq = 0; sq < 64; sq++)
            historyScore[p][sq] = 0;
}
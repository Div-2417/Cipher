#include <iostream>
#include <cmath>

#include "defs.h"
#include "bitboard.h"
#include "movegen.h"
#include "move.h"

int enpassant = no_sq;
int castle = 0;

bool helper::isSquareAttacked(int square, int side){
    //by pawns
    if((side ==white) && (pawnAttack[black][square] & bitboards[Wp])) return true;
    if((side ==black) && (pawnAttack[white][square] & bitboards[Bp])) return true;

    //by knights
    if(knightAttack[square] & ((side == white) ? bitboards[Wn] : bitboards[Bn])) return true;;

    //by bishop
    if (Magic::computeBishopAttackOTF(square, occupancies[both]) & ((side == white) ? bitboards[Wb] : bitboards[Bb])) return true;

    //by rook
    if (Magic::computeRookAttackOTF(square,occupancies[both]) & ((side == white) ? bitboards[Wr] : bitboards[Br])) return true;

    //by queen
    if (Magic::ComputeQueenAttack(square, occupancies[both]) & ((side == white) ? bitboards[Wq] : bitboards[Bq])) return true;
    
    //by king
    if (kingAttack[square] & ((side == white) ? bitboards[Wk] : bitboards[Bk])) return true;
    
    return false;
}

void helper::AddMove(int source, int target, int piece, MoveList &moveList){
    Move m = encode_move(source, target, piece, 0, 0, 0, 0, 0);
    moveList.add(m);
}

void helper::AddCapture(int source, int target, int piece, int promoted, MoveList &moveList){
    Move m = encode_move(source,target,piece,promoted,1,0,0,0);
    moveList.add(m);
}

void helper::AddEnPassant(int source, int target, int piece, MoveList &moveList){
    Move m = encode_move(source,target,piece,0,1,0,1,0);
    moveList.add(m);
}

void helper::AddCastling(int source, int target, int piece, MoveList &moveList){
    Move m = encode_move(source,target,piece,0,0,0,0,1);
    moveList.add(m);
}

int helper::moveScore(Move mv){

    if (get_move_capture(mv)) { 
        int attacker = get_move_piece(mv);
        int target = get_move_target(mv);
        int victimValue = 0;

        //enpassant capture
        if (get_move_enpassant(mv)) {
            victimValue = 100; // Absolute value of a pawn based on pieceValue array
        } 
        // Normal capture
        else {
            for (int piece = Wp; piece <= Bk; piece++) {
                if (get_bit(bitboards[piece], target)) {
                    //black pieces have -ve values in piecevalue
                    victimValue = std::abs(pieceValue[piece]);
                    break;
                }
            }
        }

        int attackerValue = std::abs(pieceValue[attacker]);

        //MVV-LVA Score
        // +10000 ensures captures are scored higher than quiet moves.
        // + victimValue prioritizes capturing the most valuable piece (MVV).
        // - (attackerValue / 100) breaks ties by preferring the least valuable attacker (LVA).
        return 10000 + victimValue - (attackerValue / 100);
    }
    
    // Quiets unscored for now
    return 0;
    }

void moveGen::generateAllMoves(int side, MoveList& movelist){
    movelist.count = 0;

    int source,target;
    bitboard attacks, bitboardPieces;

    //helper variables
    int opponent = (side == white) ? black : white;
    int pawnPiece = (side == white) ? Wp : Bp;
    int promoQ = (side == white) ? Wq : Bq;
    int promoR = (side == white) ? Wr : Br;
    int promoB = (side == white) ? Wb : Bb;
    int promoN = (side == white) ? Wn : Bn;

    for (int piece = Wp; piece <= Bk; piece++){
        bitboardPieces = bitboards[piece];
            
        //pawns
        if((piece == Wp && side == white) || (piece == Bp && side == black)){

            while(bitboardPieces){

                source = __builtin_ctzll(bitboardPieces);
                int rank = source / 8;
                target = (side == white) ? source + 8 : source - 8;

                bool onPromoRank = (side == white) ? (rank == 6) : (rank == 1);
                bool onStartRank = (side == white) ? (rank == 1) : (rank == 6);

                if (target >= 0 && target < 64 && !get_bit(occupancies[both], target)){

                    if (onPromoRank) {

                        movelist.add(encode_move(source, target, piece, promoQ, 0, 0, 0, 0));
                        movelist.add(encode_move(source, target, piece, promoR, 0, 0, 0, 0));
                        movelist.add(encode_move(source, target, piece, promoB, 0, 0, 0, 0));
                        movelist.add(encode_move(source, target, piece, promoN, 0, 0, 0, 0));
                    }else{
                        movelist.add(encode_move(source, target, piece, 0, 0, 0, 0, 0));
                    }

                    //double pawn push
                    int doubletarget = (side ==white) ? target + 8 : target - 8;
                    if (onStartRank && !get_bit(occupancies[both], doubletarget)){
                        movelist.add(encode_move(source, doubletarget, piece, 0, 0, 1, 0, 0));
                    }
                }

                //pawn captures
                attacks = pawnAttack[side][source] & occupancies[opponent];
                while(attacks){
                    target = __builtin_ctzll(attacks);

                    if (onPromoRank) {
                        movelist.add(encode_move(source, target, piece, promoQ, 1, 0, 0, 0));
                        movelist.add(encode_move(source, target, piece, promoR, 1, 0, 0, 0));
                        movelist.add(encode_move(source, target, piece, promoB, 1, 0, 0, 0));
                        movelist.add(encode_move(source, target, piece, promoN, 1, 0, 0, 0));
                    }else{
                        movelist.add(encode_move(source, target, piece, 0, 1, 0, 0, 0));
                    }
                    pop_bit(attacks, target);
                }

                //en passant
                if (enpassant != no_sq) {
                    bitboard ep_attack = pawnAttack[side][source] & (1ULL << enpassant);
                    if (ep_attack) {
                        target = __builtin_ctzll(ep_attack);
                        movelist.add(encode_move(source, target, piece, 0, 1, 0, 1, 0));
                    }
                }

                pop_bit(bitboardPieces, source);
            }
        }
        
        //knights
        else if((piece == Wn && side == white) || (piece == Bn && side == black)){

            while(bitboardPieces){

                source = __builtin_ctzll(bitboardPieces);
                // Compute full attack set, excluding own pieces
                attacks = knightAttack[source] & ~occupancies[side];

                while(attacks){
                    target = __builtin_ctzll(attacks);
                    if(get_bit(occupancies[opponent], target))
                        movelist.add(encode_move(source, target, piece, 0, 1, 0, 0, 0));
                    else
                        movelist.add(encode_move(source, target, piece, 0, 0, 0, 0, 0));

                    pop_bit(attacks, target);
                }
                pop_bit(bitboardPieces, source);
            }
        }

        //bishop,rook,queen
        else if(piece == Wb || piece == Bb || piece == Wr || piece == Br || piece == Wq || piece == Bq){
            bool isWhitePiece = (piece == Wb || piece == Wr || piece == Wq);
            if((isWhitePiece && side != white) || (!isWhitePiece && side != black)) continue;

            while(bitboardPieces){
                source = __builtin_ctzll(bitboardPieces);

                if(piece == Wb || piece == Bb)
                    attacks = Magic::getBishopAttacks(source, occupancies[both]);
                else if(piece == Wr || piece == Br)
                    attacks = Magic::getRookAttacks(source, occupancies[both]);
                else
                    attacks = Magic::getBishopAttacks(source, occupancies[both]) | Magic::getRookAttacks(source, occupancies[both]);

                attacks &= ~occupancies[side];

                while(attacks){
                    target = __builtin_ctzll(attacks);
                    if(get_bit(occupancies[opponent], target))
                        movelist.add(encode_move(source, target, piece, 0, 1, 0, 0, 0));
                    else
                        movelist.add(encode_move(source, target, piece, 0, 0, 0, 0, 0));
                    pop_bit(attacks, target);
                }
                pop_bit(bitboardPieces, source);
            }
        }

        //king
        else if(piece == Wk || piece == Bk){
            if((piece == Wk && side != white) || (piece == Bk && side != black)) continue;

            while(bitboardPieces){
                source = __builtin_ctzll(bitboardPieces);
                attacks = kingAttack[source] & ~occupancies[side];

                while(attacks){
                    target = __builtin_ctzll(attacks);
                    if(get_bit(occupancies[opponent], target))
                        movelist.add(encode_move(source, target, piece, 0, 1, 0, 0, 0));
                    else
                        movelist.add(encode_move(source, target, piece, 0, 0, 0, 0, 0));
                    pop_bit(attacks, target);
                }
                pop_bit(bitboardPieces, source);
            }
        }
    }

    //castling
        if(side == white){
                if((castle & wk) && !get_bit(occupancies[both], f1) && !get_bit(occupancies[both], g1) &&
                   !helper::isSquareAttacked(e1, black) && !helper::isSquareAttacked(f1, black) && !helper::isSquareAttacked(g1, black)){
                    movelist.add(encode_move(e1, g1, Wk, 0, 0, 0, 0, 1));}

                if((castle & wq) && !get_bit(occupancies[both], d1) && !get_bit(occupancies[both], c1) && !get_bit(occupancies[both], b1) &&
                   !helper::isSquareAttacked(e1, black) && !helper::isSquareAttacked(d1, black) && !helper::isSquareAttacked(c1, black)){
                    movelist.add(encode_move(e1, c1, Wk, 0, 0, 0, 0, 1));}
            } else {
                if((castle & bk) && !get_bit(occupancies[both], f8) && !get_bit(occupancies[both], g8) &&
                   !helper::isSquareAttacked(e8, white) && !helper::isSquareAttacked(f8, white) && !helper::isSquareAttacked(g8, white)){
                    movelist.add(encode_move(e8, g8, Bk, 0, 0, 0, 0, 1));}
                    
                if((castle & bq) && !get_bit(occupancies[both], d8) && !get_bit(occupancies[both], c8) && !get_bit(occupancies[both], b8) &&
                   !helper::isSquareAttacked(e8, white) && !helper::isSquareAttacked(d8, white) && !helper::isSquareAttacked(c8, white)){
                    movelist.add(encode_move(e8, c8, Bk, 0, 0, 0, 0, 1));}
                }

}

void moveGen::generateAllCaptures(int side, MoveList& movelist){
    MoveList all;
    generateAllMoves(side, all);
    movelist.count = 0;
    for(int i = 0; i < all.count; i++)
        if(get_move_capture(all.moves[i]))
            movelist.add(all.moves[i]);
}


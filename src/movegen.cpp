#include <cmath>
#include <algorithm>

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
    if (Magic::getBishopAttacks(square, occupancies[both]) & ((side == white) ? bitboards[Wb] : bitboards[Bb])) return true;

    //by rook
    if (Magic::getRookAttacks(square, occupancies[both]) & ((side == white) ? bitboards[Wr] : bitboards[Br])) return true;

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

namespace {
    // all pieces (either colour) currently attacking `square`, given arbitrary occupancy `occ`
    bitboard attackersTo(int square, bitboard occ){
        bitboard attackers = 0ULL;

        attackers |= pawnAttack[black][square] & bitboards[Wp] & occ;
        attackers |= pawnAttack[white][square] & bitboards[Bp] & occ;

        attackers |= knightAttack[square] & (bitboards[Wn] | bitboards[Bn]) & occ;
        attackers |= kingAttack[square]   & (bitboards[Wk] | bitboards[Bk]) & occ;

        bitboard bishopRay = Magic::getBishopAttacks(square, occ);
        attackers |= bishopRay & (bitboards[Wb] | bitboards[Bb] | bitboards[Wq] | bitboards[Bq]) & occ;

        bitboard rookRay = Magic::getRookAttacks(square, occ);
        attackers |= rookRay & (bitboards[Wr] | bitboards[Br] | bitboards[Wq] | bitboards[Bq]) & occ;

        return attackers;
    }

    // pops (conceptually) the least valuable `side` attacker from `attackers`, returns its square
    // and piece type, or -1 if `side` has no attacker left in the set.
    int popLeastValuableAttacker(bitboard attackers, int side, int& pieceType){
        int startPiece = (side == white) ? Wp : Bp;
        for(int p = startPiece; p <= startPiece + 5; p++){
            bitboard bb = attackers & bitboards[p];
            if(bb){
                pieceType = p;
                return __builtin_ctzll(bb);
            }
        }
        return -1;
    }
}

int helper::see(Move mv){
    int source = get_move_source(mv);
    int target = get_move_target(mv);
    int attackerPiece = get_move_piece(mv);
    int side = (attackerPiece <= Wk) ? white : black;

    int capturedValue = 0;
    if(get_move_enpassant(mv)){
        capturedValue = 100;
    } else {
        capturedValue = std::abs(pieceValue[mailbox[target]]);
    }

    int gain[32];
    int d = 0;
    gain[0] = capturedValue;

    bitboard occ = occupancies[both];
    pop_bit(occ, source);

    int attackingValue = std::abs(pieceValue[attackerPiece]);
    int sideToMove = side ^ 1; // opponent recaptures next
    bitboard attackers = attackersTo(target, occ);

    while(true){
        d++;
        gain[d] = attackingValue - gain[d - 1];
        if(std::max(-gain[d - 1], gain[d]) < 0) break; // further capturing can't improve result

        int nextPieceType;
        int nextSq = popLeastValuableAttacker(attackers, sideToMove, nextPieceType);
        if(nextSq < 0) break;

        pop_bit(occ, nextSq);
        attackers = attackersTo(target, occ); // removing a piece can reveal new slider attacks

        attackingValue = std::abs(pieceValue[nextPieceType]);
        sideToMove ^= 1;
        if(d >= 31) break; // safety bound, never realistically reached
    }

    while(--d) gain[d - 1] = -std::max(-gain[d - 1], gain[d]);

    return gain[0];
}

void moveGen::generateAllMoves(int side, MoveList& movelist){
    movelist.count = 0;

    int source,target;
    bitboard attacks, bitboardPieces;

    //helper variables
    int opponent = (side == white) ? black : white;
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
    movelist.count = 0;
    int opponent = (side == white) ? black : white;
    bitboard targets = occupancies[opponent];

    int source, target;
    bitboard attacks, bitboardPieces;

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
                bool onPromoRank = (side == white) ? (rank == 6) : (rank == 1);

                //pawn captures
                attacks = pawnAttack[side][source] & targets;
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
                attacks = knightAttack[source] & targets;

                while(attacks){
                    target = __builtin_ctzll(attacks);
                    movelist.add(encode_move(source, target, piece, 0, 1, 0, 0, 0));
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

                attacks &= targets;

                while(attacks){
                    target = __builtin_ctzll(attacks);
                    movelist.add(encode_move(source, target, piece, 0, 1, 0, 0, 0));
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
                attacks = kingAttack[source] & targets;

                while(attacks){
                    target = __builtin_ctzll(attacks);
                    movelist.add(encode_move(source, target, piece, 0, 1, 0, 0, 0));
                    pop_bit(attacks, target);
                }
                pop_bit(bitboardPieces, source);
            }
        }
    }
}
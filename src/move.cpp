#include <cstdint>
#include <cstring>
#include <string>

#include "defs.h"
#include "bitboard.h"
#include "move.h"
#include "movegen.h"

#define copy_board() \
    bitboard bitboards_copy[13], occupancies_copy[3]; \
    int side_copy, enpassant_copy, castle_copy; \
    memcpy(bitboards_copy, bitboards, sizeof(bitboards)); \
    memcpy(occupancies_copy, occupancies, sizeof(occupancies)); \
    side_copy = side; enpassant_copy = enpassant; castle_copy = castle;

#define take_back() \
    memcpy(bitboards, bitboards_copy, sizeof(bitboards)); \
    memcpy(occupancies, occupancies_copy, sizeof(occupancies)); \
    side = side_copy; enpassant = enpassant_copy; castle = castle_copy;

int fifty{0};
MoveUndo undo_history[MAX_UNDO];
int undo_index = 0;
uint64_t repetitionHistory[MAX_UNDO];

int move::makeMove(Move move,int side){
    if(undo_index >= MAX_UNDO - 1) return 0;
    copy_board();

    int source    = get_move_source(move);
    int target    = get_move_target(move);
    int piece     = get_move_piece(move);
    int promoted  = get_move_promoted(move);
    int capture   = get_move_capture(move);
    int doublePush= get_move_double(move);
    int ep        = get_move_enpassant(move);
    int castling  = get_move_castling(move);

    // Save undo information
    undo_history[undo_index].enpassant = enpassant;
    undo_history[undo_index].castle = castle;
    undo_history[undo_index].captured_piece = -1;
    undo_history[undo_index].fifty = fifty;
    undo_history[undo_index].hashKey = hashKey;

    //making the move
    pop_bit(bitboards[piece], source);
    set_bit(bitboards[piece], target);
    hashKey ^= pieceKeys[piece][source] ^ pieceKeys[piece][target];
    fifty++;

    if (piece == Wp || piece == Bp) {
        fifty = 0;
    }

    //capture handling
if (capture) {
    fifty = 0;

    int startPiece, endPiece;

    if (side == white){
        startPiece = Bp;
        endPiece = Bk;
    }
    else{
        startPiece = Wp;
        endPiece = Wk;
    }

    for(int bbPiece = startPiece; bbPiece <= endPiece; bbPiece++){
        if(get_bit(bitboards[bbPiece], target)){
            undo_history[undo_index].captured_piece = bbPiece;
            pop_bit(bitboards[bbPiece], target);
            hashKey ^= pieceKeys[bbPiece][target];
            break;
        }
    }
}

    //handling promotion of pawn
    if (promoted) {
        pop_bit(bitboards[piece], target);
        set_bit(bitboards[promoted], target);
        hashKey ^= pieceKeys[piece][target] ^ pieceKeys[promoted][target];
    }

    //handling enpassant
    if(ep){
        if(side == white){
            pop_bit(bitboards[Bp], target - 8);
            hashKey ^= pieceKeys[Bp][target - 8];
        }else{
            pop_bit(bitboards[Wp], target + 8);
            hashKey ^= pieceKeys[Wp][target + 8];
        }
    }

    int oldEnpassant = enpassant;
    enpassant = no_sq;
    if(doublePush){
        enpassant = (side == white) ? target - 8 : target + 8;
    }
    if(oldEnpassant != no_sq) hashKey ^= enpassantKeys[oldEnpassant];
    if(enpassant != no_sq)    hashKey ^= enpassantKeys[enpassant];

    if (castling) {
        switch (target) {
            case g1:
                pop_bit(bitboards[Wr], h1);
                set_bit(bitboards[Wr], f1);
                hashKey ^= pieceKeys[Wr][h1] ^ pieceKeys[Wr][f1];
                break;
            case c1:
                pop_bit(bitboards[Wr], a1);
                set_bit(bitboards[Wr], d1);
                hashKey ^= pieceKeys[Wr][a1] ^ pieceKeys[Wr][d1];
                break;
            case g8:
                pop_bit(bitboards[Br], h8);
                set_bit(bitboards[Br], f8);
                hashKey ^= pieceKeys[Br][h8] ^ pieceKeys[Br][f8];
                break;
            case c8:
                pop_bit(bitboards[Br], a8);
                set_bit(bitboards[Br], d8);
                hashKey ^= pieceKeys[Br][a8] ^ pieceKeys[Br][d8];
                break;
            default:
                break;
        }
    }

    hashKey ^= castleKeys[castle];   // remove old castle-rights key
    castle &= castling_rights[source];
    castle &= castling_rights[target];
    hashKey ^= castleKeys[castle];   // add new castle-rights key

    //reset occupancies
    memset(occupancies, 0ULL, sizeof(occupancies));
    for(int p = Wp; p <= Wk; p++) occupancies[white] |= bitboards[p];
    for(int p = Bp; p <= Bk; p++) occupancies[black] |= bitboards[p];
    occupancies[both] = occupancies[white] | occupancies[black];

    side ^= 1; // side is now the opponent
    hashKey ^= sideKey;

    // Find the king of the side that just moved (side ^ 1)
    int king_sq = __builtin_ctzll(bitboards[(side == white) ? Bk : Wk]);
    // Check if that king is attacked by the current side (opponent)

    if(helper::isSquareAttacked(king_sq, side)) {
        take_back();
        hashKey = undo_history[undo_index].hashKey;
        return 0;
    }

    // Move is legal: record hash for repetition detection, then bump undo index
    repetitionHistory[undo_index] = hashKey;
    undo_index++;
    return 1;
}

void move::unmakeMove(Move move, int side){
    // Decrement undo history index
    undo_index--;
    hashKey = undo_history[undo_index].hashKey;
    
    int source    = get_move_source(move);
    int target    = get_move_target(move);
    int piece     = get_move_piece(move);
    int promoted  = get_move_promoted(move);
    int capture   = get_move_capture(move);
    int doublePush= get_move_double(move);
    int ep        = get_move_enpassant(move);
    int castling  = get_move_castling(move);
    
    // Switch side back
    side ^= 1;
    
    // Restore captured piece if there was a capture
    if (capture && undo_history[undo_index].captured_piece != -1) {
        set_bit(bitboards[undo_history[undo_index].captured_piece], target);
    }
    
    // Restore the piece
    if (promoted) {
        // Remove promoted piece from target
        pop_bit(bitboards[promoted], target);
        // Put pawn back at source
        set_bit(bitboards[piece], source);
    } else {
        // Move piece back to source
        pop_bit(bitboards[piece], target);
        set_bit(bitboards[piece], source);
    }
    
    // Restore en passant captured pawn
    if (ep) {
        if (side == white) {
            set_bit(bitboards[Wp], target + 8);
        } else {
            set_bit(bitboards[Bp], target - 8);
        }
    }
    
    // Restore castling rook position
    if (castling) {
        switch (target) {
            case g1:
                pop_bit(bitboards[Wr], f1);
                set_bit(bitboards[Wr], h1);
                break;
            case c1:
                pop_bit(bitboards[Wr], d1);
                set_bit(bitboards[Wr], a1);
                break;
            case g8:
                pop_bit(bitboards[Br], f8);
                set_bit(bitboards[Br], h8);
                break;
            case c8:
                pop_bit(bitboards[Br], d8);
                set_bit(bitboards[Br], a8);
                break;
        }
    }
    
    // Restore en passant and castling rights
    enpassant = undo_history[undo_index].enpassant;
    castle = undo_history[undo_index].castle;
    
    // Restore fifty-move counter to its exact pre-move value
    fifty = undo_history[undo_index].fifty;
    
    //reset occupancies
    memset(occupancies, 0ULL, sizeof(occupancies));
    for(int p = Wp; p <= Wk; p++) occupancies[white] |= bitboards[p];
    for(int p = Bp; p <= Bk; p++) occupancies[black] |= bitboards[p];
    occupancies[both] = occupancies[white] | occupancies[black];
}
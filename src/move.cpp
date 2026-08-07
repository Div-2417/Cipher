#include <cstdint>
#include <cstring>

#include "defs.h"
#include "bitboard.h"
#include "move.h"
#include "movegen.h"

int fifty{0};
MoveUndo undo_history[MAX_UNDO];
int undo_index = 0;
uint64_t repetitionHistory[MAX_UNDO];

namespace {

    void unmakeBoardChanges(Move move, int side, const MoveUndo& undo){
        int source    = get_move_source(move);
        int target    = get_move_target(move);
        int piece     = get_move_piece(move);
        int promoted  = get_move_promoted(move);
        int capture   = get_move_capture(move);
        int ep        = get_move_enpassant(move);
        int castling  = get_move_castling(move);

        int nonMover = side ^ 1; // side whose pawn was taken en passant

        if (capture && undo.captured_piece > 0) {
            set_bit(bitboards[undo.captured_piece], target);
            mailbox[target] = (int8_t)undo.captured_piece;
        }

        if (promoted) {
            pop_bit(bitboards[promoted], target);
            set_bit(bitboards[piece], source);
            mailbox[target] = (capture && undo.captured_piece > 0) ? (int8_t)undo.captured_piece : 0;
            mailbox[source] = (int8_t)piece;
        } else {
            pop_bit(bitboards[piece], target);
            set_bit(bitboards[piece], source);
            mailbox[target] = (capture && undo.captured_piece > 0) ? (int8_t)undo.captured_piece : 0;
            mailbox[source] = (int8_t)piece;
        }

        if (ep) {
            if (nonMover == white) {
                set_bit(bitboards[Wp], target + 8);
                mailbox[target + 8] = Wp;
            } else {
                set_bit(bitboards[Bp], target - 8);
                mailbox[target - 8] = Bp;
            }
        }

        if (castling) {
            switch (target) {
                case g1:
                    pop_bit(bitboards[Wr], f1);
                    set_bit(bitboards[Wr], h1);
                    mailbox[f1] = 0;
                    mailbox[h1] = Wr;
                    break;
                case c1:
                    pop_bit(bitboards[Wr], d1);
                    set_bit(bitboards[Wr], a1);
                    mailbox[d1] = 0;
                    mailbox[a1] = Wr;
                    break;
                case g8:
                    pop_bit(bitboards[Br], f8);
                    set_bit(bitboards[Br], h8);
                    mailbox[f8] = 0;
                    mailbox[h8] = Br;
                    break;
                case c8:
                    pop_bit(bitboards[Br], d8);
                    set_bit(bitboards[Br], a8);
                    mailbox[d8] = 0;
                    mailbox[a8] = Br;
                    break;
            }
        }

        enpassant = undo.enpassant;
        castle = undo.castle;
        fifty = undo.fifty;

        //reset occupancies
        memset(occupancies, 0ULL, sizeof(occupancies));
        for(int p = Wp; p <= Wk; p++) occupancies[white] |= bitboards[p];
        for(int p = Bp; p <= Bk; p++) occupancies[black] |= bitboards[p];
        occupancies[both] = occupancies[white] | occupancies[black];
    }
}

int move::makeMove(Move move,int side){
    if(undo_index >= MAX_UNDO - 1) return 0;

    MoveUndo& undo = undo_history[undo_index];
    undo.enpassant = enpassant;
    undo.castle = castle;
    undo.captured_piece = -1;
    undo.fifty = fifty;
    undo.hashKey = hashKey;

    int source    = get_move_source(move);
    int target    = get_move_target(move);
    int piece     = get_move_piece(move);
    int promoted  = get_move_promoted(move);
    int capture   = get_move_capture(move);
    int doublePush= get_move_double(move);
    int ep        = get_move_enpassant(move);
    int castling  = get_move_castling(move);

    //capture handling
    if (capture) {
        undo.captured_piece = mailbox[target];

        if (undo.captured_piece > 0) {
            pop_bit(bitboards[undo.captured_piece], target);
            hashKey ^= pieceKeys[undo.captured_piece][target];
        }
    }

    //making the move
    pop_bit(bitboards[piece], source);
    set_bit(bitboards[piece], target);
    mailbox[source] = 0;
    mailbox[target] = (int8_t)piece;
    hashKey ^= pieceKeys[piece][source] ^ pieceKeys[piece][target];
    fifty++;

    if (piece == Wp || piece == Bp || capture) {
        fifty = 0;
    }

    //handling promotion of pawn
    if (promoted) {
        pop_bit(bitboards[piece], target);
        set_bit(bitboards[promoted], target);
        mailbox[target] = (int8_t)promoted;
        hashKey ^= pieceKeys[piece][target] ^ pieceKeys[promoted][target];
    }

    //handling enpassant
    if(ep){
        if(side == white){
            pop_bit(bitboards[Bp], target - 8);
            mailbox[target - 8] = 0;
            hashKey ^= pieceKeys[Bp][target - 8];
        }else{
            pop_bit(bitboards[Wp], target + 8);
            mailbox[target + 8] = 0;
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
                mailbox[h1] = 0;
                mailbox[f1] = Wr;
                hashKey ^= pieceKeys[Wr][h1] ^ pieceKeys[Wr][f1];
                break;
            case c1:
                pop_bit(bitboards[Wr], a1);
                set_bit(bitboards[Wr], d1);
                mailbox[a1] = 0;
                mailbox[d1] = Wr;
                hashKey ^= pieceKeys[Wr][a1] ^ pieceKeys[Wr][d1];
                break;
            case g8:
                pop_bit(bitboards[Br], h8);
                set_bit(bitboards[Br], f8);
                mailbox[h8] = 0;
                mailbox[f8] = Br;
                hashKey ^= pieceKeys[Br][h8] ^ pieceKeys[Br][f8];
                break;
            case c8:
                pop_bit(bitboards[Br], a8);
                set_bit(bitboards[Br], d8);
                mailbox[a8] = 0;
                mailbox[d8] = Br;
                hashKey ^= pieceKeys[Br][a8] ^ pieceKeys[Br][d8];
                break;
            default:
                break;
        }
    }

    hashKey ^= castleKeys[castle];
    castle &= castling_rights[source];
    castle &= castling_rights[target];
    hashKey ^= castleKeys[castle];

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
        unmakeBoardChanges(move, side ^ 1, undo_history[undo_index]);
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
    unmakeBoardChanges(move, side, undo_history[undo_index]);
    hashKey = undo_history[undo_index].hashKey;
}

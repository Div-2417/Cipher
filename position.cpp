//position.cpp

#include <iostream>
#include <string>
#include <sstream>

#include "defs.h"
#include "position.h"
#include "bitboard.h"

fen FENnotation;

int position::charToPiece(char c) {
    switch (c) {
        case 'P': return Wp;
        case 'N': return Wn;
        case 'B': return Wb;
        case 'R': return Wr;
        case 'Q': return Wq;
        case 'K': return Wk;
        case 'p': return Bp;
        case 'n': return Bn;
        case 'b': return Bb;
        case 'r': return Br;
        case 'q': return Bq;
        case 'k': return Bk;
        default: return 0;
    }
}

int position::algebraicToSquare(const std::string& s) {
    if (s.size() != 2) return -1;
    if (s[0] < 'a' || s[0] > 'h') return -1;
    if (s[1] < '1' || s[1] > '8') return -1;
    int file = s[0] - 'a';
    int rank = s[1] - '1';
    return rank * 8 + file;
}

void position::loadFEN(const std::string& fen){

    FENnotation ={};
    BitBoard::clearBoard();

    std::stringstream ss(fen);
    std::string placement, side, castling, ep;
    ss >> placement >> side >> castling >> ep >> FENnotation.halfmoveClock >> FENnotation.fullmoveNumber;

    int sq = 56;
    for (char c : placement) {
        if (c == '/') {
            sq -= 16;
        } else if (std::isdigit(static_cast<unsigned char>(c))) {
            sq += c - '0';
        } else {
            int piece = charToPiece(c);
            if (sq >= 0 && sq < 64 && piece >= 1 && piece < 13) {
                bitboards[piece] |= (1ULL << sq);
            }
            sq++;
        }
    }

    FENnotation.whiteToMove = (side == "w");

    // Set castling rights in both global variable and FENnotation struct
    FENnotation.castleWK = (castling.find('K') != std::string::npos);
    FENnotation.castleWQ = (castling.find('Q') != std::string::npos);
    FENnotation.castleBK = (castling.find('k') != std::string::npos);
    FENnotation.castleBQ = (castling.find('q') != std::string::npos);

    enpassant = (ep != "-") ? algebraicToSquare(ep) : no_sq;
    castle = 0;
    if (castling.find('K') != std::string::npos) castle |= wk;
    if (castling.find('Q') != std::string::npos) castle |= wq;
    if (castling.find('k') != std::string::npos) castle |= bk;
    if (castling.find('q') != std::string::npos) castle |= bq;

    if (ep != "-") {
        FENnotation.enPassantSquare = algebraicToSquare(ep);
    }else{
        FENnotation.enPassantSquare = -1;
    }


}

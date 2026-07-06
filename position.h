#ifndef position_H
#define position_H

#include <string>

#include "defs.h"

namespace position {
    int charToPiece(char c);
    int algebraicToSquare(const std::string& algebraic);
    void loadFEN(const std::string& fen);
}

extern fen FENnotation;

extern bitboard bitboards[13];

#endif
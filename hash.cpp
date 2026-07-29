#include <random>

#include "hash.h"
#include "bitboard.h"

uint64_t pieceKeys[13][64];
uint64_t sideKey;
uint64_t castleKeys[16];
uint64_t enpassantKeys[64];
uint64_t hashKey = 0;

void Zobrist::init(){
    std::mt19937_64 rng(20260727ULL); //fixed seed

    for(int p = Wp; p <= Bk; p++)
        for(int sq = 0; sq < 64; sq++)
            pieceKeys[p][sq] = rng();

    sideKey = rng();

    for(int i = 0; i < 16; i++) castleKeys[i] = rng();
    for(int sq = 0; sq < 64; sq++) enpassantKeys[sq] = rng();
}

uint64_t Zobrist::computeHash(int side){
    uint64_t h = 0;

    for(int p = Wp; p <= Bk; p++){
        bitboard bb = bitboards[p];
        while(bb){
            int sq = __builtin_ctzll(bb);
            h ^= pieceKeys[p][sq];
            bb &= bb - 1;
        }
    }

    if(enpassant != no_sq) h ^= enpassantKeys[enpassant];
    h ^= castleKeys[castle];
    if(side == black) h ^= sideKey;

    return h;
}
#include <iostream>

#include "defs.h"
#include "bitboard.h"

bitboard bitboards[13];

bitboard pawnAttack[2][64];
bitboard knightAttack[64];
bitboard kingAttack[64];
bitboard bishopMask[64];
bitboard rookMask[64];
bitboard bishopAttack[64][512];
bitboard rookAttack[64][4096];

bitboard Magic::bishopMagics[64] = {
    0x40040844404084ULL, 0x2004208a004208ULL, 0x10190041080202ULL, 0x108060845042010ULL,
    0x581104180800210ULL, 0x2112080446200010ULL, 0x1080820820060210ULL, 0x3c0808410220200ULL,
    0x4050404440404ULL, 0x21001420088ULL, 0x24d0080801082102ULL, 0x1020a0a020400ULL,
    0x40308200402ULL, 0x4011002100800ULL, 0x401484104104005ULL, 0x801010402020200ULL,
    0x400210c3880100ULL, 0x404022024108200ULL, 0x810018200204102ULL, 0x4002801a02003ULL,
    0x85040820080400ULL, 0x810102c808880400ULL, 0xe900410884800ULL, 0x8002020480840102ULL,
    0x220200865090201ULL, 0x2010100a02021202ULL, 0x152048408022401ULL, 0x20080002081110ULL,
    0x4001001021004000ULL, 0x800040400a011002ULL, 0xe4004081011002ULL, 0x1c004001012080ULL,
    0x8004200962a00220ULL, 0x8422100208500202ULL, 0x2000402200300c08ULL, 0x8646020080080080ULL,
    0x80020a0200100808ULL, 0x2010004880111000ULL, 0x623000a080011400ULL, 0x42008c0340209202ULL,
    0x209188240001000ULL, 0x400408a884001800ULL, 0x110400a6080400ULL, 0x1840060a44020800ULL,
    0x90080104000041ULL, 0x201011000808101ULL, 0x1a2208080504f080ULL, 0x8012020600211212ULL,
    0x500861011240000ULL, 0x180806108200800ULL, 0x4000020e01040044ULL, 0x300000261044000aULL,
    0x802241102020002ULL, 0x20906061210001ULL, 0x5a84841004010310ULL, 0x4010801011c04ULL,
    0xa010109502200ULL, 0x4a02012000ULL, 0x500201010098b028ULL, 0x8040002811040900ULL,
    0x28000010020204ULL, 0x6000020202d0240ULL, 0x8918844842082200ULL, 0x4010011029020020ULL
};

bitboard Magic::rookMagics[64] = {
    0x8a80104000800020ULL, 0x140002000100040ULL, 0x2801880a0017001ULL, 0x100081001000420ULL,
    0x200020010080420ULL, 0x3001c0002010008ULL, 0x8480008002000100ULL, 0x2080088004402900ULL,
    0x800098204000ULL, 0x2024401000200040ULL, 0x100802000801000ULL, 0x120800800801000ULL,
    0x208808088000400ULL, 0x2802200800400ULL, 0x2200800100020080ULL, 0x801000060821100ULL,
    0x80044006422000ULL, 0x100808020004000ULL, 0x12108a0010204200ULL, 0x140848010000802ULL,
    0x481828014002800ULL, 0x8094004002004100ULL, 0x4010040010010802ULL, 0x20008806104ULL,
    0x100400080208000ULL, 0x2040002120081000ULL, 0x21200680100081ULL, 0x20100080080080ULL,
    0x2000a00200410ULL, 0x20080800400ULL, 0x80088400100102ULL, 0x80004600042881ULL,
    0x4040008040800020ULL, 0x440003000200801ULL, 0x4200011004500ULL, 0x188020010100100ULL,
    0x14800401802800ULL, 0x2080040080800200ULL, 0x124080204001001ULL, 0x200046502000484ULL,
    0x480400080088020ULL, 0x1000422010034000ULL, 0x30200100110040ULL, 0x100021010009ULL,
    0x2002080100110004ULL, 0x202008004008002ULL, 0x20020004010100ULL, 0x2048440040820001ULL,
    0x101002200408200ULL, 0x40802000401080ULL, 0x4008142004410100ULL, 0x2060820c0120200ULL,
    0x1001004080100ULL, 0x20c020080040080ULL, 0x2935610830022400ULL, 0x44440041009200ULL,
    0x280001040802101ULL, 0x2100190040002085ULL, 0x80c0084100102001ULL, 0x4024081001000421ULL,
    0x20030a0244872ULL, 0x12001008414402ULL, 0x2006104900a0804ULL, 0x1004081002402ULL
};

int Magic::bishopBits[64] = {
    6,5,5,5,5,5,5,6,
    5,5,5,5,5,5,5,5,
    5,5,7,7,7,7,5,5,
    5,5,7,9,9,7,5,5,
    5,5,7,9,9,7,5,5,
    5,5,7,7,7,7,5,5,
    5,5,5,5,5,5,5,5,
    6,5,5,5,5,5,5,6
};

int Magic::rookBits[64] = {
    12,11,11,11,11,11,11,12,
    11,10,10,10,10,10,10,11,
    11,10,10,10,10,10,10,11,
    11,10,10,10,10,10,10,11,
    11,10,10,10,10,10,10,11,
    11,10,10,10,10,10,10,11,
    11,10,10,10,10,10,10,11,
    12,11,11,11,11,11,11,12
};

void BitBoard::printBoard(bitboard board) {
    std::cout << "\n";

    for (int rank = 7; rank >= 0; rank--){
        for (int file = 0; file < 8; file++){

            int square = rank * 8 + file;
            if (!file)
                printf("  %d ", rank +1);
           printf(" %d", get_bit(board, square) ? 1 : 0); 

        }
        std::cout << "\n";
    }
    
    printf("\n     a b c d e f g h\n\n");
    printf("     Bitboard: %lu\n\n", board);
}

void BitBoard::printWholeBoard(){
    const char* pieceChars = " PNBRQKpnbrqk";

    std::cout<<"\n";

    for(int rank =7; rank >=0;rank--){

        std::cout<< rank+1 << "  ";

        for(int file=0;file <8 ;file++){

            int square =rank*8 + file;
            char c{'.'};

            for(int piece=1;piece<13;piece++){

                if(get_bit(bitboards[piece],square)){
                    c = pieceChars[piece];
                    break;
                }
            }
            std::cout << c << " ";
        }
        std::cout << "\n";
    }

    std::cout << "\n   a b c d e f g h\n\n";
}

void BitBoard::clearBoard() {
    for (int i=0;i<13;i++) bitboards[i]=0ULL;
}

void BitBoard::init() {
    
}


bitboard Magic::ComputePawnAttack(int side, int square){
    bitboard attacks = 0ULL;
    bitboard pieceBB = 0ULL;

    set_bit(pieceBB, square);

    if(side == white){
        //7 for left attack, 9 for right attack
        if(pieceBB >> 7 & ~FileHBB) attacks |= (pieceBB >> 7);
        if(pieceBB >> 9 & ~FileABB) attacks |= (pieceBB >> 9);
    } else {
        if(pieceBB << 7 & ~FileABB) attacks |= (pieceBB << 7);
        if(pieceBB << 9 & ~FileHBB) attacks |= (pieceBB << 9);
    }

    return attacks;
}

bitboard Magic::ComputeKnightAttack(int square){
    bitboard attacks = 0ULL;
    bitboard pieceBB = 0ULL;

    set_bit(pieceBB, square);

    //understand the wrap around condition later
    if(pieceBB >> 17 & ~FileABB) attacks |= (pieceBB >> 17);
    if(pieceBB >> 15 & ~FileHBB) attacks |= (pieceBB >> 15);
    if(pieceBB >> 10 & ~FileABB & ~FileBBB) attacks |= (pieceBB >> 10);
    if(pieceBB >> 6 & ~FileHBB & ~FileHBB) attacks |= (pieceBB >> 6);
    if(pieceBB << 6 & ~FileABB & ~FileBBB) attacks |= (pieceBB << 6);
    if(pieceBB << 10 & ~FileHBB & ~FileHBB) attacks |= (pieceBB << 10);
    if(pieceBB << 15 & ~FileABB) attacks |= (pieceBB << 15);
    if(pieceBB << 17 & ~FileHBB) attacks |= (pieceBB << 17);

    return attacks;
}

bitboard Magic::ComputeKingAttack(int square){
    bitboard attacks = 0ULL;
    bitboard pieceBB = 0ULL;

    set_bit(pieceBB, square);

    if(pieceBB >> 9 & ~FileABB) attacks |= (pieceBB >> 9);
    if(pieceBB >> 8) attacks |= (pieceBB >> 8);
    if(pieceBB >> 7 & ~FileHBB) attacks |= (pieceBB >> 7);
    if(pieceBB >> 1 & ~FileABB) attacks |= (pieceBB >> 1);
    if(pieceBB << 1 & ~FileHBB) attacks |= (pieceBB << 1);
    if(pieceBB << 7 & ~FileABB) attacks |= (pieceBB << 7);
    if(pieceBB << 8) attacks |= (pieceBB << 8);
    if(pieceBB << 9 & ~FileHBB) attacks |= (pieceBB << 9);

    return attacks;
}

bitboard Magic::computeBishopMask(int square){
    bitboard attacks =0ULL;
    int rank,file;
    
    //target rank ad target file
    int tr = square / 8;
    int tf = square % 8;

    //masking relevat bits, stops before the edge of the board
    for(rank = tr + 1, file = tf + 1; rank <= 6 && file <= 6;rank++,file++) attacks|= (1ULL << (rank * 8 + file));
    for (rank = tr - 1, file = tf + 1; rank >= 1 && file <= 6; rank--, file++) attacks |= (1ULL << (rank * 8 + file));
    for (rank = tr + 1, file = tf - 1; rank <= 6 && file >= 1; rank++, file--) attacks |= (1ULL << (rank * 8 + file));
    for (rank = tr - 1, file = tf - 1; rank >= 1 && file >= 1; rank--, file--) attacks |= (1ULL << (rank * 8 + file));

    return attacks;
}

bitboard Magic::computeRookMask(int square){
    bitboard attacks = 0ULL;
    int rank, file;

    int tr = square / 8;
    int tf = square % 8;

    for (rank = tr + 1; rank <= 6; rank++) attacks |= (1ULL << (rank * 8 + tf));
    for (rank = tr - 1; rank >= 1; rank--) attacks |= (1ULL << (rank * 8 + tf));
    for (file = tf + 1; file <= 6; file++) attacks |= (1ULL << (tr * 8 + file));
    for (file = tf - 1; file >= 1; file--) attacks |= (1ULL << (tr * 8 + file));

    return attacks;
}

bitboard Magic::computeBishopAttackOTF(int square, bitboard blockers){
    bitboard attacks = 0ULL;
    int rank, file;

    int tr = square / 8;
    int tf = square % 8;

    //generate bishop attacks on the fly, stop when blockers are hit
    for (rank = tr + 1, file = tf + 1; rank <= 7 && file <= 7; rank++, file++){
        attacks |= (1ULL << (rank * 8 + file));
        if ((1ULL << (rank * 8 + file)) & blockers) break;
    }
    
    for (rank = tr - 1, file = tf + 1; rank >= 0 && file <= 7; rank--, file++){
        attacks |= (1ULL << (rank * 8 + file));
        if ((1ULL << (rank * 8 + file)) & blockers) break;
    }
    
    for (rank = tr + 1, file = tf - 1; rank <= 7 && file >= 0; rank++, file--){
        attacks |= (1ULL << (rank * 8 + file));
        if ((1ULL << (rank * 8 + file)) & blockers) break;
    }
    
    for (rank = tr - 1, file = tf - 1; rank >= 0 && file >= 0; rank--, file--){
        attacks |= (1ULL << (rank * 8 + file));
        if ((1ULL << (rank * 8 + file)) & blockers) break;
    }

    return attacks;
}

bitboard Magic::computeRookAttackOTF(int square, bitboard blockers){
    bitboard attacks = 0ULL;
    int rank, file;

    int tr = square / 8;
    int tf = square % 8;

    for (rank = tr + 1; rank <= 7; rank++){
        attacks |= (1ULL << (rank * 8 + tf));
        if ((1ULL << (rank * 8 + tf)) & blockers) break;
    }
    
    for (rank = tr - 1; rank >= 0; rank--){
        attacks |= (1ULL << (rank * 8 + tf));
        if ((1ULL << (rank * 8 + tf)) & blockers) break;
    }
    
    for (file = tf + 1; file <= 7; file++){
        attacks |= (1ULL << (tr * 8 + file));
        if ((1ULL << (tr * 8 + file)) & blockers) break;
    }
    
    for (file = tf - 1; file >= 0; file--){
        attacks |= (1ULL << (tr * 8 + file));
        if ((1ULL << (tr * 8 + file)) & blockers) break;
    }

    return attacks;
}

bitboard Magic::getBishopAttacks(int square, bitboard occupancy){
    occupancy &= bishopMask[square];
    occupancy *= Magic::bishopMagics[square];
    occupancy >>= 64 - Magic::bishopBits[square];
    
    return bishopAttack[square][occupancy];
}

bitboard Magic::getRookAttacks(int square, bitboard occupancy){
    occupancy &= rookMask[square];
    occupancy *= Magic::rookMagics[square];
    occupancy >>= 64 - Magic::rookBits[square];

    return rookAttack[square][occupancy];
}

bitboard Magic::ComputeQueenAttack(int square, bitboard blockers){
    bitboard bishopOccupancy = blockers & bishopMask[square];
    bishopOccupancy *= Magic::bishopMagics[square];
    bishopOccupancy >>= 64 - Magic::bishopBits[square];

    bitboard rookOccupancy = blockers & rookMask[square];
    rookOccupancy *= Magic::rookMagics[square];
    rookOccupancy >>= 64 - Magic::rookBits[square];

    return bishopAttack[square][bishopOccupancy] | rookAttack[square][rookOccupancy];
}

bitboard Magic::setOccupancy(int index, int bitsInMask, bitboard mask){
    bitboard occupancy = 0ULL;

    for(int count = 0; count < bitsInMask; count++){
        int square = __builtin_ctzll(mask); //count trailing zeros
        pop_bit(mask, square);

        //include in occupancy if ith bit of index is set
        if(index & (1 << count)) set_bit(occupancy, square);
    }

    return occupancy;
}

void Magic::UpdateOccupancy(){
    bitboard occupancies[3] = {0ULL, 0ULL, 0ULL};

    for(int piece = Wp; piece <= Wk; piece++){
        occupancies[white] |= bitboards[piece];
    }
    for(int piece = Bp; piece <= Bk; piece++){
        occupancies[black] |= bitboards[piece];
    }

    occupancies[both] = occupancies[white] | occupancies[black];
}

void Magic::initLeaperAttacks(){

    for (int sq{0}; sq < 64; sq++) {
        //pawns
        pawnAttack[white][sq] = Magic::ComputePawnAttack(white, sq);
        pawnAttack[black][sq] = Magic::ComputePawnAttack(black, sq);

        //knights
        knightAttack[sq] = Magic::ComputeKnightAttack(sq);

        //king
        kingAttack[sq] = Magic::ComputeKingAttack(sq);
    }
}

void Magic::initSliderAttacks(){
    for (int sq{0}; sq<64; sq++){
        //bishop
        bishopMask[sq] = Magic::computeBishopMask(sq);
        int bits = bishopBits[sq];
        int occupancyIndices = 1 <<bits; // 2^bits

        for(int index{0}; index < occupancyIndices; index++){
            bitboard occupancy = Magic::setOccupancy(index, bits, bishopMask[sq]);
            unsigned int magicIndex = (unsigned int)((occupancy * bishopMagics[sq]) >> (64 - bits));
            bishopAttack[sq][magicIndex] = Magic::computeBishopAttackOTF(sq, occupancy);
        }

        //rook
        rookMask[sq] = Magic::computeRookMask(sq);
        bits = rookBits[sq];
        occupancyIndices = 1<<bits;

        for(int index{0}; index < occupancyIndices; index++){
            bitboard occupancy = Magic::setOccupancy(index, bits, rookMask[sq]);
            unsigned int magicIndex = (unsigned int)((occupancy * rookMagics[sq]) >> (64 - bits));
            rookAttack[sq][magicIndex] = Magic::computeRookAttackOTF(sq, occupancy);
        }
    }
}
    
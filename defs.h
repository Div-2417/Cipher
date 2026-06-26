//defs.h

#ifndef DEFS_H
#define DEFS_H

#include <cassert>
#include <cstddef>
#include <cstdint>

using bitboard = uint64_t;

constexpr int Wp=1, Wn=2,Wb=3,Wr=4,Wq=5,Wk=6;
constexpr int Bp=7,Bn=8,Bb=9,Br=10,Bq=11,Bk=12;

const int Empty = 0;
constexpr int WHITE = 0;
constexpr int BLACK = 1;

#endif
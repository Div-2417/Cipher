#ifndef TIME_H
#define TIME_H

#include <chrono>
#include <atomic>

using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<Clock>;

extern std::atomic<bool> stopSearch;
extern TimePoint searchStart;
extern long long allocatedMs;

namespace Time{
    //white remaning time, black remaining time, white increment, black increment, moves to go, move time, side to move
    //send by GUI
    void init(int wtime, int btime, int winc, int binc,int movestogo, int movetime, int side);
    bool shouldStop();

}

#endif
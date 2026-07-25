#include "defs.h"
#include "time.h"

std::atomic<bool> stopSearch{false};
TimePoint searchStart{};
long long allocatedMs = 0;

void Time::init(int wtime, int btime, int winc, int binc,int movestogo, int movetime, int side){
    stopSearch.store(false);
    searchStart = Clock::now();
    allocatedMs = 0;

    if (movetime > 0) {
        allocatedMs = movetime;
    } else if (wtime >= 0 && btime >= 0) {
        long long timeLeft = (side == white) ? wtime : btime;
        long long inc = (side == white) ? winc : binc;

        if (movestogo > 0) {
            allocatedMs = timeLeft / movestogo + inc - 50; // Subtracting 50ms as a buffer
        } else {
            allocatedMs = timeLeft / 30 + inc - 50; // Default to 30 moves if movestogo is not provided
        }
    } else {
        allocatedMs = 1000000;
    }
}

bool Time::shouldStop() {
    if (stopSearch.load()) return true;

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        Clock::now() - searchStart).count();

    if (elapsed >= allocatedMs) {
        stopSearch.store(true);
        return true;
    }
    
    return false;
}

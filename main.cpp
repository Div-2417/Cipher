#include <iostream>
#include <vector>
#include <string>
#include <chrono>

#include "defs.h"
#include "bitboard.h"
#include "position.h"
#include "perft.h"

struct PerftTest {
    std::string fen;
    int depth;
    long long expectedNodes;
};

int side = FENnotation.whiteToMove ? white : black;

static void syncStateFromFEN()
{
    enpassant = (FENnotation.enPassantSquare == -1)
        ? no_sq
        : FENnotation.enPassantSquare;

    castle = 0;
    if (FENnotation.castleWK) castle |= wk;
    if (FENnotation.castleWQ) castle |= wq;
    if (FENnotation.castleBK) castle |= bk;
    if (FENnotation.castleBQ) castle |= bq;

    occupancies[white] = occupancies[black] = occupancies[both] = 0ULL;
    for (int piece = Wp; piece <= Wk; piece++) occupancies[white] |= bitboards[piece];
    for (int piece = Bp; piece <= Bk; piece++) occupancies[black] |= bitboards[piece];
    occupancies[both] = occupancies[white] | occupancies[black];
}

int main()
{
    BitBoard::init();

    std::vector<PerftTest> tests = {
        {"r6r/1b2k1bq/8/8/7B/8/8/R3K2R b KQ - 3 2", 1, 8},
        {"8/8/8/2k5/2pP4/8/B7/4K3 b - d3 0 3", 1, 8},
        {"r1bqkbnr/pppppppp/n7/8/8/P7/1PPPPPPP/RNBQKBNR w KQkq - 2 2", 1, 19},
        {"r3k2r/p1pp1pb1/bn2Qnp1/2qPN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQkq - 3 2", 1, 5},
        {"2kr3r/p1ppqpb1/bn2Qnp1/3PN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQ - 3 2", 1, 44},
        {"rnb2k1r/pp1Pbppp/2p5/q7/2B5/8/PPPQNnPP/RNB1K2R w KQ - 3 9", 1, 39},
        {"2r5/3pk3/8/2P5/8/2K5/8/8 w - - 5 4", 1, 9},
        {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 3, 62379},
        {"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 3, 89890},
        {"3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1", 6, 1134888},
        {"8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1", 6, 1015133},
        {"8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1", 6, 1440467},
        {"5k2/8/8/8/8/8/8/4K2R w K - 0 1", 6, 661072},
        {"3k4/8/8/8/8/8/8/R3K3 w Q - 0 1", 6, 803711},
        {"r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1", 4, 1274206},
        {"r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1", 4, 1720476},
        {"2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1", 6, 3821001},
        {"8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1", 5, 1004658},
        {"4k3/1P6/8/8/8/8/K7/8 w - - 0 1", 6, 217342},
        {"8/P1k5/K7/8/8/8/8/8 w - - 0 1", 6, 92683},
        {"K1k5/8/P7/8/8/8/8/8 w - - 0 1", 6, 2217},
        {"8/k1P5/8/1K6/8/8/8/8 w - - 0 1", 7, 567584},
        {"8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1", 4, 23527}
    };

    int passed = 0;

    for (size_t i = 0; i < tests.size(); i++) {
        const auto& t = tests[i];

        position::loadFEN(t.fen);
        syncStateFromFEN();

        int side = FENnotation.whiteToMove ? white : black;

        auto start = std::chrono::high_resolution_clock::now();
        long long actualNodes = perft::perft(t.depth, side);
        auto end = std::chrono::high_resolution_clock::now();

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        bool ok = (actualNodes == t.expectedNodes);

        std::cout << "Test " << (i + 1) << '\n';
        std::cout << "FEN:      " << t.fen << '\n';
        std::cout << "Depth:    " << t.depth << '\n';
        std::cout << "Expected: " << t.expectedNodes << '\n';
        std::cout << "Actual:   " << actualNodes << '\n';
        std::cout << "Time:     " << ms << " ms\n";
        std::cout << "Result:   " << (ok ? "PASS" : "FAIL") << "\n\n";

        if (ok) passed++;
    }

    std::cout << "Passed " << passed << " / " << tests.size() << " tests\n";

    return 0;
}
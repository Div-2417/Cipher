#include <iostream>
#include <vector>
#include <string>
#include <chrono>

#include "defs.h"
#include "bitboard.h"
#include "position.h"
#include "search.h"
#include "perft.h"

struct SearchTest {
    std::string name;
    std::string fen;
    int depth;
    std::string expectedMove; // e.g. "e2e4", or "" if just want to observe
};

static void syncStateFromFEN()
{
    enpassant = (FENnotation.enPassantSquare == -1) ? no_sq : FENnotation.enPassantSquare;

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

    const int TEST_DEPTH = 5;

    std::vector<SearchTest> tests = {
        // Tactical / forced — real expected moves, hard pass/fail
        {"Castling rights",             "r6r/1b2k1bq/8/8/7B/8/8/R3K2R b KQ - 3 2", TEST_DEPTH, ""},
        {"En passant",                  "8/8/8/2k5/2pP4/8/B7/4K3 b - d3 0 3", TEST_DEPTH, "c4d3"},
        {"Knight development",          "r1bqkbnr/pppppppp/n7/8/8/P7/1PPPPPPP/RNBQKBNR w KQkq - 2 2", TEST_DEPTH, ""},
        {"Castling & pinned pieces",    "r3k2r/p1pp1pb1/bn2Qnp1/2qPN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQkq - 3 2", TEST_DEPTH, ""},
        {"Castling after rook move",    "2kr3r/p1ppqpb1/bn2Qnp1/3PN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQ - 3 2", TEST_DEPTH, ""},
        {"Pinned bishop",               "rnb2k1r/pp1Pbppp/2p5/q7/2B5/8/PPPQNnPP/RNB1K2R w KQ - 3 9", TEST_DEPTH, "d7c8q"},
        {"Simple king vs rook endgame", "2r5/3pk3/8/2P5/8/2K5/8/8 w - - 5 4", TEST_DEPTH, ""},

        {"Kiwipete",                    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", TEST_DEPTH, "d7c8q"},
        {"Kiwipete (complex)",          "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", TEST_DEPTH, ""},

        {"Rook vs pawn race",           "3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1", TEST_DEPTH, ""},
        {"Bishop + pawn ending",        "8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1", TEST_DEPTH, ""},
        {"En passant discovered check", "8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1", TEST_DEPTH, "c4d3"},
        {"Kingside castling only",      "5k2/8/8/8/8/8/8/4K2R w K - 0 1", TEST_DEPTH, ""},
        {"Queenside castling only",     "3k4/8/8/8/8/8/8/R3K3 w Q - 0 1", TEST_DEPTH, ""},

        {"All castling rights",         "r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1", TEST_DEPTH, ""},
        {"Castling through check",      "r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1", TEST_DEPTH, ""},

        {"Promotion with check",        "2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1", TEST_DEPTH, "e7f8q"},
        {"Promotion vs knight",         "8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1", TEST_DEPTH, ""},
        {"Single promotion",            "4k3/1P6/8/8/8/8/K7/8 w - - 0 1", TEST_DEPTH, "b7b8q"},
        {"Promotion race",              "8/P1k5/K7/8/8/8/8/8 w - - 0 1", TEST_DEPTH, "a7a8q"},
        {"Immediate promotion",         "K1k5/8/P7/8/8/8/8/8 w - - 0 1", TEST_DEPTH, ""},
        {"Underpromotion race",         "8/k1P5/8/1K6/8/8/8/8 w - - 0 1", TEST_DEPTH, ""},

        {"Check evasions",              "8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1", TEST_DEPTH, ""}
    };

    int passed = 0, checkedTotal = 0;

    for (auto& t : tests) {
        std::cout << "=== " << t.name << " ===\n";
        std::cout << "FEN: " << t.fen << "\n";

        position::loadFEN(t.fen);
        syncStateFromFEN();

        int side = FENnotation.whiteToMove ? white : black;

        auto start = std::chrono::high_resolution_clock::now();
        Move best = search::searchPosition(t.depth, side);
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::string bestStr = perft::moveToString(best);
        std::cout << "Bestmove: " << bestStr << "  (" << ms << " ms)\n";

        if (!t.expectedMove.empty()) {
            checkedTotal++;
            bool ok = (bestStr.substr(0, t.expectedMove.size()) == t.expectedMove);
            std::cout << "Result: " << (ok ? "PASS" : "FAIL")
                       << " (expected " << t.expectedMove << ")\n";
            if (ok) passed++;
        }
        std::cout << "\n";
    }

    std::cout << "Passed " << passed << " / " << checkedTotal << " checked tests"
               << " (" << (tests.size() - checkedTotal) << " observe-only)\n";

    return 0;
}
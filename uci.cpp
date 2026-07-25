#include <atomic>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "bitboard.h"
#include "defs.h"
#include "move.h"
#include "movegen.h"
#include "perft.h"
#include "position.h"
#include "search.h"
#include "time.h"
#include "uci.h"

namespace {
    const std::string kStartFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    std::thread searchThread;
    std::atomic<bool> searchRunning{false};
    std::atomic<bool> quitRequested{false};
    int currentSide = white;

    void setPositionFromFen(const std::string& fen)
    {
        position::loadFEN(fen);
        currentSide = FENnotation.whiteToMove ? white : black;
    }

    bool applyMoveString(const std::string& moveStr)
    {
        MoveList moves;
        moveGen::generateAllMoves(currentSide, moves);

        for (int i = 0; i < moves.count; ++i) {
            if (perft::moveToString(moves.moves[i]) == moveStr) {
                if (!move::makeMove(moves.moves[i], currentSide)) {
                    return false;
                }
                currentSide ^= 1;
                return true;
            }
        }

        return false;
    }

    void stopSearchThread()
    {
        stopSearch.store(true);
        if (searchThread.joinable()) {
            searchThread.join();
        }
        searchRunning.store(false);
    }

    std::string trim(const std::string& input)
    {
        std::string result = input;
        const auto begin = result.find_first_not_of(" \t\r\n");
        if (begin == std::string::npos) {
            return "";
        }
        const auto end = result.find_last_not_of(" \t\r\n");
        return result.substr(begin, end - begin + 1);
    }

    void handlePosition(const std::string& line)
    {
        std::istringstream stream(line);
        std::string token;
        std::vector<std::string> parts;
        while (stream >> token) {
            parts.push_back(token);
        }

        if (parts.size() < 2 || parts[0] != "position") {
            return;
        }

        std::size_t index = 1;
        if (index < parts.size() && parts[index] == "startpos") {
            setPositionFromFen(kStartFen);
            ++index;
        } else if (index < parts.size() && parts[index] == "fen") {
            ++index;
            std::string fen;
            while (index < parts.size() && parts[index] != "moves") {
                if (!fen.empty()) {
                    fen += " ";
                }
                fen += parts[index];
                ++index;
            }
            if (!fen.empty()) {
                setPositionFromFen(fen);
            }
        }

        if (index < parts.size() && parts[index] == "moves") {
            ++index;
            while (index < parts.size()) {
                if (!applyMoveString(parts[index])) {
                    break;
                }
                ++index;
            }
        }
    }

    void handleGo(const std::string& line)
    {
        if (searchThread.joinable()) {
            stopSearchThread();
        }

        int depth = -1;
        int movetime = -1;
        int wtime = -1;
        int btime = -1;
        int winc = 0;
        int binc = 0;
        int movestogo = 0;
        bool infinite = false;

        std::istringstream stream(line);
        std::string token;
        while (stream >> token) {
            if (token == "depth") {
                stream >> depth;
            } else if (token == "movetime") {
                stream >> movetime;
            } else if (token == "wtime") {
                stream >> wtime;
            } else if (token == "btime") {
                stream >> btime;
            } else if (token == "winc") {
                stream >> winc;
            } else if (token == "binc") {
                stream >> binc;
            } else if (token == "movestogo") {
                stream >> movestogo;
            } else if (token == "infinite") {
                infinite = true;
            }
        }

        if (depth < 1) {
            depth = 64;
        }

        searchRunning.store(true);
        searchThread = std::thread([=]() {
            Time::init(wtime, btime, winc, binc, movestogo, movetime, currentSide);
            Move bestMove = search::searchPosition(depth, currentSide);
            if (!Time::shouldStop() && bestMove) {
                std::cout << "bestmove " << perft::moveToString(bestMove) << std::endl;
            } else if (bestMove) {
                std::cout << "bestmove " << perft::moveToString(bestMove) << std::endl;
            }
            searchRunning.store(false);
        });
    }

    void handleSetOption(const std::string& line)
    {
        std::istringstream stream(line);
        std::string token;
        std::vector<std::string> parts;
        while (stream >> token) {
            parts.push_back(token);
        }

        if (parts.size() >= 2 && parts[0] == "setoption") {
            // The engine accepts the option silently and keeps defaults.
            (void)parts;
        }
    }
}

namespace UCI {
    void init()
    {
        BitBoard::init();
        setPositionFromFen(kStartFen);
    }

    void loop()
    {
        std::string line;
        while (std::getline(std::cin, line)) {
            line = trim(line);
            if (line.empty()) {
                continue;
            }

            if (line == "uci") {
                std::cout << "id name Gotya 2.0" << std::endl;
                std::cout << "id author Div2417" << std::endl;
                std::cout << "option name Hash type spin default 16 min 1 max 1024" << std::endl;
                std::cout << "option name Ponder type check default false" << std::endl;
                std::cout << "uciok" << std::endl;
            } else if (line == "isready") {
                std::cout << "readyok" << std::endl;
            } else if (line == "ucinewgame") {
                setPositionFromFen(kStartFen);
            } else if (line.rfind("position", 0) == 0) {
                handlePosition(line);
            } else if (line.rfind("setoption", 0) == 0) {
                handleSetOption(line);
            } else if (line.rfind("go", 0) == 0) {
                handleGo(line);
            } else if (line == "stop") {
                stopSearchThread();
            } else if (line == "quit") {
                stopSearchThread();
                quitRequested.store(true);
                break;
            } else if (line == "register") {
                // Not required by this engine yet.
            }
        }
    }
}

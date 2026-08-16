#include <atomic>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <unistd.h>
#include <limits.h>

#include "bitboard.h"
#include "defs.h"
#include "move.h"
#include "movegen.h"
#include "perft.h"
#include "position.h"
#include "search.h"
#include "time.h"
#include "uci.h"
#include "hash.h"
#include "TT.h"
#include "nnueEval.h"

namespace {
    const std::string kStartFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    std::thread searchThread;
    std::atomic<bool> searchRunning{false};
    std::atomic<bool> quitRequested{false};
    int currentSide = white;
    int hashSizeMB = 128;
    bool ponderEnabled = false;

    std::string defaultEvalPath(){
        char buf[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
        if (len != -1) {
            buf[len] = '\0';
            std::string exePath(buf);
            std::string::size_type pos = exePath.rfind('/');
            if (pos != std::string::npos)
                return exePath.substr(0, pos) + "/src/nn-62ef826d1a6d.nnue";
        }
        return "src/nn-62ef826d1a6d.nnue";
    }

    std::string evalFileName = defaultEvalPath();

    void setPositionFromFen(const std::string& fen){
        position::loadFEN(fen);
        currentSide = FENnotation.whiteToMove ? white : black;
    }

    bool applyMoveString(const std::string& moveStr){
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

    void stopSearchThread(){
        stopSearch.store(true);
        if (searchThread.joinable()) {
            searchThread.join();
        }
        searchRunning.store(false);
    }

    std::string trim(const std::string& input){
        std::string result = input;
        const auto begin = result.find_first_not_of(" \t\r\n");
        if (begin == std::string::npos) {
            return "";
        }
        const auto end = result.find_last_not_of(" \t\r\n");
        return result.substr(begin, end - begin + 1);
    }

    void handlePosition(const std::string& line){
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

    void handleGo(const std::string& line){
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
                // parsed and ignored: search until "stop" is already the
                // default behaviour when no other limit applies
            }
        }

        if (depth < 1) {
            depth = 64;
        }

        searchRunning.store(true);
        searchThread = std::thread([=]() {
            Time::init(wtime, btime, winc, binc, movestogo, movetime, currentSide);
            Move bestMove = search::searchPosition(depth, currentSide);
            if (bestMove) {
                std::cout << "bestmove " << perft::moveToString(bestMove);
                if (ponderEnabled) {
                    Move ponder = search::getPonderMove();
                    if (ponder) {
                        std::cout << " ponder " << perft::moveToString(ponder);
                    }
                }
                std::cout << std::endl;
            } else {
                // No legal move at the root (checkmate or stalemate): UCI still
                // expects a reply, "0000" is the standard null-move convention.
                std::cout << "bestmove 0000" << std::endl;
            }
            searchRunning.store(false);
        });
    }

    void handleSetOption(const std::string& line){
        std::istringstream stream(line);
        std::string token;
        stream >> token; // "setoption"
        stream >> token; // "name"

        std::string name;
        while (stream >> token && token != "value") {
            if (!name.empty()) name += " ";
            name += token;
        }

        if (token == "value" && !name.empty()) {
            std::string value;
            if (stream >> value) {
                if (name == "Hash") {
                    int newHash = std::stoi(value);
                    if (newHash >= 1 && newHash <= 1024) {
                        hashSizeMB = newHash;
                        TT::init(hashSizeMB);
                    }
                } else if (name == "Ponder") {
                    ponderEnabled = (value == "true");
                } else if (name == "EvalFile") {
                    evalFileName = value;
                    nnue::init_nnue(&evalFileName[0]);
                }
            }
        }
    }
}

namespace UCI {

    void init(){
        BitBoard::init();
        Zobrist::init();
        TT::init(hashSizeMB);
        setPositionFromFen(kStartFen);
        nnue::init_nnue(&evalFileName[0]);
    }

    void loop(){
        std::string line;
        while (std::getline(std::cin, line)) {
            line = trim(line);
            if (line.empty()) {
                continue;
            }

            if (line == "uci") {
                std::cout << "id name Cipher" << std::endl;
                std::cout << "id author Div2417" << std::endl;
                std::cout << "option name Hash type spin default 128 min 1 max 1024" << std::endl;
                std::cout << "option name Ponder type check default false" << std::endl;
                std::cout << "option name EvalFile type string default " << evalFileName << std::endl;
                std::cout << "uciok" << std::endl;
            } else if (line == "isready") {
                std::cout << "readyok" << std::endl;
            } else if (line == "ucinewgame") {
                search::resetSearchState();
                setPositionFromFen(kStartFen);
            } else if (line.rfind("position", 0) == 0) {
                handlePosition(line);
            } else if (line.rfind("setoption", 0) == 0) {
                handleSetOption(line);
            } else if (line.rfind("go", 0) == 0) {
                handleGo(line);
            } else if (line == "ponderhit") {
                // The opponent played the expected ponder move.
                // The ongoing search becomes real; no special action needed.
            } else if (line == "stop") {
                stopSearchThread();
            } else if (line == "quit") {
                stopSearchThread();
                quitRequested.store(true);
                break;
            } else if (line == "register") {
                // Not required by this engine yet.
            } else if (line.rfind("perft", 0) == 0) {
                std::istringstream stream(line);
                std::string token;
                int depth = 0;
                stream >> token >> depth;
                if (depth > 0) {
                    perft::divide(depth, currentSide);
                }
            }
        }
    }
}
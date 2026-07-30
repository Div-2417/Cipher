# Cipher Chess Engine

Cipher is a UCI-compatible chess engine written in C++17, built from scratch and inspired by [BBC (Bit Board Chess)](https://github.com/maksimKorzh/bbc) and [VICE](https://github.com/bluefeversoft/vice_chess_engine).

---

## Features

### Board Representation
- **Bitboard-based** board representation using 64-bit integers for all 12 piece types
- **Magic bitboards** for fast sliding piece (rook, bishop, queen) attack generation
- **Zobrist hashing** for efficient position identification and repetition detection
- FEN string parsing and position setup

### Move Generation
- Full pseudo-legal move generation for all piece types
- Legal move validation via make/unmake move
- Separate capture-only generator for quiescence search
- En passant, castling, and promotion support

### Search
- **Negamax** with **alpha-beta pruning**
- **Iterative deepening** with aspiration windows
- **Quiescence search** to resolve capture sequences at leaf nodes
- **Null-move pruning** (R=2/3 adaptive reduction)
- **Late Move Reduction (LMR)** for quiet moves searched late
- **Transposition Table (TT)** with exact/alpha/beta flag storage
- **Killer move heuristic** (2 killer slots per ply)
- **History heuristic** with depth² bonus on cutoffs
- **Static Exchange Evaluation (SEE)** for capture ordering and pruning
- Mate distance detection

### Evaluation
- Material scoring
- **Piece-Square Tables (PST)** for all piece types
- **Tapered king evaluation** (middlegame/endgame interpolation based on game phase)
- **Pawn structure**: doubled pawn penalty, isolated pawn penalty, passed pawn bonus (rank-scaled)
- **Mobility scoring** for knights, bishops, rooks, and queens

### Protocol
 - Full **UCI (Universal Chess Interface)** implementation
 - `info` lines with depth, score, nodes, time, nps, and PV
 - Time management with hard/soft limits
 - `setoption` for Hash size (1–1024 MB) and Ponder toggle

### Testing
 - **Perft** function for move generation correctness verification (accessible via `perft <depth>` UCI command)

---

## Build

### Requirements
- `g++` with C++17 support
- `make`

### Compile & Run

```bash
# Build the engine
make build

# Build and run
make run

# Clean build artifacts
make clean

# Show help
make help
```

The binary will be output as `cipher`.

---

## Usage

Cipher communicates via UCI. You can connect it to any UCI-compatible chess GUI (e.g., [Arena](http://www.playwitharena.de/), [Cute Chess](https://cutechess.com/), [Lucas Chess](https://lucaschess.pythonanywhere.com/)) or use it from the terminal:

### Interactive session examples

**Basic search:**
```bash
./cipher
uci
isready
position startpos
go depth 10
```

**Set Hash size and search with time control:**
```
setoption name Hash value 128
position startpos
go wtime 300000 btime 300000 winc 2000 binc 2000
```

**Test move generation with Perft:**
```
position startpos
perft 5
```

**Ponder mode (when supported by GUI):**
```
setoption name Ponder value true
position startpos
go depth 12
# engine outputs: bestmove e2e4 ponder e7e5
```

### UCI Command Reference

| Command | Description |
|---------|-------------|
| `uci` | Handshake — engine responds with id, options, and `uciok` |
| `isready` | Engine responds `readyok` when ready |
| `ucinewgame` | Reset search state, TT, and position to startpos |
| `position startpos [moves <m1> <m2> ...]` | Set starting position with optional move list |
| `position fen <fen> [moves <m1> ...]` | Set position from FEN string |
| `go depth <d>` | Search to depth `d` |
| `go movetime <t>` | Search for `t` milliseconds |
| `go wtime <t> btime <t> [winc <t>] [binc <t>] [movestogo <n>]` | Search with time management |
| `go infinite` | Search indefinitely (use `stop` to halt) |
| `go ponder` | Start pondering (converted to normal search) |
| `ponderhit` | Opponent played expected ponder move |
| `stop` | Stop search and output `bestmove` |
| `setoption name Hash value <N>` | Set TT size in MB (1–1024, default 16) |
| `setoption name Ponder value <true/false>` | Enable/disable ponder move output |
| `perft <depth>` | Run perft divide from current position |
| `quit` | Exit the engine |

---

## Project Structure

All source code lives in the `src/` directory:

| File | Description |
|------|-------------|
| `src/main.cpp` | Entry point |
| `src/uci.cpp / .h` | UCI protocol loop |
| `src/bitboard.cpp / .h` | Board state, magic bitboards, attack tables |
| `src/movegen.cpp / .h` | Move generation (all moves + captures) |
| `src/move.cpp / .h` | Move encoding, make/unmake |
| `src/search.cpp / .h` | Negamax, quiescence, evaluation, iterative deepening |
| `src/TT.cpp / .h` | Transposition table |
| `src/hash.cpp / .h` | Zobrist key generation |
| `src/perft.cpp / .h` | Perft testing |
| `src/position.cpp / .h` | FEN parsing |
| `src/time.cpp / .h` | Time management |
| `src/defs.h` | Global constants, enums, and shared types |

---

## Roadmap

- [ ] NNUE
- [ ] Opening book support
- [ ] Endgame tablebases

---
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
- `position`, `go`, `stop`, `quit`, `ucinewgame` commands supported
- `info` lines with depth, score, nodes, time, nps, and PV
- Time management with hard/soft limits

### Testing
- **Perft** function for move generation correctness verification

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

```bash
./cipher
uci
isready
position startpos
go depth 10
```

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
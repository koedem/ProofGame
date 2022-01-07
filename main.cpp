#include <iostream>
#include <chrono>

#include "position.h"
#include "tables.h"
#include "types.h"
#include "proof_game.h"

using namespace std::chrono;

constexpr uint32_t base_depth = 9;

void perftLoop(Perft& perft) {
    for (uint32_t depth = 1; depth < base_depth; depth++) {
        switch (depth) {
            case 1: perft.basePerft<WHITE, 1>(); break;
            case 2: perft.basePerft<WHITE, 2>(); break;
            case 3: perft.basePerft<WHITE, 3>(); break;
            case 4: perft.basePerft<WHITE, 4>(); break;
            case 5: perft.basePerft<WHITE, 5>(); break;
            case 6: perft.basePerft<WHITE, 6>(); break;
            case 7: perft.basePerft<WHITE, 7>(); break;
            case 8: perft.basePerft<WHITE, 8>(); break;
            case 9: perft.basePerft<WHITE, 9>(); break;
            case 10: perft.basePerft<WHITE, 10>(); break;
            case 11: perft.basePerft<WHITE, 11>(); break;
            case 12: perft.basePerft<WHITE, 12>(); break;
            case 13: perft.basePerft<WHITE, 13>(); break;
            default: break;
        }
    }
}

int main() {
    initialise_all_databases();
    zobrist::initialise_zobrist_keys();

    Position p;
    Position::set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -", p);
    Perft perft(p);

    //std::cout << p << std::endl;
    //perft.oneSideManyMoves<BLACK, 16, true>();

    //perft.findSpecific<WHITE, 8, true>();

    perftLoop(perft);

    return 0;
}

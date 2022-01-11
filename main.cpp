#include <iostream>
#include <chrono>

#include "position.h"
#include "tables.h"
#include "types.h"
#include "proof_game.h"

using namespace std::chrono;

constexpr uint32_t base_depth = 4;

void perftLoop(Perft& perft) {
    for (uint32_t depth = 1; depth <= base_depth; depth++) {
        switch (depth) {
            case 1:  perft.generateTempoLossPositions<WHITE, 1>(); break;
            case 2:  perft.generateTempoLossPositions<WHITE, 2>(); break;
            case 3:  perft.generateTempoLossPositions<WHITE, 3>(); break;
            case 4:  perft.generateTempoLossPositions<WHITE, 4>(); break;
            case 5:  perft.generateTempoLossPositions<WHITE, 5>(); break;
            case 6:  perft.generateTempoLossPositions<WHITE, 6>(); break;
            case 7:  perft.generateTempoLossPositions<WHITE, 7>(); break;
            case 8:  perft.generateTempoLossPositions<WHITE, 8>(); break;
            case 9:  perft.generateTempoLossPositions<WHITE, 9>(); break;
            case 10: perft.generateTempoLossPositions<WHITE, 10>(); break;
            case 11: perft.generateTempoLossPositions<WHITE, 11>(); break;
            case 12: perft.generateTempoLossPositions<WHITE, 12>(); break;
            case 13: perft.generateTempoLossPositions<WHITE, 13>(); break;
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

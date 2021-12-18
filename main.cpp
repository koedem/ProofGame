#include <iostream>
#include <chrono>

#include "position.h"
#include "tables.h"
#include "types.h"
#include "proof_game.h"

using namespace std::chrono;

int main() {
    initialise_all_databases();
    zobrist::initialise_zobrist_keys();

    Position p;
    Position::set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -", p);
    Perft perft1(p);

    // perft1.findSpecific<WHITE, 8, true>();

    for (int depth = 1; depth < 8; depth++) {
        perft1.basePerft<WHITE>(depth);
    }

    return 0;
}

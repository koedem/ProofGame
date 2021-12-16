#include <iostream>
#include <chrono>

#include "position.h"
#include "tables.h"
#include "types.h"
#include "proof_game.h"

using namespace std::chrono;

//Computes the perft of the position for a given depth, using bulk-counting
//According to the https://www.chessprogramming.org/Perft site:
//Perft is a debugging function to walk the move generation tree of strictly legal moves to count
//all the leaf nodes of a certain depth, which can be compared to predetermined values and used to isolate bugs
template<Color Us>
uint64_t perft(Position& p, unsigned int depth) {
    if (depth == 0) {
        return 1;
    }

    uint64_t nodes = 0;

    MoveList<Us> list(p);

    //if (depth == 1) return list.size();

    for (Move move : list) {
        p.play<Us>(move);
        nodes += perft<~Us>(p, depth - 1);
        p.undo<Us>(move);
    }

    return nodes;
}

int main() {
    initialise_all_databases();
    zobrist::initialise_zobrist_keys();

    Position p;
    Position::set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -", p);
    Perft perft1(p);

    for (int depth = 1; depth < 8; depth++) {
        perft1.basePerft<WHITE>(depth);
    }

    /*for (int depth = 1; depth < 8; depth++) {
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        uint64_t perftResult = perft<WHITE>(p, depth);
        high_resolution_clock::time_point t2 = high_resolution_clock::now();

        duration<double, std::milli> time_span = t2 - t1;
        std::cout << "Perft depth " << depth << ": " << perftResult << " in " << time_span.count() << "ms, speed: "
                  << perftResult / (1000 * time_span.count()) << "MN/s." << std::endl;
    }*/

    return 0;
}

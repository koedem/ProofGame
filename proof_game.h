#pragma once

#include<iostream>
#include<vector>
#include<list>
#include<chrono>

#include "position.h"
#include "perftTT8.h"
#include "forbiddenTT.h"

using namespace std::chrono;

const uint64_t counterSizeMB = 64;
const uint64_t positionsMB = 16;
const uint64_t oldUniqueMB = 16;

class Perft {

    Position board;
    std::vector<PerftTT_8<counterSizeMB>> counterHelp;
    PerftTT_8<positionsMB> perftTT;
    ForbiddenTT oldUnique;
    const bool memoryIsSparse = true;

    std::list<std::string> dissimilarPositions;

    template<Color to_move>
    uint64_t generateEmptyPositions(int depth) {
        uint64_t nodes = 0;
        if (depth == 0) {
            perftTT.putEmpty(board.get_hash());
            return 1;
        }
        MoveList<to_move> moves(board);

        for (const Move& move : moves) {
            board.play<to_move>(move);

            nodes += generateEmptyPositions<~to_move>(depth - 1);
            board.undo<to_move>(move);
        }
        return nodes;
    }

    template<Color to_move>
    uint64_t collectUniquePositions(int depth, int depthSoFar) {
        uint64_t nodes = 0;
        if (depth == 0) {
            if (perftTT.isUnique(board.get_hash()) && dissimilar()) {
                std::cout << board;
                oldUnique.put(board.get_hash(), depthSoFar);
            }
            return 1;
        }
        MoveList<to_move> moves(board);

        for (const Move& move : moves) {
            board.template play<to_move>(move);
            nodes += collectUniquePositions<~to_move>(depth - 1, depthSoFar + 1);
            board.template undo<to_move>(move);
        }
        return nodes;
    }

    template<Color to_move>
    uint64_t  countDeepPositions(int depth, int depthSoFar, bool ruined) {
        if (depth == 1) {
            return incrementFinalDepth<to_move>(ruined);
        }

        if (memoryIsSparse) { // if our bottleneck is memory, don't store depth one results, therefore store up here
            uint64_t hash = board.get_hash() + depth;
            if (counterHelp[hash % 3].incrementToLimit(hash, 2, depth) >= 2) {
                return 1;
            }
        }

        MoveList<to_move> moves(board);
        uint64_t nodes = 0;

        for (const Move& move : moves) {
            board.template play<to_move>(move);
            if (!memoryIsSparse) {
                uint64_t hash = board.get_hash() + depth;
                if (counterHelp[hash % 3].incrementToLimit(hash, 2, depth) >= 2) {
                    board.template undo<to_move>(move);
                    nodes++;
                    continue;
                }
            }
            nodes += countDeepPositions<~to_move>(depth - 1, depthSoFar + 1, ruined || oldUnique.isPresent(board.get_hash()) < depthSoFar); // we get ruined by an old unique position
            board.template undo<to_move>(move);
        }
        return nodes;
    }

    template<Color to_move>
    uint64_t incrementFinalDepth(bool ruined) {
        MoveList<to_move> moves(board);
        static uint64_t hash = 0;

        for (const Move& move : moves) {
            perftTT.incrementPosition(board.get_hash() ^ board.template zobrist_change_move<to_move>(move), ruined);
        }
        return moves.size();
    }

    bool dissimilar() {
        std::string squareStr = board.squareString();
        for (const std::string& other : dissimilarPositions) {
            if (similar(other, squareStr)) {
                return false;
            }
        }
        dissimilarPositions.push_back(squareStr);
        return true;
    }

    static bool similar(const std::string& a, const std::string& b) {
        int differences = 0;
        if (a.length() != 64) {
            std::cout << "Error, incorrect square-string length." << std::endl;
        }
        for (int i = 0; i < a.length(); i++) {
            if (a[i] != b[i]) {
                differences++;
            }
        }
        return differences <= 2;
    }

public:
    explicit Perft(const Position& board) : board(board), counterHelp(3), perftTT(), oldUnique(oldUniqueMB) {
    }

    template<Color to_move>
    void basePerft(int depth) {
        perftTT.reset();
        counterHelp[0].reset();
        counterHelp[1].reset();
        counterHelp[2].reset();
        dissimilarPositions.clear();
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        uint64_t result = 0;

        result = generateEmptyPositions<to_move>(depth); // prepare shallow positions
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        duration<double, std::milli> time_span = t2 - t1;

        std::cout << depth << ": " << result << "\ttime: " << time_span.count() << " knps: " << (result / time_span.count()) << std::endl;
        perftTT.printCounts();

        t1 = high_resolution_clock::now();
        result = countDeepPositions<to_move>(depth + 3, 0, false); // TODO if changed, change the inCheck check
        t2 = high_resolution_clock::now();
        time_span = t2 - t1;

        std::cout << depth << ": " << result << "\ttime: " << time_span.count() << " knps: " << (result / time_span.count()) << std::endl;
        perftTT.printFrequencies();
        counterHelp[0].printCounts();
        counterHelp[1].printCounts();
        counterHelp[2].printCounts();

        collectUniquePositions<to_move>(depth, 0);
        oldUnique.printCounts();
    }
};
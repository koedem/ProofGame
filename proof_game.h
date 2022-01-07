#pragma once

#include<iostream>
#include<vector>
#include<list>
#include<chrono>
#include<stack>
#include <zconf.h>

#include "position.h"
#include "perftTT8.h"
#include "hash_map.h"

using namespace std::chrono;

const uint64_t counterSizeMB = 8192;
const uint64_t positionsMB = 16;
const uint64_t oldUniqueMB = 32;
const uint64_t emptyPosMB = 1;

const uint32_t additional_depth = 5;

class Perft {

    Position board;
    PerftTT_8<counterSizeMB * 3, counterSizeMB << 17> counterHelp;
    PerftTT_8<positionsMB, positionsMB << 17> perftTT;
    Hash_map oldUnique; // this only works when adding an even number of plies for the unqiue game, otherwise the
                        // out of order moves might prevent duplicating the moves in a shorter position
    Hash_map emptyPositionTT;
    static constexpr bool memoryIsSparse = false;
    static constexpr bool countFinalDepth = false;

    std::list<std::string> dissimilarPositions;

    template<Color to_move>
    uint64_t generateEmptyPositions(uint32_t depth, uint32_t depthSoFar) {
        uint64_t nodes = 0;
        if (depth == 0) {
            perftTT.putEmpty(board.get_raw_hash(), depthSoFar);
            return 1;
        }
        if (emptyPositionTT.putIfNotPresent(board.get_ep_hash(), depthSoFar)) {
            return 1;
        }
        MoveList<to_move> moves(board);

        for (const Move& move : moves) {
            board.play<to_move>(move);

            nodes += generateEmptyPositions<~to_move>(depth - 1, depthSoFar + 1);
            board.undo<to_move>(move);
        }
        return nodes;
    }

    template<Color to_move>
    uint64_t collectUniquePositions(uint32_t depth, uint32_t depthSoFar) {
        uint64_t nodes = 0;
        if (depth == 0) {
            if (perftTT.getOccurrences(board.get_raw_hash(), depthSoFar) == 1/*&& dissimilar()*/) {
                std::cout << board.fen() << std::endl;
                oldUnique.putIfNotPresent(board.get_raw_hash(), depthSoFar);
            }
            return 1;
        }
        if (emptyPositionTT.putIfNotPresent(board.get_ep_hash(), depthSoFar)) {
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
            return incrementFinalDepth<to_move>(ruined, depthSoFar + 1);
        }
        uint64_t count;
        if (memoryIsSparse) { // if our bottleneck is memory, don't store depth one results, therefore store up here
            count = counterHelp.incrementToLimit(board.get_ep_hash(), 2, depthSoFar, ruined);
            if (count >= 2) {
                return 1;
            }
        }

        MoveList<to_move> moves(board);
        uint64_t nodes = 0;

        for (const Move& move : moves) {
            board.template play<to_move>(move);
            if (!memoryIsSparse) {
                count = counterHelp.incrementToLimit(board.get_ep_hash(), 2, depthSoFar + 1, ruined);
                if (count >= 2) {
                    board.template undo<to_move>(move);
                    nodes++;
                    continue;
                }
            }
            nodes += countDeepPositions<~to_move>(depth - 1, depthSoFar + 1, ruined || count > 0); // || oldUnique.isPresent(board.get_hash()) < depthSoFar);
            board.template undo<to_move>(move);
        }
        return nodes;
    }

    template<Color to_move>
    uint64_t incrementFinalDepth(bool ruined, uint32_t depthSoFar) {
        MoveList<to_move> moves(board);
        static uint64_t hash = 0; // TODO prefetch, buffer access etc.; use + depthSoFar then

        for (const Move& move : moves) {
            hash = (board.get_raw_hash() ^ board.template zobrist_change_move<to_move>(move));
            if constexpr(countFinalDepth) {
                counterHelp.incrementToLimit(hash, 2, depthSoFar, ruined);
            }
            perftTT.incrementPosition(hash, depthSoFar - additional_depth, ruined);
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
    explicit Perft(const Position& board) : board(board), counterHelp(), perftTT(), oldUnique(oldUniqueMB),
                                            emptyPositionTT(emptyPosMB) {
    }

    template<Color to_move>
    void basePerft(uint32_t depth) {
        perftTT.reset();
        counterHelp.reduceCounts();
        dissimilarPositions.clear();
        emptyPositionTT.reset();
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        uint64_t result = 0;

        result = generateEmptyPositions<to_move>(depth, 0); // prepare shallow positions
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        duration<double, std::milli> time_span = t2 - t1;
        emptyPositionTT.printCounts();
        std::cout << depth << ": " << result << "\ttime: " << time_span.count() << " knps: " << (result / time_span.count()) << std::endl;
        perftTT.printCounts();

        t1 = high_resolution_clock::now();
        result = countDeepPositions<to_move>(depth + additional_depth, 0, false);
        t2 = high_resolution_clock::now();
        time_span = t2 - t1;

        std::cout << depth << ": " << result << "\ttime: " << time_span.count() << " knps: " << (result / time_span.count()) << std::endl;
        perftTT.printFrequencies();
        counterHelp.printCounts();

        emptyPositionTT.reset();
        t1 = high_resolution_clock::now();
        collectUniquePositions<to_move>(depth, 0);
        t2 = high_resolution_clock::now();
        time_span = t2 - t1;
        std::cout << "Collected unique positions in: " << time_span.count() << std::endl;
        oldUnique.printCounts();
    }

    template<Color to_move>
    void solveSpecific() {
        if (counterHelp.incrementToLimit(board.get_ep_hash(), 2, 10, false) >= 1) {
            return;
        }
        MoveList<to_move> moves(board);

        for (const Move& move : moves) {
            board.play<to_move>(move);
            move_stack.push(move);

            if (!permanentCondition(1)) {
                board.undo<to_move>(move);
                move_stack.pop();
                continue;
            }
            MoveList<~to_move> moves_2(board);

            for (const Move& move_2 : moves_2) {
                if (move_2.is_capture() && board.at(move_2.to()) == WHITE_BISHOP && board.at(move_2.from()) == BLACK_KNIGHT && (move_2.to() % 9) % 2 == 0) {
                    board.play<~to_move>(move_2);
                    if (board.template in_check<to_move>()) {
                        MoveList<to_move> mate(board);
                        if (mate.size() == 0) {
                            while (!move_stack.empty()) {
                                std::cout << move_stack.top() << std::endl;
                                move_stack.pop(); // note, after this while loop the program will segfault by trying to pop an empty stack
                            }
                            return;
                        }
                    }
                    board.undo<~to_move>(move_2);
                }
            }
            board.undo<to_move>(move);
            move_stack.pop();
        }
    }

    time_t start, end;

    bool finalCondition() {
        return true;
    }

    bool permanentCondition(uint32_t depth) {
        return (depth > 3 || board.at(g8) != BLACK_KNIGHT || board.at(b8) != BLACK_KNIGHT) && (depth > 1 ||
        board.at(g8) != BLACK_KNIGHT && board.at(f6) != BLACK_KNIGHT && board.at(h6) != BLACK_KNIGHT && board.at(e7) != BLACK_KNIGHT ||
                board.at(b8) != BLACK_KNIGHT && board.at(c6) != BLACK_KNIGHT && board.at(a6) != BLACK_KNIGHT && board.at(d7) != BLACK_KNIGHT);
    }

    std::stack<Move> move_stack;

    template<Color to_move, int32_t depth, bool top_level>
    void findSpecific() {
        if (counterHelp.incrementToLimit(board.get_ep_hash(), 2, depth, false) >= 1) {
            return;
        }
        MoveList<to_move> moves(board);

        for (const Move& move : moves) {
            if (top_level) {
                time(&start);
            }
            board.play<to_move>(move);
            move_stack.push(move);

            if (!permanentCondition(depth)) {
                board.undo<to_move>(move);
                move_stack.pop();
                continue;
            }
            if constexpr (depth > 1) {
                findSpecific<~to_move, depth - 1, false>();
            } else {
                solveSpecific<~to_move>();
            }
            board.undo<to_move>(move);
            move_stack.pop();
            if (top_level) {
                time(&end);
                std::cout << "Move " << move << " took " << (end - start) << " seconds." << std::endl;
            }
        }
    }

    template<Color to_move>
    void oneSideThenMate() {
        MoveList<to_move> moves(board);

        for (const Move& move : moves) {
            board.play<to_move>(move);
            MoveList<~to_move> mate_moves(board);
            for (const Move& mate_move : mate_moves) {
                board.play<~to_move>(mate_move);
                if (!board.template in_check<to_move>()) {
                    MoveList<to_move> empty(board);
                    if (empty.size() == 0) {
                        while (!move_stack.empty()) {
                            std::cout << move_stack.top() << std::endl;
                            move_stack.pop(); // note, after this while loop the program will segfault by trying to pop an empty stack
                        }
                        std::cout << move << " " << mate_move << std::endl;
                    }
                }
                board.undo<~to_move>(mate_move);
            }
            board.undo<to_move>(move);
        }
    }

    template<Color to_move, uint32_t depth, bool top_level>
    void oneSideManyMoves() {
        MoveList<to_move> moves(board);

        for (const Move& move : moves) {
            if (top_level) {
                time(&start);
            }
            board.play<to_move>(move);
            if (board.template in_check<~to_move>() || counterHelp.incrementToLimit(board.get_ep_hash(), 2, depth, false) >= 1) {
                board.undo<to_move>(move);
                continue;
            }
            move_stack.push(move);


            if constexpr (depth > 1) {
                oneSideManyMoves<to_move, depth - 1, false>();
            } else {
                oneSideThenMate<to_move>();
            }
            board.undo<to_move>(move);
            move_stack.pop();
            if (top_level) {
                time(&end);
                std::cout << "Move " << move << " took " << (end - start) << " seconds." << std::endl;
            }
        }
    }
};
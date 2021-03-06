#pragma once

#include <vector>
#include <iostream>
#include <iomanip>
#include "proof_game.h"

constexpr uint32_t bucketSize = 8;

struct __attribute__((packed, aligned(64))) Bucket {
    uint64_t slots[bucketSize] = { 0, 0, 0, 0, 0, 0, 0, 0 };
};

template<size_t SIZE_IN_MB, size_t POWER_OF_TWO>
class PerftTT_8 {

    static constexpr uint64_t size = SIZE_IN_MB * 131072 / 8;
    static constexpr uint64_t mask = (POWER_OF_TWO / 8 - 1); // eight entries per bucket
    static constexpr uint64_t    FREQUENCY_CAP = 8;
    static constexpr uint32_t depthFirstSlots = bucketSize / 2;
    alignas(64) std::vector<Bucket> entries;
    alignas(64) std::vector<uint64_t> limitHitsPerDepth;

    uint64_t entryCount = 0, dupeCount = 0, collisionCount = 0;

public:

    template<uint32_t DEPTH_SO_FAR>
    void putEmpty(uint64_t hash) {
        uint64_t depth_hash = hash + DEPTH_SO_FAR;
        uint64_t bucket = depth_hash % size;
        uint64_t entry = depth_hash & (~mask);

        for (uint32_t index = 0; index < 8; index++) {
            if (entries[bucket].slots[index] == 0) {
                entries[bucket].slots[index] = entry;
                entryCount++;
                return;
            } else if (entries[bucket].slots[index] == entry) {
                dupeCount++;
                return;
            }
        }
        collisionCount++;
    }

    template<uint32_t DEPTH_SO_FAR>
    void incrementPosition(uint64_t hash, bool ruined) {
        uint64_t depth_hash = hash + DEPTH_SO_FAR;
        uint64_t bucket = depth_hash % size;
        uint64_t entry_code = (depth_hash & (~mask));
        for (uint32_t i = 0; i < 8; i++) {
            if ((entries[bucket].slots[i] & (~mask)) == entry_code) {
                uint64_t count;
                if ((count = entries[bucket].slots[i] & mask) < mask - 1) { // once we reach the mask, i.e. equals, we stop incrementing
                    entries[bucket].slots[i] = (entries[bucket].slots[i] & (~mask)) + count + (ruined ? 2 : 1); // TODO just += instead?
                }
                break;
            }
        }
    }

    template<uint32_t DEPTH_SO_FAR>
    uint64_t incrementToLimit(uint64_t hash, uint32_t limit, bool ruined) {
        uint64_t depth_hash = hash + DEPTH_SO_FAR;
        uint64_t bucket = depth_hash % size;
        uint64_t quotient = depth_hash / size; // hopefully this gets optimized to use the result from the previous line
        uint64_t entry_code = (depth_hash & (~mask));
        for (uint32_t i = 0; i < 8; i++) {
            if (entries[bucket].slots[i] == 0) {
                entries[bucket].slots[i] = entry_code + (ruined ? 2 : 1);
                entryCount++;
                return 0;
            }
            if ((entries[bucket].slots[i] & (~mask)) == entry_code) {
                uint64_t count;
                if ((count = entries[bucket].slots[i] & mask) < limit) { // once we reach the limit, i.e. equals, we stop incrementing
                    entries[bucket].slots[i] = entry_code + count + 1;
                    if (ruined && count < limit - 1) {
                        entries[bucket].slots[i]++; // TODO this whole adding thing probably deserves a rewrite
                    }
                } else {
                    limitHitsPerDepth[DEPTH_SO_FAR]++;
                }
                dupeCount++;
                return count;
            }
        }
        if (depthFirstSlots < 8 && ruined) { // if ruined, use one of the always replace slots, a pseudo random one
            entries[bucket].slots[depthFirstSlots + (quotient % (8 - depthFirstSlots))] = entry_code + 2;
        }
        collisionCount++;
        return 0;
    }

    template<uint32_t DEPTH_SO_FAR>
    uint32_t getOccurrences(uint64_t hash) {
        uint64_t depth_hash = hash + DEPTH_SO_FAR;
        uint64_t bucket = depth_hash % size;
        uint64_t entry_code = (depth_hash & (~mask));
        for (uint32_t i = 0; i < 8; i++) {
            if ((entries[bucket].slots[i] & (~mask)) == entry_code) {
                return entries[bucket].slots[i] & mask;
            }
        }
        return 0;
    }

    void printCounts() {
        std::cout << "Unique positions: " << entryCount << ", collision losses: " << collisionCount << ", remainder (duplicates): "
                  << dupeCount << ", sum: " << (entryCount + collisionCount + dupeCount) << std::endl;
        std::cout << "Fill: " << std::fixed << std::setprecision(2) << (double) entryCount * 12.5 / (double) size << "%; "
                  << "duplication factor: " << (double) dupeCount / (double) entryCount << "; "
                  << "loss percentage: " << (double) collisionCount * 100 / (double) entryCount << "%." << std::endl;

        for (int depth = 0; depth < 10; depth++) {
            if (limitHitsPerDepth[depth] > 0) {
                std::cout << "Depth " << depth << " hits: " << limitHitsPerDepth[depth] << std::endl;
            }
        }
    }

    void printFrequencies() {
        std::vector<uint64_t> frequencies(FREQUENCY_CAP + 1);
        for (Bucket bucket : entries) {
            for (int i = 0; i < 8; i++) {
                uint64_t entry = bucket.slots[i];
                if (entry != 0) {
                    size_t frequency = entry & mask;
                    if (frequency < FREQUENCY_CAP) {
                        frequencies[frequency]++;
                    } else {
                        frequencies[FREQUENCY_CAP]++;
                    }
                }
            }
        }
        for (size_t i = 0; i < frequencies.size(); i++) {
            if (frequencies[i] > 0) {
                std::cout << "Frequency " << i << ": " << frequencies[i] << std::endl;
            }
        }
    }

    void reset() {
        for (Bucket& bucket : entries) {
            for (int i = 0; i  < 8; i++) {
                bucket.slots[i] = 0;
            }
        }
        std::fill(limitHitsPerDepth.begin(), limitHitsPerDepth.end(), 0);
        entryCount = 0;
        dupeCount = 0;
        collisionCount = 0;
    }

    void reduceCounts() {
        for (Bucket& bucket : entries) {
            for (int i = 0; i < 8; i++) {
                if ((bucket.slots[i] & mask) > 1) { // reached multiple times
                    bucket.slots[i] = (bucket.slots[i] & ~mask) + 1; // then we say it got reached once so far
                } else {
                    bucket.slots[i] = (bucket.slots[i] & ~mask); // else it never got reached so far, so won't get a cutoff
                }
            }
        }
        std::fill(limitHitsPerDepth.begin(), limitHitsPerDepth.end(), 0);
    }

    explicit PerftTT_8() : limitHitsPerDepth(10),
            entries(size) {
    }
};
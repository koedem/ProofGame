#pragma once

#include <vector>
#include <iostream>
#include <iomanip>

struct __attribute__((packed, aligned(64))) Bucket {
    uint64_t slots[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
};

template<size_t SIZE_IN_MB>
class PerftTT_8 {

    uint64_t  size;
    uint64_t mask;
    const uint64_t    FREQUENCY_CAP = 8;
    alignas(64) std::vector<Bucket> entries;
    alignas(64) std::vector<uint64_t> limitHitsPerDepth;

    uint64_t entryCount = 0, dupeCount = 0, collisionCount = 0;

public:

    void putEmpty(uint64_t hash) {
        uint64_t bucket = hash & mask;
        if (entries[bucket].slots[0] == (hash & (~mask)) + 1 || entries[bucket].slots[1] == (hash & (~mask)) + 1
            || entries[bucket].slots[2] == (hash & (~mask)) + 1 || entries[bucket].slots[3] == (hash & (~mask)) + 1
            || entries[bucket].slots[4] == (hash & (~mask)) + 1 || entries[bucket].slots[5] == (hash & (~mask)) + 1
            || entries[bucket].slots[6] == (hash & (~mask)) + 1 || entries[bucket].slots[7] == (hash & (~mask)) + 1) {
            dupeCount++;
        } else if (entries[bucket].slots[0] == 0) {
            entries[bucket].slots[0] = (hash & (~mask)) + 1;
            entryCount++;
        } else if (entries[bucket].slots[1] == 0) {
            entries[bucket].slots[1] = (hash & (~mask)) + 1;
            entryCount++;
        } else if (entries[bucket].slots[2] == 0) {
            entries[bucket].slots[2] = (hash & (~mask)) + 1;
            entryCount++;
        } else if (entries[bucket].slots[3] == 0) {
            entries[bucket].slots[3] = (hash & (~mask)) + 1;
            entryCount++;
        } else if (entries[bucket].slots[4] == 0) {
            entries[bucket].slots[4] = (hash & (~mask)) + 1;
            entryCount++;
        } else if (entries[bucket].slots[5] == 0) {
            entries[bucket].slots[5] = (hash & (~mask)) + 1;
            entryCount++;
        } else if (entries[bucket].slots[6] == 0) {
            entries[bucket].slots[6] = (hash & (~mask)) + 1;
            entryCount++;
        } else if (entries[bucket].slots[7] == 0) {
            entries[bucket].slots[7] = (hash & (~mask)) + 1;
            entryCount++;
        } else {
            collisionCount++;
        }
    }

    void incrementPosition(uint64_t hash, bool ruined) {
        uint64_t bucket = hash & mask;
        for (uint64_t i = 0; i < 8; i++) {
            if ((entries[bucket].slots[i] & (~mask)) == (hash & (~mask))) {
                uint64_t count;
                if ((count = entries[bucket].slots[i] & mask) < mask) { // once we reach the mask, i.e. equals, we stop incrementing
                    entries[bucket].slots[i] = (entries[bucket].slots[i] & (~mask)) + count + (ruined ? 2 : 1);
                }
                break;
            }
        }
    }

    bool isUnique(uint64_t hash) {
        uint64_t bucket = hash & mask;
        for (uint64_t i = 0; i < 8; i++) {
            if ((entries[bucket].slots[i] & (~mask)) == (hash & (~mask))) {
                if ((entries[bucket].slots[i] & mask) <= 7 && (entries[bucket].slots[i] & mask) == 2) { // two means once created then once reached from higher depth
                    return true;
                }
                break;
            }
        }
        return false;
    }

    uint64_t incrementToLimit(uint64_t hash, int limit, int depth) {
        uint64_t bucket = hash & mask;
        for (uint64_t i = 0; i < 8; i++) {
            if ((entries[bucket].slots[i] & (~mask)) == (hash & (~mask))) {
                uint64_t count;
                if ((count = entries[bucket].slots[i] & mask) < limit) { // once we reach the limit, i.e. equals, we stop incrementing
                    entries[bucket].slots[i] = (entries[bucket].slots[i] & (~mask)) + count + 1;
                } else {
                    limitHitsPerDepth[depth]++;
                }
                dupeCount++;
                return count;
            }
        }

        if (entries[bucket].slots[0] == 0) {
            entries[bucket].slots[0] = (hash & (~mask)) + 1;
            entryCount++;
        } else if (entries[bucket].slots[1] == 0) {
            entries[bucket].slots[1] = (hash & (~mask)) + 1;
            entryCount++;
        } else if (entries[bucket].slots[2] == 0) {
            entries[bucket].slots[2] = (hash & (~mask)) + 1;
            entryCount++;
        } else if (entries[bucket].slots[3] == 0) {
            entries[bucket].slots[3] = (hash & (~mask)) + 1;
            entryCount++;
        } else if (entries[bucket].slots[4] == 0) {
            entries[bucket].slots[4] = (hash & (~mask)) + 1;
            entryCount++;
        } else if (entries[bucket].slots[5] == 0) {
            entries[bucket].slots[5] = (hash & (~mask)) + 1;
            entryCount++;
        } else if (entries[bucket].slots[6] == 0) {
            entries[bucket].slots[6] = (hash & (~mask)) + 1;
            entryCount++;
        } else if (entries[bucket].slots[7] == 0) {
            entries[bucket].slots[7] = (hash & (~mask)) + 1;
            entryCount++;
        } else {
            collisionCount++;
        }
        return 0;
    }

    void printCounts() {
        std::cout << "Unique positions: " << entryCount << ", collision losses: " << collisionCount << ", remainder (duplicates): "
                  << dupeCount << ", sum: " << (entryCount + collisionCount + dupeCount) << std::endl;
        std::cout << "Fill: " << std::fixed << std::setprecision(2) << (double) entryCount * 100 / (double) size << "%; "
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
                size_t frequency = entry & mask;
                if (frequency < FREQUENCY_CAP) {
                    frequencies[frequency]++;
                } else {
                    frequencies[FREQUENCY_CAP]++;
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
        std::fill(entries.begin(), entries.end(), Bucket());
        for (Bucket& bucket : entries) {
            for (uint64_t & slot : bucket.slots) {
                slot = 0;
            }
        }
        std::fill(limitHitsPerDepth.begin(), limitHitsPerDepth.end(), 0);
        entryCount = 0;
        dupeCount = 0;
        collisionCount = 0;
    }

    explicit PerftTT_8() : limitHitsPerDepth(10),
            size(SIZE_IN_MB * 131072),
            mask(size / 8 - 1), // eight entries per bucket
            entries(size / 8) {
    }
};
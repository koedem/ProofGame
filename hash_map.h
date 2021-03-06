#pragma once

#include <iostream>
#include <iomanip>
#include <vector>

class Hash_map {

    const uint64_t  size;
    const uint64_t mask;
    std::vector<uint64_t> entries;

    uint64_t entryCount = 0, collisionCount = 0;

public:
    explicit Hash_map(uint64_t sizeInMB) : size(sizeInMB * 131072),
                                           mask(size / 8 - 1), // eight entries per bucket
        entries(size) {
    }

    void reset() {
        std::fill(entries.begin(), entries.end(), 0);
        entryCount = 0;
        collisionCount = 0;
    }

    template<uint32_t DEPTH_SO_FAR>
    uint64_t isPresent(uint64_t hash) {
        uint64_t depth_hash = hash + DEPTH_SO_FAR;
        uint64_t bucket = depth_hash & mask;
        for (uint64_t i = 0; i < 8; i++) {
            if ((entries[8 * bucket + i] & (~mask)) == (hash & (~mask))) {
                return entries[8 * bucket + i] & mask;
            }
        }
        return 100;
    }

    template<uint32_t DEPTH_SO_FAR>
    bool putIfNotPresent(uint64_t hash) {
        uint64_t depth_hash = hash + DEPTH_SO_FAR;
        uint64_t bucket = depth_hash & mask;
        for (uint64_t i = 0; i < 8; i++) {
            if ((entries[8 * bucket + i] & (~mask)) == (hash & (~mask))) {
                return true;
            }
        }
        for (uint64_t i = 0; i < 8; i++) {
            if (entries[8 * bucket + i] == 0) {
                entries[8 * bucket] = hash;
                entryCount++;
                return false;
            }
        }
        collisionCount++;
        return false;
    }

    void printCounts() const {
        std::cout << "Unique positions: " << entryCount << ", collision losses: " << collisionCount << std::endl;
        std::cout << "Fill: " << std::fixed << std::setprecision(2) << (double) entryCount * 100 / (double) size
                  << "%; " << "loss percentage: " << (double) collisionCount * 100 / (double) entryCount << "%." << std::endl;
    }
};
#pragma once

#include <iostream>
#include <iomanip>
#include <vector>

class Hash_map {

    const uint64_t  size;
    const uint64_t mask;
    std::vector<uint64_t> entries;

    uint64_t entryCount = 0, collisionCount = 0;

    void reset() {
        std::fill(entries.begin(), entries.end(), 0);
        entryCount = 0;
        collisionCount = 0;
    }

public:
    explicit Hash_map(uint64_t sizeInMB) : size(sizeInMB * 131072),
                                           mask(size / 8 - 1), // eight entries per bucket
        entries(size) {
    }

    //template<uint32_t depth_so_far>
    uint64_t isPresent(uint64_t hash, int depth_so_far) {
        uint64_t depth_hash = hash + depth_so_far;
        uint64_t bucket = depth_hash & mask;
        for (uint64_t i = 0; i < 8; i++) {
            if ((entries[8 * bucket + i] & (~mask)) == (hash & (~mask))) {
                return entries[8 * bucket + i] & mask;
            }
        }
        return 100;
    }

    //template<uint32_t depth_so_far>
    void put(uint64_t hash, int depth_so_far) {
        uint64_t depth_hash = hash + depth_so_far;
        uint64_t bucket = depth_hash & mask;
        for (uint64_t i = 0; i < 8; i++) {
            if ((entries[8 * bucket + i] & (~mask)) == (hash & (~mask))) {
                return;
            }
        }
        for (uint64_t i = 0; i < 8; i++) {
            if (entries[8 * bucket + i] == 0) {
                entries[8 * bucket] = hash;
                entryCount++;
                return;
            }
        }
        collisionCount++;
    }

    void printCounts() const {
        std::cout << "Unique positions: " << entryCount << ", collision losses: " << collisionCount << std::endl;
        std::cout << "Fill: " << std::fixed << std::setprecision(2) << (double) entryCount * 100 / (double) size
                  << "%; " << "loss percentage: " << (double) collisionCount * 100 / (double) entryCount << "%." << std::endl;
    }
};
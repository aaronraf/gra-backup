#ifndef CACHEADDRESS_HPP
#define CACHEADDRESS_HPP

#include "cache_config.hpp"
#include <cstdint>
#include <cmath>

struct CacheAddress {
    uint32_t index; 
    uint32_t tag;
    uint32_t offset;

    CacheAddress(uint32_t address, CacheConfig cache_config) {
        uint32_t offset_full_mask = static_cast<int>(pow(2, cache_config.number_of_offset_bits)) - 1;
        uint32_t index_full_mask= static_cast<int>(pow(2, cache_config.number_of_index_bits)) - 1;

        offset = address & offset_full_mask;
        index = (address >> cache_config.number_of_offset_bits) & index_full_mask;
        tag = (address >> cache_config.number_of_offset_bits) >> cache_config.number_of_index_bits; 
    }
};

#endif

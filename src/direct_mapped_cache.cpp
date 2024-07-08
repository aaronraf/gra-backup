#include "../includes/direct_mapped_cache.hpp"
#include "../includes/main_memory_global.hpp"
#include <iostream>
#include <cmath>
using namespace std;

DirectMappedCache::DirectMappedCache(unsigned cacheLines, CacheConfig cache_config) : cache_lines(cacheLines) {
    cache_entry = new CacheEntry[cacheLines];
    for (unsigned i = 0; i < cacheLines; i++) {
        cache_entry[i].data = new uint32_t[(int) pow(2, cache_config.number_of_offset_bits)];
    }
}

DirectMappedCache::~DirectMappedCache() {
    for (unsigned i = 0; i < cache_lines; i++) {
        delete[] cache_entry[i].data;
    }
    delete[] cache_entry;
}

void DirectMappedCache::replace(uint32_t address, CacheEntry &current_entry, int number_of_offset, CacheConfig cache_config) {
    int total_offset = (int) pow(2, number_of_offset);
    int start_address_to_fetch = (address / total_offset) * total_offset;
    int last_address_to_fetch = start_address_to_fetch + total_offset - 1;
    // cout << start_address_to_fetch << " " << last_address_to_fetch << endl;

    // cout << "ram to fetch: " << endl;
    for (int ram_address = start_address_to_fetch, offset = 0; ram_address <= last_address_to_fetch; ram_address++, offset++) { // O(1)
        current_entry.data[offset] = main_memory->read_from_ram(ram_address);
        // cout << "data " << ram_address << " " << current_entry.data[offset] << endl;
    }

    CacheAddress new_address(start_address_to_fetch, cache_config);
    current_entry.tag = new_address.tag;
}

int DirectMappedCache::read_from_cache(uint32_t address, CacheConfig cache_config, Result &result) {
    CacheAddress cache_address(address, cache_config);
    CacheEntry &current_entry = cache_entry[cache_address.index];
    bool found = true;

    // cout << "tag: " << cache_address.tag << " index: " << cache_address.index << " offset: " << cache_address.offset << " value: " << current_entry.data[cache_address.offset] << " isFirstTime: " << current_entry.is_data_first_time << endl;
    // cout << "current tag: " << current_entry.tag << "\tcache tag: " << cache_address.tag << endl;

    // Replace if tag isn't equal or this must be a cold miss
    if (current_entry.tag != cache_address.tag || current_entry.is_data_first_time) {
        replace(address, current_entry, cache_config.number_of_offset_bits, cache_config);
        current_entry.is_data_first_time = false;
        result.misses++;
        // cout << "this is miss" << endl;
        found = false;
    }
    
    if (found) {
        // cout << "this is hit" << endl;
        result.hits++;
    }
    // cout << "tag: " << cache_address.tag << " index: " << cache_address.index << " offset: " << cache_address.offset << " value: " << current_entry.data[cache_address.offset] << " isFirstTime: " << current_entry.is_data_first_time << endl;

    return current_entry.data[cache_address.offset];
}

void DirectMappedCache::write_to_cache(uint32_t address, CacheConfig cache_config, int data_to_write, Result &result) {
    CacheAddress cache_address(address, cache_config);
    CacheEntry &current_entry = cache_entry[cache_address.index];
    bool found = true;

    // cout << "tag: " << cache_address.tag << " index: " << cache_address.index << " offset: " << cache_address.offset << " value: " << current_entry.data[cache_address.offset] << " isFirstTime: " << current_entry.is_data_first_time << endl;
    // cout << "current tag: " << current_entry.tag << "\tcache tag: " << cache_address.tag << endl;

    // Replace if tag isn't equal or this must be a cold miss
    if (current_entry.tag != cache_address.tag || current_entry.is_data_first_time) {
        replace(address, current_entry, cache_config.number_of_offset_bits, cache_config);
        current_entry.is_data_first_time = false;
        result.misses++;
        found = false;
    }

    if (found) {
        result.hits++;
    }

    current_entry.data[cache_address.offset] = data_to_write;
    
    main_memory->write_to_ram(address, data_to_write); // write-through
}

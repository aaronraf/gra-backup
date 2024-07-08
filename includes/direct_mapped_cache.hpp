#ifndef DIRECTMAPPEDCACHE_HPP
#define DIRECTMAPPEDCACHE_HPP

#include "cache_address.hpp"
#include "main_memory.hpp"
#include "cache_template.hpp"
#include "cache_config.hpp"
#include "structs.hpp"
#include <cstdint>


struct CacheEntry {
    uint32_t tag;
    uint32_t* data;
    bool is_data_first_time = true;
};

class DirectMappedCache : public Cache {
private:
    CacheEntry* cache_entry;
    unsigned cache_lines;

    void replace(uint32_t address, CacheEntry &current_entry, int number_of_offset, CacheConfig cache_config);

public:
    DirectMappedCache(unsigned cacheLines, CacheConfig cache_config);

    ~DirectMappedCache();

    int read_from_cache(uint32_t address, CacheConfig cache_config, Result &result) override;

    void write_to_cache(uint32_t address, CacheConfig cache_config, int data_to_write, Result &result) override;
};

#endif
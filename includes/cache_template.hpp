#ifndef CACHETEMPLATE_HPP
#define CACHETEMPLATE_HPP

#include "cache_config.hpp"
#include "structs.hpp"
#include <cstdint>

class Cache {
public:
    virtual ~Cache() = default;
    virtual int read_from_cache(uint32_t address, CacheConfig cache_config, Result &result) = 0;
    virtual void write_to_cache(uint32_t address, CacheConfig cache_config, int data_to_write, Result &result) = 0;
};

#endif
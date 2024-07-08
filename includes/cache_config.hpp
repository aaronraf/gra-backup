#ifndef CACHECONFIG_HPP
#define CACHECONFIG_HPP

typedef struct CacheConfig {
    int number_of_index_bits;
    int number_of_tag_bits;
    int number_of_offset_bits;
} CacheConfig;

#endif

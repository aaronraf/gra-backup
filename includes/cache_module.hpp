#ifndef CACHEMODULE_HPP
#define CACHEMODULE_HPP

#include <systemc>
#include <iostream>
#include "direct_mapped_cache.hpp"
#include "four_way_lru_cache.hpp"
#include "main_memory.hpp"
#include "structs.hpp"
#include "main_memory_global.hpp"
#include "cache_address.hpp"
#include "cache_config.hpp"
#include "cache_template.hpp"
#include <cstdint>
#define CACHE_ADDRESS_LENGTH 32
using namespace std;
using namespace sc_core;

extern "C" Result run_simulation(int cycles, bool direct_mapped,  unsigned cachelines, unsigned cacheline_size, 
                        unsigned cache_latency, int memory_latency, size_t num_requests, Request requests[], const char* tf_filename);

extern MainMemory* main_memory;

SC_MODULE(CACHE_MODULE) {

    sc_signal<int> data;
    sc_in<bool> clk;
    sc_in<uint32_t> request_addr;
    sc_in<int> request_we;
    sc_in<uint32_t> request_data;
    sc_out<size_t> result_cycles;
    sc_out<size_t> result_hits;
    sc_out<size_t> result_misses;
    sc_out<size_t> result_primitive_gate_count;
    Result result_temp;

    Cache* cache; 
    CacheConfig cache_config;
    int cycles;
    int directMapped;
    unsigned cacheLines;
    unsigned cacheLineSize;
    unsigned cacheLatency;
    unsigned memoryLatency;
    int numRequests;

    SC_CTOR(CACHE_MODULE);
    CACHE_MODULE(sc_module_name name, int cycles, int directMapped, unsigned cacheLines, unsigned cacheLineSize,
                unsigned cacheLatency, unsigned memoryLatency, int numRequests);

    ~CACHE_MODULE();

    void update();  

};

#endif

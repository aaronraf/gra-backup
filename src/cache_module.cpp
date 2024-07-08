#include "../includes/cache_module.hpp"

MainMemory* main_memory = new MainMemory();

CACHE_MODULE::CACHE_MODULE(sc_module_name name, int cycles, int directMapped, unsigned cacheLines, unsigned cacheLineSize,
                unsigned cacheLatency, unsigned memoryLatency, int numRequests) : sc_module(name) {
        
    this->cycles = cycles;
    this->directMapped = directMapped;
    this->cacheLines = cacheLines;
    this->cacheLineSize = cacheLineSize;
    this->cacheLatency = cacheLatency;
    this->memoryLatency = memoryLatency;
    this->numRequests = numRequests; 

    result_temp.misses = 0;
    result_temp.hits = 0;
    result_temp.cycles = 0;  
    result_temp.primitiveGateCount = 0;

    // Determine number of index, offset, tag
    cache_config.number_of_index_bits = ceil(log2((directMapped == 1) ? cacheLines : cacheLines / 4));
    cache_config.number_of_offset_bits = ceil(log2(cacheLineSize));
    cache_config.number_of_tag_bits = CACHE_ADDRESS_LENGTH - cache_config.number_of_index_bits - cache_config.number_of_offset_bits;

    if (directMapped == 0) {
        cache = new FourWayLRUCache(cache_config);
    } else {
        cache = new DirectMappedCache(cacheLines, cache_config);
    }

    SC_THREAD(update);
    sensitive << clk.pos();
}

CACHE_MODULE::~CACHE_MODULE() {
    delete cache;
}

void CACHE_MODULE::update() {
    wait(SC_ZERO_TIME);
    for (int i = 0; i < cycles; i++){
        // Handle edge case when simulation cycles longer than number of requests
        while (i >= numRequests) {
            result_cycles.write(result_cycles.read() + 1);
            result_primitive_gate_count.write(result_temp.primitiveGateCount);
            wait(SC_ZERO_TIME);
            result_temp.cycles = result_cycles.read();
            wait();
        }

        cout << "\nCycle: " << result_cycles + 1 << endl;

        // Take cacheLatency into account before reading to/writing from cache
        int temp_cache_latency = cacheLatency;        
        while (cacheLatency-- > 1) { // TODO: change memoryLatency and cacheLatency accordingly
            result_cycles.write(result_cycles.read() + 1);
            wait(SC_ZERO_TIME);
            wait();
        }
        cacheLatency = temp_cache_latency;

        // Read to / write from cache and update result hits or misses
        int temp_data_to_write;
        int current_misses = result_misses;
        wait(SC_ZERO_TIME);
        // cout << "request addr: " << request_addr << endl;
        if (request_we) {
            cache->write_to_cache(request_addr, cache_config, request_data, result_temp);
            // cout << "request addr: " << request_addr << endl;
            // cout << "write successful: " << request_data << endl;
        } else {
            temp_data_to_write = cache->read_from_cache(request_addr, cache_config, result_temp);
            // cout << "read addr: " << request_addr << endl;
        }
        result_misses.write(result_temp.misses);
        result_hits.write(result_temp.hits);
        wait(SC_ZERO_TIME);
        
        // Take memoryLatency into account when there is cache miss
        if (static_cast<int>(result_temp.misses) > current_misses) {
            int temp_memory_latency = memoryLatency;
            while (memoryLatency-- > 1) { // TODO: change memoryLatency and cacheLatency accordingly
                result_cycles.write(result_cycles.read() + 1);
                wait(SC_ZERO_TIME);
                wait();
            }
            memoryLatency = temp_memory_latency;
        }

        // Data-to-read should only be ready after cacheLatency (+ memoryLatency)
        if (!request_we) {
            data = temp_data_to_write;
            wait(SC_ZERO_TIME);
        }

        result_cycles.write(result_cycles.read() + 1);  
        result_primitive_gate_count.write(result_temp.primitiveGateCount);
        wait(SC_ZERO_TIME);  
        result_temp.cycles = result_cycles.read();
        wait(); 
    }
}

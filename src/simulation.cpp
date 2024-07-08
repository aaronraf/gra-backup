#include <systemc>
#include <iostream>
#include "../includes/structs.hpp"
#include "../includes/cache_module.hpp"

using namespace std;
using namespace sc_core;

int sc_main(int argc, char* argv[]) {
    return 0;
}

Result run_simulation(int cycles, bool direct_mapped,  unsigned cachelines, unsigned cacheline_size, 
                        unsigned cache_latency, int memory_latency, size_t num_requests, Request requests[], const char* tf_filename) {

    sc_clock clk("clk", 1, SC_SEC);
    sc_signal<uint32_t> request_addr;
    sc_signal<uint32_t> request_data;
    sc_signal<int> request_we;
    sc_signal<size_t> result_cycles;
    sc_signal<size_t> result_hits;
    sc_signal<size_t> result_misses;
    sc_signal<size_t> result_primitive_gate_count; // would lead to overflow if use older system since the minsize of size_t is 16 bits


    // Tracefile initialization
    sc_trace_file* tracefile;
    bool tracefile_created = false;
    if (strcmp(tf_filename, "") != 0) {
        string tf_path = string("../out/") + string(tf_filename);
        tracefile = sc_create_vcd_trace_file(tf_path.c_str());
        tracefile_created = true;
    }

    if (tracefile_created) {
        // Check if tracefile successfully initialized
        if (tracefile == NULL) {
            fprintf(stderr, "Tracefile not opened.\n");
            exit(EXIT_FAILURE);
        }
        
        sc_trace(tracefile, clk, "Clock");
        sc_trace(tracefile, request_addr, " Request Address");
        sc_trace(tracefile, request_data, "Request Data");
        sc_trace(tracefile, request_we, "Request WE");
        sc_trace(tracefile, cycles, "Result Cycles");
        sc_trace(tracefile, result_misses, "Result Misses");
        sc_trace(tracefile, result_hits, "Result Hits");
        sc_trace(tracefile, result_primitive_gate_count, "Result Primitive Gate Count");
    }

    CACHE_MODULE cache ("cache", cycles, direct_mapped, cachelines, cacheline_size, cache_latency, memory_latency, num_requests);
    
    // Connnect ports to signals
    cache.clk(clk);
    cache.request_addr(request_addr);
    cache.request_we(request_we);
    cache.request_data(request_data);
    cache.result_cycles(result_cycles);
    cache.result_hits(result_hits);
    cache.result_misses(result_misses);
    cache.result_primitive_gate_count(result_primitive_gate_count);

    // Simulation: Matrix multiplication, only use for inputs.csv
    int entry_matrix_a = 0, entry_matrix_b = 0, entry_matrix_c = 0;
    bool read_matrix_a = true;
    bool read_matrix_b = false;
    bool read_matrix_c = false;
    int temp_mul_res;
    bool is_initialization_finished = false;
    int full_adder_primitive_gate_count = 5;
    int half_adder_primitive_gate_count = 2;
    int number_of_bits = 32;
    int additional_and_gates_in_multiplication = number_of_bits * number_of_bits;
    for (int i = 0; i < cycles; i++) {
    
        if (i < static_cast<int>(num_requests)) {
            request_addr = requests[i].addr;
            request_we = requests[i].we;
            request_data = requests[i].data;
        }

        // cout << request_we << endl;
        
        sc_start(1, SC_SEC);
        
        if (i < static_cast<int>(num_requests)) {
            if (requests[i].we) {
                if (is_initialization_finished) {
                    int result = entry_matrix_c + temp_mul_res;
                    // this alone should be 10048 = (16 elemen in matrix C * 4 times zwischenaddition) * 157
                    cache.result_temp.primitiveGateCount += (full_adder_primitive_gate_count * (number_of_bits - 1) + half_adder_primitive_gate_count); 
                    if (static_cast<uint32_t>(result) != request_data) {
                        cout << " Result: " << result << " is different from req data: " << request_data << endl;
                    }
                }
            } else {
                is_initialization_finished = true;
                if (read_matrix_a) {
                    entry_matrix_a = cache.data.read();
                    read_matrix_a = false;
                    read_matrix_b = true;
                } else if (read_matrix_b) {
                    entry_matrix_b = cache.data.read();
                    read_matrix_b = false;
                    read_matrix_c = true;
                } else if (read_matrix_c) {
                    entry_matrix_c = cache.data.read();
                    temp_mul_res = entry_matrix_a * entry_matrix_b;
                    // n-bit multiplication: AND Gates = n^2 ; HA = n ; FA = n(n-2)
                    // number of FA:
                    // 2-bit multiplier: 0
                    // 3-bit multplier: 3
                    // 4-bit multiplier: 8
                    // 0, 3, 8, 15, ... -> quadratic sequence
                    // a(2)^2 + b(2) + c = 0 <=> 4a + 2b + c = 0
                    // a(3)^2 + b(3) + c = 3 <=> 9a + 3b + c = 3
                    // a(4)^2 + b(4) + c = 8 <=> 16a + 4b + c = 8
                    // a = 1 ; b = -2 ; c = 0 -> n^2 + 2n = n(n-2) for n>=2
                    // with this formula alone, if we use 4-bit, then it will use 64 gates per multiplication
                    // 1 entry in matrix use 4 multiplication, there are 16 entries so 16*4 = 64 multplication -> this alone should use 4096 gates
                    // for 32 bit it should use 5888 per multiplication so 376832 in total + addition (10048) = 386880
                    cache.result_temp.primitiveGateCount += additional_and_gates_in_multiplication +
                                                            (number_of_bits * half_adder_primitive_gate_count) +
                                                            (number_of_bits * (number_of_bits - 2) * full_adder_primitive_gate_count);
                    read_matrix_c = false;
                    read_matrix_a = true;
                }
                // cout << "data to read: " << cache.data.read() << endl;
            }
        }
    }


    

    cout << "\nFinal result cycles: " << cache.result_cycles << endl;

    // Close and delete resources
    if (tracefile_created) {
        sc_close_vcd_trace_file(tracefile);
    }
    delete main_memory;

    return cache.result_temp;
}

 // g++ -std=c++14 -I../../systemc/include -L../../systemc/lib -Wl,-rpath,../../systemc/lib -o cache_module direct_mapped_cache.cpp four_way_lru_cache.cpp lru_cache.cpp main_memory.cpp simulation.cpp -lsystemc
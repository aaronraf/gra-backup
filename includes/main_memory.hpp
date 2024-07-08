#ifndef MAINMEMORY_HPP
#define MAINMEMORY_HPP

#include <cstdint>
#define MEMORY_SIZE 1'000'000'000U

class MainMemory {
private:
    // TODO: change size
    int memory[MEMORY_SIZE];

public: 
    int read_from_ram(uint32_t address);

    void write_to_ram(uint32_t address, int data_to_write);
};

#endif
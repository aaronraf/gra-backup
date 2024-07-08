#include "../includes/main_memory.hpp"
#include <iostream>
using namespace std;

int MainMemory::read_from_ram(uint32_t address) {
    if (address >= MEMORY_SIZE) {
        cerr << "Error: Invalid memory address " << address << endl;
        return -1;
    }
    // cout << "request addr: " << address << " | with value: " << memory[address] << endl;
    return memory[address];
}

void MainMemory::write_to_ram(uint32_t address, int data_to_write) {
    if (address >= MEMORY_SIZE) {
        cerr << "Error: Invalid memory address " << address << endl;
    }
    memory[address] = data_to_write;
    // cout << "request addr: " << address << " | with value: " << memory[address] << endl;
}   
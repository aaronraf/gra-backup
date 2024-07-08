#ifndef FOURWAYLRUCACHE_HPP
#define FOURWAYLRUCACHE_HPP

#include "cache_address.hpp"
#include "cache_template.hpp"
#include "main_memory.hpp"
#include "cache_config.hpp"
#include <unordered_map>
#include "structs.hpp"
#include <vector>
#include <cstdint>
using namespace std;

class LRUCache {
private:
    class Node {
    public:
        // TODO: sc_bv<8> data[number_of_offset];
        int data[10000];
        uint32_t map_key; // key of map which is tag
        bool is_first_time;
        Node* next;
        Node* prev;

        Node(int number_of_offset); // constructor: initialize all data to 0 and is_first_time to true
    };

    // TODO: unordered_map<sc_bv<8>, Node*> map;
    unordered_map<uint32_t, Node*> map; // int = tag
    Node* head; // head = MRU
    Node* tail; // tail = LRU
    
    void push_to_head(Node* node); // move LRU to MRU
    void add(Node* node);
    void remove(Node* node);
        
public:
    // doubly linkedlist: head - 4 nodes - tail with O(1) replace
    // map: {key 1: node 1 ; key 2: node 2; key 3: node 3 ; key 4: node 4} with O(1) read/write

    LRUCache(CacheConfig cache_config); // constructor: initialize linkedlist and map
    
    ~LRUCache(); // destructor: ensure delete all nodes to avoid memory leaks

    // TODO: sc_bv<8> read(sc_bv<number_of_tag> tag, sc_bv<number_of_offset> offset)
    int read_from_cache(uint32_t address, CacheAddress cache_address, CacheConfig cache_config, Result &result);

    // TODO : void write(sc_bv<number_of_tag> tag, sc_bv<number_of_offset> offset, sc_bv<8> data)
    void write_to_cache(uint32_t address, CacheAddress cache_address, CacheConfig cache_config, int data_to_write, Result &result);

    void replace_lru(uint32_t address, uint32_t cache_address_tag, CacheConfig cache_config);
};

class FourWayLRUCache : public Cache {
private:
    vector<LRUCache*> cache_sets;

public:
    FourWayLRUCache(CacheConfig cache_config);
    ~FourWayLRUCache();

    int read_from_cache(uint32_t address, CacheConfig cache_config, Result &result) override;
    void write_to_cache(uint32_t address, CacheConfig cache_config, int data_to_write, Result &result) override;
};

#endif
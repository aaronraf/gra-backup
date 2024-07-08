#include <iostream>
#include <unordered_map>
#include <cmath>
#include <algorithm> // std::fill()
#include "../includes/four_way_lru_cache.hpp"
#include "../includes/cache_config.hpp"
#include "../includes/main_memory_global.hpp"
using namespace std;

LRUCache::Node::Node(int number_of_offset) : next(nullptr), prev(nullptr) {
    fill(data, data + number_of_offset, 0); // initialize array to 0
    is_first_time = true;
}

void LRUCache::push_to_head(Node* node) {
    remove(node);
    add(node);
}

void LRUCache::add(Node* node) {
    node->next = head->next;
    node->next->prev = node;
    node->prev = head;
    head->next = node;
}

void LRUCache::remove(Node* node) {
    Node* prev = node->prev;
    Node* next = node->next;
    prev->next = next;
    next->prev = prev;
}

LRUCache::LRUCache(CacheConfig cache_config) {
    // add head and tail
    head = new Node(cache_config.number_of_offset_bits);
    tail = new Node(cache_config.number_of_offset_bits);
    head->next = tail;
    tail->prev = head;

    // add 4 nodes and set each of these to become the value of map (key doesn't matter here since the node is_first_time is true)
    for (int i = 0; i < cache_config.number_of_tag_bits; i++) {
        Node* node = new Node(cache_config.number_of_offset_bits);
        node->map_key = i; 
        add(node);
        map[i] = node; //equivalent to map.insert(make_pair(i, node));
    }
}

LRUCache::~LRUCache() {
    Node* current = head;
    while (current != nullptr) {
        Node* next = current->next;
        delete current;
        current = next;
    }
}

int LRUCache::read_from_cache(uint32_t address, CacheAddress cache_address, CacheConfig cache_config, Result &result) {
    // cout << "tag: " << cache_address.tag << " index: " << cache_address.index << " offset: " << cache_address.offset << endl;
    // if not found -> replace
    bool found = true;
    if (map.find(cache_address.tag) == map.end()) { // map.find() returns map.end() if not found
        replace_lru(address, cache_address.tag, cache_config);
        result.misses++;
        found = false;
    }
    // if accidentally found during first iteration so is_first_time is true -> also replace
    else if (map[cache_address.tag]->is_first_time) {
        replace_lru(address, cache_address.tag, cache_config);
        result.misses++;
        found = false;
    }
    if (found) {
        result.hits++;
    }
    // read from lru and update lru to mru
    Node* node = map[cache_address.tag]; 
    push_to_head(node);                         // replace O(1)
    return node->data[cache_address.offset];    // read O(1)
}

void LRUCache::write_to_cache(uint32_t address, CacheAddress cache_address, CacheConfig cache_config, int data_to_write, Result &result) {
    // cout << "tag: " << cache_address.tag << " index: " << cache_address.index << " offset: " << cache_address.offset << endl;
    
    // if not found -> replace
    bool found = true;
    if (map.find(cache_address.tag) == map.end()) { // map.find() returns map.end() if not found
        replace_lru(address, cache_address.tag, cache_config);
        result.misses++;
        found = false;
    }
    // if accidentally found during first iteration so is_first_time is true -> also replace
    else if (map[cache_address.tag]->is_first_time) {
        replace_lru(address, cache_address.tag, cache_config);
        result.misses++;
        found = false;
    }
    if (found) {
        result.hits++;
    }
    // write to lru and ram and update lru to mru
    Node* node = map[cache_address.tag];
    node->data[cache_address.offset] = data_to_write;   // write O(1)
    main_memory->write_to_ram(address, data_to_write);  // write-through: write directly to memory
    push_to_head(node);                                 // replace O(1)
}

void LRUCache::replace_lru(uint32_t address, uint32_t cache_address_tag, CacheConfig cache_config) {
    // replace lru node with new node at lru place (tail)
    Node* lru_node = tail->prev;
    Node* new_node = new Node(cache_config.number_of_offset_bits);
    new_node->next = tail;
    new_node->prev = lru_node->prev;
    lru_node->prev->next = new_node;
    tail->prev = new_node;

    // update map_key and is_first_time
    new_node->map_key = cache_address_tag;
    new_node->is_first_time = false;

    // delete entry of lrunode of map and add the new one
    map.erase(lru_node->map_key);
    map[cache_address_tag] = new_node;

    delete lru_node;

    // get appropriate 4 (number_of_offset) byte data from ram
    // 1 -> 0 1 2 3    1 / 4 = 0   0 * 4 = 0     fetch 0 - 3
    // 2 -> 0 1 2 3    2 / 4 = 0   0 * 4 = 0     fetch 0 - 3
    // 5 -> 4 5 6 7    5 / 4 = 1   1 * 4 = 4     fetch 4 - 7
    // 9 -> 8 9 10 11  9 / 4 = 2   2 * 4 = 8     fetch 8 - 11
    int total_offset = (int) pow(2, cache_config.number_of_offset_bits);
    uint32_t start_address_to_fetch = (address / total_offset) * total_offset;
    uint32_t last_address_to_fetch = start_address_to_fetch + total_offset - 1;
    
    for (uint32_t ram_address = start_address_to_fetch, offset = 0; ram_address <= last_address_to_fetch; ram_address++, offset++) { // O(1)
        new_node->data[offset] = main_memory->read_from_ram(ram_address);
    }
}


FourWayLRUCache::FourWayLRUCache(CacheConfig cache_config) {
    // instantiate objects as many as number of index
    for (int i = 0; i < (int) pow(2, cache_config.number_of_index_bits); i++) {
        cache_sets.push_back(new LRUCache(cache_config));
    }
}

FourWayLRUCache::~FourWayLRUCache() {
    // delete all LRUCache instances
    for (auto lru_cache : cache_sets) {
        delete lru_cache;
    }
}

int FourWayLRUCache::read_from_cache(uint32_t address, CacheConfig cache_config, Result &result) {
    CacheAddress cache_address(address, cache_config);
    uint32_t set_index = cache_address.index;
    return cache_sets[set_index]->read_from_cache(address, cache_address, cache_config, result);
}

void FourWayLRUCache::write_to_cache(uint32_t address, CacheConfig cache_config, int data_to_write, Result &result) {
    CacheAddress cache_address(address, cache_config);
    uint32_t set_index = cache_address.index;
    cache_sets[set_index]->write_to_cache(address, cache_address, cache_config, data_to_write, result);
}